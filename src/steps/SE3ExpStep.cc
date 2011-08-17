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

#include "cvd-cl/steps/SE3ExpStep.hh"
#include "cvd-cl/core/Expect.hh"

#include "kernels/se3-exp.hh"

namespace CVD {
namespace CL  {

SE3ExpStep::SE3ExpStep(MatrixState & i_vecs, MatrixState & o_mats) :
    WorkerStep (i_vecs.worker),
    i_vecs     (i_vecs),
    o_mats     (o_mats)
{

    // Check the input consists of 6-vectors.
    assert(i_vecs.count >= 1);
    assert(i_vecs.rows  == 6);
    assert(i_vecs.cols  == 1);

    // Check the output consists of 4x4 matrices.
    assert(o_mats.count >= 1);
    assert(o_mats.rows  == 4);
    assert(o_mats.cols  == 4);

    // Check that the counts match.
    assert(o_mats.count == i_vecs.count);

    worker.compile(&program, &kernel, OCL_SE3_EXP, "se3_exp");
}

SE3ExpStep::~SE3ExpStep() {
    // Do nothing.
}

void SE3ExpStep::execute() {
    // Assign kernel parameters.
    kernel.setArg(0, i_vecs.memory);
    kernel.setArg(1, o_mats.memory);

    // Number of matrices to calculate in parallel.
    size_t const count = i_vecs.count;

    // Queue kernel with global size set to number of matrices.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(count), cl::NullRange);
}

} // namespace CL
} // namespace CVD
