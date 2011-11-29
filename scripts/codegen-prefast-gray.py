#!/usr/bin/env python
#
# Copyright (C) 2011  Dmitri Nikulin
# Copyright (C) 2011  Monash University
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
# Corresponds to SUBSET OF lines 17-32 in fast_9_detect.cxx
OFFSETS = [
    ( 0,  3),
    ( 3,  0),
    ( 0, -3),
    (-3,  0),
]

print """// Copyright (C) 2011  Dmitri Nikulin
// Copyright (C) 2011  Monash University
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

kernel void prefast_gray(
    read_only  image2d_t   image,
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
    int  const p00 = read_imageui(image, sampler, xy).x;
"""

print "    // Read other pixels in a circle around the candidate pixel."""

for (shift, (x, y)) in enumerate(OFFSETS):
    print ("    int  const p%02d = read_imageui(image, sampler, xy + (int2)(%2d, %2d)).x;" % (shift + 1, x, y))
print

print "    // Check the absolute difference of each circle pixel."
for (shift, _) in enumerate(OFFSETS):
    print ("    int  const d%02d = (abs(p%02d - p00) > FAST_THRESH);" % (shift + 1, shift + 1))
print

print "    // Check if any two adjacent circle pixels have a high absolute difference."
print "    int  const yes = ("
print " ||\n".join([
    ("        (d%02d && d%02d)" % (shift + 1, ((shift + 1) % len(OFFSETS)) + 1))
    for (shift, _) in enumerate(OFFSETS)
])
print """    );

    if (yes) {
        // Atomically append to corner buffer.
        int const icorn = atom_inc(icorner);
        if (icorn < FAST_COUNT)
            corners[icorn] = xy;
    }
}
"""
