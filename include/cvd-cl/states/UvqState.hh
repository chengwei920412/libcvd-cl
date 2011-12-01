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

#ifndef __CVD_CL_UVQ_STATE_HH__
#define __CVD_CL_UVQ_STATE_HH__

#include <cvd-cl/states/UvState.hh>

namespace CVD {
namespace CL  {

/// \brief WorkerState representing a fixed-size sequence of sets of (u,v,q) records.
template<size_t setSize>
class UvqState : public UvState<setSize> {
public:

    /// \brief Construct the UvqState with a given \a worker and \a setCount.
    ///
    /// The \a setCount is the number of sets of (u,v,q) records.
    /// The \a setSize template parameter is the size of each set of (u,v,q) records.
    /// The total number of #records is the product of these two numbers.
    ///
    /// \param worker   Worker for which this UvqState will be allocated.
    /// \param setCount Number of (u,v,q) record sets contained in this state.
    explicit UvqState(Worker & worker, size_t setCount) :
        UvState<setSize> (worker, setCount),
        qs               (worker, setCount)
    {
        // Do nothing.
    }

    /// \brief De-construct the UvqState (releases memory).
    virtual ~UvqState() {
        // Do nothing.
    }

    /// \brief MatrixState of each q component.
    MatrixState<setSize, 1> qs;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_UVQ_STATE_HH__ */
