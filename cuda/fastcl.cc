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

#include "common.h"

#include <cvd-cl/steps/PreFastGrayStep.hh>
#include <cvd-cl/steps/FastGrayStep.hh>

static void clfast(cl::Device & device, CVD::Image<CVD::byte> const & image) {
    // Re-interpret image pointer.
    cl_uchar const * const data = reinterpret_cast<cl_uchar const *>(image.data());
    int const nx = image.size().x;
    int const ny = image.size().y;

    CVD::CL::Worker worker(device);

    // Create states.
    CVD::CL::GrayImageState cl_image(worker, ny, nx, CL_MEM_READ_ONLY);
    CVD::CL::PointListState cl_corners1(worker, FAST_COUNT);
    CVD::CL::PointListState cl_corners2(worker, FAST_COUNT);

    // Create steps.
    CVD::CL::PreFastGrayStep fast1(cl_image, cl_corners1, FAST_THRESH);
    CVD::CL::FastGrayStep    fast2(cl_image, cl_corners1, cl_corners2, FAST_THRESH, FAST_RING);

    // Write image data.
    cl_image.set(data);

    // Run kernels for time.

    long const time1 = time(NULL);

    for (int i = 0; i < REPEAT; i++)
        fast1.execute();

    long const time2 = time(NULL);

    for (int i = 0; i < REPEAT; i++)
        fast2.execute();

    long const time3 = time(NULL);

    // Read number of corners.
    int const ncorners1 = cl_corners1.getCount();
    int const ncorners2 = cl_corners2.getCount();

    // Calculate microseconds per kernel.
    int const us1 = (((time2 - time1) * 1000000) / REPEAT);
    int const us2 = (((time3 - time2) * 1000000) / REPEAT);

    // Report timing and number of corners.
    std::cerr << "OpenCL" << std::endl;
    std::cerr << std::setw(12) << ncorners1 << " corners 1" << std::endl;
    std::cerr << std::setw(12) << ncorners2 << " corners 2" << std::endl;
    std::cerr << std::setw(12) << us1 << " microseconds 1" << std::endl;
    std::cerr << std::setw(12) << us2 << " microseconds 2" << std::endl;
    std::cerr << std::endl;
}

void clfast(CVD::Image<CVD::byte> const & image) {
    try {
        // Prepare list for all OpenCL devices on all platforms.
        std::vector<cl::Device> devices;

        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        std::cerr << "Found " << platforms.size() << " OpenCL platforms" << std::endl;

        for (size_t ip = 0; ip < platforms.size(); ip++) {
            cl::Platform &pf = platforms.at(ip);

            std::cerr
                << pf.getInfo<CL_PLATFORM_NAME   >() << " ("
                << pf.getInfo<CL_PLATFORM_VENDOR >() << ", "
                << pf.getInfo<CL_PLATFORM_VERSION>() << ")"
                << std::endl;

            std::vector<cl::Device> newDevices;

            try {
                pf.getDevices(CL_DEVICE_TYPE_ALL, &newDevices);
                std::cerr << "  Found " << newDevices.size() << " OpenCL devices" << std::endl;
            } catch (cl::Error & err) {
                std::cerr << err.what() << " (code " << err.err() << ")" << std::endl;
            }

            for (size_t id = 0; id < newDevices.size(); id++) {
                cl::Device &dev = newDevices.at(id);

                std::cerr << "    " << dev.getInfo<CL_DEVICE_NAME>() << std::endl;

                // Add to list of all devices.
                devices.push_back(dev);
            }
        }

        std::cerr << std::endl << std::endl;

        for (size_t id = 0; id < devices.size(); id++) {
            cl::Device &dev = devices.at(id);

            std::cerr << "Running pipeline for \"" << dev.getInfo<CL_DEVICE_NAME>() << "\"" << std::endl;

            try {
                clfast(dev, image);
            } catch (cl::Error & err) {
                std::cerr << err.what() << " (code " << err.err() << ")" << std::endl;
            }
        }
    } catch (cl::Error & err) {
        std::cerr << err.what() << " (code " << err.err() << ")" << std::endl;
    }
}
