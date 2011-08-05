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

#ifndef __CVD_CL_PRE_FAST_STEP_HH__
#define __CVD_CL_PRE_FAST_STEP_HH__

#include <cvd-cl/worker/WorkerStep.hh>
#include <cvd-cl/states/ImageState.hh>

namespace CVD {
namespace CL  {

class PreFastStep : public WorkerStep<IState, OState> {
public:

    // Type alias for the convenience of sub-classes.
    typedef WorkerStep<IState, OState> Super;

    explicit WorkerStep(Worker & worker) :
        worker(worker) {
        // Do nothing.
    }

    virtual ~WorkerStep() {
        // Do nothing.
    }

protected:

    Worker & worker;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_PRE_FAST_STEP_HH__ */
