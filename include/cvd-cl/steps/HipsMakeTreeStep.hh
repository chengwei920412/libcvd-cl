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

#ifndef __CVD_CL_HIPS_MAKE_TREE_STEP_HH__
#define __CVD_CL_HIPS_MAKE_TREE_STEP_HH__

#include <cvd-cl/states/HipsTreeState.hh>
#include <cvd-cl/states/ListState.hh>
#include <cvd-cl/worker/WorkerStep.hh>

namespace CVD {
namespace CL  {

/// \brief WorkerStep to compute a balanced descriptor tree.
///
/// This delegates to DescriptorTree and so is run on the host,
/// not the worker.  Being associated with the Worker allows it to
/// read from and write to worker states.
class HipsMakeTreeStep : public WorkerStep {
public:

    /// \brief Construct the step.
    ///
    /// \param i_hips   Input descriptor list.
    /// \param o_tree   Output dense descriptor tree.
    explicit HipsMakeTreeStep(HipsListState & i_hips, HipsTreeState & o_tree);

    /// \brief De-construct the step.
    virtual ~HipsMakeTreeStep();

    virtual void execute();

protected:

    /// \brief Input descriptor list.
    HipsListState  & i_hips;

    /// \brief Output dense descriptor tree.
    HipsTreeState  & o_tree;

    // No program or kernel, this is done on the host.
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_HIPS_MAKE_TREE_STEP_HH__ */
