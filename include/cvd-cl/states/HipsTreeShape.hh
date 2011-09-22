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

#ifndef __CVD_CL_HIPS_TREE_SHAPE_HH__
#define __CVD_CL_HIPS_TREE_SHAPE_HH__

#include <cvd-cl/worker/Worker.hh>

namespace CVD {
namespace CL  {

class HipsTreeShape {
public:

    HipsTreeShape(cl_uint nLeaves, cl_uint nKeepLevels);
    ~HipsTreeShape();

    /** Leaves in the HIPS tree. Must be a power of 2 and [8 <= nLeaves <= 2048]. */
    cl_uint const nLeaves;

    /** Levels kept of the tree, from leaves up. Must satisfy [1 < nKeepLevels < nTreeLevels]. */
    cl_uint const nKeepLevels;

    // Calculations from parameters.

    /** Levels in the full tree = log2(nLeaves). */
    cl_uint const nTreeLevels;

    /** Round-up to next power of 2. */
    cl_uint const nFullNodes;

    /** Nodes stored in the full tree. */
    cl_uint const nTreeNodes;

    /** Levels dropped from the full tree. */
    cl_uint const nDropLevels;

    /** Tree roots stored in the forest. */
    cl_uint const nTreeRoots;

    /** Number of nodes dropped from the tree. */
    cl_uint const nDropNodes;

    /** Nodes kept of the tree. */
    cl_uint const nKeepNodes;

    /** Index of the first leaf in the full tree. */
    cl_uint const iTreeLeaf0;

    /** Index of the first leaf in the stored forest. */
    cl_uint const iKeepLeaf0;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_HIPS_TREE_SHAPE_HH__ */
