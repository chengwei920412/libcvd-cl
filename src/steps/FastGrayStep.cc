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

#include "cvd-cl/steps/FastGrayStep.hh"
#include "kernels/fast-gray.hh"

namespace CVD {
namespace CL  {

FastGrayStep::FastGrayStep(GrayImageState & iimage, PointListState & ipoints, GrayImageState & oscores, PointListState & opoints) :
    WorkerStep (iimage.worker),
    iimage     (iimage),
    ipoints    (ipoints),
    oscores    (oscores),
    opoints    (opoints)
{
    worker.compile(&program, &kernel, OCL_FAST_GRAY, "fast_gray");
}

FastGrayStep::~FastGrayStep() {
    // Do nothing.
}

void FastGrayStep::execute() {
    // Assign kernel parameters.
    kernel.setArg(0, iimage.image);
    kernel.setArg(1, oscores.image);
    kernel.setArg(2, ipoints.buffer);
    kernel.setArg(3, opoints.buffer);
    kernel.setArg(4, opoints.count);

    // Read number of input points.
    size_t const np = ipoints.getCount();

    // Reset number of output points.
    opoints.setCount(0);

    // Zero scores buffer (may be slow).
    // oscores.zero();

    // Queue kernel with global size set to number of input points.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(np), cl::NullRange);
}

} // namespace CL
} // namespace CVD
