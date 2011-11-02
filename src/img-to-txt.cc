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

#include <fstream>
#include <iomanip>
#include <iostream>

#include <cvd/image_io.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "usage: img-to-txt <ipath> <opath>" << std::endl;
        return 1;
    }

    char const * const ipath = argv[1];
    char const * const opath = argv[2];

    CVD::Image<CVD::byte> image = CVD::img_load(ipath);

    int const nx = image.size().x;
    int const ny = image.size().y;

    FILE * out = fopen(opath, "wb");

    fprintf(out, "%5d\n", int(nx));
    fprintf(out, "%5d\n", int(ny));
    fprintf(out, "\n");

    for (int y = 0; y < ny; y++) {
        for (int x = 0; x < nx; x++) {
            fprintf(out, "%3d\n", int(image[y][x]));
        }
        fprintf(out, "\n");
    }

    fflush(out);
    fclose(out);

    return 0;
}
