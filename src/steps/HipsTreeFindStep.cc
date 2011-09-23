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

namespace CVD {
namespace CL  {

HipsTreeFindStep::HipsTreeFindStep(HipsTreeState & i_tree, HipsListState & i_hips, PointListState & o_matches, cl_int maxerr) :
    WorkerStep (i_tree.worker),
    i_tree     (i_tree),
    i_hips     (i_hips),
    o_matches  (o_matches),
    maxerr     (maxerr)
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

void HipsTreeFindStep::execute() {
    // Read number of descriptors.
    size_t const np2 = i_hips.getCount();

    // Round down number of descriptors.
    size_t const np2_16 = (np2 / 16) * 16;

    // Create 1D work size (no rotation).
    cl::NDRange const global(np2_16, 1);
    cl::NDRange const local(16, 1);

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
}

} // namespace CL
} // namespace CVD
