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

#ifndef __CVD_CL_UVQ_UV_STATE_HH__
#define __CVD_CL_UVQ_UV_STATE_HH__

#include <cvd-cl/states/UvqState.hh>

namespace CVD {
namespace CL  {

template<size_t setSize>
class UvqUvState : public WorkerState {
public:

    explicit UvqUvState(Worker & worker, size_t maxCount) :
        WorkerState (worker),
        maxCount    (maxCount),
        maxRecords  (maxCount * setSize),
        setCount    (maxCount),
        uvq         (worker, maxCount),
        uv          (worker, maxCount)
    {
        // Do nothing.
    }

    virtual ~UvqUvState() {
        // Do nothing.
    }

    // Public immutable members.
    size_t       const maxCount;
    size_t       const maxRecords; // = maxCount * setSize

    // Public mutable member.
    size_t             setCount;

    // Public sub-states.
    UvqState <setSize> uvq;
    UvState  <setSize> uv;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_UVQ_UV_STATE_HH__ */
