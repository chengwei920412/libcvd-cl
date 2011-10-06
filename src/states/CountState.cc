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

#include "cvd-cl/states/CountState.hh"
#include "cvd-cl/core/Expect.hh"

namespace CVD {
namespace CL  {

CountState::CountState(Worker & worker, cl_uint size) :
    WorkerState (worker),
    size        (size)
{
    expect("CountState::CountState() must have positive size",
        size > 0);

    // Allocate buffer (may throw a CL exception).
    count = cl::Buffer(worker.context, CL_MEM_READ_WRITE, sizeof(cl_uint));

    // Reset counter.
    setCount(0);
}

CountState::~CountState() {
    // Do nothing.
}

void CountState::setCount(cl_uint const ncount) {
    expect("CountState::setCount() must fit within size", (ncount <= size));

    // Write from parameter to device buffer.
    worker.queue.enqueueWriteBuffer(count, CL_TRUE, 0, sizeof(cl_uint), &ncount);
}

cl_uint CountState::getCount() {
    cl_uint ncount = 0;

    // Read from device buffer into variable.
    worker.queue.enqueueReadBuffer(count, CL_TRUE, 0, sizeof(cl_uint), &ncount);

    // Crop against maximum size.
    return std::min(ncount, size);
}

} // namespace CL
} // namespace CVD
