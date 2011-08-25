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

#include <cvd/fast_corner.h>
#include <cvd/gl_helpers.h>
#include <cvd/image_io.h>
#include <cvd/videodisplay.h>

#include <cvd-cl/steps/BlurGrayStep.hh>
#include <cvd-cl/steps/PreFastGrayStep.hh>
#include <cvd-cl/steps/FastGrayStep.hh>
#include <cvd-cl/steps/FastBestStep.hh>
#include <cvd-cl/steps/HipsGrayStep.hh>
#include <cvd-cl/steps/HipsFindStep.hh>

#include <boost/foreach.hpp>

// Size constants.
static const size_t KiB = 1024;
static const size_t MiB = KiB * KiB;

// Cropped image size.
static const CVD::ImageRef ref1024(1024, 1024);

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
    for (size_t i = 0; i < corners.size(); i++) {
        CVD::ImageRef const & xy = corners.at(i);
        glVertex2i(xy.x, xy.y);
    }
    glEnd();
    glFlush();

    sleep(5);
}

static void testFAST(CVD::Image<CVD::byte> const & image, cl::Device & device) {
    const CVD::ImageRef size = image.size();
    const int           nx   = size.x;
    const int           ny   = size.y;
    const int           nxy  = nx * ny;

    // Create OpenCL worker.
    CVD::CL::Worker          worker      (device);

    // Create states.
    CVD::CL::GrayImageState  imageNeat   (worker, ref1024);
    CVD::CL::GrayImageState  imageBlur   (worker, ref1024);
    CVD::CL::GrayImageState  scores      (worker, ref1024);
    CVD::CL::PointListState  corners1    (worker, nxy);
    CVD::CL::PointListState  corners2    (worker, nxy);
    CVD::CL::PointListState  corners3    (worker, nxy);
    CVD::CL::HipsListState   hips        (worker, nxy);
    CVD::CL::PointListState  cornersM    (worker, nxy);

    // Create steps.
    CVD::CL::BlurGrayStep    runBlur     (imageNeat, imageBlur);
    CVD::CL::PreFastGrayStep runPreFast  (           imageBlur, corners1);
    CVD::CL::FastGrayStep    runFast     (           imageBlur, corners1, scores, corners2);
    CVD::CL::FastBestStep    runMaxFast  (                                scores, corners2, corners3);
    CVD::CL::HipsGrayStep    runHips     (           imageBlur,                             corners3, hips);
    CVD::CL::HipsFindStep    runMatch    (                                                            hips, hips, corners3, cornersM);

    // Write image to device.
    int64_t const timeWrite = imageNeat.measure(image);

    // Zero FAST scores.
    scores.zero();
    corners1.zero();
    corners2.zero();
    corners3.zero();
    hips.zero();
    cornersM.zero();

    // Run warmups.
    runBlur.measure();
    runPreFast.measure();
    corners1.getCount();
    runFast.measure();
    corners2.getCount();
    runMaxFast.measure();
    corners3.getCount();
    runHips.measure();
    runMatch.measure();

    // Run and time steps.
    int64_t const timeBlur     = runBlur.measure();
    int64_t const timePreFast  = runPreFast.measure();
    cl_int  const nculled      = corners1.getCount();
    int64_t const timeFast     = runFast.measure();
    cl_int  const nfasted      = corners2.getCount();
    int64_t const timeMaxFast  = runMaxFast.measure();
    cl_int  const nfilted      = corners3.getCount();
    int64_t const timeHips     = runHips.measure();
    int64_t const timeMatch    = runMatch.measure();

    // Time a single burst.
    boost::system_time const t3 = boost::get_system_time();
    imageNeat.set(image);
    runBlur.execute();
    runPreFast.execute();
    runFast.execute();
    runMaxFast.execute();
    runHips.execute();
    runMatch.execute();
    worker.finish();
    boost::system_time const t4 = boost::get_system_time();
    int64_t const timeBurst     = (t4 - t3).total_microseconds();

    // Read out final corner list.
    std::vector<cl_int2> corners;
    corners3.get(&corners);

    // Read out matching corner table.
    std::vector<cl_int2> matches;
    cornersM.get(&matches);
    matches.resize(128);

    CVD::ImageRef size2(nx * 2, ny);

    std::cerr << std::endl;
    std::cerr << std::setw(8) << timeWrite    << " us writing image" << std::endl;
    std::cerr << std::setw(8) << timeBlur     << " us blurring image" << std::endl;
    std::cerr << std::setw(8) << timePreFast  << " us culling corners" << std::endl;
    std::cerr << std::setw(8) << timeFast     << " us running FAST" << std::endl;
    std::cerr << std::setw(8) << timeMaxFast  << " us filtering corners" << std::endl;
    std::cerr << std::setw(8) << timeHips     << " us making HIPS descriptors" << std::endl;
    std::cerr << std::setw(8) << timeMatch    << " us matching HIPS descriptors" << std::endl;
    std::cerr << std::setw(8) << timeBurst    << " us in a single burst" << std::endl;
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
    for (size_t icorner1 = 0; (icorner1 < matches.size()) && (icorner1 < corners.size()); icorner1++) {
        try {
            cl_int2 const xy1 = corners.at(icorner1);
            cl_int2 const xy2 = matches.at(icorner1);

            glVertex2i(xy1.x, xy1.y);
            glVertex2i(xy2.x + nx, xy2.y);
        } catch (...) {
            std::cerr << "Bad corner " << icorner1 << " of " << matches.size() << std::endl;
        }
    }
    glEnd();

    glColor3f(1, 0, 0);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < corners.size(); i++) {
        cl_int2 const & xy = corners.at(i);
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

        for (size_t ip = 0; ip < platforms.size(); ip++) {
            cl::Platform &pf = platforms.at(ip);

            std::cerr
                << pf.getInfo<CL_PLATFORM_NAME   >() << " ("
                << pf.getInfo<CL_PLATFORM_VENDOR >() << ", "
                << pf.getInfo<CL_PLATFORM_VERSION>() << ")"
                << std::endl;

            std::vector<cl::Device> devices;
            pf.getDevices(CL_DEVICE_TYPE_ALL, &devices);

            for (size_t id = 0; id < devices.size(); id++) {
                cl::Device &dev = devices.at(id);

                std::cerr << "  " << dev.getInfo<CL_DEVICE_NAME>() << std::endl;

                std::cerr << "    Compute units:  " << std::setw(8) <<
                        dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;

                std::cerr << "    Global memory:  " << std::setw(8) <<
                        (dev.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / MiB) <<
                        " MiB" << std::endl;

                try {
                    testFAST(cropImage, dev);
                } catch (cl::Error & err) {
                    std::cerr << err.what() << " (code " << err.err() << ")" << std::endl;
                }
            }
        }
    } catch (cl::Error & err) {
        std::cerr << err.what() << " (code " << err.err() << ")" << std::endl;
    }
    return 0;
}
