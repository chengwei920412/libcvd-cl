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

#include "cvd-cl/steps/PreFastStep.hh"
#include "kernels/cull.hh"

namespace CVD {
namespace CL  {

PreFastStep::PreFastStep(GrayImageState & image, PointListState & points) :
    WorkerStep (image.worker),
    image      (image),
    points     (points)
{
    worker.compile(&program, &kernel, OCL_CULL, "cull_gray");
}

PreFastStep::~PreFastStep() {
    // Do nothing.
}

void PreFastStep::execute() {
    // Assign kernel parameters.
    kernel.setArg(0, image.image);
    kernel.setArg(1, points.buffer);
    kernel.setArg(2, points.count);

    // Read image dimensions.
    size_t const nx = image.size.x;
    size_t const ny = image.size.y;

    // Reset number of output points.
    points.setCount(0);

    // Queue kernel with square local size.
    // 16x16 appears to give good performance on most devices.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(nx, ny), cl::NDRange(16, 16));
}

} // namespace CL
} // namespace CVD
