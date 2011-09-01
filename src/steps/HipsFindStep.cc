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

#include "cvd-cl/steps/HipsFindStep.hh"
#include "kernels/hips-find.hh"

namespace CVD {
namespace CL  {

HipsFindStep::HipsFindStep(HipsListState & i_hips1, HipsListState & i_hips2, PointListState & o_matches) :
    WorkerStep (i_hips1.worker),
    i_hips1    (i_hips1),
    i_hips2    (i_hips2),
    o_matches  (o_matches)
{
    worker.compile(&program, &kernel, OCL_HIPS_FIND, "hips_find");
}

HipsFindStep::~HipsFindStep() {
    // Do nothing.
}

void HipsFindStep::execute() {
    // Read number of descriptors.
    size_t const np1 = i_hips1.getCount();
    size_t const np2 = i_hips2.getCount();

    // Round down number of descriptors.
    cl_int const np1_16 = (np1 / 16) * 16;
    cl_int const np2_16 = (np2 / 16) * 16;

    // Assign kernel parameters.
    kernel.setArg(0, i_hips1.buffer);
    kernel.setArg(1, i_hips2.buffer);
    kernel.setArg(2, o_matches.buffer);
    kernel.setArg(3, o_matches.count);
    kernel.setArg(4, o_matches.size);

    // Reset number of output pairs.
    o_matches.setCount(0);

    // Queue kernel with global size set to number of input points.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(np1_16, np2_16), cl::NDRange(16, 16));
}

} // namespace CL
} // namespace CVD
