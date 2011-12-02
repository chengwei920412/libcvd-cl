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

#ifndef __CVD_CL_SE3_RUN1_STEP_HH__
#define __CVD_CL_SE3_RUN1_STEP_HH__

#include <cvd-cl/states/ListState.hh>
#include <cvd-cl/states/UvqUvState.hh>
#include <cvd-cl/worker/WorkerStep.hh>

namespace CVD {
namespace CL  {

/// \brief WorkerStep to execute matrix multiplication based on a single matrix.
///
/// \see SE3ScoreStep
class SE3Run1Step : public WorkerStep {
public:

    /// \brief Construct the step.
    ///
    /// \param i_uvquv   Input ((u,v,q),(u,v)) records (only first (u,v,q) is used).
    /// \param i_mats    Input SE3 matrices.
    /// \param i_which   Input buffer selecting the matrix to use.
    /// \param o_uvs     Output multiplied (u,v) vectors.
    explicit SE3Run1Step(UvqUvState<1> & i_uvquv, MatrixState<4, 4> & i_mats, CountState & i_which, Float2ListState & o_uvs);

    /// \brief De-construct the step.
    virtual ~SE3Run1Step();

    virtual void execute();

protected:

    /// \brief Input ((u,v,q),(u,v)) records (only first (u,v,q) is used).
    UvqUvState<1>     & i_uvquv;

    /// \brief Input SE3 matrices.
    MatrixState<4, 4> & i_mats;

    /// \brief Input buffer selecting the matrix to use.
    CountState        & i_which;

    /// \brief Output multiplied (u,v) vectors.
    Float2ListState   & o_uvs;

    /// \brief OpenCL program.
    cl::Program      program;

    /// \brief OpenCL kernel.
    cl::Kernel       kernel;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_SE3_RUN1_STEP_HH__ */
