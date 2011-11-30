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

#ifndef __CVD_CL_WORKER_STATE_HH__
#define __CVD_CL_WORKER_STATE_HH__

#include <cvd-cl/core/State.hh>
#include <cvd-cl/worker/Worker.hh>

namespace CVD {
namespace CL  {

/// \brief An abstract State associated with a Worker.
///
/// States contain data in between processing by WorkerState objects.
/// States should expose member functions that allow their data to be
/// copied to and from host (C++) code.
/// States should allocate resources when they are constructed, if possible,
/// so that any exceptions can occur early and be avoided when executing steps.
///
/// \see WorkerStep
class WorkerState : public State {
public:

    /// \brief Construct the WorkerState for the given \a worker.
    ///
    /// This simply assigns the #worker reference.
    ///
    /// \param worker Worker to associate with this state.
    explicit WorkerState(Worker & worker);

    /// \brief De-construct the WorkerState (does nothing).
    virtual ~WorkerState();

    /// \brief Worker associated with this WorkerState.
    Worker & worker;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_WORKER_STATE_HH__ */
