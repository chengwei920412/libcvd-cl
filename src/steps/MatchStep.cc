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

#include "cvd-cl/steps/MatchStep.hh"
#include "kernels/find.hh"

namespace CVD {
namespace CL  {

MatchStep::MatchStep(HipsListState & ihips1, HipsListState & ihips2, IntListState & obest) :
    WorkerStep (ihips1.worker),
    ihips1     (ihips1),
    ihips2     (ihips2),
    obest      (obest)
{
    worker.compile(&program, &kernel, OCL_FIND, "hips_find");
}

MatchStep::~MatchStep() {
    // Do nothing.
}

void MatchStep::execute() {
    // Assign kernel parameters.
    kernel.setArg(0, ihips1.buffer);
    kernel.setArg(1, ihips2.buffer);
    kernel.setArg(2, obest.buffer);

    // Read number of input points.
    size_t const np1 = ihips1.getCount();
    size_t const np2 = ihips2.getCount();

    // Reset number of output points.
    // TODO: Generalise kernel to arbitrary sizes.
    obest.setCount(256);

    // Queue kernel with global size set to number of input points.
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(256, 16), cl::NDRange(16, 16));
}

} // namespace CL
} // namespace CVD
