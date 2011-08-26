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

#include "cvd-cl/steps/MatIdentStep.hh"
#include "cvd-cl/core/Expect.hh"

#include "kernels/mat-mki-3.hh"
#include "kernels/mat-mki-4.hh"
#include "kernels/mat-mki-5.hh"
#include "kernels/mat-mki-6.hh"

namespace CVD {
namespace CL  {

MatIdentStep::MatIdentStep(MatrixState & o_a) :
    WorkerStep (o_a.worker),
    o_a        (o_a)
{

    // Check that the matrix size is supported.
    assert(o_a.count >= 1);
    assert(o_a.rows  >= 3);
    assert(o_a.cols  <= 6);

    // Check that matrix is square.
    assert(o_a.rows == o_a.cols);

    // Select a kernel based on the size.
    switch (o_a.rows) {
    case 3: worker.compile(&program, &kernel, OCL_MAT_MKI_3, "mat_mki_3"); break;
    case 4: worker.compile(&program, &kernel, OCL_MAT_MKI_4, "mat_mki_4"); break;
    case 5: worker.compile(&program, &kernel, OCL_MAT_MKI_5, "mat_mki_5"); break;
    case 6: worker.compile(&program, &kernel, OCL_MAT_MKI_6, "mat_mki_6"); break;
    default: break;
    }
}

MatIdentStep::~MatIdentStep() {
    // Do nothing.
}

void MatIdentStep::execute() {
    // Assign kernel parameters.
    kernel.setArg(0, o_a.memory);

    // Number of matrices to calculate in parallel.
    size_t const count = o_a.count;

    // Queue kernel with global size set to number of matrices.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(count), cl::NullRange);
}

} // namespace CL
} // namespace CVD
