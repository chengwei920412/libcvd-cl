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

#include "cvd-cl/steps/FxyStep.hh"

#include "kernels/fxy.hh"

namespace CVD {
namespace CL  {

FxyStep::FxyStep(PointListState & i_xy, FloatImageState & i_map, FloatListState & o_f) :
    WorkerStep (i_xy.worker),
    i_xy       (i_xy),
    i_map      (i_map),
    o_f        (o_f)
{
    worker.compile(&program, &kernel, OCL_FXY, "fxy");
}

FxyStep::~FxyStep() {
    // Do nothing.
}

void FxyStep::execute() {
    // Assign kernel parameters.
    kernel.setArg(0, i_xy.buffer);
    kernel.setArg(1, i_map.image);
    kernel.setArg(2, o_f.buffer);

    // Read number of input points.
    size_t const count = i_xy.getCount();

    // Reset number of output points.
    o_f.setCount(count);

    // Queue kernel with global size set to number of input points.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(count), cl::NullRange);
}

} // namespace CL
} // namespace CVD
