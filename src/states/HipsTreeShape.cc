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

#include "cvd-cl/states/HipsTreeShape.hh"
#include "cvd-cl/core/Expect.hh"

#ifdef CVD_CL_VERBOSE
#include <iomanip>
#include <iostream>
#endif

namespace CVD {
namespace CL  {

static cl_uint leavesToLevels(cl_uint nLeaves) {
    expect("nLeaves must be at least 8",   nLeaves >= 8);
    expect("nLeaves must be at most 2048", nLeaves <= 2048);

    cl_uint nTreeLevels = 1;
    while ((nLeaves & 1) == 0) {
        nLeaves >>= 1;
        nTreeLevels++;
    }

    expect("nLeaves must be a power of 2", nLeaves == 1);
    return nTreeLevels;
}

HipsTreeShape::HipsTreeShape(cl_uint nLeaves, cl_uint nKeepLevels) :
    // Assign parameters.
    nLeaves     (nLeaves),
    nKeepLevels (nKeepLevels),
    // Calculate other sizes.
    nTreeLevels (leavesToLevels(nLeaves)),
    nFullNodes  (nLeaves * 2),
    nTreeNodes  (nFullNodes - 1),
    nDropLevels (nTreeLevels - nKeepLevels),
    nTreeRoots  (1 << nDropLevels),
    nDropNodes  (nTreeRoots - 1),
    nKeepNodes  (nTreeNodes - nDropNodes),
    iTreeLeaf0  (nTreeNodes - nLeaves),
    iKeepLeaf0  (nKeepNodes - nLeaves)
{
#ifdef CVD_CL_VERBOSE
    std::cerr << "HIPS tree shape:" << std::endl;
    std::cerr << std::setw(9) << nLeaves     << " nLeaves"     << std::endl;
    std::cerr << std::setw(9) << nFullNodes  << " nFullNodes"  << std::endl;
    std::cerr << std::setw(9) << nTreeLevels << " nTreeLevels" << std::endl;
    std::cerr << std::setw(9) << nDropLevels << " nDropLevels" << std::endl;
    std::cerr << std::setw(9) << nKeepLevels << " nKeepLevels" << std::endl;
    std::cerr << std::setw(9) << nTreeRoots  << " nTreeRoots"  << std::endl;
    std::cerr << std::setw(9) << nTreeNodes  << " nTreeNodes"  << std::endl;
    std::cerr << std::setw(9) << nDropNodes  << " nDropNodes"  << std::endl;
    std::cerr << std::setw(9) << nKeepNodes  << " nKeepNodes"  << std::endl;
    std::cerr << std::setw(9) << iTreeLeaf0  << " iTreeLeaf0"  << std::endl;
    std::cerr << std::setw(9) << iKeepLeaf0  << " iKeepLeaf0"  << std::endl;
    std::cerr << std::endl;
#endif // CVD_CL_VERBOSE
}

HipsTreeShape::~HipsTreeShape() {
    // Do nothing.
}

} // namespace CL
} // namespace CVD
