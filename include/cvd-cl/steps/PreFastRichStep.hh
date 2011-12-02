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

#ifndef __CVD_CL_PRE_FAST_RICH_STEP_HH__
#define __CVD_CL_PRE_FAST_RICH_STEP_HH__

#include <cvd-cl/states/ImageState.hh>
#include <cvd-cl/states/ListState.hh>
#include <cvd-cl/worker/WorkerStep.hh>

namespace CVD {
namespace CL  {

/// \brief WorkerStep to select points that are reasonably likely to later satisfy a FAST corner test.
class PreFastRichStep : public WorkerStep {
public:

    /// \brief Construct the step.
    ///
    /// \param i_image     Input image.
    /// \param o_points    Output point list.
    /// \param threshold   FAST threshold.
    explicit PreFastRichStep(RichImageState & i_image, PointListState & o_points, cl_int threshold=40);

    /// \brief De-construct the step.
    virtual ~PreFastRichStep();

    virtual void execute();

protected:

    /// \brief Input image.
    RichImageState & i_image;

    /// \brief Output point list.
    PointListState & o_points;

    /// \brief FAST threshold.
    cl_int const     threshold;

    /// \brief OpenCL program.
    cl::Program      program;

    /// \brief OpenCL kernel.
    cl::Kernel       kernel;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_PRE_FAST_RICH_STEP_HH__ */
