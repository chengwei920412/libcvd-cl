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

#ifndef __CVD_CL_RANDOM_INT_STEP_HH__
#define __CVD_CL_RANDOM_INT_STEP_HH__

#include <cvd-cl/states/ListState.hh>
#include <cvd-cl/worker/WorkerStep.hh>

namespace CVD {
namespace CL  {

/// \brief WorkerStep to select random integers.
class RandomIntStep : public WorkerStep {
public:

    /// \brief Construct the step.
    ///
    /// \param i_max    Buffer for integer maximum.
    /// \param o_ints   Output list of integers.
    explicit RandomIntStep(CountState & i_max, IntListState & o_ints);

    /// \brief De-construct the step.
    virtual ~RandomIntStep();

    virtual void execute();

protected:

    /// \brief Buffer for integer maximum.
    CountState      & i_max;

    /// \brief Output list of integers.
    IntListState    & o_ints;

    /// \brief OpenCL program.
    cl::Program       program;

    /// \brief OpenCL kernel.
    cl::Kernel        kernel;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_RANDOM_INT_STEP_HH__ */
