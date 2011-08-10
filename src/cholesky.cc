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
#include <boost/random.hpp>
#include <boost/thread/thread_time.hpp>

// Include official OpenCL C++ wrapper, with exceptions enabled.
#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

#include <TooN/Cholesky.h>

// Wrapper OpenCL source code.
static const char   clCode[] = "#include <cholesky5.cl>";
static const size_t clSize   = sizeof(clCode) - 1; // Exclude \0
static const std::pair<const char *, size_t> clPair(clCode, clSize);
static const cl::Program::Sources clSources(1, clPair);

// Size constants.
static const size_t KiB = 1024;
static const size_t MiB = KiB * KiB;

// Timing repetition constant.
int const static REPEAT = 1000;

// Number of matrices and vectors to process.
int const static COUNT = 8192;

int const static SIZE  = 5;
int const static SIZE2 = SIZE * SIZE;

typedef TooN::Vector<SIZE,       cl_float> vec_t;
typedef TooN::Matrix<SIZE, SIZE, cl_float> mat_t;

typedef std::vector<vec_t> vecs_t;
typedef std::vector<mat_t> mats_t;

typedef boost::mt19937 rng_t;
typedef boost::uniform_real<cl_float> dist_t;

static void reportBuild(cl::Device &device, cl::Program &program) {
    std::string log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
    if (log.empty() == false)
        std::cerr << log << std::endl;
}

static void randomVector(rng_t &rng, dist_t & dist, vec_t &v) {
    for (int i = 0; i < SIZE; i++)
        v[i] = dist(rng);
}

static void randomMatrix(rng_t &rng, dist_t & dist, mat_t &m) {
    m = (TooN::Identity * 0.0001f);

    for (int i = 0; i < COUNT; i++) {
        vec_t vec;
        randomVector(rng, dist, vec);
        m += (vec.as_col() * vec.as_row());
    }
}

static void testToonCholesky(mats_t const & mats, vecs_t const & vecs) {
    std::vector<double> total(SIZE, 0.0);

    boost::system_time const t1 = boost::get_system_time();

    for (int repeat = 0; repeat < REPEAT; repeat++) {
        for (int i = 0; i < COUNT; i++) {
            TooN::Cholesky<5, cl_float> chol;
            chol.compute(mats.at(i));
            vec_t x = chol.backsub(vecs.at(i));

            for (int j = 0; j < SIZE; j++)
                total[j] += x[j];
        }
    }

    boost::system_time const t2 = boost::get_system_time();
    int const td = ((t2 - t1).total_microseconds() / REPEAT);

    std::cerr << std::setw(8) << td << " us running TooN Cholesky" << std::endl;

    for (int i = 0; i < SIZE; i++) {
        std::cerr << "total[" << i << "] = " << total[i] << std::endl;
    }
}

static void testClCholesky(
        cl::Context & context, cl::Device & device, cl::Program & program,
        mats_t const & mats, vecs_t const & vecs
) {

    int const nmcells = SIZE2 * COUNT;
    int const nvcells = SIZE  * COUNT;
    int const smcells = nmcells * sizeof(cl_float);
    int const svcells = nvcells * sizeof(cl_float);

    cl::Buffer clA (context, CL_MEM_READ_ONLY,  smcells);
    cl::Buffer clB (context, CL_MEM_READ_ONLY,  svcells);
    cl::Buffer clX (context, CL_MEM_WRITE_ONLY, svcells);

    cl::Kernel clKernel(program, "cholesky5");
    clKernel.setArg(0, clA);
    clKernel.setArg(1, clB);
    clKernel.setArg(2, clX);

    cl::CommandQueue queue (context, device);

    std::vector<cl_float> mbuf(nmcells);

    for (int imatrix = 0; imatrix < COUNT; imatrix++) {
        mat_t const & matrix = mats.at(imatrix);

        for (int col = 0, off = 0; col < SIZE; col++) {
            for (int row = 0; row < SIZE; row++, off += COUNT)
                mbuf.at(off + imatrix) = matrix(col, row);
        }
    }

    std::vector<cl_float> vbuf(nvcells);

    for (int ivector = 0; ivector < COUNT; ivector++) {
        vec_t const & vector = vecs.at(ivector);

        for (int col = 0, off = 0; col < SIZE; col++, off += COUNT)
            vbuf.at(off + ivector) = vector[col];
    }

    queue.enqueueWriteBuffer (clA, CL_TRUE, 0, smcells, mbuf.data());
    queue.enqueueWriteBuffer (clB, CL_TRUE, 0, svcells, vbuf.data());

    boost::system_time const t1 = boost::get_system_time();

    for (int repeat = 0; repeat < REPEAT; repeat++) {
        queue.enqueueNDRangeKernel(clKernel, cl::NullRange, cl::NDRange(COUNT), cl::NullRange);
    }
    queue.finish();

    boost::system_time const t2 = boost::get_system_time();
    int const td = ((t2 - t1).total_microseconds() / REPEAT);

    std::cerr << std::setw(8) << td << " us running CL Cholesky" << std::endl;

    queue.enqueueReadBuffer  (clX, CL_TRUE, 0, svcells, vbuf.data());

    vecs_t ovecs(COUNT);

    for (int ivector = 0; ivector < COUNT; ivector++) {
        vec_t & vector = ovecs.at(ivector);

        for (int col = 0, off = 0; col < SIZE; col++, off += COUNT)
            vector[col] = vbuf.at(off + ivector);
    }

    std::vector<double> total(SIZE, 0.0);

    for (int i = 0; i < COUNT; i++) {
        vec_t & v = ovecs.at(i);

        for (int j = 0; j < SIZE; j++)
            total[j] += (double(v[j]) * REPEAT);

        TooN::Cholesky<5, cl_float> chol;
        chol.compute(mats.at(i));
        vec_t x = chol.backsub(vecs.at(i));

        float sum1 = 0;
        float sum2 = 0;

        for (int j = 0; j < SIZE; j++) {
            sum1 += x[j];
            sum2 += v[j];
        }

        if (std::abs(sum1 - sum2) >= 0.00001f) {
            std::cerr << std::setw(4) << " has sums " << sum1 << "   " << sum2 << std::endl;
        }
    }

    for (int i = 0; i < SIZE; i++) {
        std::cerr << "total[" << i << "] = " << total[i] << std::endl;
    }
}

int main(int argc, char **argv) {
    rng_t rng;
    dist_t dist(-3.0f, 3.0f);

    mats_t mats(COUNT);
    vecs_t vecs(COUNT);

    for (int i = 0; i < COUNT; i++) {
        randomMatrix(rng, dist, mats.at(i));
        randomVector(rng, dist, vecs.at(i));
    }

    testToonCholesky(mats, vecs);

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
                cl::Program program(context, clSources);

                try {
                    program.build(thisdev, "-Iopencl");
                    reportBuild(dev, program);

                    testClCholesky(context, dev, program, mats, vecs);
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
