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

#ifndef __CVD_CL_WORKER_STEP_HH__
#define __CVD_CL_WORKER_STEP_HH__

#include <cvd-cl/core/Step.hh>
#include <cvd-cl/worker/Worker.hh>

namespace CVD {
namespace CL  {

/// \brief An abstract Step associated with a Worker and some set of WorkerState objects.
///
/// Like Step, a WorkerStep should be constructed with references to input and output WorkerState objects,
/// possibly none.
/// A WorkerStep may allocate resources such as a compiled program, see Worker::compile.
///
/// \see WorkerState
class WorkerStep : public Step {
public:

    /// \brief Construct the WorkerStep for the given \a worker.
    ///
    /// This simply assigns the #worker reference.
    ///
    /// \param worker Worker to associate with this step.
    explicit WorkerStep(Worker & worker);

    /// \brief De-construct the WorkerStep (does nothing).
    virtual ~WorkerStep();

    virtual int64_t measure(int repeat=10);

    /// \brief Worker associated with this Worker.
    Worker & worker;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_WORKER_STEP_HH__ */
