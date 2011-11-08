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

#include <cassert>
#include <cstdio>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <cvd/camera.h>
#include <cvd/fast_corner.h>
#include <cvd/gl_helpers.h>
#include <cvd/image_io.h>
#include <cvd/videodisplay.h>

#include <cvd-cl/states/PointSpiral.hh>

static void recorner(std::vector<cl_int2> & vecs, std::vector<CVD::ImageRef> const & refs) {
    size_t const nrefs = refs.size();
    vecs.resize(nrefs);

    for (size_t i = 0; i < nrefs; i++) {
        CVD::ImageRef const & ref = refs.at(i);
        cl_int2             & vec = vecs.at(i);

        vec.x = ref.x;
        vec.y = ref.y;
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        ::fprintf(stderr, "usage: test-spiral <path1> <path2>\n");
        return 1;
    }

    char const * const path1 = argv[1];
    char const * const path2 = argv[2];

    CVD::Image<CVD::byte> const image1 = CVD::img_load(path1);
    CVD::Image<CVD::byte> const image2 = CVD::img_load(path2);

    std::vector<CVD::ImageRef> corners1;
    std::vector<CVD::ImageRef> corners2;

    CVD::fast_corner_detect_9(image1, corners1, 150);
    CVD::fast_corner_detect_9(image2, corners2, 150);

    std::vector<int> scores1;
    std::vector<int> scores2;

    CVD::fast_corner_score_9(image1, corners1, 0, scores1);
    CVD::fast_corner_score_9(image2, corners2, 0, scores2);

    int const ncorners1 = corners1.size();
    int const ncorners2 = corners2.size();

    std::vector<cl_int2> positions1;
    std::vector<cl_int2> positions2;

    recorner(positions1, corners1);
    recorner(positions2, corners2);

    CVD::CL::SpiralPoints spiral1;
    CVD::CL::SpiralPoints spiral2;

    ::fprintf(stderr, "Image 1 has %8d corners\n", ncorners1);
    ::fprintf(stderr, "Image 2 has %8d corners\n", ncorners2);

    CVD::CL::makePointSpiral(spiral1, positions1, scores1);
    CVD::CL::makePointSpiral(spiral2, positions2, scores2);

    std::vector<cl_int2> matches;
    CVD::CL::matchPointSpirals(matches, spiral1, spiral2);
    int const nmatches = matches.size();

    ::fprintf(stderr, "Match found %8d corner pairs\n", nmatches);

    CVD::ImageRef const size1 = image1.size();
    CVD::ImageRef const size2 = image2.size();

    CVD::ImageRef const sizeF(size1.x + size2.x, std::max(size1.y, size2.y));

    CVD::VideoDisplay window(sizeF);
    CVD::glDrawPixels(image1);
    CVD::glRasterPos(CVD::ImageRef(size1.x, 0));
    CVD::glDrawPixels(image2);

    glBegin(GL_LINE_STRIP);

    // Blue for spirals.
    glColor3f(0, 0, 1);

    for (int ipoint = 0; ipoint < ncorners1; ipoint++) {
        CVD::CL::SpiralPoint const & point = spiral1.at(ipoint);
        glVertex2i(point.position.x, point.position.y);
    }

    for (int ipoint = 0; ipoint < ncorners2; ipoint++) {
        CVD::CL::SpiralPoint const & point = spiral2.at(ipoint);
        glVertex2i(point.position.x + size1.x, point.position.y);
    }

    glEnd();

    glBegin(GL_LINES);

    // Red for matches.
    glColor3f(1, 0, 0);

    for (int imatch = 0; imatch < nmatches && imatch < 200; imatch++) {
        cl_int2 const pair = matches.at(imatch);
        cl_int2 const xy1  = spiral1.at(pair.x).position;
        cl_int2 const xy2  = spiral2.at(pair.y).position;

        glVertex2i(xy1.x,           xy1.y);
        glVertex2i(xy2.x + size1.x, xy2.y);
    }

    glEnd();
    glFlush();

    ::sleep(30);

    return 0;
}
