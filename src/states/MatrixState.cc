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

#include "cvd-cl/states/MatrixState.hh"
#include "cvd-cl/core/Expect.hh"

namespace CVD {
namespace CL  {

MatrixState::MatrixState(Worker & worker, size_t count, size_t rows, size_t cols) :
    WorkerState (worker),
    count       (count),
    rows        (rows),
    cols        (cols),
    floats      (count * rows * cols),
    bytes       (floats * sizeof(cl_float))
{
    expect("MatrixState::MatrixState() must have positive size",
        bytes > 0);

    // Allocate buffer (may throw a CL exception).
    memory = cl::Buffer(worker.context, CL_MEM_READ_WRITE, bytes);
}

MatrixState::~MatrixState() {
    // Do nothing.
}

void MatrixState::setFloats(std::vector<cl_float> const & items) {
    expect("MatrixState::setFloats() must have exact size",
        floats == items.size());

    worker.queue.enqueueWriteBuffer(memory, CL_TRUE, 0, bytes, items.data());
}

void MatrixState::getFloats(std::vector<cl_float>       * items) {
    expect("MatrixState::getFloats() must have exact size",
        floats == items->size());

    worker.queue.enqueueReadBuffer(memory, CL_TRUE, 0, bytes, items->data());
}

void MatrixState::copyFrom(MatrixState & that) {
    worker.queue.finish();
    worker.queue.enqueueCopyBuffer(that.memory, memory, 0, 0, bytes);
    worker.queue.finish();
}

void MatrixState::copyFromViaHost(MatrixState & that) {
    // Create host-side buffer for float data.
    std::vector<cl_float> items(floats);

    // Read items from other buffer.
    that.getFloats(&items);

    // Write items to this buffer.
    setFloats(items);
}

} // namespace CL
} // namespace CVD
