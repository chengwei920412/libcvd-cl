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
static const char   clCode[] = "#include <blur.cl>\n#include <cull.cl>\n#include <fast.cl>\n#include <filt.cl>\n#include <hips.cl>\n#include <find.cl>";
static const size_t clSize   = sizeof(clCode) - 1; // Exclude \0
static const std::pair<const char *, size_t> clPair(clCode, clSize);
static const cl::Program::Sources clSources(1, clPair);

// Size constants.
static const size_t KiB = 1024;
static const size_t MiB = KiB * KiB;

// Timing repetition constant.
static const int REPEAT = 10;

// Cropped image size.
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

    CVD::VideoDisplay window(size);
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

    // typedef CVD::Rgba<CVD::byte> Pixel;
    typedef CVD::byte Pixel;

    const CVD::ImageRef size = image.size();
    const int           nx   = size.x;
    const int           ny   = size.y;
    const int           nxy  = nx * ny;

    // For cropped and RGBA-extended image.
    CVD::Image<Pixel> cropImage(ref1024);

    // For initial scores, then written sparsely.
    CVD::Image<Pixel> zeroImage(ref1024);

    for (int x = 0; x < nx; x++) {
        for (int y = 0; y < ny; y++) {
            cropImage[x][y]/*.red*/ = image[x][y];
            zeroImage[x][y]/*.red*/ = 0;
        }
    }

    // Store for HIPS descriptors.
    // They will be matched against themselves.
    cl_ulong4 hash0 = {{0, 0, 0, 0}};
    std::vector<cl_ulong4> hashes(512, hash0);

    cl::CommandQueue queue (context, device);

    cl::ImageFormat format (CL_INTENSITY, CL_UNSIGNED_INT8);
    cl::Image2D clImage    (context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,  format, nx, ny, 0);
    cl::Image2D clBlur     (context, CL_MEM_READ_WRITE, format, nx, ny, 0);
    cl::Image2D clScores   (context, CL_MEM_READ_WRITE, format, nx, ny, 0);

    cl::Buffer  clCorners  (context, CL_MEM_READ_WRITE, nxy * sizeof(cl_int2  ));
    cl::Buffer  clFiltered (context, CL_MEM_READ_WRITE, nxy * sizeof(cl_int2  ));
    cl::Buffer  clCursor   (context, CL_MEM_READ_WRITE,       sizeof(cl_int   ));
    cl::Buffer  clBins1    (context, CL_MEM_READ_WRITE, nxy * sizeof(cl_ulong4));
    cl::Buffer  clBins2    (context, CL_MEM_READ_WRITE, nxy * sizeof(cl_ulong4));
    cl::Buffer  clMatches  (context, CL_MEM_READ_WRITE, nxy * sizeof(cl_int   ));

    const cl_int cursor0 = 0;
    cl_int nculled = 0;
    cl_int nfasted = 0;
    cl_int nfilted = 0;
    queue.enqueueWriteBuffer(clCursor, CL_TRUE, 0, sizeof(cursor0), &cursor0);

    cl::size_t<3> origin;
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;

    cl::size_t<3> region;
    region[0] = nx;
    region[1] = ny;
    region[2] = 1;

    // Calculate image row pitch in bytes.
    size_t image_row_pitch = nx * sizeof(Pixel);

    // Allocate pinned memory for fast IO.
    void * clPinImage = queue.enqueueMapImage(clImage, CL_TRUE, CL_MAP_WRITE, origin, region, &image_row_pitch, NULL);

    // Copy image data to the pinned memory.
    ::memcpy(clPinImage, cropImage.data(), nxy * sizeof(Pixel));

    std::vector<cl_int> matches(512, 0);

    cl::Kernel clKernelBLUR(program, "blur_gray");
    clKernelBLUR.setArg(0, clImage);
    clKernelBLUR.setArg(1, clBlur);

    cl::Kernel clKernelCULL(program, "cull_gray");
    clKernelCULL.setArg(0, clBlur);
    clKernelCULL.setArg(1, clFiltered);
    clKernelCULL.setArg(2, clCursor);

    cl::Kernel clKernelFAST(program, "fast_gray_9");
    clKernelFAST.setArg(0, clBlur);
    clKernelFAST.setArg(1, clScores);
    clKernelFAST.setArg(2, clFiltered);
    clKernelFAST.setArg(3, clCorners);
    clKernelFAST.setArg(4, clCursor);

    cl::Kernel clKernelFILT(program, "fast_filter");
    clKernelFILT.setArg(0, clScores);
    clKernelFILT.setArg(1, clCorners);
    clKernelFILT.setArg(2, clFiltered);
    clKernelFILT.setArg(3, clCursor);

    cl::Kernel clKernelHIPS(program, "hips_gray");
    clKernelHIPS.setArg(0, clBlur);
    clKernelHIPS.setArg(1, clFiltered);
    clKernelHIPS.setArg(2, clBins1);

    cl::Kernel clKernelFIND(program, "hips_find");
    clKernelFIND.setArg(0, clBins1);
    clKernelFIND.setArg(1, clBins2);
    clKernelFIND.setArg(2, clMatches);

    // Warmup OpenCL runtime.
    for (int i = 0; i < REPEAT; i++) {
        queue.enqueueWriteImage(clImage, CL_TRUE, origin, region, 0, 0, clPinImage);
        queue.enqueueWriteImage(clScores, CL_TRUE, origin, region, 0, 0, (void *) zeroImage.data());

        queue.enqueueNDRangeKernel(clKernelBLUR, cl::NullRange, cl::NDRange(nx, ny), cl::NDRange(16, 16));

        queue.enqueueWriteBuffer(clCursor, CL_FALSE, 0, sizeof(cursor0), &cursor0);

        queue.enqueueNDRangeKernel(clKernelCULL, cl::NullRange, cl::NDRange(nx, ny), cl::NDRange(16, 16));

        queue.enqueueReadBuffer(clCursor, CL_TRUE, 0, sizeof(nculled), &nculled);

        queue.enqueueNDRangeKernel(clKernelFAST, cl::NullRange, cl::NDRange(nculled), cl::NullRange);

        queue.enqueueReadBuffer(clCursor, CL_TRUE, 0, sizeof(nfasted), &nfasted);

        queue.enqueueWriteBuffer(clCursor, CL_FALSE, 0, sizeof(cursor0), &cursor0);
        queue.enqueueNDRangeKernel(clKernelFILT, cl::NullRange, cl::NDRange(nfasted), cl::NullRange);

        queue.enqueueReadBuffer(clCursor, CL_TRUE, 0, sizeof(nfilted), &nfilted);

        queue.enqueueWriteBuffer(clBins1, CL_TRUE, 0, sizeof(cl_ulong4) * nfilted, hashes.data());
        queue.enqueueNDRangeKernel(clKernelHIPS, cl::NullRange, cl::NDRange(1), cl::NullRange);

        queue.enqueueWriteBuffer(clMatches, CL_TRUE, 0, sizeof(cl_int) * 512, matches.data());
        queue.enqueueNDRangeKernel(clKernelFIND, cl::NullRange, cl::NDRange(1, 512), cl::NDRange(1, 512));
    }
    queue.finish();

    queue.enqueueWriteImage(clScores, CL_TRUE, origin, region, 0, 0, (void *) zeroImage.data());

    boost::system_time const t1 = boost::get_system_time();
    for (int i = 0; i < REPEAT; i++) {
        queue.enqueueWriteImage(clImage, CL_TRUE, origin, region, 0, 0, clPinImage);
    }
    queue.finish();

    boost::system_time const t2 = boost::get_system_time();

    for (int i = 0; i < REPEAT; i++) {
        queue.enqueueNDRangeKernel(clKernelBLUR, cl::NullRange, cl::NDRange(nx, ny), cl::NDRange(16, 16));
    }
    queue.finish();

    boost::system_time const t3 = boost::get_system_time();

    for (int i = 0; i < REPEAT; i++) {
        queue.enqueueWriteBuffer(clCursor, CL_FALSE, 0, sizeof(cursor0), &cursor0);
        queue.enqueueNDRangeKernel(clKernelCULL, cl::NullRange, cl::NDRange(nx, ny), cl::NDRange(16, 16));
    }
    queue.finish();

    queue.enqueueReadBuffer(clCursor, CL_TRUE, 0, sizeof(nculled), &nculled);

    boost::system_time const t4 = boost::get_system_time();

    for (int i = 0; i < REPEAT; i++) {
        queue.enqueueWriteBuffer(clCursor, CL_FALSE, 0, sizeof(cursor0), &cursor0);
        queue.enqueueNDRangeKernel(clKernelFAST, cl::NullRange, cl::NDRange(nculled), cl::NullRange);
    }
    queue.finish();

    queue.enqueueReadBuffer(clCursor, CL_TRUE, 0, sizeof(nfasted), &nfasted);

    boost::system_time const t5 = boost::get_system_time();

    for (int i = 0; i < REPEAT; i++) {
        queue.enqueueWriteBuffer(clCursor, CL_FALSE, 0, sizeof(cursor0), &cursor0);
        queue.enqueueNDRangeKernel(clKernelFILT, cl::NullRange, cl::NDRange(nfasted), cl::NullRange);
    }
    queue.finish();

    queue.enqueueReadBuffer(clCursor, CL_TRUE, 0, sizeof(nfilted), &nfilted);

    boost::system_time const t6 = boost::get_system_time();

    std::vector<cl_int2> corners(nfilted);
    queue.enqueueReadBuffer(clFiltered, CL_TRUE, 0, sizeof(cl_int2) * nfilted, corners.data());

    boost::system_time const t7 = boost::get_system_time();

    for (int i = 0; i < REPEAT; i++) {
        queue.enqueueNDRangeKernel(clKernelHIPS, cl::NullRange, cl::NDRange(nfilted), cl::NullRange);
    }
    queue.finish();

    boost::system_time const t8 = boost::get_system_time();

    queue.enqueueCopyBuffer(clBins1, clBins2, 0, 0, nxy * sizeof(cl_ulong4));

    for (int i = 0; i < REPEAT; i++) {
        queue.enqueueNDRangeKernel(clKernelFIND, cl::NullRange, cl::NDRange(nfilted , 512), cl::NDRange(1, 512));
    }
    queue.finish();

    boost::system_time const t9 = boost::get_system_time();


    // Release pinned memory.
    queue.enqueueUnmapMemObject(clImage, clPinImage);

    queue.enqueueReadBuffer(clMatches, CL_TRUE, 0, sizeof(cl_int) * nfilted, matches.data());

    CVD::ImageRef size2(nx * 2, ny);

    std::cerr << std::endl;
    std::cerr << std::setw(8) << (t2 - t1).total_microseconds() / REPEAT << " us writing image" << std::endl;
    std::cerr << std::setw(8) << (t3 - t2).total_microseconds() / REPEAT << " us blurring image" << std::endl;
    std::cerr << std::setw(8) << (t4 - t3).total_microseconds() / REPEAT << " us culling corners" << std::endl;
    std::cerr << std::setw(8) << (t5 - t4).total_microseconds() / REPEAT << " us running FAST" << std::endl;
    std::cerr << std::setw(8) << (t6 - t5).total_microseconds() / REPEAT << " us filtering corners" << std::endl;
    std::cerr << std::setw(8) << (t8 - t7).total_microseconds() / REPEAT << " us making HIPS descriptors" << std::endl;
    std::cerr << std::setw(8) << (t9 - t8).total_microseconds() / REPEAT << " us matching HIPS descriptors" << std::endl;
    std::cerr << std::endl;
    std::cerr << std::setw(8) << nxy     << " corner candidates in image" << std::endl;
    std::cerr << std::setw(8) << nculled << " corners after culling" << std::endl;
    std::cerr << std::setw(8) << nfasted << " corners after FAST" << std::endl;
    std::cerr << std::setw(8) << nfilted << " corners after filtering" << std::endl;
    std::cerr << std::endl;

    CVD::VideoDisplay window(size2);
    CVD::glDrawPixels(image);
    CVD::glRasterPos(CVD::ImageRef(nx, 0));
    CVD::glDrawPixels(image);

    glColor3f(0, 0, 1);
    glBegin(GL_LINES);
    for (int icorner1 = 0; icorner1 < nfilted; icorner1++) {
        int const icorner2 = matches.at(icorner1);

        if (icorner2 >= 0) {
            cl_int2 const xy1 = corners.at(icorner1);
            cl_int2 const xy2 = corners.at(icorner2);

            glVertex2i(xy1.x, xy1.y);
            glVertex2i(xy2.x + nx, xy2.y);
        }
    }
    glEnd();

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
    cropImage.copy_from(fullImage.sub_image(CVD::ImageRef(600, 300), ref1024));

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
