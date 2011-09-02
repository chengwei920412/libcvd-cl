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

#include "cvd-cl/steps/HipsBlendGrayStep.hh"
#include "kernels/hips-gray.hh"
#include "kernels/hips-blend.hh"

namespace CVD {
namespace CL  {

// Create (0,0) offset.
cl_int2 static const offset00 = {{0, 0}};

HipsBlendGrayStep::HipsBlendGrayStep(GrayImageState & i_image, PointListState & i_points, HipsListState & o_hips) :
    WorkerStep (i_image.worker),
    i_image    (i_image),
    i_points   (i_points),
    o_hips     (o_hips),
    m_hips     (worker, o_hips.size)
{
    worker.compile(&program_hips, &kernel_hips, OCL_HIPS_GRAY, "hips_gray");
    worker.compile(&program_blend, &kernel_blend, OCL_HIPS_BLEND, "hips_blend");
}

HipsBlendGrayStep::~HipsBlendGrayStep() {
    // Do nothing.
}

void HipsBlendGrayStep::execute() {
    // Read number of input points.
    size_t const np = i_points.getCount();

    // Reset number of output points.
    o_hips.setCount(np);
    m_hips.setCount(np);

    // Create work dimensions.
    cl::NDRange const global(np);

    // Assign HIPS kernel parameters.
    kernel_hips.setArg(0, i_image.image);
    kernel_hips.setArg(1, i_points.buffer);
    kernel_hips.setArg(2, o_hips.buffer);
    kernel_hips.setArg(3, offset00);

    // Build initial descriptors at centre.
    worker.queue.enqueueNDRangeKernel(kernel_hips, cl::NullRange, global, cl::NullRange);
    worker.queue.enqueueBarrier();

    // Switch output buffer to internal one.
    kernel_hips.setArg(2, m_hips.buffer);
    kernel_hips.setArg(3, m_hips.count);

    // Assign blend kernel parameters.
    kernel_blend.setArg(0, o_hips.buffer);
    kernel_blend.setArg(1, m_hips.buffer);

    // Blend from different offsets.
    for (int xo = -1; xo <= +1; xo++) {
        for (int yo = -1; yo <= +1; yo++) {
            // Skip diagonals.
            if (xo == yo)
                continue;

            // Assign 2D offset.
            cl_int2 const offset = {{xo, yo}};
            kernel_hips.setArg(3, offset);

            // Create new HIPS descriptors.
            worker.queue.enqueueBarrier();
            worker.queue.enqueueNDRangeKernel(kernel_hips, cl::NullRange, global, cl::NullRange);

            // Blend into existing descriptors.
            worker.queue.enqueueBarrier();
            worker.queue.enqueueNDRangeKernel(kernel_blend, cl::NullRange, global, cl::NullRange);
        }
    }

    // Finish any outstanding work.
    worker.queue.finish();
}

} // namespace CL
} // namespace CVD
