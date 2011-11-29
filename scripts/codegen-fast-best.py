#!/usr/bin/env python
#
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

kernel void fast_best(
    read_only  image2d_t   scores,
    global     int2      * corners,
    global     int2      * filtered,
    global     int       * icorner
) {

    // Prepare a suitable OpenCL image sampler.
    sampler_t const sampler = CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    // Use global work item as 1D offset into corners.
    int  const ic  = get_global_id(0);
    int2 const xy  = corners[ic];

    // Read the candidate score.
    int  const p00 = read_imageui(scores, sampler, xy).x;

    // Read other scores in a tight square around the candidate score."""

OFFSETS = [
    (x, y)
    for y in [-1, 0, 1]
    for x in [-1, 0, 1]
]

for (shift, (x, y)) in enumerate(OFFSETS):
    print ("    int  const p%02d = read_imageui(scores, sampler, xy + (int2)(%2d, %2d)).x;" % (shift + 1, x, y))
print

print "    // Select the maximum score."
print "    int        sco = p00;"
for (shift, _) in enumerate(OFFSETS):
    print "               sco = max(sco, p%02d);" % (shift + 1)

print """
    // Keep this score if it is as good as the maximum.
    if (p00 >= sco) {
        // Atomically append to filtered corner buffer.
        filtered[atom_inc(icorner)] = xy;
    }
}
"""
