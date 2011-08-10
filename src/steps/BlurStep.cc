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

#include "cvd-cl/steps/BlurStep.hh"
#include "cvd-cl/core/Expect.hh"
#include "kernels/blur-gray.hh"

namespace CVD {
namespace CL  {

BlurStep::BlurStep(GrayImageState & imagei, GrayImageState & imageo) :
    WorkerStep (imagei.worker),
    iimage     (imagei),
    oimage     (imageo)
{
    // Expect identical image dimensions.
    expect("BlurStep::BlurStep() must have identical image sizes",
        imagei.size == imageo.size);

    worker.compile(&program, &kernel, OCL_BLUR_GRAY, "blur_gray");
}

BlurStep::~BlurStep() {
    // Do nothing.
}

void BlurStep::execute() {
    // Assign kernel parameters.
    kernel.setArg(0, iimage.image);
    kernel.setArg(1, oimage.image);

    // Read image dimensions.
    size_t const nx = iimage.size.x;
    size_t const ny = iimage.size.y;

    // Queue kernel with square local size.
    // 16x16 appears to give good performance on most devices.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(nx, ny), cl::NDRange(16, 16));
}

} // namespace CL
} // namespace CVD
