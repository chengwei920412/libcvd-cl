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

#ifndef __CVD_CL_HIPS_TREE_STATE_HH__
#define __CVD_CL_HIPS_TREE_STATE_HH__

#include <cvd-cl/worker/WorkerState.hh>
#include <cvd-cl/states/HipsTreeShape.hh>

namespace CVD {
namespace CL  {

class HipsTreeState : public WorkerState {
public:

    // Bytes in constant memory:                32768
    // Bytes per descriptor:                       32
    //                                          -----
    // Maximum descriptors in array:             1024
    // Maximum leaf descriptors:                  512

    // Forest structure in 992 descriptors:
    // [32 nodes] [64 nodes] [128 nodes] [256 nodes] [512 leaves]
    // Each thread starts from the same 16 roots (excluded) to select 16 from 32 initial nodes (included).
    // Each thread will do exactly 10 (5 levels * 2 children) error calculations per root.
    // Each thread will pick 0 or 1 leaf per root, so 0-16 in total per thread.

    cl_uint static const NLEAF = 512;
    cl_uint static const NROOT =  32;
    cl_uint static const NNODE = 992;
    cl_uint static const START = (NNODE - NLEAF);
    cl_uint static const LEVEL =   5;

    explicit HipsTreeState(Worker & worker, cl_uint nLeaves = 512, cl_uint nKeepLevels = 5);
    virtual ~HipsTreeState();

    void setTree(std::vector<cl_ulong4> const & list);
    void setMaps(std::vector<cl_ushort> const & list);


    /** Virtual and actual shape of the HIPS descriptor forest. */
    HipsTreeShape const shape;

    /**
     * OpenCL image object for HIPS descriptor forest.
     *
     * Each pixel is an RGBA of 32-bit unsigned integers, 128 bits in total.
     * Each row is a 256 bit HIPS descriptor, contiguous in host-side memory.
     *
     * height = nKeepNodes
     * width  = 2
     *
     * This order is used to keep cl_ulong4 halves adjacent.
     */
    cl::Image2D    tree;

    /**
     * OpenCL image object for the original index of each tree leaf.
     *
     * Each pixel is a 16-bit unsigned integer.
     *
     * height = nLeaves
     * width  = 1
     */
    cl::Image2D    maps;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_HIPS_TREE_STATE_HH__ */
