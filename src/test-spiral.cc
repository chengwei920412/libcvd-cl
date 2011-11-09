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
#include <cvd/glwindow.h>

#include <cvd-cl/states/PointSpiral.hh>

#include <boost/date_time.hpp>
#include <boost/thread.hpp>

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

static cl_int2 findCenter(std::vector<cl_int2> const & positions) {
    // Allocate spiral points for each position.
    cl_int const npoints = positions.size();

    // Start with center estimate at (0,0).
    cl_int2 center = {{0, 0}};

    for (cl_int i = 0; i < npoints; i++) {
        cl_int2 const & position = positions.at(i);

        // Count towards total x and y.
        center.x += position.x;
        center.y += position.y;
    }

    // Estimate center as average of x and y.
    if (npoints > 0) {
        center.x /= npoints;
        center.y /= npoints;
    }

    return center;
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

    CVD::ImageRef const size1 = image1.size();
    CVD::ImageRef const size2 = image2.size();

    ::fprintf(stderr, "Image 1 has size %8d x %8d\n", int(size1.x), int(size1.y));
    ::fprintf(stderr, "Image 2 has size %8d x %8d\n", int(size2.x), int(size2.y));

    std::vector<CVD::ImageRef> corners1;
    std::vector<CVD::ImageRef> corners2;

    CVD::fast_corner_detect_9(image1, corners1, 150);
    CVD::fast_corner_detect_9(image2, corners2, 150);

    int const ncorners1 = corners1.size();
    int const ncorners2 = corners2.size();

    ::fprintf(stderr, "Image 1 has %8d corners\n", ncorners1);
    ::fprintf(stderr, "Image 2 has %8d corners\n", ncorners2);

    std::vector<int> scores1;
    std::vector<int> scores2;

    CVD::fast_corner_score_9(image1, corners1, 0, scores1);
    CVD::fast_corner_score_9(image2, corners2, 0, scores2);

    std::vector<cl_int2> positions1;
    std::vector<cl_int2> positions2;

    recorner(positions1, corners1);
    recorner(positions2, corners2);

    // Image centers, which may be adjusted by clicking.
    cl_int2 center1 = findCenter(positions1);
    cl_int2 center2 = findCenter(positions2);

    CVD::ImageRef const sizeF(size1.x + size2.x, std::max(size1.y, size2.y));

    CVD::GLWindow window(sizeF);

    window.make_current();

    while (true) {
        CVD::CL::SpiralPoints spiral1;
        CVD::CL::SpiralPoints spiral2;

        boost::system_time const t1 = boost::get_system_time();
        CVD::CL::makePointSpiral(spiral1, positions1, scores1, center1);
        boost::system_time const t2 = boost::get_system_time();
        CVD::CL::makePointSpiral(spiral2, positions2, scores2, center2);
        boost::system_time const t3 = boost::get_system_time();

        std::vector<cl_int2> matches;
        CVD::CL::matchPointSpirals(matches, spiral1, spiral2);
        int const nmatches = matches.size();

        boost::system_time const t4 = boost::get_system_time();

        ::fprintf(stderr, "Match found %8d corner pairs\n", nmatches);

        ::fprintf(stderr, "%9ld us making spiral 1\n",  long((t2 - t1).total_microseconds()));
        ::fprintf(stderr, "%9ld us making spiral 2\n",  long((t3 - t2).total_microseconds()));
        ::fprintf(stderr, "%9ld us matching spirals\n", long((t4 - t3).total_microseconds()));

        CVD::glRasterPos(CVD::ImageRef(0, 0));
        CVD::glDrawPixels(image1);
        CVD::glRasterPos(CVD::ImageRef(size1.x, 0));
        CVD::glDrawPixels(image2);

        // Blue for spirals.
        glColor3f(0, 0, 1);

        glBegin(GL_LINE_STRIP);
        for (int ipoint = 0; ipoint < ncorners1; ipoint++) {
            CVD::CL::SpiralPoint const & point = spiral1.at(ipoint);
            glVertex2i(point.position.x, point.position.y);
        }
        glEnd();

        glBegin(GL_LINE_STRIP);
        for (int ipoint = 0; ipoint < ncorners2; ipoint++) {
            CVD::CL::SpiralPoint const & point = spiral2.at(ipoint);
            glVertex2i(point.position.x + size1.x, point.position.y);
        }
        glEnd();

        // Red for matches.
        glColor3f(1, 0, 0);

        for (int imatch = 0; imatch < nmatches; imatch++) {
            cl_int2 const pair = matches.at(imatch);
            cl_int2 const xy1  = spiral1.at(pair.x).position;
            cl_int2 const xy2  = spiral2.at(pair.y).position;

            glBegin(GL_LINES);
            glVertex2i(xy1.x,           xy1.y);
            glVertex2i(xy2.x + size1.x, xy2.y);
            glEnd();
        }

        // Green for centers.
        glColor3f(0, 1, 0);

        glBegin(GL_LINES);
        glVertex2i(          center1.x - 10, center1.y     );
        glVertex2i(          center1.x + 10, center1.y     );
        glVertex2i(          center1.x     , center1.y - 10);
        glVertex2i(          center1.x     , center1.y + 10);
        glVertex2i(size1.x + center2.x - 10, center2.y     );
        glVertex2i(size1.x + center2.x + 10, center2.y     );
        glVertex2i(size1.x + center2.x     , center2.y - 10);
        glVertex2i(size1.x + center2.x     , center2.y + 10);
        glEnd();

        glFlush();

        window.swap_buffers();

        ::sleep(1);

        // Get all window events.
        std::vector<CVD::GLWindow::Event> events;
        window.get_events(events);

        for (size_t ievent = 0; ievent < events.size(); ievent++) {
            CVD::GLWindow::Event const & event = events.at(ievent);

            switch (event.type) {
                case CVD::GLWindow::Event::MOUSE_DOWN: {
                    switch (event.which) {
                        case CVD::GLWindow::BUTTON_LEFT: {
                            // Find mouse coordinate.
                            int const mx = event.where.x;
                            int const my = event.where.y;

                            if (mx >= size1.x) {
                                // Click in right image.
                                center2.x = (mx - size1.x);
                                center2.y = my;
                            } else {
                                // Click in left image.
                                center1.x = mx;
                                center1.y = my;
                            }

                            break;
                        }

                        default: {
                            break;
                        }
                    }

                    break;
                }

                default: {
                    break;
                }
            }
        }
    }

    return 0;
}
