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

#include "cvd-cl/steps/SE3ScoreStep.hh"
#include "cvd-cl/core/Expect.hh"

#include "kernels/se3-score.hh"

namespace CVD {
namespace CL  {

SE3ScoreStep::SE3ScoreStep(UvqUvState<1> & i_uvquv, MatrixState<4, 4> & i_mats, FloatListState & o_scores) :
    WorkerStep (i_uvquv.worker),
    i_uvquv    (i_uvquv),
    i_mats     (i_mats),
    o_scores   (o_scores)
{
    // State consistency checks.
    assert(i_mats.count == o_scores.size);

    worker.compile(&program, &kernel, OCL_SE3_SCORE, "se3_score");
}

SE3ScoreStep::~SE3ScoreStep() {
    // Do nothing.
}

void SE3ScoreStep::execute() {
    // Number of matrices to calculate in parallel.
    size_t const nmatrices = i_mats.count;

    // Number of uvquv sets to score.
    cl_int const nsets     = i_uvquv.setCount;

    // Reset output size.
    o_scores.setCount(nmatrices);

    // Assign kernel parameters.
    kernel.setArg(0, i_uvquv.uvq.us.memory);
    kernel.setArg(1, i_uvquv.uvq.vs.memory);
    kernel.setArg(2, i_uvquv.uvq.qs.memory);
    kernel.setArg(3, i_uvquv.uv.us.memory);
    kernel.setArg(4, i_uvquv.uv.vs.memory);
    kernel.setArg(5, i_mats.memory);
    kernel.setArg(6, o_scores.buffer);
    kernel.setArg(7, nsets);

    // Queue kernel with global size set to number of matrices.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(nmatrices), cl::NullRange);
}

} // namespace CL
} // namespace CVD
