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

#ifndef __CVD_CL_BLITZ_TOOLS_HH__
#define __CVD_CL_BLITZ_TOOLS_HH__

#include <cvd-cl/states/ImageState.hh>

#include <blitz/array.h>

#include <cassert>

namespace CVD {
namespace CL  {

template<class Pixel, cl_uint channels>
void setImage(ImageState<Pixel, channels> & image, blitz::Array<Pixel, 3> const & array) {
    assert(image.ny == (cl_uint) array.length(0));
    assert(image.nx == (cl_uint) array.length(1));
    assert(channels == (cl_uint) array.length(2));

    image.set(array.data());
}

template<class Pixel, cl_uint channels>
void getImage(ImageState<Pixel, channels> & image, blitz::Array<Pixel, 3> & array) {
    assert(image.ny == (cl_uint) array.length(0));
    assert(image.nx == (cl_uint) array.length(1));
    assert(channels == (cl_uint) array.length(2));

    image.get(array.data());
}

void glDrawPixelsRGBA(blitz::Array<cl_uchar, 3> const & array);

void readTextRGBD(
    blitz::Array<cl_uchar, 3> & colour,
    blitz::Array<cl_float, 3> & depth,
    char const * path
);

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_BLITZ_TOOLS_HH__ */
