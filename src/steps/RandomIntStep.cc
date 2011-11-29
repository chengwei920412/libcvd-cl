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

#include "cvd-cl/steps/RandomIntStep.hh"

#include "kernels/random-int.hh"

#include <boost/cstdlib.hpp>

namespace CVD {
namespace CL  {

RandomIntStep::RandomIntStep(CountState & i_max, IntListState & o_ints) :
    WorkerStep (i_max.worker),
    i_max      (i_max),
    o_ints     (o_ints)
{
    worker.compile(&program, &kernel, OCL_RANDOM_INT, "random_int");
}

RandomIntStep::~RandomIntStep() {
    // Do nothing.
}

void RandomIntStep::execute() {
    // Sample system time.
    cl_uint const seed1 = (cl_uint) ::time(NULL);

    // Sample address of an array.
    cl_uint const seed2 = (cl_uint) (size_t) (void *) &o_ints;

    // Assign kernel parameters.
    kernel.setArg(0, i_max.count);
    kernel.setArg(1, o_ints.buffer);
    kernel.setArg(2, seed1);
    kernel.setArg(3, seed2);

    // Reset number of output points.
    size_t const count = o_ints.size;
    o_ints.setCount(count);

    // Queue kernel with global size set to number of output points.
    worker.queue.enqueueBarrier();
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(count), cl::NullRange);
}

} // namespace CL
} // namespace CVD
