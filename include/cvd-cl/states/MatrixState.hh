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

/// \brief WorkerState representing a fixed-size sequence of fixed-size matrices.
///
/// Rows and columns are given as template parameters and form part of the reified type.
template<size_t rows, size_t cols>
class MatrixState : public WorkerState {
private:

    BOOST_STATIC_ASSERT(rows > 0);
    BOOST_STATIC_ASSERT(cols > 0);
    BOOST_STATIC_ASSERT(rows < 0x10000);
    BOOST_STATIC_ASSERT(rows < 0x10000);

public:

    /// \brief Total number of elements per matrix.
    size_t static const elms = (rows * cols);

    /// \brief Construct the MatrixState with a given \a worker and \a count.
    ///
    /// \param worker   Worker for which this MatrixState will be allocated.
    /// \param count    Number of matrices contained in this state.
    explicit MatrixState(Worker & worker, size_t count)  :
        WorkerState (worker),
        count       (count),
        floats      (count * elms),
        bytes       (floats * sizeof(cl_float))
    {
        // Allocate buffer (may throw a CL exception).
        memory = cl::Buffer(worker.context, CL_MEM_READ_WRITE, bytes);
    }

    /// \brief De-construct the MatrixState (releases memory).
    virtual ~MatrixState() {
        // Do nothing.
    }

    /// \brief Assign the matrix data from a vector of floats.
    ///
    /// \pre \code
    /// #floats == items.size()
    /// \endcode
    ///
    /// \param items   Vector of floats of the same total size as #memory.
    void setFloats(std::vector<cl_float> const & items) {
        expect("MatrixState::setFloats() must have exact size",
            floats == items.size());

        worker.queue.enqueueWriteBuffer(memory, CL_TRUE, 0, bytes, items.data());
    }

    /// \brief Query the matrix data into a vector of floats.
    ///
    /// \param items   Vector of floats.
    void getFloats(std::vector<cl_float> * items) {
        items->resize(floats);

        worker.queue.enqueueReadBuffer(memory, CL_TRUE, 0, bytes, items->data());
    }

    /// \brief Assign the matrix data from a matrix of the same total size,
    /// regardless of row and column dimensions, directly on the #worker.
    ///
    /// \pre \code
    /// bytes == that.bytes
    /// \endcode
    ///
    /// \param that    MatrixState of same total size.
    template<size_t rows2, size_t cols2>
    void copyFrom(MatrixState<rows2, cols2> & that) {
        expect("MatrixState::copyFrom() must have exact size",
            bytes == that.bytes);

        worker.queue.finish();
        worker.queue.enqueueCopyBuffer(that.memory, memory, 0, 0, bytes);
        worker.queue.finish();
    }

    /// \brief Assign the matrix data from a matrix of the same total size,
    /// regardless of row and column dimensions, using host memory.
    ///
    /// \pre \code
    /// bytes == that.bytes
    /// \endcode
    ///
    /// \param that    MatrixState of same total size.
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

    /// \brief Number of matrices contained in #memory.
    size_t       const count;

    /// \brief Number of floating point numbers contained in #memory.
    size_t       const floats;

    /// \brief Number of bytes contained in #memory.
    size_t       const bytes;

    /// \brief OpenCL buffer for matrix data.
    cl::Buffer         memory;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_MATRIX_STATE_HH__ */
