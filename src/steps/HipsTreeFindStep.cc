// Copyright (C) 2011  Dmitri Nikulin, Monash University
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

#include "cvd-cl/steps/HipsTreeFindStep.hh"
#include "kernels/hips-tfind.hh"

#include <algorithm>

#ifdef CVD_CL_VERBOSE
#include <iomanip>
#include <iostream>
#endif

namespace CVD {
namespace CL  {

HipsTreeFindStep::HipsTreeFindStep(HipsTreeState & i_tree, HipsListState & i_hips, PointListState & o_matches, cl_uint maxerr, bool rotate) :
    WorkerStep (i_tree.worker),
    i_tree     (i_tree),
    i_hips     (i_hips),
    o_matches  (o_matches),
    maxerr     (maxerr),
    rotate     (rotate)
{

    // Refer to tree shape.
    HipsTreeShape const & shape = i_tree.shape;

    // Number of "pre-roots", pairing roots in the forest.
    cl_uint const nPreRoot = (shape.nTreeRoots / 2);

    // Format OpenCL compiler options.
    char opt[512] = {0,};
    snprintf(opt, sizeof(opt) - 1,
        "-DHIPS_MAX_ERROR=%d -DTREE_PRE_ROOTS=%d -DTREE_LEVELS=%d -DTREE_DROP_NODES=%d -DTREE_LEAF0=%d",
        int(maxerr), int(nPreRoot), int(shape.nKeepLevels), int(shape.nDropNodes), int(shape.iTreeLeaf0));

    worker.compile(&program, &kernel, OCL_HIPS_TFIND, "hips_tree_find", opt);
}

HipsTreeFindStep::~HipsTreeFindStep() {
    // Do nothing.
}

template <typename T>
static int bitcount (T v){
  v = v - ((v >> 1) & (T)~(T)0/3);                           // temp
  v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);      // temp
  v = (v + (v >> 4)) & (T)~(T)0/255*15;                      // temp
  return (T)(v * ((T)~(T)0/255)) >> (sizeof(T) - 1) * CHAR_BIT; // count
}

static cl_uint bitcount8(cl_ulong8 const & t, cl_ulong8 const & r) {
    cl_uint total = 0;
    for (cl_uint i = 0; i < 8; i++)
        total += bitcount(t.s[i] & ~r.s[i]);
    return total;
}

static cl_ulong8 rotate8(cl_ulong8 const & t, cl_ulong lshift) {
    cl_ulong const rshift = (64 - lshift);
    cl_ulong8 out;
    for (cl_uint i = 0; i < 8; i++)
        out.s[i] = ((t.s[i] << lshift) | (t.s[i] >> rshift));
    return out;
}

void HipsTreeFindStep::findByQueue() {
    // Refer to tree shape.
    HipsTreeShape const & shape = i_tree.shape;

    // Refer to saved tree data.
    std::vector<cl_ulong8> const & tree = i_tree.lastTree;
    std::vector<cl_ushort> const & maps = i_tree.lastMaps;

    // Read test descriptors from worker.
    std::vector<cl_ulong8> tests;
    i_hips.get(&tests);
    cl_uint const ntests = tests.size();

    // Prepare pair vector.
    std::vector<cl_int2> pairs;
    pairs.reserve(ntests * 4);

    // Set number of rotations by parameter.
    cl_uint const nrot = (rotate ? 16 : 1);

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
            cl_ulong8 const & test0 = tests.at(itest);

            // Try all rotations.
            for (cl_uint irot = 0; irot < nrot; irot++) {
                cl_ulong8 const test = rotate8(test0, irot * 4);

                // Seed stack with single tree root.
                stack.push_back(0);

                // Perform selective depth-first search of the tree.
                while (stack.empty() == false) {
                    // Pop last element in the stack.
                    cl_uint const inode = stack.back();
                    stack.pop_back();

                    // Refer to descriptor node.
                    cl_ulong8 const & node = tree.at(inode);

                    // Calculate descriptor pair error.
                    cl_uint const error = bitcount8(test, node);

                    // Check error against threshold.
                    if (error <= maxerr) {
                        if (inode >= shape.iKeepLeaf0) {
                            // This is a leaf, record the match.
                            cl_uint const ileaf = maps.at(inode - shape.iKeepLeaf0);
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

    // Crop pair list.
    if (pairs.size() > o_matches.size)
        pairs.resize(o_matches.size);

    // Write matched pairs to worker.
    o_matches.set(pairs);
}

void HipsTreeFindStep::execute() {
#ifdef CVD_CL_VERBOSE
        boost::system_time t1 = boost::get_system_time();
        findByQueue();
        boost::system_time t2 = boost::get_system_time();

        int64_t const t_cfind = (t2 - t1).total_microseconds();
        cl_uint const ncmatches = o_matches.getCount();

        std::cerr << "C++ tree search in " << std::setw(9) << t_cfind << " us" << std::endl;
        std::cerr << "  Found            " << std::setw(9) << ncmatches << " matches" << std::endl;

        worker.finish();
#endif

    // Read number of descriptors.
    size_t const nh = i_hips.getCount();

    // Round down number of descriptors.
    size_t const nh_16 = (nh / 16) * 16;

    // Set number of rotations by parameter.
    size_t const nr = (rotate ? 16 : 1);

    // Calculate local size in first dimension.
    size_t const local1 = std::min((worker.defaultLocalSize / nr), size_t(16));

    // Create 1D work size.
    cl::NDRange const global(nh_16, nr);
    cl::NDRange const local(local1, nr);

    // Assign kernel parameters.
    kernel.setArg(0, i_tree.tree);
    kernel.setArg(1, i_tree.maps);
    kernel.setArg(2, i_hips.buffer);
    kernel.setArg(3, o_matches.buffer);
    kernel.setArg(4, o_matches.count);
    kernel.setArg(5, o_matches.size);

    // Reset number of output pairs.
    o_matches.setCount(0);

    // Queue kernel with global size set to number of input points in the test list.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, local);

#ifdef CVD_CL_VERBOSE
        worker.finish();
        boost::system_time t3 = boost::get_system_time();

        int64_t const t_dfind = (t3 - t2).total_microseconds();
        cl_uint const ndmatches = o_matches.getCount();

        std::cerr << "OCL tree search in " << std::setw(9) << t_dfind << " us" << std::endl;
        std::cerr << "  Found            " << std::setw(9) << ndmatches << " matches" << std::endl;
#endif
}

} // namespace CL
} // namespace CVD
