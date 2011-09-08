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

#include <cuda.h>

#include <iostream>

namespace CVD {
namespace CL  {

#define THRESHOLD 3

// Parallel bit counting magic adapted from
// http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
__device__ uint bitcount32(uint v) {
    v = (v - ((v >> 1) & 0x55555555));
    v = ((v & 0x33333333) + ((v >> 2) & 0x33333333));
    v = ((((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24);
    return v;
}

__device__ uint bitcount64(ulong v) {
    uint const lo = (uint)(v);
    uint const hi = (uint)(v >> 32UL);
    return bitcount32(lo) + bitcount32(hi);
}

__device__ uint bitcount64_4(ulong4 t, ulong4 r) {
    return bitcount64(t.x & ~r.x) + bitcount64(t.y & ~r.y) + bitcount64(t.z & ~r.z) + bitcount64(t.w & ~r.w);
}

__global__ void hips_find_kernel(
    // N.B.: These uint8 are actually ulong4.
    ulong4 const * hashes1,  // T
    ulong4 const * hashes2,  // R
    int2         * matches,  // Pairs of indices into hashes1 and hashes2.
    uint         * imatch,   // Output number of hash1 matches.
    uint           nmatch    // Maximum number of matches.
) {

    // Use global work items for hash1, hash2 indices.
    uint   const ihash1  = (blockIdx.x * blockDim.x + threadIdx.x);
    uint   const ihash2  = (blockIdx.y * blockDim.y + threadIdx.y);

    // Read hashes.
    ulong4 const hash1   = hashes1[ihash1];
    ulong4 const hash2   = hashes2[ihash2];

    // Calculate error.
    uint   const error   = bitcount64_4(hash1, hash2);

    // Record match if within error threshold.
    if (error <= THRESHOLD) {
        uint const i = atomicAdd(imatch, 1);
        if (i < nmatch) {
            matches[i] = make_int2(ihash1, ihash2);
        }
    }
}

void hips_find(
    ulong4 const * hashes1,  // T
    ulong4 const * hashes2,  // R
    int2         * matches,  // Pairs of indices into hashes1 and hashes2.
    uint         * imatch,   // Output number of hash1 matches.
    uint           nmatch,   // Maximum number of matches.
    uint           np1,
    uint           np2
) {

    // Divide number of descriptors.
    unsigned int const np1_16 = (np1 / 16);
    unsigned int const np2_16 = (np2 / 16);

    // Prepare work sizes.
    dim3 blockDim(16, 16, 1);
    dim3 gridDim(np1_16, np2_16, 1);

    // Run kernel.
    std::cerr << "running kernel" << std::endl;
    hips_find_kernel<<<gridDim, blockDim>>>(hashes1, hashes2, matches, imatch, nmatch);
    std::cerr << "ran kernel" << std::endl;
}

} // namespace CL
} // namespace CVD
