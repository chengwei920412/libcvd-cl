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

#ifndef __CVD_CL_POSE_UVQ_WLS_STEP_HH__
#define __CVD_CL_POSE_UVQ_WLS_STEP_HH__

#include <cvd-cl/states/UvqUvState.hh>
#include <cvd-cl/worker/WorkerStep.hh>

namespace CVD {
namespace CL  {

/// \brief WorkerStep to compute WLS for 3-point pose.
///
/// \see CholeskyStep
class PoseUvqWlsStep : public WorkerStep {
public:

    /// \brief Construct the step.
    ///
    /// \param i_uvquv   Randomly selected sets of 3 ((u,v,q),(u,v)) records.
    /// \param i_m       Input previous SE3 pose estimate.
    /// \param o_a       Output matrix.
    /// \param o_b       Output vector.
    explicit PoseUvqWlsStep(UvqUvState<3> & i_uvquv, MatrixState<4, 4> & i_m, MatrixState<6, 6> & o_a, MatrixState<6, 1> & o_b);

    /// \brief De-construct the step.
    virtual ~PoseUvqWlsStep();

    virtual void execute();

protected:

    /// \brief Randomly selected sets of 3 ((u,v,q),(u,v)) records.
    UvqUvState<3>     & i_uvquv;

    /// \brief Input previous SE3 pose estimate.
    MatrixState<4, 4> & i_m;

    /// \brief Output matrix.
    MatrixState<6, 6> & o_a;

    /// \brief Output vector.
    MatrixState<6, 1> & o_b;

    /// \brief OpenCL program.
    cl::Program         program;

    /// \brief OpenCL kernel.
    cl::Kernel          kernel;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_POSE_UVQ_WLS_STEP_HH__ */
