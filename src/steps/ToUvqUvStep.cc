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

#include "kernels/to-uvquv.hh"

namespace CVD {
namespace CL  {

ToUvqUvStep::ToUvqUvStep(CameraState & i_camera, PointListState & i_xy1, PointListState & i_xy2, PointListState & i_matches, UvqUvState & o_uvquv) :
    WorkerStep (i_camera.worker),
    i_camera   (i_camera),
    i_xy1      (i_xy1),
    i_xy2      (i_xy2),
    i_matches  (i_matches),
    o_uvquv    (o_uvquv)
{
    worker.compile(&program, &kernel, OCL_TO_UVQUV, "to_uvquv");
}

ToUvqUvStep::~ToUvqUvStep() {
    // Do nothing.
}

void ToUvqUvStep::execute() {
    // Set kernel arguments.
    kernel.setArg( 0, i_camera.umap.image);
    kernel.setArg( 1, i_camera.vmap.image);
    kernel.setArg( 2, i_camera.qmap.image);
    kernel.setArg( 3, i_xy1.buffer);
    kernel.setArg( 4, i_xy2.buffer);
    kernel.setArg( 5, i_matches.buffer);
    kernel.setArg( 6, o_uvquv.uvq.us.memory);
    kernel.setArg( 7, o_uvquv.uvq.vs.memory);
    kernel.setArg( 8, o_uvquv.uvq.qs.memory);
    kernel.setArg( 9, o_uvquv.uv.us.memory);
    kernel.setArg(10, o_uvquv.uv.vs.memory);

    // Read number of input pairs.
    size_t const count = i_matches.getCount();

    // Reset number of output points.
    o_uvquv.setCount = count;

    // Enqueue kernel with one thread per input pair.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(count), cl::NullRange);
}

} // namespace CL
} // namespace CVD
