// Copyright (C) 2011  Dmitri Nikulin
// Copyright (C) 2011  Monash University
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#ifndef __CVD_CL_DESCRIPTOR_TREE_HH__
#define __CVD_CL_DESCRIPTOR_TREE_HH__

#include <cvd-cl/states/BitDescriptor.hh>
#include <cvd-cl/states/HipsTreeShape.hh>

#include <cassert>

namespace CVD {
namespace CL  {

// Dummy structure to combine several functions.
template<class BDT>
struct DescriptorTree {
public:

    typedef struct TreeLevel {
        std::vector<BDT>        descs;
        std::vector<cl_ushort2> pairs;
    } TreeLevel;

    static void pairBitDescriptors(std::vector<BDT> const & hips, std::vector<cl_ushort2> & pairs) {
        // Number of bit descriptors.
        size_t const nhips = hips.size();

        // Vector of "consumed" descriptors.
        std::vector<bool> used(nhips, false);

        // Clear vector for pairs.
        pairs.clear();

        // Perform greedy algorithm for best-pair matching.
        for (size_t i1 = 0; i1 < nhips; i1++) {
            // Skip if already used.
            if (used.at(i1) == true)
                continue;

            // Refer to descriptor 1.
            BDT const & d1 = hips.at(i1);

            // Note if any unused descriptors exist.
            bool more = false;

            // Placeholder for the best matching index, and the error.
            size_t be2 = 10000;
            size_t bi2 =     0;

            for (size_t i2 = 0; i2 < nhips; i2++) {
                // Skip if already used.
                if (used.at(i2) == true)
                    continue;

                // Note that a descriptor remains available.
                more = true;

                // Skip if repeating the first descriptor.
                if (i1 == i2)
                    continue;

                // Refer to descriptor 2.
                BDT const & d2 = hips.at(i2);

                // Calculate the error between the descriptors.
                size_t const err = diffBitDescriptors(d1, d2);

                // Update best.
                if (err < be2) {
                    be2 = err;
                    bi2 = i2;

                    // If the error is now 0, it cannot be improved.
                    // This is actually a bad sign for later traversing the tree.
                    if (be2 < 1)
                        break;
                }
            }

            // If no unused descriptors exist, return now.
            if (more == false)
                return;

            // Consume the best pairing found.
            used.at(i1)  = true;
            used.at(bi2) = true;

            // Record the pairing.
            cl_ushort2 const pair = {{i1, bi2}};
            pairs.push_back(pair);
        }
    }

    static void blendBitDescriptors(
        std::vector<BDT>        const & i_hips,
        std::vector<cl_ushort2> const & pairs,
        std::vector<BDT>              & o_hips
    ) {

        // Number of bit descriptor pairs.
        size_t const npairs = pairs.size();

        // Allocate output.
        o_hips.resize(npairs);

        for (size_t i = 0; i < npairs; i++) {
            // Refer to descriptor pairing and component descriptors.
            cl_ushort2 const & pair = pairs.at(i);
            BDT        const & d1   = i_hips.at(pair.x);
            BDT        const & d2   = i_hips.at(pair.y);

            // Calculate blended descriptor.
            BDT                d3;
            mergeBitDescriptors(&d3, d1, d2);

            // Store blended descriptor.
            o_hips.at(i) = d3;
        }
    }

    static void fillDescriptorTree(
        HipsTreeShape              const & shape,
        std::vector<TreeLevel>     const & levels,
        std::vector<BDT>                 & tree,
        std::vector<cl_ushort>           & maps,
        size_t                             ilevel,
        size_t                             inode,
        size_t                             icell
    ) {

        // Refer to tree level.
        TreeLevel const & level = levels.at(ilevel);

        // Refer to tree descriptor.
        BDT & cell = tree.at(icell);

        // Fill descriptor at this level.
        cell = level.descs.at(inode);

        // If at lowest level, conclude.
        if (ilevel < 1) {
            assert(icell >= shape.iTreeLeaf0);

            // Fill map at this level.
            maps.at(icell - shape.iTreeLeaf0) = inode;
            return;
        } else {
            assert(icell <  shape.iTreeLeaf0);
        }

        // Refer to pair constructing this descriptor.
        cl_ushort2 const & pair = level.pairs.at(inode);

        // Find child positions in next level.
        size_t const inode1 = pair.x;
        size_t const inode2 = pair.y;
        assert(inode1 != inode2);

        // Calculate indices in final tree.
        size_t const icell0 = (icell  * 2);
        size_t const icell1 = (icell0 + 1);
        size_t const icell2 = (icell0 + 2);
        assert(icell1 != icell2);
        assert(icell1 >  icell);
        assert(icell2 >  icell);

        // Recurse into child positions.
        fillDescriptorTree(shape, levels, tree, maps, ilevel - 1, inode1, icell1);
        fillDescriptorTree(shape, levels, tree, maps, ilevel - 1, inode2, icell2);
    }

