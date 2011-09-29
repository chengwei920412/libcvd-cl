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

#include "cvd-cl/steps/SE3Run1Step.hh"
#include "cvd-cl/core/Expect.hh"

#include "kernels/se3-run1.hh"

namespace CVD {
namespace CL  {

SE3Run1Step::SE3Run1Step(UvqUvState<1> & i_uvquv, MatrixState<4, 4> & i_mats, CountState & i_which, Float2ListState & o_uvs) :
    WorkerStep (i_uvquv.worker),
    i_uvquv    (i_uvquv),
    i_mats     (i_mats),
    i_which    (i_which),
    o_uvs      (o_uvs)
{
    // State consistency checks.
    assert(o_uvs.size >= i_uvquv.maxCount);

    worker.compile(&program, &kernel, OCL_SE3_RUN1, "se3_run1");
}

SE3Run1Step::~SE3Run1Step() {
    // Do nothing.
}

void SE3Run1Step::execute() {
    // Number of uvq sets to transform in parallel.
    cl_int const nsets     = i_uvquv.setCount;

    // Number of matrices.
    cl_int const nmatrices = i_mats.count;

    // Choice of matrix to use for transformation.
    cl_int const imatrix   = i_which.getCount();

    // Reset output size.
    o_uvs.setCount(nsets);

    // Assign kernel parameters.
    kernel.setArg(0, i_uvquv.uvq.us.memory);
    kernel.setArg(1, i_uvquv.uvq.vs.memory);
    kernel.setArg(2, i_uvquv.uvq.qs.memory);
    kernel.setArg(3, i_mats.memory);
    kernel.setArg(4, o_uvs.buffer);
    kernel.setArg(5, nmatrices);
    kernel.setArg(6, imatrix);

    // Queue kernel with global size set to number of transformations.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(nsets), cl::NullRange);
}

} // namespace CL
} // namespace CVD
