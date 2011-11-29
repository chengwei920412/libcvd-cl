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

#ifndef __CVD_CL_CVD_IMAGE_FORMATS_HH__
#define __CVD_CL_CVD_IMAGE_FORMATS_HH__

#include <cvd-cl/worker/Worker.hh>

#include <cvd/byte.h>
#include <cvd/image.h>
#include <cvd/rgba.h>
#include <cvd/rgb.h>

namespace CVD {
namespace CL  {

template<class Pixel>
struct CVD2CL {
    // Invalid by default.
    // Only overrides will work.
};

template<>
struct CVD2CL<CVD::byte> {
    ::cl_channel_order const static order = CL_INTENSITY;
    ::cl_channel_type  const static type  = CL_UNSIGNED_INT8;
};

template<>
struct CVD2CL<cl_float> {
    ::cl_channel_order const static order = CL_INTENSITY;
    ::cl_channel_type  const static type  = CL_FLOAT;
};

template<>
struct CVD2CL<CVD::Rgba<CVD::byte> > {
    ::cl_channel_order const static order = CL_RGBA;
    ::cl_channel_type  const static type  = CL_UNSIGNED_INT8;
};

template<>
struct CVD2CL<CVD::Rgb<CVD::byte> > {
    ::cl_channel_order const static order = CL_RGB;
    ::cl_channel_type  const static type  = CL_UNSIGNED_INT8;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_CVD_IMAGE_FORMATS_HH__ */
