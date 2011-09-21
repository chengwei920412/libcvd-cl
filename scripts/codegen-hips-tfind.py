#!/usr/bin/env python
#
# Copyright (C) 2011  Dmitri Nikulin, Monash University
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

print """// Copyright (C) 2011  Dmitri Nikulin, Monash University
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

// Enable OpenCL 32-bit integer atomic functions.
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

// Parallel bit counting magic adapted from
// http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
uint bitcount8(uint8 v) {
    v = (v - ((v >> 1) & 0x55555555));
    v = ((v & 0x33333333) + ((v >> 2) & 0x33333333));
    v = (((v + (v >> 4) & 0xF0F0F0F) * 0x1010101) >> 24);
    return (v.s0 + v.s1 + v.s2 + v.s3 + v.s4 + v.s5 + v.s6 + v.s7);
}

uint error(uint8 t, uint8 r) {
    return bitcount8(t & ~r);
}

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

#define CELL_OFF 32

kernel void hips_tree_find(
    // N.B.: These uint8 are actually ulong4.
    constant uint8        * hashes1,  // R (forest of descriptors, as above)
    constant ushort       * indices,  // Original index of each hash in R, defined only for leaves.
    global   uint8  const * hashes2,  // T (list of descriptors)
    global   uint2        * matches,  // Pairs of indices into hashes1 and hashes2.
    global   uint         * imatch,   // Output number of hash1 matches.
             uint           nmatch    // Maximum number of matches.
) {

    // Use global work item for hash2 index.
    uint   const ihash2 = get_global_id(0);
    uint8  const  hash2 = hashes2[ihash2];

    // Loop over 16 roots.
    #pragma unroll
    for (uint iroot = 0; iroot < 16; iroot++) {
        // Start traversal at root.
        uint icell = (iroot + (CELL_OFF / 2));
        uint last  = 10000;

        // Recurse exactly 5 levels deep (including first level).
        #pragma unroll
        for (uint idepth = 0; idepth < 5; idepth++) {
            // Calculate positions of both children.
            uint const icell0 = (icell * 2);
            uint const icell1 = (icell0    );
            uint const icell2 = (icell0 + 1);

            // Calculate errors for both children.
            uint const erra = error(hash2, hashes1[icell1 - CELL_OFF]);
            uint const errb = error(hash2, hashes1[icell2 - CELL_OFF]);

            // Determine lower error.
            last = min(erra, errb);

            // Keep child with lower error.
            icell = select(icell1, icell2, erra > errb);
        }

        // Record match if within error threshold.
        if (last <= HIPS_MAX_ERROR) {
            uint const i = atom_inc(imatch);
            if (i < nmatch) {
                // Store pair against original index.
                matches[i] = (uint2)(indices[icell - CELL_OFF], ihash2);
            }
        }
    }
}"""
