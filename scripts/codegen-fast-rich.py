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

// Calculate vector distance as a float.
float dist4(uint4 a, uint4 b) {
    return distance(convert_float4(a), convert_float4(b));
}

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

kernel void fast_rich(
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
    int    const ic  = get_global_id(0);
    if (ic >= ncorners)
        return;

    int2   const xy  = corners[ic];

    // Read the candidate pixel.
    uint4  const p00 = read_imageui(image, sampler, xy);
"""

print "    // Read other pixels in a circle around the candidate pixel."""

for (shift, (x, y)) in enumerate(OFFSETS):
    print ("    uint4  const p%02d = read_imageui(image, sampler, xy + (int2)(%2d, %2d));" % (shift + 1, x, y))
print

print "    // Calculate the absolute difference of each circle pixel."
for (shift, _) in enumerate(OFFSETS):
    print ("    float  const d%02d = dist4(p%02d, p00);" % (shift + 1, shift + 1))
print

print "    // Select the maximum difference."
print "    float        sco = 0;"
for (shift, _) in enumerate(OFFSETS):
    print "                 sco = max(sco, d%02d);" % (shift + 1)
print

print "    // Record maximum difference as score."
print "    write_imageui(scores, xy, (uint4)((uint) sco, 0, 0, 0));"
print

print "    // Threshold the absolute difference of each circle pixel."
print "    int    const sum = ("
print " |\n".join([
    ("        ((d%02d > FAST_THRESH) << %2d)" % (shift + 1, shift))
    for shift in range(0, 16)
])

print "    );"
print

print "    // Check if at least one mask applies entirely."
print "    int    const yes = ("
print " ||\n".join([
    ("        mask_test(sum, 9, 16, %2d)" % (shift))
    for shift in range(0, 16)
])
print "    );"

print """
    if (yes) {
        // Atomically append to corner buffer.
        int const icorn = atom_inc(icorner);
        if (icorn < FAST_COUNT)
            filtered[icorn] = xy;
    }
}
"""
