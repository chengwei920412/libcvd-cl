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

#include "cvd-cl/steps/MatMulStep.hh"
#include "cvd-cl/core/Expect.hh"

#include "kernels/mat-mul-3.hh"
#include "kernels/mat-mul-4.hh"
#include "kernels/mat-mul-5.hh"
#include "kernels/mat-mul-6.hh"

namespace CVD {
namespace CL  {

MatMulStep::MatMulStep(MatrixState & i_a, MatrixState & io_b) :
    WorkerStep (i_a.worker),
    i_a        (i_a),
    io_b       (io_b)
{

    // Check that the matrix size is supported.
    assert(i_a.count >= 1);
    assert(i_a.rows  >= 3);
    assert(i_a.cols  <= 6);

    // Check that matrix is square.
    assert(i_a.rows == i_a.cols);

    // Check that the matrix dimensions are identical.
    assert(i_a.count == io_b.count);
    assert(i_a.rows  == io_b.rows);
    assert(i_a.cols  == io_b.cols);

    // Select a kernel based on the size.
    switch (i_a.rows) {
    case 3: worker.compile(&program, &kernel, OCL_MAT_MUL_3, "mat_mul_3"); break;
    case 4: worker.compile(&program, &kernel, OCL_MAT_MUL_4, "mat_mul_4"); break;
    case 5: worker.compile(&program, &kernel, OCL_MAT_MUL_5, "mat_mul_5"); break;
    case 6: worker.compile(&program, &kernel, OCL_MAT_MUL_6, "mat_mul_6"); break;
    default: break;
    }
}

MatMulStep::~MatMulStep() {
    // Do nothing.
}

void MatMulStep::execute() {
    // Assign kernel parameters.
    kernel.setArg(0, i_a.memory);
    kernel.setArg(1, io_b.memory);

    // Number of matrices to calculate in parallel.
    size_t const count = i_a.count;

    // Queue kernel with global size set to number of matrices.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(count), cl::NullRange);
}

} // namespace CL
} // namespace CVD
