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

#ifndef __CVD_CL_UVQ_UV_STATE_HH__
#define __CVD_CL_UVQ_UV_STATE_HH__

#include <cvd-cl/states/UvqState.hh>

namespace CVD {
namespace CL  {

/// \brief WorkerState representing a variable-size sequence of sets of ((u,v,q),(u,v)) records.
template<size_t setSize>
class UvqUvState : public WorkerState {
public:

    /// \brief Construct the UvqUvState with a given \a worker and \a maxCount.
    ///
    /// The \a maxCount is the maximum number of sets of records.
    /// The \a setSize template parameter is the size of each set of records.
    ///
    /// \param worker   Worker for which this UvqUvState will be allocated.
    /// \param maxCount Maximum number of record sets contained in this state.
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

    /// \brief De-construct the UvqUvState (releases memory).
    virtual ~UvqUvState() {
        // Do nothing.
    }

    /// \brief Maximum number of record sets contained in this state.
    size_t       const maxCount;

    /// \brief Maximum number of records contained in this state.
    size_t       const maxRecords;

    /// \brief Current number of record sets contained in this state.
    size_t             setCount;

    /// \brief UvqState for (u,v,q) part of each record.
    UvqState <setSize> uvq;

    /// \brief UvState for (u,v) part of each record.
    UvState  <setSize> uv;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_UVQ_UV_STATE_HH__ */
