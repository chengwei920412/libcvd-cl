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

#ifndef __CVD_CL_MATRIX_STATE_HH__
#define __CVD_CL_MATRIX_STATE_HH__

#include <cvd-cl/worker/WorkerState.hh>
#include <cvd-cl/core/Expect.hh>

#include <boost/static_assert.hpp>

namespace CVD {
namespace CL  {

template<size_t rows, size_t cols>
class MatrixState : public WorkerState {
private:

    BOOST_STATIC_ASSERT(rows > 0);
    BOOST_STATIC_ASSERT(cols > 0);
    BOOST_STATIC_ASSERT(rows < 0x10000);
    BOOST_STATIC_ASSERT(rows < 0x10000);

public:

    size_t static const elms = (rows * cols);

    explicit MatrixState(Worker & worker, size_t count)  :
        WorkerState (worker),
        count       (count),
        floats      (count * elms),
        bytes       (floats * sizeof(cl_float))
    {
        // Allocate buffer (may throw a CL exception).
        memory = cl::Buffer(worker.context, CL_MEM_READ_WRITE, bytes);
    }

    virtual ~MatrixState() {
        // Do nothing.
    }

    void setFloats(std::vector<cl_float> const & items) {
        expect("MatrixState::setFloats() must have exact size",
            floats == items.size());

        worker.queue.enqueueWriteBuffer(memory, CL_TRUE, 0, bytes, items.data());
    }

    void getFloats(std::vector<cl_float> * items) {
        expect("MatrixState::getFloats() must have exact size",
            floats == items->size());

        worker.queue.enqueueReadBuffer(memory, CL_TRUE, 0, bytes, items->data());
    }

    template<size_t rows2, size_t cols2>
    void copyFrom(MatrixState<rows2, cols2> & that) {
        expect("MatrixState::copyFrom() must have exact size",
            bytes == that.bytes);

        worker.queue.finish();
        worker.queue.enqueueCopyBuffer(that.memory, memory, 0, 0, bytes);
        worker.queue.finish();
    }

    template<size_t rows2, size_t cols2>
    void copyFromViaHost(MatrixState<rows2, cols2> & that) {
        expect("MatrixState::copyFrom() must have exact size",
            bytes == that.bytes);

        // Create host-side buffer for float data.
        std::vector<cl_float> items(floats);

        // Read items from other buffer.
        that.getFloats(&items);

        // Write items to this buffer.
        setFloats(items);
    }

    // Public immutable members.
    size_t       const count;
    size_t       const floats;
    size_t       const bytes;

    // Public OpenCL buffer for state access.
    cl::Buffer         memory;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_MATRIX_STATE_HH__ */
