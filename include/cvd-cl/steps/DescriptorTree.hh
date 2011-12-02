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

/// \brief Option structure to configure dense descriptor tree search.
struct TreeSearchOptions {
    int  threshold;
    int  rotations;
    bool exhaustive;
};

/// \brief Empty structure grouping types and functions for dense descriptor tree construction and search.
template<class BDT>
struct DescriptorTree {
public:

    /// \brief A single level of a dense descriptor tree.
    typedef struct TreeLevel {
        /// \brief Vector of binary descriptors, comprising nodes at this level \f$L\f$.
        std::vector<BDT>        descs;

        /// \brief Vector of pairs of indices into the lower level \f$L+1\f$,
        /// corresponding to descriptors at this level.
        std::vector<cl_ushort2> pairs;
    } TreeLevel;

    /// \brief Choose pairs of similar binary descriptors.
    ///
    /// This is used to build a higher tree level from a lower tree level.
    /// The current algorithm does not produce globally optimal difference.
    ///
    /// Avoid using this function directly, and use the higher-level
    /// buildDescriptorTree().
    ///
    /// \param hips    Vector of binary descriptors.
    /// \param pairs   Buffer to fill with index pairs.
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

    /// \brief Blend binary descriptor pairs into new binary descriptors.
    ///
    /// This is used following pairBitDescriptors() to blend chosen pairs.
    ///
    /// Avoid using this function directly, and use the higher-level
    /// buildDescriptorTree().
    ///
    /// \param i_hips   Input descriptors at existing lower level \f$L+1\f$.
    /// \param pairs    Pairs of indices within i_hips.
    /// \param o_hips   Output descriptors at new level \f$L\f$.
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

    /// \brief Populate a single dense vector from multiple tree levels.
    ///
    /// This function is recursive, using the stack to remember tree
    /// structure in both the dense tree and layered tree.
    ///
    /// Avoid using this function directly, and use the higher-level
    /// buildDescriptorTree().
    ///
    /// \param shape     Tree shape.
    /// \param levels    Pre-computed tree levels.
    /// \param tree      Vector of binary descriptors in binary tree format (output).
    /// \param maps      Vector of leaf indices mapping dense tree to original descriptors (output).
    /// \param ilevel    Current level \f$L\f$, starting from \f$L=0\f$.
    /// \param inode     Current node index within level.
    /// \param icell     Current cell index within dense \a tree.
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

    /// \brief Compute and populate a dense binary descriptor tree
    /// for a given set of binary descriptors.
    ///
    /// \param i_hips    Vector of binary descriptors.
    /// \param shape     Intended tree shape.
    /// \param tree      Vector of binary descriptors in binary tree format (output).
    /// \param maps      Vector of leaf indices mapping dense tree to original descriptors (output).
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

    /// \brief Search dense binary descriptor using a set of query descriptors.
    ///
    /// \param tree      Vector of binary descriptors in binary tree format.
    /// \param maps      Vector of leaf indices mapping dense tree to original descriptors.
    /// \param shape     Tree shape used during construction.
    /// \param tests     Vector of test descriptors used as search query.
    /// \param options   Tree search options.
    /// \param pairs     Buffer to fill with matched pairs.
    static void searchDescriptorTree(
        std::vector<BDT>       const & tree,
        std::vector<cl_ushort> const & maps,
        HipsTreeShape          const & shape,
        std::vector<BDT>       const & tests,
        TreeSearchOptions      const & options,
        std::vector<cl_int2>         & pairs
    ) {

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
                for (int irot = 0; irot < options.rotations; irot++) {
                    // TODO: General rotation function.
                    BDT const test = test0;

                    // Seed stack with single tree root.
                    stack.clear();
                    stack.push_back(0);

                    // Perform selective depth-first search of the tree.
                    while (stack.empty() == false) {
                        // Pop last element in the stack.
                        cl_uint const inode = stack.back();
                        stack.pop_back();

                        // Refer to descriptor node.
                        cl_ulong4 const & node = tree.at(inode);

                        // Calculate descriptor pair error.
                        int const error = errorBitDescriptors(test, node);

                        // Check error against threshold.
                        if (error <= options.threshold) {
                            if (inode >= shape.iTreeLeaf0) {
                                // This is a leaf, record the match.
                                cl_uint const ileaf = maps.at(inode - shape.iTreeLeaf0);
                                cl_int2 const pair = {{ileaf, itest}};
                                mypairs.push_back(pair);

                                // If not in exhaustive mode, break out for this rotation.
                                if (options.exhaustive == false)
                                    break;
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
