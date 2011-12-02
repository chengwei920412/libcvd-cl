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

#ifndef __CVD_CL_MIX_UVQ_UV_STEP_HH__
#define __CVD_CL_MIX_UVQ_UV_STEP_HH__

#include <cvd-cl/states/UvqUvState.hh>
#include <cvd-cl/steps/RandomIntStep.hh>
#include <cvd-cl/worker/WorkerStep.hh>

namespace CVD {
namespace CL  {

/// \brief WorkerStep to randomly select sets of 3 ((u,v,q),(u,v)) records.
class MixUvqUvStep : public WorkerStep {
public:

    /// \brief Construct the step.
    ///
    /// \param i_uvquv  Input records (sets of 1 record).
    /// \param o_uvquv  Output records (sets of 3 records).
    explicit MixUvqUvStep(UvqUvState<1> & i_uvquv, UvqUvState<3> & o_uvquv);

    /// \brief De-construct the step.
    virtual ~MixUvqUvStep();

    virtual void execute();

protected:

    /// \brief Input records (sets of 1 record).
    UvqUvState<1>   & i_uvquv;

    /// \brief Output records (sets of 3 records).
    UvqUvState<3>   & o_uvquv;

    /// \brief Maximum number of records, used for random generator.
    CountState        m_max;

    /// \brief Buffer for randomly generated integers.
    IntListState      m_ints;

    /// \brief RandomIntStep to generate random integers.
    RandomIntStep     randomise;

    /// \brief OpenCL program.
    cl::Program       program;

    /// \brief OpenCL kernel.
    cl::Kernel        kernel;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_MIX_UVQ_UV_STEP_HH__ */
