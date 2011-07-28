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

#include <boost/cstdint.hpp>
#include <boost/foreach.hpp>

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

// Include official OpenCL C++ wrapper, with exceptions enabled.
#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

#include <cvd/image_io.h>
#include <cvd/videodisplay.h>
#include <cvd/gl_helpers.h>

// Wrapper OpenCL source code.
static const char   clCode[] = "#include <fast.cl>\n";
static const size_t clSize   = sizeof(clCode) - 1; // Exclude \0
static const std::pair<const char *, size_t> clPair(clCode, clSize);
static const cl::Program::Sources clSources(1, clPair);

// Size constants.
static const size_t KiB = 1024;
static const size_t MiB = KiB * KiB;

static void reportBuild(cl::Device &device, cl::Program &program) {
    std::string log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    if (log.empty() == false)
        std::cerr << log << std::endl;
}

static void testFAST(CVD::Image<CVD::byte> const & image,
        cl::Context &context, cl::Device &device, cl::Program &program) {

    const CVD::ImageRef size = image.size();

    cl::CommandQueue queue(context, device);

    cl::ImageFormat format(CL_INTENSITY, CL_UNSIGNED_INT8);
    cl::Image2D clImage(context, CL_MEM_READ_ONLY, format, 1024, 1024, 0);

    cl::Buffer clCorners(context, CL_MEM_WRITE_ONLY, 1024 * 1024 * sizeof(cl_int2));
    cl::Buffer clCursor(context, CL_MEM_READ_WRITE, sizeof(cl_int));

    const cl_int cursor0 = 0;
    queue.enqueueWriteBuffer(clCursor, CL_TRUE, 0, sizeof(cursor0), &cursor0);

    cl::size_t<3> origin;
    origin[0] =    0;
    origin[1] =    0;
    origin[2] =    0;

    cl::size_t<3> region;
    region[0] = 1024;
    region[1] = 1024;
    region[2] =    1;

    queue.enqueueWriteImage(clImage, CL_TRUE, origin, region, size.x, 0, (void *) image.data());

    cl::Kernel clKernel(program, "fast_gray_9");
    clKernel.setArg(0, clImage);
    clKernel.setArg(1, clCorners);
    clKernel.setArg(2, clCursor);

    queue.enqueueNDRangeKernel(clKernel, cl::NullRange, cl::NDRange(1024, 1024), cl::NDRange(32, 32));
    queue.finish();

    cl_int cursorN = 0;
    queue.enqueueReadBuffer(clCursor, CL_TRUE, 0, sizeof(cursorN), &cursorN);

    std::cerr << "    Total runtime:  " << std::endl;
    std::cerr << "    Found corners:  " << cursorN << std::endl;

    std::vector<cl_int2> corners(cursorN);
    queue.enqueueReadBuffer(clCorners, CL_TRUE, 0, sizeof(cl_int2) * cursorN, corners.data());

    CVD::VideoDisplay window(size);           //Create an OpenGL window with the dimensions of `in'
    CVD::glDrawPixels(image);

    glColor3f(1, 0, 0);
    glBegin(GL_POINTS);
    BOOST_FOREACH(const cl_int2 &xy, corners) {
        glVertex2i(xy.x, xy.y);
    }
    glEnd();
    glFlush();

    sleep(30);
}

int main(int argc, char **argv) {
    CVD::Image<CVD::byte> image;

    image = CVD::img_load("/usr/share/backgrounds/Grey_day_by_Drew__.jpg");

    const CVD::ImageRef size = image.size();

    std::cerr << "Image size: " << size.x << " x " << size.y << std::endl;

    try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        BOOST_FOREACH(cl::Platform &pf, platforms) {
            std::cerr
                << pf.getInfo<CL_PLATFORM_NAME   >() << " ("
                << pf.getInfo<CL_PLATFORM_VENDOR >() << ", "
                << pf.getInfo<CL_PLATFORM_VERSION>() << ")"
                << std::endl;

            std::vector<cl::Device> devices;
            pf.getDevices(CL_DEVICE_TYPE_ALL, &devices);

            BOOST_FOREACH(cl::Device &dev, devices) {
                std::cerr << "  " << dev.getInfo<CL_DEVICE_NAME>() << std::endl;

                std::cerr << "    Compute units:  " << std::setw(8) <<
                        dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;

                std::cerr << "    Global memory:  " << std::setw(8) <<
                        (dev.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / MiB) <<
                        " MiB" << std::endl;

                std::vector<cl::Device> thisdev(1);
                thisdev[0] = dev;

                cl::Context context(thisdev);
                cl::Program program = cl::Program(context, clSources);

                try {
                    program.build(thisdev, "-Iopencl");
                    reportBuild(dev, program);

                    testFAST(image, context, dev, program);
                } catch (cl::Error & err) {
                    reportBuild(dev, program);
                    std::cerr << err.what() << " (code " << err.err() << ")" << std::endl;
                }
            }
        }
    } catch (cl::Error & err) {
        std::cerr << err.what() << " (code " << err.err() << ")" << std::endl;
    }
    return 0;
}
