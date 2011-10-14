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

#ifndef __CVD_CL_LIST_STATE_HH__
#define __CVD_CL_LIST_STATE_HH__

#include <cvd-cl/states/CountState.hh>
#include <cvd-cl/core/Expect.hh>

namespace CVD {
namespace CL  {

template<class Item>
class ListState : public CountState {
public:

    explicit ListState(Worker & worker, cl_uint size) :
        CountState (worker, size),
        nbytes     (size * sizeof(Item))
    {
        // Allocate buffers (may throw a CL exception).
        buffer = cl::Buffer(worker.context, CL_MEM_READ_WRITE, nbytes);
    }

    virtual ~ListState() {
        // Do nothing.
    }

    void set(std::vector<Item> const & items) {
        // Write new count to device.
        setCount(items.size());

        // Write item data directly to device.
        size_t const pnbytes = items.size() * sizeof(Item);
        worker.queue.enqueueWriteBuffer(buffer, CL_TRUE, 0, pnbytes, items.data());
    }

    void get(std::vector<Item>       * items) {
        // Read count from device.
        size_t const ncount = getCount();

        // Allocate memory for item data.
        items->resize(ncount);

        // Read item data directly from device.
        size_t const pnbytes = ncount * sizeof(Item);
        worker.queue.enqueueReadBuffer(buffer, CL_TRUE, 0, pnbytes, items->data());
    }

    void zero() {
        // Create zero-filled sample item.
        Item zero;
        ::memset(&zero, 0, sizeof(zero));

        // Extend item to full-sized vector.
        std::vector<Item> const items(size, zero);

        // Write vector memory.
        worker.queue.finish();
        worker.queue.enqueueWriteBuffer(buffer, CL_TRUE, 0, nbytes, items.data());
        worker.queue.finish();
    }

    void copyFrom(ListState<Item> & that) {
        worker.queue.finish();
        worker.queue.enqueueCopyBuffer(that.buffer, buffer, 0, 0, nbytes);
        worker.queue.enqueueCopyBuffer(that.count,  count,  0, 0, sizeof(cl_uint));
        worker.queue.finish();
    }

    void copyFromViaHost(ListState<Item> & that) {
        // Create host-side buffer for item data.
        std::vector<Item> items;

        // Read items from other list.
        that.get(&items);

        // Write items to this list.
        set(items);
    }

    // Public immutable member.
    cl_uint const nbytes;

    // Members left public for WorkerStep access.
    cl::Buffer   buffer;
};

typedef ListState<cl_int2  > PointListState;
typedef ListState<cl_float > FloatListState;
typedef ListState<cl_float2> Float2ListState;
typedef ListState<cl_ulong8> HipsListState;
typedef ListState<cl_int   > IntListState;
typedef ListState<cl_ushort> ShortListState;

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_LIST_STATE_HH__ */
