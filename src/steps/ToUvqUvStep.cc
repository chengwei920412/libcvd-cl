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

#include "cvd-cl/steps/ToUvqUvStep.hh"

#include "kernels/fxy.hh"

#include <algorithm>

namespace CVD {
namespace CL  {

ToUvqUvStep::ToUvqUvStep(CameraState & i_camera, PointListState & i_xy1, PointListState & i_xy2, UvqUvState & o_uvquv) :
    WorkerStep (i_camera.worker),
    i_camera   (i_camera),
    i_xy1      (i_xy1),
    i_xy2      (i_xy2),
    o_uvquv    (o_uvquv)
{
    worker.compile(&program, &kernel, OCL_FXY, "fxy");
}

ToUvqUvStep::~ToUvqUvStep() {
    // Do nothing.
}

void ToUvqUvStep::execute() {
    // Read number of input points.
    size_t const count1 = i_xy1.getCount();
    size_t const count2 = i_xy2.getCount();
    size_t const count  = std::min(std::min(count1, count2), o_uvquv.maxCount);
    cl::NDRange const global(count);

    // Reset number of output points.
    o_uvquv.setCount = count;

    // Finish any outstanding work.
    worker.finish();

    // Set kernel for u-mapping.
    kernel.setArg(0, i_camera.umap.image);

    // Translate u in uvq.
    kernel.setArg(1, i_xy1.buffer);
    kernel.setArg(2, o_uvquv.uvq.us.memory);
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    worker.finish();

    // Translate u in uv.
    kernel.setArg(1, i_xy2.buffer);
    kernel.setArg(2, o_uvquv.uv.us.memory);
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    worker.finish();

    // Set kernel for v-mapping.
    kernel.setArg(0, i_camera.vmap.image);

    // Translate v in uvq.
    kernel.setArg(1, i_xy1.buffer);
    kernel.setArg(2, o_uvquv.uvq.vs.memory);
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    worker.finish();

    // Translate v in uv.
    kernel.setArg(1, i_xy2.buffer);
    kernel.setArg(2, o_uvquv.uv.vs.memory);
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    worker.finish();

    // Set kernel for q-mapping.
    kernel.setArg(0, i_camera.qmap.image);

    // Translate q in uvq.
    kernel.setArg(1, i_xy1.buffer);
    kernel.setArg(2, o_uvquv.uvq.qs.memory);
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    worker.finish();
}

} // namespace CL
} // namespace CVD
