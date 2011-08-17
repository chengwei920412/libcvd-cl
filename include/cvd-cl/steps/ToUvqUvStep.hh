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

#ifndef __CVD_CL_FXY_STEP_HH__
#define __CVD_CL_FXY_STEP_HH__

#include <cvd-cl/states/CameraState.hh>
#include <cvd-cl/states/ListState.hh>
#include <cvd-cl/states/UvqUvState.hh>
#include <cvd-cl/worker/WorkerStep.hh>

namespace CVD {
namespace CL  {

class ToUvqUvStep : public WorkerStep {
public:

    explicit ToUvqUvStep(CameraState & i_camera, PointListState & i_xy1, PointListState & i_xy2, UvqUvState & o_uvquv);
    virtual ~ToUvqUvStep();

    virtual void execute();

protected:

    // Inputs.
    CameraState     & i_camera;
    PointListState  & i_xy1;
    PointListState  & i_xy2;

    // Outputs.
    UvqUvState      & o_uvquv;

    cl::Program       program;
    cl::Kernel        kernel;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_FXY_STEP_HH__ */
