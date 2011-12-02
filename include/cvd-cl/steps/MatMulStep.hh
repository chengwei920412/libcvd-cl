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

#ifndef __CVD_CL_MAT_MUL_STEP_HH__
#define __CVD_CL_MAT_MUL_STEP_HH__

#include <cvd-cl/states/MatrixState.hh>
#include <cvd-cl/worker/WorkerStep.hh>

#include "kernels/mat-mul-3.hh"
#include "kernels/mat-mul-4.hh"
#include "kernels/mat-mul-5.hh"
#include "kernels/mat-mul-6.hh"

namespace CVD {
namespace CL  {

/// \brief WorkerStep to compute matrix multiplication.
template<size_t rows>
class MatMulStep : public WorkerStep {
private:

    // Check that the matrix size is supported.
    BOOST_STATIC_ASSERT(rows >= 3);
    BOOST_STATIC_ASSERT(rows <= 6);

public:

    /// \brief Typedef to reify MatrixState.
    typedef MatrixState<rows, rows> MyMatrix;

    /// \brief Construct the step to compute \f$C = AB\f$.
    ///
    /// \param i_a   Input matrix \f$A\f$.
    /// \param io_b  Input matrix \f$B\f$ before execution, output matrix \f$C\f$ after execution.
    explicit MatMulStep(MyMatrix & i_a, MyMatrix & io_b) :
        WorkerStep (i_a.worker),
        i_a        (i_a),
        io_b       (io_b)
    {

        // Select a kernel based on the size.
        // This is decidable at compile-time.
        switch (rows) {
        case 3: worker.compile(&program, &kernel, OCL_MAT_MUL_3, "mat_mul_3"); break;
        case 4: worker.compile(&program, &kernel, OCL_MAT_MUL_4, "mat_mul_4"); break;
        case 5: worker.compile(&program, &kernel, OCL_MAT_MUL_5, "mat_mul_5"); break;
        case 6: worker.compile(&program, &kernel, OCL_MAT_MUL_6, "mat_mul_6"); break;
        default: break;
        }
    }

    /// \brief De-construct the step.
    virtual ~MatMulStep() {
        // Do nothing.
    }

    virtual void execute() {
        // Assign kernel parameters.
        kernel.setArg(0, i_a.memory);
        kernel.setArg(1, io_b.memory);

        // Number of matrices to calculate in parallel.
        size_t const count = i_a.count;

        // Queue kernel with global size set to number of matrices.
        worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(count), cl::NullRange);
    }

protected:

    /// \brief Input matrix \f$A\f$.
    MyMatrix       & i_a;

    /// \brief Input matrix \f$B\f$ before execution, output matrix \f$C\f$ after execution.
    MyMatrix       & io_b;

    /// \brief OpenCL program.
    cl::Program      program;

    /// \brief OpenCL kernel.
    cl::Kernel       kernel;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_MAT_MUL_STEP_HH__ */
