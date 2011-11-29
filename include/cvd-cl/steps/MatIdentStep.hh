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

#ifndef __CVD_CL_MAT_IDENT_STEP_HH__
#define __CVD_CL_MAT_IDENT_STEP_HH__

#include <cvd-cl/states/MatrixState.hh>
#include <cvd-cl/worker/WorkerStep.hh>

namespace CVD {
namespace CL  {

// Include kernels inside namespace.
namespace kernels {
#include "kernels/mat-mki-3.hh"
#include "kernels/mat-mki-4.hh"
#include "kernels/mat-mki-5.hh"
#include "kernels/mat-mki-6.hh"
}

template<size_t rows>
class MatIdentStep : public WorkerStep {
private:

    // Check that the matrix size is supported.
    BOOST_STATIC_ASSERT(rows >= 3);
    BOOST_STATIC_ASSERT(rows <= 6);

public:

    typedef MatrixState<rows, rows> MyMatrix;

    explicit MatIdentStep(MyMatrix & o_a) :
        WorkerStep (o_a.worker),
        o_a        (o_a)
    {

        // Select a kernel based on the size.
        // This is decidable at compile-time.
        using namespace CVD::CL::kernels;
        switch (rows) {
        case 3: worker.compile(&program, &kernel, OCL_MAT_MKI_3, "mat_mki_3"); break;
        case 4: worker.compile(&program, &kernel, OCL_MAT_MKI_4, "mat_mki_4"); break;
        case 5: worker.compile(&program, &kernel, OCL_MAT_MKI_5, "mat_mki_5"); break;
        case 6: worker.compile(&program, &kernel, OCL_MAT_MKI_6, "mat_mki_6"); break;
        default: break;
        }
    }

    virtual ~MatIdentStep() {
        // Do nothing.
    }

    virtual void execute() {
        // Assign kernel parameters.
        kernel.setArg(0, o_a.memory);

        // Number of matrices to calculate in parallel.
        size_t const count = o_a.count;

        // Queue kernel with global size set to number of matrices.
        worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(count), cl::NullRange);
    }

protected:

    // Outputs.
    MyMatrix       & o_a;

    cl::Program      program;
    cl::Kernel       kernel;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_MAT_IDENT_STEP_HH__ */
