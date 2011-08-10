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

#include <cvd-cl/steps/BlurRichStep.hh>
#include <cvd-cl/steps/PreFastRichStep.hh>
#include <cvd-cl/steps/FastRichStep.hh>
#include <cvd-cl/steps/FastBestStep.hh>

#include <iomanip>
#include <iostream>

#include <cvd/gl_helpers.h>
#include <cvd/image_io.h>
#include <cvd/videodisplay.h>

// Typedefs for image format.
typedef CVD::Rgba<CVD::byte> Pixel;
typedef CVD::Image<Pixel>    Image;

// Size constants.
static const size_t KiB = 1024;
static const size_t MiB = KiB * KiB;

static void testRich(Image const & image, cl::Device & device) {
    const CVD::ImageRef size = image.size();
    const int           nx   = size.x;
    const int           ny   = size.y;
    const int           nxy  = nx * ny;

    // Create OpenCL worker.
    CVD::CL::Worker          worker      (device);

    // Create states.
    CVD::CL::RichImageState  imageNeat   (worker, size);
    CVD::CL::RichImageState  imageBlur   (worker, size);
    CVD::CL::GrayImageState  scores      (worker, size);
    CVD::CL::PointListState  corners1    (worker, nxy);
    CVD::CL::PointListState  corners2    (worker, nxy);
    CVD::CL::PointListState  corners3    (worker, nxy);

    // Create steps.
    CVD::CL::BlurRichStep    runBlur     (imageNeat, imageBlur);
    CVD::CL::PreFastRichStep runPreFast  (           imageBlur, corners1);
    CVD::CL::FastRichStep    runFast     (           imageBlur, corners1, scores, corners2);
    CVD::CL::FastBestStep    runMaxFast  (                                scores, corners2, corners3);

    // Zero FAST scores.
    scores.zero();

    // Prepare image for writing to device.
    imageNeat.set(image);

    // Write image to device.
    boost::system_time const t1  = boost::get_system_time();
    imageNeat.copyToWorker();
    boost::system_time const t2  = boost::get_system_time();
    int64_t const timeWrite      = (t2 - t1).total_microseconds();

    // Run warmups.
    runBlur.measure();
    runPreFast.measure();
    corners1.getCount();
    runFast.measure();
    corners2.getCount();
    runMaxFast.measure();
    corners3.getCount();

    // Run and time steps.
    int64_t const timeBlur     = runBlur.measure();
    int64_t const timePreFast  = runPreFast.measure();
    cl_int  const nculled      = corners1.getCount();
    int64_t const timeFast     = runFast.measure();
    cl_int  const nfasted      = corners2.getCount();
    int64_t const timeMaxFast  = runMaxFast.measure();
    cl_int  const nfilted      = corners3.getCount();

    std::cerr << std::endl;
    std::cerr << std::setw(8) << timeWrite    << " us writing image" << std::endl;
    std::cerr << std::setw(8) << timeBlur     << " us blurring image" << std::endl;
    std::cerr << std::setw(8) << timePreFast  << " us culling corners" << std::endl;
    std::cerr << std::setw(8) << timeFast     << " us running FAST" << std::endl;
    std::cerr << std::setw(8) << timeMaxFast  << " us filtering corners" << std::endl;
    std::cerr << std::endl;
    std::cerr << std::setw(8) << nxy     << " corner candidates in image" << std::endl;
    std::cerr << std::setw(8) << nculled << " corners after culling" << std::endl;
    std::cerr << std::setw(8) << nfasted << " corners after FAST" << std::endl;
    std::cerr << std::setw(8) << nfilted << " corners after filtering" << std::endl;
    std::cerr << std::endl;

    // Read blurred image.
    Image image2(size);
    imageBlur.get(&image2);

    CVD::ImageRef const size2(nx * 2, ny);
    CVD::VideoDisplay window(size2);
    CVD::glDrawPixels(image);
    CVD::glRasterPos(CVD::ImageRef(nx, 0));
    CVD::glDrawPixels(image2);

    sleep(5);
}

int main(int argc, char **argv) {
    Image const image(CVD::img_load("images/berry1.png"));

    const CVD::ImageRef size = image.size();
    std::cerr << "Image size: " << size.x << " x " << size.y << std::endl;

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
                    testRich(image, dev);
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
