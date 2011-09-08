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

#include "cvd-cl/worker/CuWorker.hh"

namespace CVD {
namespace CL  {

CuWorker::CuWorker(int device) :
    device           (device),
    defaultLocalSize (256),
    defaultLocal     (defaultLocalSize)
{
    // Do nothing.
}

CuWorker::~CuWorker() {
    // Do nothing.
}

size_t CuWorker::padGlobalSize(size_t items) {
    // Perform integer arithmetic without assuming power-of-two group size.
    return (((items + defaultLocalSize - 1) / defaultLocalSize) * defaultLocalSize);
}

dim3 CuWorker::padGlobal(size_t items) {
    return dim3(padGlobalSize(items), 1, 1);
}

} // namespace CL
} // namespace CVD
