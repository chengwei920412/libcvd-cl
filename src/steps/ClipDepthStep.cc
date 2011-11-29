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

#include "cvd-cl/steps/ClipDepthStep.hh"
#include "kernels/clip-depth.hh"

namespace CVD {
namespace CL  {

ClipDepthStep::ClipDepthStep(FloatImageState & i_depth, PointListState & i_points, PointListState & o_points) :
    WorkerStep (i_depth.worker),
    i_depth    (i_depth),
    i_points   (i_points),
    o_points   (o_points)
{
    worker.compile(&program, &kernel, OCL_CLIP_DEPTH, "clip_depth");
}

ClipDepthStep::~ClipDepthStep() {
    // Do nothing.
}

void ClipDepthStep::execute() {
    // Assign kernel parameters.
    kernel.setArg(0, i_depth.image);
    kernel.setArg(1, i_points.buffer);
    kernel.setArg(2, o_points.buffer);
    kernel.setArg(3, o_points.count);

    // Read number of input points.
    size_t const np = i_points.getCount();

    // Reset number of output points.
    o_points.setCount(0);

    // Queue kernel with global size set to number of input points.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(np), cl::NullRange);
}

} // namespace CL
} // namespace CVD
