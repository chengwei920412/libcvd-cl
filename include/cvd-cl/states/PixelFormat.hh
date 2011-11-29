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

#ifndef __CVD_CL_PIXEL_FORMAT_HH__
#define __CVD_CL_PIXEL_FORMAT_HH__

#include <cvd-cl/worker/Worker.hh>

namespace CVD {
namespace CL  {

template<class _ChannelScalar>
class PixelType : private boost::noncopyable {
public:

    // Reveal _ChannelScalar template parameter as public type.
    typedef _ChannelScalar ChannelScalar;

    ::cl_uint static const bytesPerChannel = sizeof(ChannelScalar);

    // Define in explicit specialisations:
    // ::cl_channel_type static const type  = ...;

private:

    BOOST_STATIC_ASSERT(bytesPerChannel >= 1);
    BOOST_STATIC_ASSERT(bytesPerChannel <= 4);

    // Block instantiation.
    PixelType();
};

template<cl_uint _channels>
class PixelOrder : private boost::noncopyable {
public:

    // Reveal channels template parameter as public constant.
    ::cl_uint static const channels = _channels;

    // Define in explicit specialisations:
    // ::cl_channel_order const static order = ...;

private:

    BOOST_STATIC_ASSERT(channels >= 1);
    BOOST_STATIC_ASSERT(channels <= 4);

    // Block instantiation.
    PixelOrder();
};

#define PIXEL_TYPE(cl_type, CL_CODE) \
template<> struct PixelType<cl_type> {::cl_channel_type const static type = CL_CODE;};

#define PIXEL_ORDER(channels, CL_CODE) \
template<> struct PixelOrder<channels> {::cl_channel_order const static order = CL_CODE;};

// Declare pixel channel types.

PIXEL_TYPE(cl_uchar,   CL_UNSIGNED_INT8);
PIXEL_TYPE(cl_ushort,  CL_UNSIGNED_INT16);
PIXEL_TYPE(cl_uint,    CL_UNSIGNED_INT32);

PIXEL_TYPE(cl_char,    CL_SIGNED_INT8);
PIXEL_TYPE(cl_short,   CL_SIGNED_INT16);
PIXEL_TYPE(cl_int,     CL_SIGNED_INT32);

PIXEL_TYPE(cl_float,   CL_FLOAT);

// Declare pixel channel orders.

PIXEL_ORDER(1,         CL_INTENSITY);
PIXEL_ORDER(2,         CL_RG);
PIXEL_ORDER(3,         CL_RGB);
PIXEL_ORDER(4,         CL_RGBA);

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_PIXEL_FORMAT_HH__ */
