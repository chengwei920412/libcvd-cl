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

#include "cvd-cl/states/HipsTreeState.hh"

namespace CVD {
namespace CL  {

cl::ImageFormat static const HipsFormat(CL_RGBA, CL_UNSIGNED_INT32);
cl::ImageFormat static const MapsFormat(CL_R,    CL_UNSIGNED_INT16);

HipsTreeState::HipsTreeState(Worker & worker) :
    WorkerState (worker),
    tree        (worker.context, CL_MEM_READ_ONLY, HipsFormat, 2, 1024),
    maps        (worker.context, CL_MEM_READ_ONLY, MapsFormat, 1, 1024)
{
    // Do nothing.
}

HipsTreeState::~HipsTreeState() {
    // Do nothing.
}

} // namespace CL
} // namespace CVD
