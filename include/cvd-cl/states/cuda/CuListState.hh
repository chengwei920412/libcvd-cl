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

#ifndef __CVD_CL_CU_LIST_STATE_HH__
#define __CVD_CL_CU_LIST_STATE_HH__

#include <boost/static_assert.hpp>

#include <cstring>
#include <vector>

#include <cvd-cl/states/cuda/CuCountState.hh>
#include <cvd-cl/core/Expect.hh>

namespace CVD {
namespace CL  {

template<class Item>
class CuListState : public CuCountState {
public:

    explicit CuListState(CuWorker & worker, unsigned int size) :
        CuCountState (worker, size),
        nbytes       (size * sizeof(Item))
    {
        cutry(cudaMalloc((void * *) &d_buffer, nbytes));
    }

    virtual ~CuListState() {
        cudaFree(d_buffer);
    }

    void set(std::vector<Item> const & items) {
        // Write new count to device.
        setCount(items.size());

        // Write item data directly to device.
        size_t const pnbytes = items.size() * sizeof(Item);
        cudaMemcpy(d_buffer, items.data(), pnbytes, cudaMemcpyHostToDevice);
    }

    void get(std::vector<Item>       * items) {
        // Read count from device.
        unsigned int const ncount = getCount();

        // Allocate memory for item data.
        items->resize(ncount);

        // Read item data directly from device.
        size_t const pnbytes = ncount * sizeof(Item);
        cudaMemcpy(d_buffer, items->data(), pnbytes, cudaMemcpyDeviceToHost);
    }

    void zero() {
        // Create zero-filled sample item.
        Item zero;
        ::memset(&zero, 0, sizeof(zero));

        // Extend item to full-sized vector.
        std::vector<Item> const items(size, zero);

        // Write vector memory.
        cudaMemcpy(d_buffer, items.data(), nbytes, cudaMemcpyHostToDevice);
    }

    template<typename Item2>
    void setCast(std::vector<Item2> const & items) {
        BOOST_STATIC_ASSERT(sizeof(Item) == sizeof(Item2));

        // Write new count to device.
        setCount(items.size());

        // Write item data directly to device.
        size_t const pnbytes = items.size() * sizeof(Item2);
        cudaMemcpy(d_buffer, items.data(), pnbytes, cudaMemcpyHostToDevice);
    }

    // Public immutable member.
    unsigned int const nbytes;

    // Members left public for CuWorkerStep access.
    Item * d_buffer;
};

typedef CuListState<int2  > CuPointListState;
typedef CuListState<float > CuFloatListState;
typedef CuListState<float2> CuFloat2ListState;
typedef CuListState<ulong4> CuHipsListState;
typedef CuListState<int   > CuIntListState;

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_CU_LIST_STATE_HH__ */
