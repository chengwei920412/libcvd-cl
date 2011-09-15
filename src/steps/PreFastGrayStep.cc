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

#include "cvd-cl/steps/PreFastGrayStep.hh"
#include "kernels/prefast-gray.hh"

#include <boost/cstdlib.hpp>

namespace CVD {
namespace CL  {

PreFastGrayStep::PreFastGrayStep(GrayImageState & image, PointListState & points, cl_int threshold) :
    WorkerStep (image.worker),
    image      (image),
    points     (points),
    threshold  (threshold)
{
    char opt[256] = {0,};
    snprintf(opt, sizeof(opt) - 1, "-DFAST_THRESH=%d", int(threshold));
    worker.compile(&program, &kernel, OCL_PRE_FAST_GRAY, "prefast_gray", opt);
}

PreFastGrayStep::~PreFastGrayStep() {
    // Do nothing.
}

void PreFastGrayStep::execute() {
    // Reset number of output points.
    points.setCount(0);

    // Assign kernel parameters.
    kernel.setArg(0, image.image);
    kernel.setArg(1, points.buffer);
    kernel.setArg(2, points.count);

    // Read image dimensions.
    size_t const nx = image.size.x - 16;
    size_t const ny = image.size.y - 16;

    // Construct global, local and offset with a safety boundary.
    // 16x16 appears to give good performance on most devices.
    cl::NDRange const global (nx - 16, ny - 16);
    cl::NDRange const local  (     16,      16);
    cl::NDRange const offset (      8,       8);

    // Queue kernel.
    worker.queue.enqueueNDRangeKernel(kernel, offset, global, local);
}

} // namespace CL
} // namespace CVD
