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

#ifndef __CVD_CL_UV_STATE_HH__
#define __CVD_CL_UV_STATE_HH__

#include <cvd-cl/states/MatrixState.hh>

namespace CVD {
namespace CL  {

class UvState : public WorkerState {
public:

    explicit UvState(Worker & worker, size_t setCount, size_t setSize);
    virtual ~UvState();

    // Public immutable members.
    size_t       const setCount;
    size_t       const setSize;
    size_t       const records; // = setCount * setSize

    // Public sub-states.
    MatrixState        us;
    MatrixState        vs;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_UV_STATE_HH__ */
