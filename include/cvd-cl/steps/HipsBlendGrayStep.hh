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

#ifndef __CVD_CL_HIPS_BLEND_GRAY_STEP_HH__
#define __CVD_CL_HIPS_BLEND_GRAY_STEP_HH__

#include <cvd-cl/states/ImageState.hh>
#include <cvd-cl/states/ListState.hh>
#include <cvd-cl/worker/WorkerStep.hh>

namespace CVD {
namespace CL  {

/// \brief Keep only the center descriptor.
///
/// \code
/// [ ][ ][ ]
/// [ ][+][ ]
/// [ ][ ][ ]
/// \endcode
cl_int static const HipsBlend1 = 1;

/// \brief Blend the center descriptor with descriptors
/// north, south, east, west of the center.
///
/// \code
/// [ ][+][ ]
/// [+][+][+]
/// [ ][+][ ]
/// \endcode
cl_int static const HipsBlend5 = 5;

/// \brief Blend the center descriptor with descriptors
/// north, south, east, west,
/// north-east, north-west, south-east, south-west
/// of the center.
///
/// \code
/// [+][+][+]
/// [+][+][+]
/// [+][+][+]
/// \endcode
cl_int static const HipsBlend9 = 9;

/// \brief Step to compute blended HIPS descriptors for a grayscale image.
class HipsBlendGrayStep : public WorkerStep {
public:

    /// \brief Construct the step.
    ///
    /// \param i_image     Input image.
    /// \param i_points    Input point list.
    /// \param o_hips      Output HIPS descriptor list.
    /// \param blendSize   Blend size (1, 5, 9).
    explicit HipsBlendGrayStep(GrayImageState & i_image, PointListState & i_points, HipsListState & o_hips, cl_int blendSize = HipsBlend5);

    /// \brief De-construct the step.
    virtual ~HipsBlendGrayStep();

    virtual void execute();

protected:

    /// \brief Input image.
    GrayImageState & i_image;

    /// \brief Input point list.
    PointListState & i_points;

    /// \brief Output HIPS descriptor list.
    HipsListState  & o_hips;

    /// \brief Buffer for HIPS descriptors during blending.
    HipsListState    m_hips1;

    /// \brief Buffer for HIPS descriptors during blending.
    HipsListState    m_hips2;

    /// \brief Blend size (1, 5, 9).
    cl_int const     blendSize;

    /// \brief OpenCL program for HIPS generation.
    cl::Program      program_hips;

    /// \brief OpenCL kernel for HIPS generation.
    cl::Kernel       kernel_hips;

    /// \brief OpenCL program for HIPS blending.
    cl::Program      program_blend;

    /// \brief OpenCL kernel for HIPS blending.
    cl::Kernel       kernel_blend;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_HIPS_BLEND_GRAY_STEP_HH__ */
