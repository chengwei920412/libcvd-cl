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

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <boost/cstdint.hpp>
#include <boost/date_time.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/thread_time.hpp>

// Include official OpenCL C++ wrapper, with exceptions enabled.
#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

#include <cvd/fast_corner.h>
#include <cvd/gl_helpers.h>
#include <cvd/image_io.h>
#include <cvd/videodisplay.h>

// Wrapper OpenCL source code.
static const char   clCode[] = "#include <fast.cl>\n#include <filt.cl>\n#include <hips.cl>";
static const size_t clSize   = sizeof(clCode) - 1; // Exclude \0
static const std::pair<const char *, size_t> clPair(clCode, clSize);
static const cl::Program::Sources clSources(1, clPair);

// Size constants.
static const size_t KiB = 1024;
static const size_t MiB = KiB * KiB;

static const CVD::ImageRef ref0(0, 0);
static const CVD::ImageRef ref1024(1024, 1024);

static void reportBuild(cl::Device &device, cl::Program &program) {
    std::string log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    if (log.empty() == false)
        std::cerr << log << std::endl;
}

static void testCVDFAST(CVD::BasicImage<CVD::byte> const & image) {
    const CVD::ImageRef size = image.size();
    std::vector<CVD::ImageRef> corners;

    boost::system_time const t1 = boost::get_system_time();
    CVD::fast_corner_detect_9(image, corners, 60);
    boost::system_time const t2 = boost::get_system_time();

    std::cerr << "CVD FAST9 runtime:  " << std::setw(8) << (t2 - t1).total_microseconds() << std::endl;
    std::cerr << "CVD FAST9 corners:  " << std::setw(8) << corners.size() << std::endl << std::endl;

    CVD::VideoDisplay window(size);           //Create an OpenGL window with the dimensions of `in'
    CVD::glDrawPixels(image);

    glColor3f(1, 0, 0);
    glBegin(GL_POINTS);
    BOOST_FOREACH(CVD::ImageRef const & xy, corners) {
        glVertex2i(xy.x, xy.y);
    }
    glEnd();
    glFlush();

    sleep(5);
}

