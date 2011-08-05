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

#ifndef __CVD_CL_IMAGE_STATE_HH__
#define __CVD_CL_IMAGE_STATE_HH__

#include <cvd-cl/worker/WorkerState.hh>

#include <cvd/byte.h>
#include <cvd/image.h>

namespace CVD {
namespace CL  {

typedef CVD::BasicImage<CVD::byte> ByteImage;
typedef CVD::SubImage<CVD::byte> ByteSubImage;

class ImageState : public WorkerState<ByteSubImage> {
public:

    explicit ImageState(Worker & worker, CVD::ImageRef const & size);
    virtual ~ImageState();

    virtual void set(ByteSubImage const & image);
    virtual void get(ByteSubImage       * image);

    ByteImage asImage();

    void copyToWorker();
    void copyFromWorker();

    // Public immutable members.
    CVD::ImageRef const   size;
    size_t        const   nbytes;

protected:

    CVD::byte           * m_bytes;
    cl::Image2D           m_image;

    cl::size_t<3>         m_origin;
    cl::size_t<3>         m_region;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_IMAGE_STATE_HH__ */
