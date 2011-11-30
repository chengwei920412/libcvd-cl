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

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <boost/cstdint.hpp>
#include <boost/date_time.hpp>
#include <boost/random.hpp>
#include <boost/thread/thread_time.hpp>

#include <TooN/se3.h>
#include <TooN/so3.h>

#include <cvd-cl/worker/Worker.hh>
#include "kernels/se3-exp.hh"

// Size constants.
static const size_t KiB = 1024;
static const size_t MiB = KiB * KiB;

// Timing repetition constant.
int const static REPEAT = 1000;

// Number of matrices and vectors to process.
int const static COUNT = 8192;

typedef TooN::Vector<6,    cl_float> vec_t;
typedef TooN::Vector<3,    cl_float> tra_t;
typedef TooN::Matrix<3, 3, cl_float> mat_t;

typedef std::vector<vec_t> vecs_t;
typedef std::vector<tra_t> tras_t;
typedef std::vector<mat_t> mats_t;

typedef boost::mt19937 rng_t;
typedef boost::uniform_real<cl_float> dist_t;

static void reportBuild(cl::Device &device, cl::Program &program) {
    std::string log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    if (log.empty() == false)
        std::cerr << log << std::endl;
}

static void randomVector(rng_t &rng, dist_t & dist, vec_t &v) {
    for (int i = 0; i < 6; i++)
        v[i] = dist(rng);
}

static void testToonExp(mats_t & mats, tras_t & tras, vecs_t const & vecs) {
    boost::system_time const t1 = boost::get_system_time();

    for (int repeat = 0; repeat < REPEAT; repeat++) {
        for (int i = 0; i < COUNT; i++) {
            TooN::SE3<cl_float> se3 = TooN::SE3<cl_float>::exp(vecs.at(i));
            se3.get_rotation().coerce();
            mats.at(i) = se3.get_rotation().get_matrix();
            tras.at(i) = se3.get_translation();
        }
    }

    boost::system_time const t2 = boost::get_system_time();
    int const td = ((t2 - t1).total_microseconds() / REPEAT);

    std::cerr << std::setw(8) << td << " us running TooN SE3::exp" << std::endl;
}

static void testClExp(cl::Device & device, mats_t const & imats, tras_t const & itras, vecs_t const & vecs) {
    int const nmcells = 4 * 4 * COUNT;
    int const nvcells = 6     * COUNT;
    int const smcells = nmcells * sizeof(cl_float);
    int const svcells = nvcells * sizeof(cl_float);

    CVD::CL::Worker worker(device);

    cl::Context & context = worker.context;

    cl::Buffer clM (context, CL_MEM_WRITE_ONLY, smcells);
    cl::Buffer clV (context, CL_MEM_READ_ONLY,  svcells);

    cl::Program program;
    cl::Kernel  kernel;

    worker.compile(&program, &kernel, CVD::CL::OCL_SE3_EXP, "se3_exp");

    kernel.setArg(0, clV);
    kernel.setArg(1, clM);

    cl::CommandQueue queue (context, device);

    std::vector<cl_float> mbuf(nmcells);
    std::vector<cl_float> vbuf(nvcells);

    for (int ivector = 0; ivector < COUNT; ivector++) {
        vec_t const & vector = vecs.at(ivector);

        for (int col = 0, off = 0; col < 6; col++, off += COUNT)
            vbuf.at(off + ivector) = vector[col];
    }

    queue.enqueueWriteBuffer(clV, CL_TRUE, 0, svcells, vbuf.data());

    boost::system_time const t1 = boost::get_system_time();

    for (int repeat = 0; repeat < REPEAT; repeat++) {
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(COUNT), cl::NullRange);
    }
    queue.finish();

    boost::system_time const t2 = boost::get_system_time();
    int const td = ((t2 - t1).total_microseconds() / REPEAT);

    std::cerr << std::setw(8) << td << " us running CL se3_exp" << std::endl;

    queue.enqueueReadBuffer(clM, CL_TRUE, 0, smcells, mbuf.data());

    mats_t mats(COUNT);
    tras_t tras(COUNT);

    for (int imatrix = 0; imatrix < COUNT; imatrix++) {
        mat_t & matrix = mats.at(imatrix);
        tra_t & tra    = tras.at(imatrix);

        for (int row = 0, off = 0; row < 3; row++) {
            for (int col = 0; col < 4; col++, off += COUNT) {
                cl_float const value = mbuf.at(off + imatrix);

                if (col < 3) {
                    matrix(row, col) = value;
                } else {
                    tra[row] = value;
                }
            }
        }
    }

    for (int i = 0; i < COUNT; i++) {
        double sum1  = 0;
        double sum2  = 0;
        double error = 0;

        mat_t const & m1 = imats.at(i);
        mat_t const & m2 =  mats.at(i);
        tra_t const & t1 = itras.at(i);
        tra_t const & t2 =  tras.at(i);

        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                double const v1 = m1(row, col);
                double const v2 = m2(row, col);

                sum1  += v1;
                sum2  += v2;
                error += std::abs(v1 - v2);
            }

            double const v1 = t1[row];
            double const v2 = t2[row];

            sum1  += v1;
            sum2  += v2;
            error += std::abs(v1 - v2);
        }

        if (error > 1e-6) {
            std::cerr << "Index " << i << " has sum1 " << sum1 << ", sum2 " << sum2 << ", absolute error " << error << std::endl;
        }
    }
}

int main(int argc, char **argv) {
    rng_t rng;
    dist_t dist(-0.5f, 0.5f);

    vecs_t vecs(COUNT);
    tras_t tras(COUNT);
    mats_t mats(COUNT);

    for (int i = 0; i < COUNT; i++)
        randomVector(rng, dist, vecs.at(i));

    testToonExp(mats, tras, vecs);

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
                    testClExp(dev, mats, tras, vecs);
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