    static void buildDescriptorTree(
        std::vector<BDT>       const & i_hips,
        HipsTreeShape          const & shape,
        std::vector<BDT>             & tree,
        std::vector<cl_ushort>       & maps
    ) {

#ifdef CVD_CL_VERBOSE
        std::cerr << "Descriptor tree build started for " << std::setw(5) << i_hips.size() << " descriptors" << std::endl;
#endif

        // Truncate or extend HIPS list to exact leaf count.
        std::vector<BDT> hips(i_hips);
        hips.resize(shape.nLeaves);

        // Zero any extra descriptors created.
        for (size_t i = i_hips.size(); i < hips.size(); i++)
            clearBitDescriptor(&hips.at(i));

#ifdef CVD_CL_VERBOSE
        std::cerr << "  Size set to " << std::setw(5) << hips.size() << " descriptors" << std::endl;
#endif

        // Create tree levels.
        std::vector<TreeLevel> levels(shape.nTreeLevels);

        // Populate lowest level (leaves) with original descriptors.
        // Pairings for this level are stored one level higher.
        levels.front().descs = hips;

        // Iteratively create pairings and blended descriptors.
        for (size_t ilevel = 1; ilevel < shape.nTreeLevels; ilevel++) {
            TreeLevel const & l1 = levels.at(ilevel - 1);
            TreeLevel       & l2 = levels.at(ilevel    );

#ifdef CVD_CL_VERBOSE
            boost::system_time t1 = boost::get_system_time();
#endif

            // Create pairings, storing pairings on the next level.
            pairBitDescriptors(l1.descs, l2.pairs);

#ifdef CVD_CL_VERBOSE
            boost::system_time t2 = boost::get_system_time();
#endif

            // Create blended descriptors at the next level.
            blendBitDescriptors(l1.descs, l2.pairs, l2.descs);

#ifdef CVD_CL_VERBOSE
            boost::system_time t3 = boost::get_system_time();

            int64_t const t_pairs = (t2 - t1).total_microseconds();
            int64_t const t_blend = (t3 - t2).total_microseconds();

            std::cerr << "  Level " << std::setw(2) << ilevel << ":      " << std::setw(9) << t_pairs << " us pairing, " << std::setw(9) << t_blend << " us blending" << std::endl;
#endif
        }

        // Verify the tree converged to a single root.
        TreeLevel const & roots = levels.back();
        assert(levels.back().descs.size() == 1);

        // Create vector representing final tree.
        // Only shape.nKeepNodes will actually be written out,
        // starting from shape.nDropNodes.
        tree.clear();
        tree.resize(shape.nTreeNodes);

        // Create vector representing descriptor index maps.
        maps.clear();
        maps.resize(shape.nLeaves, 0);

#ifdef CVD_CL_VERBOSE
        boost::system_time t1 = boost::get_system_time();
#endif

        // Recursively populate final tree.
        fillDescriptorTree(shape, levels, tree, maps, shape.nTreeLevels - 1, 0, 0);

#ifdef CVD_CL_VERBOSE
        boost::system_time t2 = boost::get_system_time();

        int64_t const t_fill = (t2 - t1).total_microseconds();

        std::cerr << "  Tree filled in " << std::setw(9) << t_fill <<  " us" << std::endl;
        std::cerr << std::endl;
#endif

#ifdef CVD_CL_DEBUG
        // Check that every position was used exactly once.
        std::set<cl_ushort> used;
        for (size_t icell = 0; icell < shape.nLeaves; icell++) {
            cl_ushort const map = maps.at(icell);

            assert(used.count(map) == 0);
            used.insert(map);
            assert(used.count(map) == 1);
        }
#endif
    }

    static void searchDescriptorTree(
        std::vector<BDT>       const & tree,
        std::vector<cl_ushort> const & maps,
        HipsTreeShape          const & shape,
        std::vector<BDT>       const & tests,
        std::vector<cl_int2>         & pairs
    ) {

        // FIXED parameters.
        // TODO: Pass from option structure.
        cl_uint const maxerr = 3;
        cl_uint const nrot = 1;

        // Note test vector size.
        cl_uint const ntests = tests.size();

        // Prepare pair vector.
        pairs.clear();
        pairs.reserve(ntests * 4);

        // Prepare integer for atomic counter.
        cl_uint atom_itest = 0;

        #pragma omp parallel default(shared)
        {
            // Prepare node stack.
            std::vector<cl_uint> stack;
            stack.reserve(shape.nKeepNodes);

            // Prepare pair vector.
            std::vector<cl_int2> mypairs;
            mypairs.reserve(ntests * 4);

            while (true) {
                // Atomically retrieve test descriptor index.
                cl_uint const itest = __sync_fetch_and_add(&atom_itest, 1);
                if (itest >= ntests)
                    break;

                // Refer to test descriptor.
                BDT const & test0 = tests.at(itest);

                // Try all rotations.
                for (cl_uint irot = 0; irot < nrot; irot++) {
                    // TODO: General rotation function.
                    BDT const test = test0;

                    // Seed stack with single tree root.
                    stack.push_back(0);

                    // Perform selective depth-first search of the tree.
                    while (stack.empty() == false) {
                        // Pop last element in the stack.
                        cl_uint const inode = stack.back();
                        stack.pop_back();

                        // Refer to descriptor node.
                        cl_ulong4 const & node = tree.at(inode);

                        // Calculate descriptor pair error.
                        cl_uint const error = errorBitDescriptors(test, node);

                        // Check error against threshold.
                        if (error <= maxerr) {
                            if (inode >= shape.iTreeLeaf0) {
                                // This is a leaf, record the match.
                                cl_uint const ileaf = maps.at(inode - shape.iTreeLeaf0);
                                cl_int2 const pair = {{ileaf, itest}};
                                mypairs.push_back(pair);
                            } else {
                                // This is an internal node, add its children to the stack.
                                cl_uint const inext0 = (inode  * 2);
                                cl_uint const inext1 = (inext0 + 1);
                                cl_uint const inext2 = (inext0 + 2);
                                stack.push_back(inext2);
                                stack.push_back(inext1);
                            }
                        }
                    }
                }
            }

            // Synchronously accumulate pairs.
            #pragma omp critical
            {
                pairs.insert(pairs.end(), mypairs.begin(), mypairs.end());
            }
        }
    }
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_DESCRIPTOR_TREE_HH__ */
