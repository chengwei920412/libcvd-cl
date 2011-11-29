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

#ifndef __CVD_CL_HIPS_TREE_FIND_STEP_HH__
#define __CVD_CL_HIPS_TREE_FIND_STEP_HH__

#include <cvd-cl/states/HipsTreeState.hh>
#include <cvd-cl/states/ListState.hh>
#include <cvd-cl/worker/WorkerStep.hh>

namespace CVD {
namespace CL  {

class HipsTreeFindStep : public WorkerStep {
public:

    explicit HipsTreeFindStep(HipsTreeState & i_tree, HipsListState & i_hips, PointListState & o_matches, cl_uint maxerr=3, bool rotate=true);
    virtual ~HipsTreeFindStep();

    virtual void execute();

protected:

    void findByQueue();

    // Inputs.
    HipsTreeState  & i_tree;
    HipsListState  & i_hips;

    // Outputs.
    PointListState & o_matches;

    // Parameters.
    cl_uint const    maxerr;
    bool    const    rotate;

    cl::Program      program;
    cl::Kernel       kernel;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_HIPS_TREE_FIND_STEP_HH__ */
