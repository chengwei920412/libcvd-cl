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

# Generate indices from -7 to +7 by 2 (-7, -5, ..., 5, 7)
I_16_2 = range(-7, 8, 2)

# Generate a grid of 2D coordinates.
OFFSETS = [
    (x, y)
    for y in I_16_2
    for x in I_16_2
]

# Expect exactly 64 coordinates.
assert (len(OFFSETS) == 64)

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

// Square an integer, for standard deviation calculation.
int sq(int x) {
    return (x * x);
}

// Shorthand for ulong cast.
#define L(x) ((ulong)(x))

#define FACTOR (0.675)

kernel void hips_gray(
    read_only image2d_t   image,
    global    int2      * corners,
    global    ulong4    * bins
) {

    // Prepare a suitable OpenCL image sampler.
    sampler_t const sampler = CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    // Use global work item as corner index.
    int  const ic  = get_global_id(0);
    int2 const xy  = corners[ic];

    // Read pixels in a grid around the corner pixel."""

for (shift, (x, y)) in enumerate(OFFSETS):
    print ("    int   const p%02d = read_imageui(image, sampler, xy + (int2)(%2d, %2d)).x;" % (shift + 1, x, y))

print

print "    // Calculate the sum of the pixel values."
print "    float const sum1 = ("
print " +\n".join([
    ("        p%02d" % (shift + 1))
    for (shift, _) in enumerate(OFFSETS)
])
print "    );"
print

print "    // Calculate the mean of the pixel values."
print "    float const mean = (sum1 / %d);" % len(OFFSETS)
print

print "    // Calculate the sum of squares of differences of the pixel values."
print "    float const sum2 = ("
print " +\n".join([
    ("        sq(p%02d)" % (shift + 1))
    for (shift, _) in enumerate(OFFSETS)
])
print "    );"
print

print "    // Calculate the standard deviation of the pixel values."
print "    float const dev  = (FACTOR * sqrt((sum2 / %d) - (sum1 / %d)));" % (len(OFFSETS), len(OFFSETS) * len(OFFSETS))
print
print "    // Calculate thresholds for standard deviation bins."
print "    int   const dev1 = (int)(mean - dev);"
print "    int   const dev2 = (int)(mean + dev);"
print
print "    // Bin all values lower than a standard deviation from the mean."
print "    ulong const  b1  = ("
print " |\n".join([
    ("        (L(p%02d < dev1) << L(%2d))" % (shift + 1, shift))
    for (shift, _) in enumerate(OFFSETS)
])
print "    );"
print

print "    // Bin all values higher than a standard deviation from the mean."
print "    ulong const  b4  = ("
print " |\n".join([
    ("        (L(p%02d > dev2) << L(%2d))" % (shift + 1, shift))
    for (shift, _) in enumerate(OFFSETS)
])
print "    );"
print

print "    // Bin all values lower than the mean but not a standard deviation."
print "    ulong const  b2  = ("
print " |\n".join([
    ("        (L(p%02d < mean) << L(%2d))" % (shift + 1, shift))
    for (shift, _) in enumerate(OFFSETS)
])
print "    );"
print

print "    // Bin all values higher than the mean but not a standard deviation."
print "    ulong const  b3  = ("
print " |\n".join([
    ("        (L(p%02d > mean) << L(%2d))" % (shift + 1, shift))
    for (shift, _) in enumerate(OFFSETS)
])
print "    );"

print """
    // Record in output buffer.
    // Use and-not to exclude known overlaps.
    bins[ic] = (ulong4)(b1, b2 & ~b1, b3 & ~b4, b4);
}
"""
