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

#ifndef __CVD_CL_CHOLESKY_STEP_HH__
#define __CVD_CL_CHOLESKY_STEP_HH__

#include <cvd-cl/states/MatrixState.hh>
#include <cvd-cl/worker/WorkerStep.hh>

#include "kernels/cholesky3.hh"
#include "kernels/cholesky4.hh"
#include "kernels/cholesky5.hh"
#include "kernels/cholesky6.hh"

namespace CVD {
namespace CL  {

template<size_t rows>
class CholeskyStep : public WorkerStep {
private:

    // Check that the matrix size is supported.
    BOOST_STATIC_ASSERT(rows >= 3);
    BOOST_STATIC_ASSERT(rows <= 6);

public:

    typedef MatrixState<rows, rows> MyMatrix;
    typedef MatrixState<rows, 1   > MyVector;

    explicit CholeskyStep(MyMatrix & i_a, MyVector & i_b, MyVector & o_x) :
        WorkerStep (i_a.worker),
        i_a        (i_a),
        i_b        (i_b),
        o_x        (o_x)
    {

        // Check that the counts match.
        assert(i_a.count == i_b.count);
        assert(i_a.count == o_x.count);

        // Select a kernel based on the size.
        // This is decidable at compile-time.
        switch (rows) {
        case 3: worker.compile(&program, &kernel, OCL_CHOLESKY_3, "cholesky3"); break;
        case 4: worker.compile(&program, &kernel, OCL_CHOLESKY_4, "cholesky4"); break;
        case 5: worker.compile(&program, &kernel, OCL_CHOLESKY_5, "cholesky5"); break;
        case 6: worker.compile(&program, &kernel, OCL_CHOLESKY_6, "cholesky6"); break;
        default: break;
        }
    }

    virtual ~CholeskyStep() {
        // Do nothing.
    }

    virtual void execute() {
        // Assign kernel parameters.
        kernel.setArg(0, i_a.memory);
        kernel.setArg(1, i_b.memory);
        kernel.setArg(2, o_x.memory);

        // Number of vectors to calculate in parallel.
        size_t const count = i_a.count;

        // Queue kernel with global size set to number of matrices.
        worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(count), cl::NullRange);
    }

protected:

    // Inputs.
    MyMatrix       & i_a;
    MyVector       & i_b;

    // Outputs.
    MyVector       & o_x;

    cl::Program      program;
    cl::Kernel       kernel;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_CHOLESKY_STEP_HH__ */