static void testFAST(CVD::Image<CVD::byte> const & image,
        cl::Context &context, cl::Device &device, cl::Program &program) {

    const CVD::ImageRef size = image.size();

    CVD::Image<CVD::Rgba<CVD::byte> > cropImage(ref1024);
    for (int x = 0; x < size.x; x++) {
        for (int y = 0; y < size.y; y++)
            cropImage[x][y].red = image[x][y];
    }

    cl::CommandQueue queue(context, device);

    cl::ImageFormat format(CL_RGBA, CL_UNSIGNED_INT8);
    cl::Image2D clImage(context, CL_MEM_READ_ONLY, format, 1024, 1024, 0);
    cl::Image2D clScores(context, CL_MEM_WRITE_ONLY, format, 1024, 1024, 0);

    cl::Buffer clCorners(context, CL_MEM_WRITE_ONLY, 1024 * 1024 * sizeof(cl_int2));
    cl::Buffer clFiltered(context, CL_MEM_WRITE_ONLY, 1024 * 1024 * sizeof(cl_int2));
    cl::Buffer clCursor(context, CL_MEM_READ_WRITE, sizeof(cl_int));
    cl::Buffer clBins(context, CL_MEM_READ_WRITE, 1024 * 1024 * sizeof(cl_ulong4));

    const cl_int cursor0 = 0;
    cl_int cursorN = 0;
    cl_int filterN = 0;
    queue.enqueueWriteBuffer(clCursor, CL_TRUE, 0, sizeof(cursor0), &cursor0);

    cl::size_t<3> origin;
    origin[0] =    0;
    origin[1] =    0;
    origin[2] =    0;

    cl::size_t<3> region;
    region[0] = 1024;
    region[1] = 1024;
    region[2] =    1;

    cl::Kernel clKernelFAST(program, "fast_gray_9");
    clKernelFAST.setArg(0, clImage);
    clKernelFAST.setArg(1, clScores);
    clKernelFAST.setArg(2, clCorners);
    clKernelFAST.setArg(3, clCursor);

    cl::Kernel clKernelFILT(program, "fast_filter");
    clKernelFILT.setArg(0, clScores);
    clKernelFILT.setArg(1, clCorners);
    clKernelFILT.setArg(2, clFiltered);
    clKernelFILT.setArg(3, clCursor);

    cl::Kernel clKernelHIPS(program, "hips_gray");
    clKernelHIPS.setArg(0, clImage);
    clKernelHIPS.setArg(1, clCorners);
    clKernelHIPS.setArg(2, clBins);

    // Warmup OpenCL runtime.
    for (int i = 0; i < 100; i++) {
        queue.enqueueWriteImage(clImage, CL_TRUE, origin, region, 0, 0, (void *) cropImage.data());

        queue.enqueueWriteBuffer(clCursor, CL_FALSE, 0, sizeof(cursor0), &cursor0);
        queue.enqueueNDRangeKernel(clKernelFAST, cl::NullRange, cl::NDRange(1024, 1024), cl::NDRange(16, 16));

        queue.enqueueReadBuffer(clCursor, CL_TRUE, 0, sizeof(cursorN), &cursorN);

        queue.enqueueWriteBuffer(clCursor, CL_FALSE, 0, sizeof(cursor0), &cursor0);
        queue.enqueueNDRangeKernel(clKernelFILT, cl::NullRange, cl::NDRange(cursorN), cl::NullRange);

        queue.enqueueReadBuffer(clCursor, CL_TRUE, 0, sizeof(cursorN), &cursorN);

        queue.enqueueNDRangeKernel(clKernelHIPS, cl::NullRange, cl::NDRange(cursorN), cl::NullRange);
    }
    queue.finish();

    boost::system_time const t1 = boost::get_system_time();
    queue.enqueueWriteImage(clImage, CL_TRUE, origin, region, 0, 0, (void *) cropImage.data());
    boost::system_time const t2 = boost::get_system_time();
    std::cerr << "    Image runtime:  " << std::setw(8) << (t2 - t1).total_microseconds() << std::endl;

    boost::system_time const t3 = boost::get_system_time();
    for (int i = 0; i < 10; i++) {
        queue.enqueueWriteBuffer(clCursor, CL_FALSE, 0, sizeof(cursor0), &cursor0);
        queue.enqueueNDRangeKernel(clKernelFAST, cl::NullRange, cl::NDRange(1024, 1024), cl::NDRange(16, 16));
    }
    queue.finish();
    boost::system_time const t4 = boost::get_system_time();

    queue.enqueueReadBuffer(clCursor, CL_TRUE, 0, sizeof(cursorN), &cursorN);
    std::cerr << "    FAST9 runtime:  " << std::setw(8) << (t4 - t3).total_microseconds() / 10 << std::endl;
    std::cerr << "    FAST9 corners:  " << std::setw(8) << cursorN << std::endl;

    boost::system_time const t5 = boost::get_system_time();
    for (int i = 0; i < 10; i++) {
        queue.enqueueWriteBuffer(clCursor, CL_FALSE, 0, sizeof(cursor0), &cursor0);
        queue.enqueueNDRangeKernel(clKernelFILT, cl::NullRange, cl::NDRange(cursorN), cl::NullRange);
    }
    queue.finish();
    boost::system_time const t6 = boost::get_system_time();

    queue.enqueueReadBuffer(clCursor, CL_TRUE, 0, sizeof(filterN), &filterN);
    std::cerr << "    FILT9 runtime:  " << std::setw(8) << (t6 - t5).total_microseconds() / 10 << std::endl;
    std::cerr << "    FILT9 corners:  " << std::setw(8) << filterN << std::endl;

    std::vector<cl_int2> corners(filterN);
    queue.enqueueReadBuffer(clFiltered, CL_TRUE, 0, sizeof(cl_int2) * filterN, corners.data());

    boost::system_time const t7 = boost::get_system_time();
    for (int i = 0; i < 10; i++) {
        queue.enqueueNDRangeKernel(clKernelHIPS, cl::NullRange, cl::NDRange(filterN), cl::NullRange);
    }
    queue.finish();
    boost::system_time const t8 = boost::get_system_time();
    std::cerr << "    HIPS  runtime:  " << std::setw(8) << (t8 - t7).total_microseconds() / 10 << std::endl;

    CVD::VideoDisplay window(size);           //Create an OpenGL window with the dimensions of `in'
    CVD::glDrawPixels(image);

    glColor3f(1, 0, 0);
    glBegin(GL_POINTS);
    BOOST_FOREACH(cl_int2 const & xy, corners) {
        glVertex2i(xy.x, xy.y);
    }
    glEnd();
    glFlush();

    sleep(5);
}

int main(int argc, char **argv) {
    CVD::Image<CVD::byte> fullImage;

    fullImage = CVD::img_load("/usr/share/backgrounds/Grey_day_by_Drew__.jpg");

    const CVD::ImageRef size = fullImage.size();

    std::cerr << "Image size: " << size.x << " x " << size.y << std::endl;

    CVD::Image<CVD::byte> cropImage(ref1024);
    cropImage.copy_from(fullImage.sub_image(ref0, ref1024));

    testCVDFAST(cropImage);

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

                    testFAST(cropImage, context, dev, program);
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
