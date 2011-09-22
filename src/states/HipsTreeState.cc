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
#include "cvd-cl/core/Expect.hh"

#ifdef CVD_CL_VERBOSE
#include <iomanip>
#include <iostream>
#endif

namespace CVD {
namespace CL  {

cl::ImageFormat static const HipsFormat(CL_RGBA, CL_UNSIGNED_INT32);
cl::ImageFormat static const MapsFormat(CL_R,    CL_UNSIGNED_INT16);

HipsTreeState::HipsTreeState(Worker & worker, size_t nLeaves, size_t nKeepLevels) :
    WorkerState (worker),
    shape       (nLeaves, nKeepLevels),
    // Allocate image objects.
    tree        (worker.context, CL_MEM_READ_ONLY, HipsFormat, 2, shape.nFullNodes),
    maps        (worker.context, CL_MEM_READ_ONLY, MapsFormat, 1, shape.nLeaves)
{
    // Do nothing.
}

HipsTreeState::~HipsTreeState() {
    // Do nothing.
}

void HipsTreeState::setTree(std::vector<cl_ulong4> const & list) {
    assert(list.size() == shape.nKeepNodes);

    cl::size_t<3> origin;
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;

    cl::size_t<3> region;
    region[0] = 2;
    region[1] = shape.nKeepNodes;
    region[2] = 1;

    // Cast to non-const void due to error in cl.hpp.
    cl_ulong4 * data = const_cast<cl_ulong4 *>(list.data());

    worker.queue.enqueueWriteImage(tree, CL_TRUE, origin, region, 0, 0, data);
}

void HipsTreeState::setMaps(std::vector<cl_ushort> const & list) {
    assert(list.size() == shape.nLeaves);

    cl::size_t<3> origin;
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;

    cl::size_t<3> region;
    region[0] = 1;
    region[1] = shape.nLeaves;
    region[2] = 1;

    // Cast to non-const void due to error in cl.hpp.
    cl_ushort * data = const_cast<cl_ushort *>(list.data());

    worker.queue.enqueueWriteImage(maps, CL_TRUE, origin, region, 0, 0, data);
}

} // namespace CL
} // namespace CVD
