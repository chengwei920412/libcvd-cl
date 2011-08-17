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

int bitcount(ulong x) {
    // http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetTable
    uchar const bits[256] = {
#   define B2(n)     n,     n+1,     n+1,     n+2
#   define B4(n)  B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n)  B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
    };

    uchar8 const uc = as_uchar8(x);

    return (
        bits[uc.s0] +
        bits[uc.s1] +
        bits[uc.s2] +
        bits[uc.s3] +
        bits[uc.s4] +
        bits[uc.s5] +
        bits[uc.s6] +
        bits[uc.s7]
    );
}

int bitcount4(ulong4 v) {
    return (
        bitcount(v.x) +
        bitcount(v.y) +
        bitcount(v.z) +
        bitcount(v.w)
    );
}

kernel void hips_find(
    global ulong4 const * hashes1,  // T
    global ulong4 const * hashes2,  // R
    global int2   const * ixy2,     // For each hash2, its coordinate.
    global int2         * oxy2      // For each hash1, coordinate of its best hash2.
) {

    // Prepare local memory for hash caching.
    local ulong4 cache1  [16];
    local ulong4 cache2  [16];
    local int    errors [256];
    local int    idxs   [256];
    local int    ebest   [16];
    local int    ibest   [16];

    // Use global work item for dimension 1 as hash1 index.
    // Global work item for dimension 2 is *unused*.
    int    const ihash1  = get_global_id(0);

    // Use local work items for indexing into errors and hashes.
    int    const ithr1   = get_local_id(0);
    int    const ithr2   = get_local_id(1);

    // Cache from hash1.
    if (ithr2 == 0) {
        cache1 [ithr1]    = hashes1[ihash1];
        ebest  [ithr1]    = 0xffff;
        ibest  [ithr1]    = 0;
    }

    // Loop while consuming hash2.
    for (int offset = 0; offset < 256; offset += 16) {
        // Cache from hash2.
        if (ithr2 == 0) {
            cache2[ithr1]  = hashes2[offset + ithr1];
        }

        // Synchronise work group.
        barrier(CLK_LOCAL_MEM_FENCE);

        // Calculate pairwise error.
        ulong4 const hash1 = cache1[ithr1];
        ulong4 const hash2 = cache2[ithr2];
        int    const cell  = mad24(ithr1, 16, ithr2);
        errors [cell]      = bitcount4(hash1 & ~hash2);
        idxs   [cell]      = offset + ithr2;

        // Synchronise work group.
        barrier(CLK_LOCAL_MEM_FENCE);

        // Prepare state for parallel reduction.
        int te1            = ebest [ithr1];
        int ti1            = ibest [ithr1];

        // Perform parallel reduction.
        for (int width = 8; width > 1; width >>= 1) {
            if (ithr2 < width) {
                int const te2  = errors [cell + width];
                int const ti2  = idxs   [cell + width];

                if (te2 < te1) {
                    // Update with lower error.
                    errors [cell] = te1 = te2;
                    idxs   [cell] = ti1 = ti2;
                }
            }

            // Synchronise after this round of reduction.
            barrier(CLK_LOCAL_MEM_FENCE);
        }

        if (ithr2 == 0) {
            ebest [ithr1] = te1;
            ibest [ithr1] = ti1;
        }
    }

    // Synchronise work group.
    barrier(CLK_LOCAL_MEM_FENCE);

    if (ithr2 == 0) {
        oxy2[ihash1] = ixy2[ibest[ithr1]];
    }
}
"""
