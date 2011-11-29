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

#include "cvd-cl/states/blitz/BlitzTools.hh"

#include <GL/gl.h>

#include <fstream>

namespace CVD {
namespace CL  {

void glDrawPixelsRGBA(blitz::Array<cl_uchar, 3> const & array) {
    int const ny = array.length(0);
    int const nx = array.length(1);
    int const nc = array.length(2);

    // For RGBA images only.
    assert(nc == 4);

    ::glPixelStorei(GL_UNPACK_ALIGNMENT, 0);
    ::glPixelStorei(GL_UNPACK_ROW_LENGTH, nx);
    ::glDrawPixels(nx, ny, GL_RGBA, GL_UNSIGNED_BYTE, array.data());
    ::glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

void readTextRGBD(
    blitz::Array<cl_uchar, 3> & colour,
    blitz::Array<cl_float, 3> & depth,
    char const * path
) {

    // Open file, enabling exceptions.
    std::ifstream file;
    file.exceptions(~std::ios_base::goodbit);
    file.open(path, std::ios::in | std::ios::binary);

    int nx = 0;
    int ny = 0;

    // Read image size.
    file >> nx;
    file >> ny;

    assert(nx > 0);
    assert(ny > 0);

    // Allocate images of given size.
    colour.resize(ny, nx, 4);
     depth.resize(ny, nx, 1);

    // Reset image data.
    colour = 0;
     depth = 0;

    for (int y = 0; y < ny; y++) {
        for (int x = 0; x < nx; x++) {
            cl_uint r = 0;
            cl_uint g = 0;
            cl_uint b = 0;
            cl_uint d = 0;

            file >> r;
            file >> g;
            file >> b;
            file >> d;

            assert(r <= 0xFF);
            assert(g <= 0xFF);
            assert(b <= 0xFF);
            assert(d <= 0xFFFF);

            colour(y, x, 0) = r;
            colour(y, x, 1) = g;
            colour(y, x, 2) = b;
             depth(y, x, 0) = d;
        }
    }

    // Close file.
    file.close();
}

} // namespace CL
} // namespace CVD
