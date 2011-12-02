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

#ifndef __CVD_CL_FAST_RICH_STEP_HH__
#define __CVD_CL_FAST_RICH_STEP_HH__

#include <cvd-cl/states/ImageState.hh>
#include <cvd-cl/states/ListState.hh>
#include <cvd-cl/worker/WorkerStep.hh>

namespace CVD {
namespace CL  {

/// \brief WorkerStep to filter a point list by FAST corner criteria.
class FastRichStep : public WorkerStep {
public:

    /// \brief Construct the step.
    ///
    /// \param i_image     Input image.
    /// \param i_points    Input point list.
    /// \param o_points    Output point list.
    /// \param threshold   FAST threshold.
    /// \param ring        Consecutive bits required in FAST ring.
    explicit FastRichStep(RichImageState & i_image, PointListState & i_points, PointListState & o_points, cl_int threshold=40, cl_int ring=9);

    /// \brief De-construct the step.
    virtual ~FastRichStep();

    virtual void execute();

protected:

    /// \brief Input image.
    RichImageState & i_image;

    /// \brief Input point list.
    PointListState & i_points;

    /// \brief Output point list.
    PointListState & o_points;

    /// \brief FAST threshold.
    cl_int const     threshold;

    /// \brief Consecutive bits required in FAST ring.
    cl_int const     ring;

    /// \brief OpenCL program.
    cl::Program      program;

    /// \brief OpenCL kernel.
    cl::Kernel       kernel;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_FAST_RICH_STEP_HH__ */
