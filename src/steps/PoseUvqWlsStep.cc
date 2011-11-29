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

#include "cvd-cl/steps/PoseUvqWlsStep.hh"
#include "cvd-cl/core/Expect.hh"

#include "kernels/wls-uvq.hh"

namespace CVD {
namespace CL  {

PoseUvqWlsStep::PoseUvqWlsStep(UvqUvState<3> & i_uvquv, MatrixState<4, 4> & i_m, MatrixState<6, 6> & o_a, MatrixState<6, 1> & o_b) :
    WorkerStep (i_uvquv.worker),
    i_uvquv    (i_uvquv),
    i_m        (i_m),
    o_a        (o_a),
    o_b        (o_b)
{

    // State consistency checks.
    assert(i_m.count        == o_a.count);
    assert(o_a.count        == o_b.count);
    assert(i_uvquv.setCount == o_a.count);
    assert(i_uvquv.setCount == o_b.count);
    assert(i_uvquv.setCount >= 1);

    worker.compile(&program, &kernel, OCL_WLS_UVQ, "wls_uvq");
}

PoseUvqWlsStep::~PoseUvqWlsStep() {
    // Do nothing.
}

void PoseUvqWlsStep::execute() {
    // Repeat state consistency checks.
    // i_uvquv.setCount is actually mutable.
    assert(i_m.count        == o_a.count);
    assert(o_a.count        == o_b.count);
    assert(i_uvquv.setCount == o_a.count);
    assert(i_uvquv.setCount == o_b.count);
    assert(i_uvquv.setCount >= 1);

    // Assign kernel parameters.
    kernel.setArg(0, i_uvquv.uvq.us.memory);
    kernel.setArg(1, i_uvquv.uvq.vs.memory);
    kernel.setArg(2, i_uvquv.uvq.qs.memory);
    kernel.setArg(3, i_uvquv.uv.us.memory);
    kernel.setArg(4, i_uvquv.uv.vs.memory);
    kernel.setArg(5, i_m.memory);
    kernel.setArg(6, o_a.memory);
    kernel.setArg(7, o_b.memory);

    // Number of matrices to calculate in parallel.
    size_t const count = i_uvquv.setCount;

    // Queue kernel with global size set to number of matrices.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(count), cl::NullRange);
}

} // namespace CL
} // namespace CVD
