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

#ifndef __CVD_CL_HIPS_TURN_STEP_HH__
#define __CVD_CL_HIPS_TURN_STEP_HH__

#include <cvd-cl/states/ListState.hh>
#include <cvd-cl/worker/WorkerStep.hh>

namespace CVD {
namespace CL  {

class HipsTurnStep : public WorkerStep {
public:

    explicit HipsTurnStep(HipsListState & i_hips1, HipsListState & i_hips2, PointListState & o_matches, cl_int maxerr=3);
    virtual ~HipsTurnStep();

    virtual void execute();

protected:

    // Inputs.
    HipsListState  & i_hips1;
    HipsListState  & i_hips2;

    // Outputs.
    PointListState & o_matches;

    // Internal.
    HipsListState    m_hips1;
    HipsListState    m_hips2;
    PointListState   m_best;

    // Parameters.
    cl_int const     maxerr;

    cl::Program      program_turn;
    cl::Kernel       kernel_turn;

    cl::Program      program_find;
    cl::Kernel       kernel_find;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_HIPS_TURN_STEP_HH__ */
