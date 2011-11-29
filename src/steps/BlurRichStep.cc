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

#include "cvd-cl/steps/BlurRichStep.hh"
#include "cvd-cl/core/Expect.hh"
#include "kernels/blur-rich.hh"

namespace CVD {
namespace CL  {

BlurRichStep::BlurRichStep(RichImageState & i_image, RichImageState & o_image) :
    WorkerStep (i_image.worker),
    i_image    (i_image),
    o_image    (o_image)
{
    // Expect identical image dimensions.
    expect("BlurRichStep::BlurRichStep() must have identical image sizes",
        (i_image.nx == o_image.nx) && (i_image.ny == o_image.ny));

    worker.compile(&program, &kernel, OCL_BLUR_RICH, "blur_rich");
}

BlurRichStep::~BlurRichStep() {
    // Do nothing.
}

void BlurRichStep::execute() {
    // Assign kernel parameters.
    kernel.setArg(0, i_image.image);
    kernel.setArg(1, o_image.image);

    // Read image dimensions.
    size_t const nx = i_image.nx;
    size_t const ny = i_image.ny;

    // Queue kernel with square local size.
    // 16x16 appears to give good performance on most devices.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(nx, ny), cl::NDRange(16, 16));
}

} // namespace CL
} // namespace CVD
