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
    global    ulong4   * hashes1,  // T
    global    ulong4   * hashes2,  // R
    global    int      * ihashes2  // For each hash1, index of best hash2.
) {

    // Prepare local memory for error and ihash2.
    local  int   errors  [512];
    local  int   ihashes [512];

    // Use global work item as hash1, hash2 index.
    int    const ihash1  = get_global_id(0);
    int    const ihash2  = get_global_id(1);

    // Use local work item for indexing into errors and hashes.
    int    const ithread = get_local_id(1);

    // Cache first hash.
    local  ulong4 hash1;
    if (ithread == 0) {
        hash1            = hashes1[ihash1];
    }

    // Synchronise work group.
    barrier(CLK_LOCAL_MEM_FENCE);

    // Take hash2 for comparison.
    ulong4 const hash2   = hashes2[ihash2];

    // Calculate number of bits in error.
    int    const error   = bitcount4(hash1 & ~hash2);

    // Initialise local memory.
    errors  [ithread]    = error;
    ihashes [ithread]    = ihash2;

    // Synchronise work group.
    barrier(CLK_LOCAL_MEM_FENCE);

    // Prepare state for parallel reduction.
    int te1              = error;
    int ti1              = ihash2;

    // Perform parallel reduction.
    for (int width = 256; width > 1; width >>= 1) {
        if (ithread < width) {
            int const te2  = errors  [ithread + width];
            int const ti2  = ihashes [ithread + width];

            if (te2 < te1) {
                // Update with lower error.
                errors  [ithread] = te1 = te2;
                ihashes [ithread] = ti1 = ti2;
            }
        }

        // Synchronise after this round of reduction.
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    if (ithread == 0) {
        ihashes2[ihash1] = ti1;
    }
}
"""
