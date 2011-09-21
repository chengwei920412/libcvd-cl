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

#include "cvd-cl/steps/HipsMakeTreeStep.hh"

#include <climits>
#include <set>

#ifdef CVD_CL_VERBOSE
#include <iomanip>
#include <iostream>
#endif

namespace CVD {
namespace CL  {

template <typename T>
static int bitcount (T v){
  v = v - ((v >> 1) & (T)~(T)0/3);                           // temp
  v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);      // temp
  v = (v + (v >> 4)) & (T)~(T)0/255*15;                      // temp
  return (T)(v * ((T)~(T)0/255)) >> (sizeof(T) - 1) * CHAR_BIT; // count
}

static int bitcount4(cl_ulong4 t, cl_ulong4 r) {
    return
        bitcount(t.x ^ r.x) +
        bitcount(t.y ^ r.y) +
        bitcount(t.z ^ r.z) +
        bitcount(t.w ^ r.w);
}

// Zeroed descriptor as filler.
cl_ulong4 static const HIPS_ZERO = {{0, 0, 0, 0}};

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

static void pairup(std::vector<cl_ulong4> const & hips, std::vector<cl_ushort2> & pairs) {
    // Number of HIPS descriptors.
    size_t const nhips = hips.size();

    // Vector of "consumed" descriptors.
    std::vector<bool> used(nhips, false);

    // Clear vector for pairs.
    pairs.clear();

    // Perform greedy algorithm for best-pair matching.
    for (size_t i1 = 0; i1 < nhips; i1++) {
        // Skip if already used.
        if (used.at(i1) == true)
            continue;

        // Refer to descriptor 1.
        cl_ulong4 const & d1 = hips.at(i1);

        // Note if any unused descriptors exist.
        bool more = false;

        // Placeholder for the best matching index, and the error.
        int    be2 = 10000;
        size_t bi2 =     0;

        for (size_t i2 = 0; i2 < nhips; i2++) {
            // Skip if already used.
            if (used.at(i2) == true)
                continue;

            // Note that a descriptor remains available.
            more = true;

            // Skip if repeating the first descriptor.
            if (i1 == i2)
                continue;

            // Refer to descriptor 2.
            cl_ulong4 const & d2 = hips.at(i2);

            // Calculate the error between the descriptors.
            int const err = bitcount4(d1, d2);

            // Update best.
            if (err < be2) {
                be2 = err;
                bi2 = i2;
            }
        }

        // If no unused descriptors exist, return now.
        if (more == false)
            break;

        // Consume the best pairing found.
        used.at(i1)  = true;
        used.at(bi2) = true;

        // Record the pairing.
        cl_ushort2 const pair = {{i1, bi2}};
        pairs.push_back(pair);
    }
}

static cl_ulong4 blend(cl_ulong4 const & d1, cl_ulong4 const & d2) {
    cl_ulong4 const out = {{d1.x | d2.x, d1.y | d2.y, d1.z | d2.z, d1.w | d2.w}};
    return out;
}

static void blend(std::vector<cl_ulong4> const & ihips, std::vector<cl_ushort2> const & pairs, std::vector<cl_ulong4> & ohips) {
    // Number of HIPS descriptor pairs.
    size_t const npairs = pairs.size();

    // Allocate output.
    ohips.resize(npairs, HIPS_ZERO);

    for (size_t i = 0; i < npairs; i++) {
        // Refer to descriptor pairing and component descriptors.
        cl_ushort2 const & pair = pairs.at(i);
        cl_ulong4  const & d1   = ihips.at(pair.x);
        cl_ulong4  const & d2   = ihips.at(pair.y);

        // Calculate blended descriptor.
        cl_ulong4  const   d3   = blend(d1, d2);

        // Store blended descriptor.
        ohips.at(i) = d3;
    }
}

struct HipsTreeLevel {
    std::vector<cl_ulong4 > hips;
    std::vector<cl_ushort2> pairs;
};

size_t static const CELL_OFF = 32;

static void fillTree(std::vector<HipsTreeLevel> const & levels, std::vector<cl_ulong4> & tree, std::vector<cl_ushort> & maps, size_t ilevel, size_t inode, size_t icell) {
    // Refer to tree level.
    HipsTreeLevel const & level = levels.at(ilevel);

    // Refer to tree HIPS descriptor.
    cl_ulong4 & cell = tree.at(icell);

    // Check that the descriptor has not been filled.
    assert((cell.x | cell.y | cell.z | cell.w) == 0);

    // Fill descriptor at this level.
    cell = level.hips.at(inode);

    // If at lowest level, conclude.
    if (ilevel < 1) {
        assert(icell >= HipsTreeState::START);

        // Fill map at this level.
        maps.at(icell) = inode;
        return;
    } else {
        assert(icell <  HipsTreeState::START);
    }

    // Refer to pair constructing this descriptor.
    cl_ushort2 const & pair = level.pairs.at(inode);

    // Find child positions in next level.
    size_t const inode1 = pair.x;
    size_t const inode2 = pair.y;
    assert(inode1 != inode2);

    // Calculate indices in final tree.
    size_t const icell0 = ((icell + CELL_OFF) * 2);
    size_t const icell1 = ((icell0    ) - CELL_OFF);
    size_t const icell2 = ((icell0 + 1) - CELL_OFF);
    assert(icell1 != icell2);
    assert(icell1 >  icell);
    assert(icell2 >  icell);

    // Recurse into child positions.
    fillTree(levels, tree, maps, ilevel - 1, inode1, icell1);
    fillTree(levels, tree, maps, ilevel - 1, inode2, icell2);
}

void HipsMakeTreeStep::execute() {
    // Read descriptor list.
    std::vector<cl_ulong4> hips;
    i_hips.get(&hips);

#ifdef CVD_CL_VERBOSE
    std::cerr << "HIPS tree build started for " << std::setw(5) << hips.size() << " descriptors" << std::endl;
#endif

    // Truncate or extend HIPS list to exact leaf count.
    hips.resize(HipsTreeState::NLEAF, HIPS_ZERO);

#ifdef CVD_CL_VERBOSE
    std::cerr << "  Size set to " << std::setw(5) << hips.size() << " descriptors" << std::endl;
#endif

    // Create tree levels.
    std::vector<HipsTreeLevel> levels(HipsTreeState::LEVEL);

    // Populate lowest level (leaves) with original descriptors.
    // Pairings for this level are stored one level higher.
    levels.front().hips = hips;

    // Iteratively create pairings and blended descriptors.
    for (size_t ilevel = 1; ilevel < HipsTreeState::LEVEL; ilevel++) {
        HipsTreeLevel const & l1 = levels.at(ilevel - 1);
        HipsTreeLevel       & l2 = levels.at(ilevel    );

#ifdef CVD_CL_VERBOSE
        boost::system_time t1 = boost::get_system_time();
#endif

        // Create pairings, storing pairings on the next level.
        pairup(l1.hips, l2.pairs);

#ifdef CVD_CL_VERBOSE
        boost::system_time t2 = boost::get_system_time();
#endif

        // Create blended descriptors at the next level.
        blend(l1.hips, l2.pairs, l2.hips);

#ifdef CVD_CL_VERBOSE
        boost::system_time t3 = boost::get_system_time();

        int64_t const t_pairs = (t2 - t1).total_microseconds();
        int64_t const t_blend = (t3 - t2).total_microseconds();

        std::cerr << "  Level " << std::setw(2) << ilevel << ":      " << std::setw(9) << t_pairs << " us pairing, " << std::setw(9) << t_blend << " us blending" << std::endl;
#endif
    }

    // Verify dimensions at highest level (roots).
    HipsTreeLevel const & roots = levels.back();
    assert(levels.back().hips.size() == HipsTreeState::NROOT);

    // Create vector representing final tree.
    std::vector<cl_ulong4> tree(HipsTreeState::NNODE, HIPS_ZERO);

    // Create vector representing descriptor index maps.
    std::vector<cl_ushort> maps(HipsTreeState::NNODE, 0);

#ifdef CVD_CL_VERBOSE
    boost::system_time t1 = boost::get_system_time();
#endif

    // Recursively populate final tree.
    for (size_t inode = 0; inode < HipsTreeState::NROOT; inode++)
        fillTree(levels, tree, maps, HipsTreeState::LEVEL - 1, inode, inode);

#ifdef CVD_CL_VERBOSE
    boost::system_time t2 = boost::get_system_time();

    int64_t const t_fill = (t2 - t1).total_microseconds();

    std::cerr << "  Tree filled in " << std::setw(9) << t_fill <<  " us" << std::endl;
    std::cerr << std::endl;
#endif

    // Check that every descriptor was filled.
    for (size_t icell = 0; icell < HipsTreeState::NNODE; icell++) {
        // Refer to tree HIPS descriptor.
        cl_ulong4 const & cell = tree.at(icell);

        // Check that the descriptor has been filled.
        assert((cell.x | cell.y | cell.z | cell.w) > 0);
    }

    // Check that every position was used exactly once.
    std::set<cl_ushort> used;
    for (size_t icell = HipsTreeState::START; icell < HipsTreeState::NNODE; icell++) {
        cl_ushort const map = maps.at(icell);

        assert(used.count(map) == 0);
        used.insert(map);
        assert(used.count(map) == 1);
    }

    // Undo all previous effort by writing flat list.
    for (size_t inode = 0; inode < HipsTreeState::NLEAF; inode++) {
        //tree.at(inode + HipsTreeState::START) = hips.at(inode);
        //maps.at(inode + HipsTreeState::START) = inode;
    }

    // Write tree to device state.
    o_tree.tree.set(tree);

    // Write maps to device state.
    o_tree.maps.set(maps);
}

} // namespace CL
} // namespace CVD
