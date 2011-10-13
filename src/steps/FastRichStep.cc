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

#include "cvd-cl/steps/FastRichStep.hh"
#include "kernels/fast-rich.hh"

namespace CVD {
namespace CL  {

FastRichStep::FastRichStep(RichImageState & i_image, PointListState & i_points, GrayImageState & o_scores, PointListState & o_points, cl_int threshold, cl_int ring) :
    WorkerStep (i_image.worker),
    i_image    (i_image),
    i_points   (i_points),
    o_scores   (o_scores),
    o_points   (o_points),
    threshold  (threshold),
    ring       (ring)
{
    char opt[256] = {0,};
    snprintf(opt, sizeof(opt) - 1, "-DFAST_THRESH=%d -DFAST_RING=%d -DFAST_COUNT=%d", int(threshold), int(ring), int(o_points.size));
    worker.compile(&program, &kernel, OCL_FAST_RICH, "fast_rich", opt);
}

FastRichStep::~FastRichStep() {
    // Do nothing.
}

void FastRichStep::execute() {
    // Assign kernel parameters.
    kernel.setArg(0, i_image.image);
    kernel.setArg(1, o_scores.image);
    kernel.setArg(2, i_points.buffer);
    kernel.setArg(3, o_points.buffer);
    kernel.setArg(4, o_points.count);

    // Read number of input points.
    size_t const np = i_points.getCount();

    // Reset number of output points.
    o_points.setCount(0);

    // Zero scores buffer (may be slow).
    // o_scores.zero();

    // Queue kernel with global size set to number of input points.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(np), cl::NullRange);
}

} // namespace CL
} // namespace CVD
