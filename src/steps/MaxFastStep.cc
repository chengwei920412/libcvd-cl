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

#include "cvd-cl/steps/MaxFastStep.hh"
#include "kernels/filt.hh"

namespace CVD {
namespace CL  {

MaxFastStep::MaxFastStep(GrayImageState & scores, PointListState & ipoints, PointListState & opoints) :
    WorkerStep (scores.worker),
    iscores    (scores),
    ipoints    (ipoints),
    opoints    (opoints)
{
    worker.compile(&program, &kernel, OCL_FILT, "fast_filter");
}

MaxFastStep::~MaxFastStep() {
    // Do nothing.
}

void MaxFastStep::execute() {
    // Assign kernel parameters.
    kernel.setArg(0, iscores.image);
    kernel.setArg(1, ipoints.buffer);
    kernel.setArg(2, opoints.buffer);
    kernel.setArg(3, opoints.count);

    // Read number of input points.
    size_t const np = ipoints.getCount();

    // Reset number of output points.
    opoints.setCount(0);

    // Queue kernel with global size set to number of input points.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(np), cl::NullRange);
}

} // namespace CL
} // namespace CVD
