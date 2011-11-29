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

#include "cvd-cl/steps/HipsTreeFindStep.hh"
#include "cvd-cl/steps/DescriptorTree.hh"
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

void HipsTreeFindStep::findByQueue() {
    // Refer to tree shape.
    HipsTreeShape const & shape = i_tree.shape;

    // Refer to saved tree data.
    std::vector<cl_ulong4> const & tree = i_tree.lastTree;
    std::vector<cl_ushort> const & maps = i_tree.lastMaps;

    // Read test descriptors from worker.
    std::vector<cl_ulong4> tests;
    i_hips.get(&tests);

    // Prepare pair vector.
    std::vector<cl_int2> pairs;

    // Prepare option structure.
    TreeSearchOptions options;
    options.threshold  = maxerr;
    options.rotations  = (rotate ? 16 : 1);
    options.exhaustive = true;
    DescriptorTree<cl_ulong4>::searchDescriptorTree(tree, maps, shape, tests, options, pairs);

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
