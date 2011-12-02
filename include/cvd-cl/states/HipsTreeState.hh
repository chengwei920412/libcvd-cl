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

#ifndef __CVD_CL_HIPS_TREE_STATE_HH__
#define __CVD_CL_HIPS_TREE_STATE_HH__

#include <cvd-cl/worker/WorkerState.hh>
#include <cvd-cl/states/HipsTreeShape.hh>

namespace CVD {
namespace CL  {

/// \brief WorkerState representing a dense HIPS tree or forest.
///
/// \see HipsTreeShape
/// \see DescriptorTree
class HipsTreeState : public WorkerState {
public:

    /// \brief Construct the HipsTreeState with a given \a worker,
    /// number of leaves and number of forest levels.
    ///
    /// \pre See HipsTreeShape::HipsTreeShape() for preconditions.
    ///
    /// \param worker       Worker for which this state will be allocated.
    /// \param nLeaves      Leaves in the HIPS tree
    /// \param nKeepLevels  Levels kept of the tree, from leaves up.
    explicit HipsTreeState(Worker & worker, cl_uint nLeaves = 512, cl_uint nKeepLevels = 5);

    /// \brief De-construct the HipsTreeState (releases memory).
    virtual ~HipsTreeState();

    /// \brief Write tree data from a flat descriptor vector.
    ///
    /// \pre \code
    /// list.size() == shape.nTreeNodes
    /// \endcode
    ///
    /// \param list Descriptor vector.
    void setTree(std::vector<cl_ulong4> const & list);

    /// \brief Write tree map data from a flat integer vector.
    ///
    /// \pre \code
    /// list.size() == shape.nLeaves
    /// \endcode
    ///
    /// \param list Integer vector.
    void setMaps(std::vector<cl_ushort> const & list);

    /// \brief Stored copy from last setTree().
    std::vector<cl_ulong4> lastTree;

    /// \brief Stored copy from last setMaps().
    std::vector<cl_ushort> lastMaps;

    /// \brief Calculated tree shape.
    HipsTreeShape const shape;

    /// \brief OpenCL image object for HIPS descriptor forest.
    ///
    /// Each pixel is an RGBA of 32-bit unsigned integers, 128 bits in total.
    /// Each row is a 256 bit HIPS descriptor, contiguous in host-side memory.
    ///
    /// height = nKeepNodes
    /// width  = 2
    ///
    /// This order is used to keep cl_ulong4 elements adjacent.
    cl::Image2D    tree;

    /// \brief OpenCL image object for the original index of each tree leaf.
    ///
    /// Each pixel is an RGBA of 16-bit unsigned integers, only .x of which is used.
    ///
    /// height = nLeaves
    /// width  = 1
    cl::Image2D    maps;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_HIPS_TREE_STATE_HH__ */
