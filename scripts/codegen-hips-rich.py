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

# Grid of 2D coordinates, forming a circle.
# Corresponds to lines 42-60 in featurepatch.cc
OFFSETS = [
  ( 3,  0), ( 6, -1), ( 8,  0), ( 9, -2),
  ( 3, -1), ( 5, -3), ( 7, -3), ( 8, -5),
  ( 2, -2), ( 3, -5), ( 5, -5), ( 5, -8),
  ( 1, -3), ( 1, -6), ( 3, -7), ( 2, -9),

  ( 0, -3), (-1, -6), ( 0, -8), (-2, -9),
  (-1, -3), (-3, -5), (-3, -7), (-5, -8),
  (-2, -2), (-5, -3), (-5, -5), (-8, -5),
  (-3, -1), (-6, -1), (-7, -3), (-9, -2),

  (-3,  0), (-6,  1), (-8,  0), (-9,  2),
  (-3,  1), (-5,  3), (-7,  3), (-8,  5),
  (-2,  2), (-3,  5), (-5,  5), (-5,  8),
  (-1,  3), (-1,  6), (-3,  7), (-2,  9),

  ( 0,  3), ( 1,  6), ( 0,  8), ( 2,  9),
  ( 1,  3), ( 3,  5), ( 3,  7), ( 5,  8),
  ( 2,  2), ( 5,  3), ( 5,  5), ( 8,  5),
  ( 3,  1), ( 6,  1), ( 7,  3), ( 9,  2),
]

# Expect exactly 64 coordinates.
assert (len(OFFSETS) == 64)

IS_NOT = [" ", "~"]

CHOICES = [(XI, YI, ZI) for XI in IS_NOT for YI in IS_NOT for ZI in IS_NOT]

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

// Shorthand for ulong cast.
#define L(x) ((ulong)(x))

kernel void hips_rich(
    read_only image2d_t   image,
    global    int2      * corners,
    global    ulong4    * bins,
              int2        offset
) {

    // Prepare a suitable OpenCL image sampler.
    sampler_t const sampler = CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    // Use global work item as corner index.
    int    const ic  = get_global_id(0);
    int2   const xy  = corners[ic];

    // Read pixels in a grid around the corner pixel."""

for (shift, (x, y)) in enumerate(OFFSETS, 1):
    print ("    uint4  const p%02d = read_imageui(image, sampler, xy + (int2)(%2d, %2d));" % (shift, x, y))

print

print "    // Calculate the sum of the pixel values."
print "    uint4  const sum  = ("
print " +\n".join([
    ("        p%02d" % shift)
    for (shift, _) in enumerate(OFFSETS, 1)
])
print "    );"
print

print "    // Calculate the mean of the pixel values."
print "    uint4  const mean = (sum / %d);" % len(OFFSETS)
print

print "    // Pack a 'greaterness' row per colour element."
for colour in ("x", "y", "z"):
    print "    ulong g%s = (" % colour
    print " |\n".join([
        ("        (L(p%02d.%s > mean.%s) << L(%2d))" % (shift, colour, colour, shift - 1))
        for (shift, _) in enumerate(OFFSETS, 1)
    ])
    print "    );"
    print

print "    // Populate elements of the output descriptor from the 8 (g.x, g.y, g.z) combinations."
print "    ulong4 hash;"
for (bin, (xi, yi, zi)) in enumerate(CHOICES[:4]):
    print "    hash.s%d = ((%sgx) & (%sgy) & (%sgz));" % (bin, xi, yi, zi)

print """
    // Record in output buffer.
    bins[ic] = hash;
}
"""
