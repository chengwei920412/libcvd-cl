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

# 2D offsets from "corner" candidate pixel.
# Corresponds to lines 17-32 in fast_9_detect.cxx
OFFSETS = [
    ( 0,  3),
    ( 1,  3),
    ( 2,  2),
    ( 3,  1),
    ( 3,  0),
    ( 3, -1),
    ( 2, -2),
    ( 1, -3),
    ( 0, -3),
    (-1, -3),
    (-2, -2),
    (-3, -1),
    (-3,  0),
    (-3,  1),
    (-2,  2),
    (-1,  3),
]

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

int mask_test(uint x16) {
    // Duplicate bit pattern to simulate barrel shift.
    uint const x = (x16 | (x16 << 16));

    // Accumulator.
    uint x1 = x;

    // AND against down-shifts.
    #pragma unroll
    for (uint i = 1; i < FAST_RING; i++)
        x1 &= (x >> i);

    // Return of 1 here proves that FAST_RING
    // consecutive bits were 1.
    return (x1 > 0);
}

kernel void fast_gray(
    read_only  image2d_t   image,
    write_only image2d_t   scores,
    global     int2      * corners,
    global     int2      * filtered,
    global     int       * icorner,
               uint        ncorners
) {

    // Prepare a suitable OpenCL image sampler.
    sampler_t const sampler = CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    // Use global work item as corner index.
    int  const ic  = get_global_id(0);
    if (ic >= ncorners)
        return;

    int2 const xy  = corners[ic];

    // Read the candidate pixel.
    int  const p00 = read_imageui(image, sampler, xy).x;
"""

print "    // Read other pixels in a circle around the candidate pixel."""

for (shift, (x, y)) in enumerate(OFFSETS):
    print ("    int  const p%02d = read_imageui(image, sampler, xy + (int2)(%2d, %2d)).x;" % (shift + 1, x, y))
print

print "    // Calculate the absolute difference of each circle pixel."
for (shift, _) in enumerate(OFFSETS):
    print ("    int  const d%02d = abs(p%02d - p00);" % (shift + 1, shift + 1))
print

print "    // Select the maximum difference."
print "    int        sco = 0;"
for (shift, _) in enumerate(OFFSETS):
    print "               sco = max(sco, d%02d);" % (shift + 1)
print

print "    // Record maximum difference as score."
print "    write_imageui(scores, xy, (uint4)(sco, 0, 0, 0));"
print

print "    // Threshold the absolute difference of each circle pixel."
print "    int  const sum = ("
print " |\n".join([
    ("        ((d%02d > FAST_THRESH) << %2d)" % (shift + 1, shift))
    for shift in range(0, 16)
])

print "    );"
print

print """
    if (mask_test(sum)) {
        // Atomically append to corner buffer.
        int const icorn = atom_inc(icorner);
        if (icorn < FAST_COUNT)
            filtered[icorn] = xy;
    }
}
"""
