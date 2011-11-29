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

template<class Pixel, cl_uint channels>
class ImageState : public WorkerState {
public:

    typedef PixelType  <Pixel>    Type;
    typedef PixelOrder <channels> Order;

    cl_uint static const bytesPerChannel = sizeof(Pixel);
    cl_uint static const bytesPerPixel   = (bytesPerChannel * channels);

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

    virtual ~ImageState()
    {
        // De-allocate image.
        image = cl::Image2D();
    }

    void set(Pixel const * data) {
        // Cast to non-const void due to error in cl.hpp.
        Pixel * vdata = const_cast<Pixel *>(data);
        worker.queue.enqueueWriteImage(image, CL_TRUE, origin, region, rowPitch, 0, vdata);
    }

    void get(Pixel * data) {
        worker.queue.enqueueReadImage(image, CL_TRUE, origin, region, rowPitch, 0, data);
    }

    void zero() {
        std::vector<Pixel> const zero(elements, static_cast<Pixel>(0));
        set(zero.data());
    }

    // Public immutable members.
    cl_uint      const  ny;
    cl_uint      const  nx;
    cl_mem_flags const  flags;
    cl_uint      const  pixels;
    cl_uint      const  elements;
    cl_uint      const  rowPitch;
    cl_uint      const  bytesPerImage;

    // Members left public for WorkerStep access.
    cl::Image2D         image;

private:

    // Origin and region.
    cl::size_t<3>       origin;
    cl::size_t<3>       region;
};

typedef ImageState<cl_uchar, 4>  GrayImageState;
typedef ImageState<cl_uchar, 4>  RichImageState;
typedef ImageState<cl_float, 1> FloatImageState;

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_IMAGE_STATE_HH__ */
