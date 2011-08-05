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

#ifndef __CVD_CL_IMAGE_STATE_HH__
#define __CVD_CL_IMAGE_STATE_HH__

#include <cvd-cl/worker/WorkerState.hh>

#include <cvd/byte.h>
#include <cvd/image.h>

namespace CVD {
namespace CL  {

template<class Item>
class ListState : public WorkerState<std::vector<Item> > {
public:

    explicit ListState(Worker & worker, size_t size) :
        Super(worker),
        size(size),
        nbytes(size * sizeof(cl_int2)),
        m_fill(0) {

        // Allocate buffer (may throw a CL exception).
        m_buffer = cl::Buffer(worker.context, CL_MEM_READ_WRITE, nbytes);
    }

    virtual ~ListState() {
        // Do nothing.
    }

    void setFill(size_t fill) {
        expect("PointListState::setFill() must be within size",
                fill <= size);

        m_fill = fill;
    }

    size_t fill() const {
        assert(m_fill <= size);
        return m_fill;
    }

    virtual void set(PointList const & points) {
        expect("ListState::set() must be given a vector small enough",
                    points.size() <= size);

        size_t const pnbytes = points.size() * sizeof(Item);
        worker.queue.enqueueWriteBuffer(m_buffer, CL_TRUE, 0, pnbytes, points.data());
    }

    virtual void get(PointList       * points) {
        points->resize(m_fill);

        size_t const pnbytes = m_fill * sizeof(Item);
        worker.queue.enqueueReadBuffer(m_buffer, CL_TRUE, 0, pnbytes, points->data());
    }

    // Public immutable member.
    size_t const size;
    size_t const nbytes;

protected:

    cl::Buffer   m_buffer;
    size_t       m_fill;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_IMAGE_STATE_HH__ */
