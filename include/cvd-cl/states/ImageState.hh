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

#include <cvd-cl/states/BaseImageState.hh>
#include <cvd-cl/tools/ImageFormats.hh>

#include <boost/date_time.hpp>
#include <boost/thread/thread_time.hpp>

namespace CVD {
namespace CL  {

template<class Pixel>
class ImageState : public BaseImageState {
public:

    // Convenient type aliases.
    typedef CVD::BasicImage <Pixel> AsBasic;
    typedef CVD::SubImage   <Pixel> AsSub;

    ::cl_channel_order const static clChannelOrder = CVD2CL<Pixel>::order;
    ::cl_channel_type  const static clChannelType  = CVD2CL<Pixel>::type;

    explicit ImageState(Worker & worker, CVD::ImageRef const & size) :
        BaseImageState(worker, size, clChannelOrder, clChannelType, sizeof(Pixel))
    {
        // Do nothing.
    }

    virtual ~ImageState() {
        // Do nothing.
    }

    AsBasic asImage() {
        Pixel * const pixels = reinterpret_cast<Pixel *>(mapping);
        return AsBasic(pixels, size);
    }

    void set(AsSub const & image) {
        asImage().copy_from(image);
        copyToWorker();
    }

    void get(AsSub       * image) {
        copyFromWorker();
        image->copy_from(asImage());
    }

    int64_t measure(AsSub const & image, int repeat=10) {
        // Finish worker queue before starting timing.
        worker.finish();

        boost::system_time const t1 = boost::get_system_time();

        for (int i = 0; i < repeat; i++)
            set(image);

        // Finish worker queue before stopping timing.
        worker.finish();
        boost::system_time const t2 = boost::get_system_time();
        return ((t2 - t1).total_microseconds() / repeat);
    }
};

typedef ImageState<CVD::byte            >  GrayImageState;
typedef ImageState<CVD::Rgba<CVD::byte> >  RichImageState;
typedef ImageState<cl_float             > FloatImageState;

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_IMAGE_STATE_HH__ */
