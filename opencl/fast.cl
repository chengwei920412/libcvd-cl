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

// Create a bitwise mask with the given rotation, modulo 16.
#define MASK(x) ((((1 << 9) - 1) << (x)) | (((1 << 9) - 1) >> (15 - (x))))

kernel void fast_gray_9(
    read_only image2d_t   image,
    global    int2      * corners,
    global    int       * icorner
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

    // Threshold the absolute difference of each circle pixel.
    int  const sum = (
        ((abs(p01 - p00) > THRESH) <<  0) |
        ((abs(p02 - p00) > THRESH) <<  1) |
        ((abs(p03 - p00) > THRESH) <<  2) |
        ((abs(p04 - p00) > THRESH) <<  3) |
        ((abs(p05 - p00) > THRESH) <<  4) |
        ((abs(p06 - p00) > THRESH) <<  5) |
        ((abs(p07 - p00) > THRESH) <<  6) |
        ((abs(p08 - p00) > THRESH) <<  7) |
        ((abs(p09 - p00) > THRESH) <<  8) |
        ((abs(p10 - p00) > THRESH) <<  9) |
        ((abs(p11 - p00) > THRESH) << 10) |
        ((abs(p12 - p00) > THRESH) << 11) |
        ((abs(p13 - p00) > THRESH) << 12) |
        ((abs(p14 - p00) > THRESH) << 13) |
        ((abs(p15 - p00) > THRESH) << 14) |
        ((abs(p16 - p00) > THRESH) << 15)
    );

    // Check if at least one mask applies entirely.
    int  const yes = (
        ((sum & MASK( 0)) == MASK( 0)) ||
        ((sum & MASK( 1)) == MASK( 1)) ||
        ((sum & MASK( 2)) == MASK( 2)) ||
        ((sum & MASK( 3)) == MASK( 3)) ||
        ((sum & MASK( 4)) == MASK( 4)) ||
        ((sum & MASK( 5)) == MASK( 5)) ||
        ((sum & MASK( 6)) == MASK( 6)) ||
        ((sum & MASK( 7)) == MASK( 7)) ||
        ((sum & MASK( 8)) == MASK( 8)) ||
        ((sum & MASK( 9)) == MASK( 9)) ||
        ((sum & MASK(10)) == MASK(10)) ||
        ((sum & MASK(11)) == MASK(11)) ||
        ((sum & MASK(12)) == MASK(12)) ||
        ((sum & MASK(13)) == MASK(13)) ||
        ((sum & MASK(14)) == MASK(14)) ||
        ((sum & MASK(15)) == MASK(15))
    );

    if (yes) {
        // Atomically append to corner buffer.
        corners[atom_inc(icorner)] = xy;
    }
}

