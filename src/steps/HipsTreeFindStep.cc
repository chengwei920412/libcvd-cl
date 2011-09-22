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

#include "cvd-cl/steps/HipsTreeFindStep.hh"
#include "kernels/hips-tfind.hh"

#include <iomanip>
#include <iostream>

namespace CVD {
namespace CL  {

HipsTreeFindStep::HipsTreeFindStep(HipsTreeState & i_tree, HipsListState & i_hips, PointListState & o_matches, cl_int maxerr) :
    WorkerStep (i_tree.worker),
    i_tree     (i_tree),
    i_hips     (i_hips),
    o_matches  (o_matches),
    maxerr     (maxerr)
{
    char opt[256] = {0,};
    snprintf(opt, sizeof(opt) - 1, "-DHIPS_MAX_ERROR=%d", int(maxerr));
    worker.compile(&program, &kernel, OCL_HIPS_TFIND, "hips_tree_find", opt);
}

HipsTreeFindStep::~HipsTreeFindStep() {
    // Do nothing.
}

template <typename T>
static int bitcount (T v){
  v = v - ((v >> 1) & (T)~(T)0/3);                           // temp
  v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);      // temp
  v = (v + (v >> 4)) & (T)~(T)0/255*15;                      // temp
  return (T)(v * ((T)~(T)0/255)) >> (sizeof(T) - 1) * CHAR_BIT; // count
}

static int error(cl_ulong4 t, cl_ulong4 r) {
    return
        bitcount(t.x &~ r.x) +
        bitcount(t.y &~ r.y) +
        bitcount(t.z &~ r.z) +
        bitcount(t.w &~ r.w);
}

size_t static const CELL_OFF = 32;

void HipsTreeFindStep::execute() {
    // Refer to HIPS tree and indices.
    std::vector<cl_ulong4> const & tree = i_tree.treeVector;
    std::vector<cl_ushort> const & maps = i_tree.mapsVector;

    // Read HIPS descriptors.
    std::vector<cl_ulong4> hips;
    i_hips.get(&hips);
    size_t const nhips = hips.size();

    // Record of matching points.
    // Reserve expected size.
    std::vector<cl_int2> matches(o_matches.size);

    // Count of points.
    uint32_t const maxmatches = matches.size();
    uint32_t nmatches = 0;

    boost::system_time t1 = boost::get_system_time();

    // Loop over HIPS descriptors with OpenMP.
    #pragma omp parallel for
    for (size_t ihashT = 0; ihashT < nhips; ihashT++) {
        cl_ulong4 const & hashT = hips.at(ihashT);

        // Try each tree root.
        for (size_t iroot = 0; iroot < 16; iroot++) {
            // Start traversal at root.
            size_t icell = (iroot + (CELL_OFF / 2));
            size_t last  = 10000;

            // Recurse exactly 5 levels deep (including first level).
            for (size_t idepth = 0; idepth < 5; idepth++) {
                // Calculate positions of both children.
                size_t const icell0 = (icell * 2);
                size_t const icell1 = (icell0    );
                size_t const icell2 = (icell0 + 1);

                // Refer to hashes for both children.
                cl_ulong4 const & hashR1 = tree.at(icell1 - CELL_OFF);
                cl_ulong4 const & hashR2 = tree.at(icell2 - CELL_OFF);

                // Calculate errors for both children.
                uint const err1 = error(hashT, hashR1);
                uint const err2 = error(hashT, hashR2);

                // Determine lower error.
                if (err1 < err2) {
                    last = err1;
                    icell = icell1;
                } else {
                    last = err2;
                    icell = icell2;
                }
            }

            // Record match if within error threshold.
            if (last <= (size_t) maxerr) {
                // Increment index with atomic intrinsics.
                uint32_t const count = __sync_add_and_fetch(&nmatches, 1);

                // Check against list fill.
                if (count <= maxmatches) {
                    // Read index.
                    cl_ushort const index = maps.at(icell - CELL_OFF);

                    // Pack match struct.
                    cl_int2 const pair = {{index, ihashT}};

                    // Record in output array.
                    matches.at(count - 1) = pair;
                }
            }
        }
    }

    boost::system_time t2 = boost::get_system_time();

    int64_t const t_find = (t2 - t1).total_microseconds();
    std::cerr << "CPU tree search in " << std::setw(9) << t_find << " us" << std::endl;

    // Write match list.
    o_matches.set(matches);

    // Truncate match list.
    o_matches.setCount(nmatches);
}

} // namespace CL
} // namespace CVD
