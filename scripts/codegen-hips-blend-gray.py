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

# Generate indices from -8 to +8 inclusive.
INDICES = range(-8, 9)

def xs(x):
    return ("%2d" % x).replace("-", "m").replace(" ", "p")

def xys(x, y):
    return xs(x) + xs(y)

COORDS = [
    (x, y, xys(x, y))
    for x in INDICES
    for y in INDICES
]

# Expect exactly 289 (17*17) coordinates.
assert (len(COORDS) == 289)

# Generate indices from -7 to +7 by 2 (-7, -5, ..., 5, 7)
I_16_2 = range(-7, 8, 2)

# Generate a grid of 2D coordinates.
OFFSETS = [
    (x, y, xys(x, y))
    for y in I_16_2
    for x in I_16_2
]

# Expect exactly 64 offsets.
assert (len(OFFSETS) == 64)

# Generate indices from -1 to +1.
I_3 = range(-1, 2)

BOX = [
    (x, y)
    for y in I_3
    for x in I_3
]

# Expect exactly 9 offsets in the box.
# assert (len(BOX) == 9)

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

// Square an integer, for standard deviation calculation.
int sq(int x) {
    return (x * x);
}

// Shorthand for ulong cast.
#define L(x) ((ulong)(x))

#define FACTOR (0.675f)

// Maximum bits set in descriptor.
#define MAXBITS (100)

kernel void hips_blend_gray(
    read_only image2d_t   image,
    global    int2      * corners,
    global    ulong4    * bins,
    global    int       * ibin,
              int         nbins
) {

    // Prepare a suitable OpenCL image sampler.
    sampler_t const sampler = CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    // Use global work item as corner index.
    int  const ic  = get_global_id(0);
    int2 const xy  = corners[ic];

    // Read pixels in a grid around the corner pixel."""

for (x, y, name) in COORDS:
    print ("    int   const v_%s = read_imageui(image, sampler, xy + (int2)(%2d, %2d)).x;" % (name, x, y))

print

print "    // Calculate the sum of the pixel values."
print "    int   const sum1 = ("
print " +\n".join([
    ("        v_%s" % name)
    for (_, _, name) in COORDS
])
print "    );"
print

print "    // Calculate the mean of the pixel values."
print "    int   const mean = (sum1 / %d);" % len(COORDS)
print

print "    // Calculate the sum of squares of differences of the pixel values."
print "    float const sum2 = ("
print " +\n".join([
    ("        sq(v_%s - mean)" % name)
    for (_, _, name) in COORDS
])
print "    );"
print

print "    // Calculate the standard deviation of the pixel values."
print "    float const dev  = (FACTOR * sqrt(sum2 / %d));" % len(COORDS)
print
print "    // Calculate thresholds for standard deviation bins."
print "    int   const dev1 = (int)(mean - dev);"
print "    int   const dev2 = (int)(mean + dev);"
print
print "    // Select one bit per pixel."
for (_, _, name) in COORDS:
    print "    int   const b1_%s =  (v_%s < dev1);" % (name, name)
    print "    int   const b4_%s =  (v_%s > dev2);" % (name, name)
    print "    int   const b2_%s = ((v_%s < mean) &~ b1_%s);" % (name, name, name)
    print "    int   const b3_%s = ((v_%s > mean) &~ b4_%s);" % (name, name, name)
print
print "    // Combine grids of pixels by offsets."
for (x, y, name) in OFFSETS:
    for i in range(1, 5):
        parts = " | ".join([
            ("b%d_%s" % (i, xys(x + bx, y + by)))
            for (bx, by) in BOX
        ])
        print "    int   const o%d_%s = (%s);" % (i, name, parts)
print
print "    // Combine into 64-bit integers."
for i in range(1, 5):
    print "    ulong const l%d = (" % i
    print " |\n".join([
        ("        (L(o%d_%s) << L(%2d))" % (i, name, shift))
        for (shift, (_, _, name)) in enumerate(OFFSETS)
    ])
    print "    );"
    print

print "    // Combine into 4-vector."
print "    ulong4 const vec = (ulong4)(l1, l2, l3, l4);"
print
print "    // Count bits."
print "    if (bitcount8(as_uint8(vec)) <= MAXBITS) {"
print "        int const i = atom_inc(ibin);"
print "        if (i < nbins)"
print "            bins[i] = vec;"
print "    }"
print "}"
