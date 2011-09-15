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

#define THRESHOLD  (0.01f)
#define THRESHOLD2 (THRESHOLD * THRESHOLD)

float sq(float x) {
    return (x * x);
}

kernel void se3_score(
    global float const * u1s,
    global float const * v1s,
    global float const * q1s,
    global float const * u2s,
    global float const * v2s,
    global float const * mats,
    global float       * scores,
           int   const   nsets
) {

    // Use global work item as matrix index.
    int const imatrix   = get_global_id(0);
    int const nmatrices = get_global_size(0);

    // Read entire matrix."""

for row in range(3):
    for col in range(4):
        factor = ((row * 4) + col)
        print "    float const r%dc%d = mats[mad24(%2d, nmatrices, imatrix)];" % (row, col, factor)

print """
    // Keep score for a single matrix.
    float score = 0;

    // Loop over all correspondence sets.
    for (int iset = 0; iset < nsets; iset++) {
        // Read out correspondence set elements.
        float const u1 = u1s[iset];
        float const v1 = v1s[iset];
        float const q1 = q1s[iset];
        float const u2 = u2s[iset];
        float const v2 = v2s[iset];

        // Prepare 4-vector for uvq.
        float const a0 = u1;
        float const a1 = v1;
        float const a2 =  1;
        float const a3 = q1;

        // Multiply through matrix.
        float const b0 = ((a0 * r0c0) + (a1 * r0c1) + (a2 * r0c2) + (a3 * r0c3));
        float const b1 = ((a0 * r1c0) + (a1 * r1c1) + (a2 * r1c2) + (a3 * r1c3));
        float const b2 = ((a0 * r2c0) + (a1 * r2c1) + (a2 * r2c2) + (a3 * r2c3));
        // Last row is unused.

        // Divide transformed (u,v).
        float const u3 = (b0 / b2);
        float const v3 = (b1 / b2);

        // Calculate error from actual transformed (u,v).
        float const error = (sq(u3 - u2) + sq(v3 - v2));

        // Contribute error towards score.
        score += max((1.0f - (error / THRESHOLD2)), 0.0f);
    }

    // Record matrix score.
    scores[imatrix] = score;
}
"""
