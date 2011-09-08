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

#include "cvd-cl/states/cuda/CuCountState.hh"
#include "cvd-cl/core/Expect.hh"

namespace CVD {
namespace CL  {

CuCountState::CuCountState(CuWorker & worker, unsigned int size) :
    CuWorkerState (worker),
    size          (size),
    d_count       (NULL)
{
    expect("CuCountState::CuCountState() must have positive size",
        size > 0);

    // Allocate memory.
    cutry(cudaMalloc((void * *) &d_count, sizeof(unsigned int)));
    assert(d_count != NULL);

    // Reset counter.
    setCount(0);
}

CuCountState::~CuCountState() {
    cudaFree(d_count);
    d_count = NULL;
}

void CuCountState::setCount(unsigned int ncount) {
    assert(d_count != NULL);
    expect("CountState::setCount() must fit within size", (ncount <= size));
    cutry(cudaMemcpy(d_count, &ncount, sizeof(ncount), cudaMemcpyHostToDevice));
}

unsigned int CuCountState::getCount() {
    assert(d_count != NULL);
    unsigned int ncount = 0;
    cutry(cudaMemcpy(&ncount, d_count, sizeof(ncount), cudaMemcpyDeviceToHost));
    // Crop against maximum size.
    return std::min(ncount, size);
}

} // namespace CL
} // namespace CVD
