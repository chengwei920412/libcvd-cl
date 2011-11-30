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

#ifndef __CVD_CL_COUNT_STATE_HH__
#define __CVD_CL_COUNT_STATE_HH__

#include <cvd-cl/worker/WorkerState.hh>

namespace CVD {
namespace CL  {

/// \brief A WorkerState representing a bounded counter.
///
/// Use this as a component of more complicated states
/// such as variable-sized lists (see ListState).
class CountState : public WorkerState {
public:

    /// \brief Construct the CountState with a given \a worker and \a size.
    ///
    /// \param worker  Worker for which this CountState will be allocated.
    /// \param size    Upper bound for #count.
    explicit CountState(Worker & worker, cl_uint size);

    /// De-construct the CountState (releases memory).
    virtual ~CountState();

    /// \brief Update the count, blocking until completion.
    ///
    /// \param ncount  New count value.
    void setCount(cl_uint ncount);

    /// \brief Query the count, blocking until it is available.
    ///
    /// \return Current count value.
    cl_uint getCount();

    /// \brief Upper bound for #count.
    ///
    /// This may represent a related quantity such as the maximum size of a list.
    cl_uint      const size;

    /// \brief OpenCL buffer of a single 32-bit unsigned integer.
    cl::Buffer         count;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_COUNT_STATE_HH__ */
