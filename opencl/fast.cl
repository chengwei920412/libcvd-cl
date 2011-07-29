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

// Enable OpenCL 32-bit integer atomic functions.
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

// Specify a threshold for pixel difference.
#define THRESH  60

// Generate bitwise mask of n bits.
int mask(int n) {
    return ((1 << n) - 1);
}

// Create a bitwise mask of n bits, rotated by r bits, in a ring of w bits.
int mask_turn(int n, int w, int r) {
    int const m = mask(n);
    return (((m << r) | (m >> (w - r))) & mask(w));
}

// Test a value x against a bitwise mask of n bits, rotated by r bits, in a ring of w bits.
int mask_test(int x, int n, int w, int r) {
    int const m = mask_turn(n, w, r);
    return ((x & m) == m);
}

int mask_test_9_16(int x, int r) {
    return mask_test(x, 9, 16, r);
}

kernel void fast_gray_9(
    read_only  image2d_t   image,
    write_only image2d_t   scores,
    global     int2      * corners,
    global     int       * icorner
) {

    // Prepare a suitable OpenCL image sampler.
    sampler_t const sampler = CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    // Use global work item as 2D image coordinates.
    int  const x   = get_global_id(0);
    int  const y   = get_global_id(1);
    int2 const xy  = (int2)(x, y);

    // Read the candidate pixel.
    int  const p00 = read_imagei(image, sampler, xy).x;

    // Read other pixels in a circle around the candidate pixel.
    int  const p01 = read_imagei(image, sampler, xy + (int2)( 0,  3)).x;
    int  const p02 = read_imagei(image, sampler, xy + (int2)( 1,  3)).x;
    int  const p03 = read_imagei(image, sampler, xy + (int2)( 2,  2)).x;
    int  const p04 = read_imagei(image, sampler, xy + (int2)( 3,  1)).x;
    int  const p05 = read_imagei(image, sampler, xy + (int2)( 3,  0)).x;
    int  const p06 = read_imagei(image, sampler, xy + (int2)( 3, -1)).x;
    int  const p07 = read_imagei(image, sampler, xy + (int2)( 2, -2)).x;
    int  const p08 = read_imagei(image, sampler, xy + (int2)( 1, -3)).x;
    int  const p09 = read_imagei(image, sampler, xy + (int2)( 0, -3)).x;
    int  const p10 = read_imagei(image, sampler, xy + (int2)(-1, -3)).x;
    int  const p11 = read_imagei(image, sampler, xy + (int2)(-2, -2)).x;
    int  const p12 = read_imagei(image, sampler, xy + (int2)(-3, -1)).x;
    int  const p13 = read_imagei(image, sampler, xy + (int2)(-3,  0)).x;
    int  const p14 = read_imagei(image, sampler, xy + (int2)(-3,  1)).x;
    int  const p15 = read_imagei(image, sampler, xy + (int2)(-2,  2)).x;
    int  const p16 = read_imagei(image, sampler, xy + (int2)(-1,  3)).x;

    // Calculate the absolute difference of each circle pixel.
    int  const d01 = abs(p01 - p00);
    int  const d02 = abs(p02 - p00);
    int  const d03 = abs(p03 - p00);
    int  const d04 = abs(p04 - p00);
    int  const d05 = abs(p05 - p00);
    int  const d06 = abs(p06 - p00);
    int  const d07 = abs(p07 - p00);
    int  const d08 = abs(p08 - p00);
    int  const d09 = abs(p09 - p00);
    int  const d10 = abs(p10 - p00);
    int  const d11 = abs(p11 - p00);
    int  const d12 = abs(p12 - p00);
    int  const d13 = abs(p13 - p00);
    int  const d14 = abs(p14 - p00);
    int  const d15 = abs(p15 - p00);
    int  const d16 = abs(p16 - p00);

    // Select the maximum difference.
    int        sco = 0;
               sco = max(sco, d01);
               sco = max(sco, d02);
               sco = max(sco, d03);
               sco = max(sco, d04);
               sco = max(sco, d05);
               sco = max(sco, d06);
               sco = max(sco, d07);
               sco = max(sco, d08);
               sco = max(sco, d09);
               sco = max(sco, d10);
               sco = max(sco, d11);
               sco = max(sco, d12);
               sco = max(sco, d13);
               sco = max(sco, d14);
               sco = max(sco, d15);
               sco = max(sco, d16);

    // Record maximum difference as score.
    write_imageui(scores, xy, (uint4)(sco, 0, 0, 0));

    // Threshold the absolute difference of each circle pixel.
    int  const sum = (
        ((d01 > THRESH) <<  0) |
        ((d02 > THRESH) <<  1) |
        ((d03 > THRESH) <<  2) |
        ((d04 > THRESH) <<  3) |
        ((d05 > THRESH) <<  4) |
        ((d06 > THRESH) <<  5) |
        ((d07 > THRESH) <<  6) |
        ((d08 > THRESH) <<  7) |
        ((d09 > THRESH) <<  8) |
        ((d10 > THRESH) <<  9) |
        ((d11 > THRESH) << 10) |
        ((d12 > THRESH) << 11) |
        ((d13 > THRESH) << 12) |
        ((d14 > THRESH) << 13) |
        ((d15 > THRESH) << 14) |
        ((d16 > THRESH) << 15)
    );

    // Check if at least one mask applies entirely.
    int  const yes = (
        mask_test_9_16(sum,  0) ||
        mask_test_9_16(sum,  1) ||
        mask_test_9_16(sum,  2) ||
        mask_test_9_16(sum,  3) ||
        mask_test_9_16(sum,  4) ||
        mask_test_9_16(sum,  5) ||
        mask_test_9_16(sum,  6) ||
        mask_test_9_16(sum,  7) ||
        mask_test_9_16(sum,  8) ||
        mask_test_9_16(sum,  9) ||
        mask_test_9_16(sum, 10) ||
        mask_test_9_16(sum, 11) ||
        mask_test_9_16(sum, 12) ||
        mask_test_9_16(sum, 13) ||
        mask_test_9_16(sum, 14) ||
        mask_test_9_16(sum, 15)
    );

    if (yes) {
        // Atomically append to corner buffer.
        corners[atom_inc(icorner)] = xy;
    }
}

