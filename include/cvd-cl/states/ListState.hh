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

#ifndef __CVD_CL_LIST_STATE_HH__
#define __CVD_CL_LIST_STATE_HH__

#include <cvd-cl/states/CountState.hh>
#include <cvd-cl/core/Expect.hh>

namespace CVD {
namespace CL  {

/// \brief WorkerState representing a variable-sized list of a given Plain-Old-Data type.
///
/// The memory is synchronised to/from the Worker directly,
/// so use only types that are compatible between C++ and OpenCL,
/// such as cl_uint4.
template<class Item>
class ListState : public CountState {
public:

    /// \brief Construct the ListState with a given \a worker and maximum \a size.
    ///
    /// \param worker  Worker for which this ListState will be allocated.
    /// \param size    Upper bound for list size (see CountState::count).
    ///
    /// \see CountState::CountState()
    explicit ListState(Worker & worker, cl_uint size) :
        CountState (worker, size),
        nbytes     (size * sizeof(Item))
    {
        // Allocate buffers (may throw a CL exception).
        buffer = cl::Buffer(worker.context, CL_MEM_READ_WRITE, nbytes);
    }

    /// \brief De-construct the ListState (releases memory).
    virtual ~ListState() {
        // Do nothing.
    }

    /// \brief Assign the size and memory contents from \a items.
    ///
    /// \pre \code
    /// items.size() <= size
    /// \endcode
    ///
    /// \param items   Standard vector of the same type as the ListState.
    void set(std::vector<Item> const & items) {
        size_t const ncount = items.size();

        // Write new count to device.
        setCount(ncount);

        if (ncount < 1)
            return;

        // Write item data directly to device.
        size_t const pnbytes = ncount * sizeof(Item);
        worker.queue.enqueueWriteBuffer(buffer, CL_TRUE, 0, pnbytes, items.data());
    }

    /// \brief Query the size and memory contents, storing them in \a items.
    ///
    /// \param items   Standard vector of the same type as the ListState.
    void get(std::vector<Item>       * items) {
        // Read count from device.
        size_t const ncount = getCount();

        // Allocate memory for item data.
        items->resize(ncount);

        if (ncount < 1)
            return;

        // Read item data directly from device.
        size_t const pnbytes = ncount * sizeof(Item);
        worker.queue.enqueueReadBuffer(buffer, CL_TRUE, 0, pnbytes, items->data());
    }

    /// \brief Reset memory for the entire buffer, without changing the #count.
    ///
    /// \see CountState::setCount()
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

    /// \brief Copy the count and memory contents from another ListState of the same type,
    /// attempting to perform all IO on the Worker.
    ///
    /// It is assumed but not enforced that the lists are of the same size.
    ///
    /// \param that   ListState of the same type and size.
    void copyFrom(ListState<Item> & that) {
        worker.queue.finish();
        worker.queue.enqueueCopyBuffer(that.buffer, buffer, 0, 0, nbytes);
        worker.queue.enqueueCopyBuffer(that.count,  count,  0, 0, sizeof(cl_uint));
        worker.queue.finish();
    }

    /// \brief Copy the count and memory contents from another ListState of the same type,
    /// going via host memory (thereby allowing transfer across different Worker objects).
    ///
    /// Only contents within the source #count are copied, other contents remain as-is.
    ///
    /// \param that   ListState of the same type.
    void copyFromViaHost(ListState<Item> & that) {
        // Create host-side buffer for item data.
        std::vector<Item> items;

        // Read items from other list.
        that.get(&items);

        // Write items to this list.
        set(items);
    }

    /// \brief Total number of bytes allocated for #buffer.
    /// \code
    /// nbytes == (size * sizeof(Item))
    /// \endcode
    cl_uint const nbytes;

    /// \brief OpenCL buffer allocated for list contents.
    cl::Buffer   buffer;
};

typedef ListState<cl_int2  > PointListState;
typedef ListState<cl_float > FloatListState;
typedef ListState<cl_float2> Float2ListState;
typedef ListState<cl_ulong4> HipsListState;
typedef ListState<cl_int   > IntListState;
typedef ListState<cl_ushort> ShortListState;

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_LIST_STATE_HH__ */
