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

#ifndef __CVD_CL_BLITZ_TOOLS_HH__
#define __CVD_CL_BLITZ_TOOLS_HH__

#include <cvd-cl/states/ImageState.hh>

#include <blitz/array.h>

#include <cassert>

namespace CVD {
namespace CL  {

/// \brief Assign an ImageState from a Blitz pixel data array.
///
/// \pre \code
/// image.ny == array.length(0)
/// image.nx == array.length(1)
/// channels == array.length(2)
/// \endcode
///
/// \param image  ImageState to assign to.
/// \param array  Pixel data to assign from.
template<class Pixel, cl_uint channels>
void setImage(ImageState<Pixel, channels> & image, blitz::Array<Pixel, 3> const & array) {
    assert(image.ny == (cl_uint) array.length(0));
    assert(image.nx == (cl_uint) array.length(1));
    assert(channels == (cl_uint) array.length(2));

    image.set(array.data());
}


/// \brief Assign Blitz pixel data array from an ImageState.
///
/// \pre \code
/// image.ny == array.length(0)
/// image.nx == array.length(1)
/// channels == array.length(2)
/// \endcode
///
/// \param array  Pixel data to assign to.
/// \param image  ImageState to assign from.
template<class Pixel, cl_uint channels>
void getImage(ImageState<Pixel, channels> & image, blitz::Array<Pixel, 3> & array) {
    assert(image.ny == (cl_uint) array.length(0));
    assert(image.nx == (cl_uint) array.length(1));
    assert(channels == (cl_uint) array.length(2));

    image.get(array.data());
}

/// \brief Invoke \c glDrawPixels for a Blitz pixel data array of 32-bit RGBA.
///
/// \pre \code
/// array.length(2) == 4
/// \endcode
///
/// \param array  Pixel data to draw from.
void glDrawPixelsRGBA(blitz::Array<cl_uchar, 3> const & array);

/// \brief Read plaintext RGBD data into a Blitz pixel data array.
///
/// The arrays will be resized based on the first two lines of the
/// plaintext file.
///
/// \param colour  Colour pixel data array for output.
/// \param depth   Depth pixel data array for output.
/// \param path    Path to plaintext file.
void readTextRGBD(
    blitz::Array<cl_uchar, 3> & colour,
    blitz::Array<cl_float, 3> & depth,
    char const * path
);

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_BLITZ_TOOLS_HH__ */
