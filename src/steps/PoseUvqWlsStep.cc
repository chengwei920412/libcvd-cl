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

#include "cvd-cl/steps/PoseUvqWlsStep.hh"
#include "cvd-cl/core/Expect.hh"

#include "kernels/wls-uvq.hh"

namespace CVD {
namespace CL  {

PoseUvqWlsStep::PoseUvqWlsStep(UvqState & i_uvq, UvState & i_uv, MatrixState & o_a, MatrixState & o_b) :
    WorkerStep (i_uvq.worker),
    i_uvq      (i_uvq),
    i_uv       (i_uv),
    o_a        (o_a),
    o_b        (o_b)
{
    // Individual state sanity checks.
    assert(i_uvq.setSize  == 6);
    assert(i_uv.setSize   == 6);
    assert(o_a.rows       == 6);
    assert(o_b.rows       == 6);
    assert(o_a.cols       == 6);
    assert(o_b.cols       == 6);

    // State consistency checks.
    assert(i_uvq.setCount >= 1);
    assert(i_uvq.setCount == i_uv.setCount);
    assert(i_uvq.setCount == o_a.count);
    assert(i_uvq.setCount == o_b.count);

    worker.compile(&program, &kernel, OCL_WLS_UVQ, "wls_uvq");
}

PoseUvqWlsStep::~PoseUvqWlsStep() {
    // Do nothing.
}

void PoseUvqWlsStep::execute() {
    // Assign kernel parameters.
    kernel.setArg(0, i_uvq.us.memory);
    kernel.setArg(1, i_uvq.vs.memory);
    kernel.setArg(2, i_uvq.qs.memory);
    kernel.setArg(3, i_uv.us.memory);
    kernel.setArg(4, i_uv.vs.memory);
    kernel.setArg(5, o_a.memory);
    kernel.setArg(6, o_b.memory);

    // Number of matrices to calculate in parallel.
    size_t const count = i_uvq.setCount;

    // Queue kernel with global size set to number of matrices.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(count), cl::NullRange);
}

} // namespace CL
} // namespace CVD
