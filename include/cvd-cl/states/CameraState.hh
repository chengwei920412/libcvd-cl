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

#ifndef __CVD_CL_CAMERA_STATE_HH__
#define __CVD_CL_CAMERA_STATE_HH__

#include <cvd-cl/states/ImageState.hh>
#include <cvd-cl/states/blitz/BlitzTools.hh>

namespace CVD {
namespace CL  {

/// \brief WorkerState representing (u,v,q) mappings for each pixel of a camera.
class CameraState : public WorkerState {
public:

    /// \brief Typedef to reify associated Blitz array type.
    typedef blitz::Array<cl_float, 3> BlitzArray;

    /// \brief Construct the CameraState with a given \a worker and size.
    ///
    /// \pre \code
    /// ny > 0
    /// nx > 0
    /// \endcode
    ///
    /// \param worker   Worker for which this CameraState will be allocated.
    /// \param ny       Number of pixels in the Y axis.
    /// \param nx       Number of pixels in the X axis.
    explicit CameraState(Worker & worker, cl_uint ny, cl_uint nx);

    /// \brief De-construct the CameraState (releases memory).
    virtual ~CameraState();

    /// \brief Number of pixels in the Y axis.
    cl_uint      const  ny;

    /// \brief Number of pixels in the X axis.
    cl_uint      const  nx;

    /// \brief Array of u data.
    BlitzArray          udata;

    /// \brief Array of v data.
    BlitzArray          vdata;

    /// \brief Array of q data.
    BlitzArray          qdata;

    /// \brief ImageState for u data.
    FloatImageState     umap;

    /// \brief ImageState for v data.
    FloatImageState     vmap;

    /// \brief ImageState for q data.
    FloatImageState     qmap;

    /// \brief Copy camera data to worker memory.
    void copyFromWorker();

    /// \brief Copy camera data from worker memory.
    void copyToWorker();
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_CAMERA_STATE_HH__ */
