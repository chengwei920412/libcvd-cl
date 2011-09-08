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

#include "cvd-cl/steps/cuda/CuHipsFindStep.hh"

#include <iostream>

namespace CVD {
namespace CL  {

void hips_find(
    ulong4 const * hashes1,  // T
    ulong4 const * hashes2,  // R
    int2         * matches,  // Pairs of indices into hashes1 and hashes2.
    uint         * imatch,   // Output number of hash1 matches.
    uint           nmatch,   // Maximum number of matches.
    uint           np1,
    uint           np2
);

CuHipsFindStep::CuHipsFindStep(CuHipsListState & i_hips1, CuHipsListState & i_hips2, CuPointListState & o_matches) :
    CuWorkerStep (i_hips1.worker),
    i_hips1      (i_hips1),
    i_hips2      (i_hips2),
    o_matches    (o_matches)
{
    // Do nothing.
    std::cerr << "CuHipsFindStep::CuHipsFindStep()" << std::endl;
}

CuHipsFindStep::~CuHipsFindStep() {
    // Do nothing.
    std::cerr << "CuHipsFindStep::~CuHipsFindStep()" << std::endl;
}

void CuHipsFindStep::execute() {
    std::cerr << "CuHipsFindStep::execute()" << std::endl;

    // Read number of descriptors.
    unsigned int const np1 = i_hips1.getCount();
    unsigned int const np2 = i_hips2.getCount();

    // Reset number of output pairs.
    o_matches.setCount(0);

    hips_find(i_hips1.d_buffer, i_hips2.d_buffer, o_matches.d_buffer, o_matches.d_count, o_matches.size, np1, np2);
}

} // namespace CL
} // namespace CVD
