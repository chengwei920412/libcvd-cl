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

#include "cvd-cl/steps/HipsMakeTreeStep.hh"
#include "cvd-cl/steps/DescriptorTree.hh"

namespace CVD {
namespace CL  {

HipsMakeTreeStep::HipsMakeTreeStep(HipsListState & i_hips, HipsTreeState & o_tree) :
    WorkerStep (i_hips.worker),
    i_hips     (i_hips),
    o_tree     (o_tree)
{
    // Do nothing.
}

HipsMakeTreeStep::~HipsMakeTreeStep() {
    // Do nothing.
}

void HipsMakeTreeStep::execute() {
    // Refer to tree shape.
    HipsTreeShape const & shape = o_tree.shape;

    // Read descriptor list.
    std::vector<cl_ulong4> hips;
    i_hips.get(&hips);

    // Prepare output buffers.
    std::vector<cl_ulong4> tree;
    std::vector<cl_ushort> maps;

    // Construct descriptor tree.
    DescriptorTree<cl_ulong4>::buildDescriptorTree(hips, shape, tree, maps);

    // Write tree and mapping to device.
    o_tree.setTree(tree);
    o_tree.setMaps(maps);
}

} // namespace CL
} // namespace CVD
