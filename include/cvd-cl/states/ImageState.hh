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

#ifndef __CVD_CL_IMAGE_STATE_HH__
#define __CVD_CL_IMAGE_STATE_HH__

#include <cvd-cl/worker/WorkerState.hh>
#include <cvd-cl/states/PixelFormat.hh>
#include <cvd-cl/core/Expect.hh>

namespace CVD {
namespace CL  {

/// \brief WorkerState representing a 2D image with a given pixel type and number of channels per pixel.
template<class Pixel, cl_uint channels>
class ImageState : public WorkerState {
public:

    /// \brief Typedef to reify PixelType (type-based OpenCL adapter).
    typedef PixelType  <Pixel>    Type;

    /// \brief Typedef to reify PixelOrder (type-based OpenCL adapter).
    typedef PixelOrder <channels> Order;

    /// \brief Number of bytes per channel value.
    cl_uint static const bytesPerChannel = sizeof(Pixel);

    /// \brief Number of bytes per pixel.
    cl_uint static const bytesPerPixel   = (bytesPerChannel * channels);

    /// \brief Construct the ImageState with a given \a worker and size.
    ///
    /// \pre \code
    /// ny > 0
    /// nx > 0
    /// \endcode
    ///
    /// \param worker   Worker for which this ImageState will be allocated.
    /// \param ny       Number of pixels in the Y axis.
    /// \param nx       Number of pixels in the X axis.
    /// \param flags    OpenCL memory flags.
    explicit ImageState(Worker & worker, cl_uint ny, cl_uint nx, cl_mem_flags flags = CL_MEM_READ_WRITE) :
        WorkerState    (worker),
        ny             (ny),
        nx             (nx),
        flags          (flags),
        pixels         (ny * nx),
        elements       (pixels * channels),
        rowPitch       (nx * bytesPerPixel),
        bytesPerImage  (ny * rowPitch)
    {

        // Check parameters.
        expect("ImageState must have positive size", (ny > 0) && (nx > 0));

        // Create image format.
        cl::ImageFormat format(Order::order, Type::type);

        // Allocate image (may throw a CL exception).
        // Most exceptions here are from an unsupported format or a lack of memory.
        image = cl::Image2D(worker.context, flags, format, nx, ny, 0);

        // Create origin at 0, 0, 0.
        origin[0] = 0;
        origin[1] = 0;
        origin[2] = 0;

        // Create 3D region for full 2D plane.
        region[0] = nx;
        region[1] = ny;
        region[2] = 1;
    }

    /// \brief De-construct the ImageState (releases memory).
    virtual ~ImageState()
    {
        // De-allocate image.
        image = cl::Image2D();
    }

    /// \brief Assign the image data from unmanaged memory.
    ///
    /// \param data  Pointer to unmanaged but type-safe memory.
    void set(Pixel const * data) {
        // Cast to non-const void due to error in cl.hpp.
        Pixel * vdata = const_cast<Pixel *>(data);
        worker.queue.enqueueWriteImage(image, CL_TRUE, origin, region, rowPitch, 0, vdata);
    }

    /// \brief Query the image data to unmanaged memory.
    ///
    /// \param data  Pointer to unmanaged but type-safe memory.
    void get(Pixel * data) {
        worker.queue.enqueueReadImage(image, CL_TRUE, origin, region, rowPitch, 0, data);
    }

    /// \brief Assign all image data to zero regardless of size and type.
    void zero() {
        std::vector<Pixel> const zero(elements, static_cast<Pixel>(0));
        set(zero.data());
    }

    /// \brief Number of pixels in the Y axis.
    cl_uint      const  ny;

    /// \brief Number of pixels in the X axis.
    cl_uint      const  nx;

    /// \brief OpenCL memory flags.
    cl_mem_flags const  flags;

    /// \brief Total number of pixels in the image (#ny * #nx).
    cl_uint      const  pixels;

    /// \brief Total number of numbers in the image (#pixels * \a channels).
    cl_uint      const  elements;

    /// \brief Size of each row of the image data in bytes (#nx * #bytesPerPixel).
    cl_uint      const  rowPitch;

    /// \brief Size of the image data in bytes (#ny * #rowPitch).
    cl_uint      const  bytesPerImage;

    /// \brief OpenCL image memory.
    cl::Image2D         image;

private:

    /// \brief Convenience origin (0,0,0).
    cl::size_t<3>       origin;

    /// \brief Convenience region (nx,ny,1).
    cl::size_t<3>       region;
};

typedef ImageState<cl_uchar, 4>  GrayImageState;
typedef ImageState<cl_uchar, 4>  RichImageState;
typedef ImageState<cl_float, 1> FloatImageState;

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_IMAGE_STATE_HH__ */
