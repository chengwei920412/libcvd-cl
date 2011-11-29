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

#include "cvd-cl/worker/Worker.hh"

#include <iostream>

namespace CVD {
namespace CL  {

Worker::Worker(cl::Device & device) :
    device  (device),
    devices (1, device),
    context (devices),
    queue   (context, device),
    maxLocalSize     (device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>()),
    defaultLocalSize (std::min(size_t(512), maxLocalSize)),
    defaultLocal     (defaultLocalSize)
{
    // Do nothing.
}

Worker::~Worker() {
    // Do nothing.
}

static void reportBuild(cl::Device & device, cl::Program & program) {
    std::string const log(program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
    if (log.empty() == false)
        std::cerr << log << std::endl;
}

void Worker::compile(
    cl::Program * program, cl::Kernel * kernel,
    char const  * source,  char const * name,
    char const  * options
) {

    // Create vector of program sources.
    std::pair<char const *, size_t> const pair    (source, strlen(source));
    cl::Program::Sources            const sources (1, pair);

    // Create program object into given pointer.
    program[0] = cl::Program(context, sources);

    try {
        // Attempt to build program for the worker's device with given options.
        program->build(devices, options);

        // Report on build even if successful.
        reportBuild(device, program[0]);

        // Create kernel object into given pointer.
        kernel[0] = cl::Kernel(program[0], name);
    } catch (cl::Error & err) {
        // Report on build given the failure.
        reportBuild(device, program[0]);

        // Re-throw exception.
        throw err;
    }
}

void Worker::barrier() {
    queue.enqueueBarrier();
}

void Worker::finish() {
    queue.finish();
}

size_t Worker::padGlobalSize(size_t items) {
    // Perform integer arithmetic without assuming power-of-two group size.
    return (((items + defaultLocalSize - 1) / defaultLocalSize) * defaultLocalSize);
}

cl::NDRange Worker::padGlobal(size_t items) {
    return cl::NDRange(padGlobalSize(items));
}

} // namespace CL
} // namespace CVD
