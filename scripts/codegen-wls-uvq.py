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

float sq(float x) {
    return (x * x);
}

kernel void wls_uvq(
    global float const * u1s,
    global float const * v1s,
    global float const * q1s,
    global float const * u2s,
    global float const * v2s,
    global float const * Ms,
    global float       * As,
    global float       * bs
) {

    // Use global work item as correspondence set index.
    int const iset   = get_global_id(0);
    int const nsets  = get_global_size(0);

    // Read initial transformation matrix."""

for row in range(4):
    for col in range(4):
        factor = ((row * 4) + col)
        core   = ("r%dc%d" % (row, col))
        print "    float const %sm = Ms[(%2d * nsets) + iset];" % (core, factor)

print """
    // Prepare 6 vector elements."""

for row in range(6):
    print "    float b%d   = 0;" % row

print """
    // Prepare 6x6 matrix elements, top-right only."""

for row in range(6):
    for col in range(row, 6):
        print "    float r%dc%d = 0;" % (row, col)

print """
    #pragma unroll
    for (int ipair = 0; ipair < 3; ipair++) {
        // Read coordinate pair elements.
        int   const of = mad24(ipair, nsets, iset);

        // Vector for (uv1q).
        float const a0 = u1s[of];
        float const a1 = v1s[of];
        float const a2 =  1;
        float const a3 = q1s[of];

        // Vector for (uv).
        float const u2 = u2s[of];
        float const v2 = v2s[of];

        // Vector for transformed (uv1q).
        float const x0 = ((a0 * r0c0m) + (a1 * r0c1m) + (a2 * r0c2m) + (a3 * r0c3m));
        float const x1 = ((a0 * r1c0m) + (a1 * r1c1m) + (a2 * r1c2m) + (a3 * r1c3m));
        float const x2 = ((a0 * r2c0m) + (a1 * r2c1m) + (a2 * r2c2m) + (a3 * r2c3m));
        float const x3 = ((a0 * r3c0m) + (a1 * r3c1m) + (a2 * r3c2m) + (a3 * r3c3m));

        // Vector for normalised transformed (uvq).
        float const u1 = (x0 / x2);
        float const v1 = (x1 / x2);
        float const q1 = (x3 / x2);

        /* add_mJ for u error */ {
            // Calculate Ju.
            float const J0 = (q1             );
            float const J1 = (0              );
            float const J2 = (-u1 * q1       );
            float const J3 = (-u1 * v1       );
            float const J4 = (1.0f + sq(u1)  );
            float const J5 = (-v1            );

            // Calculate error in u.
            float const du = (u2 - u1);

            // Update vector."""

for row in range(6):
    print "            b%d   += (J%d * du);" % (row, row)

print
print "            // Update matrix."

for row in range(6):
    for col in range(row, 6):
        print "            r%dc%d += (J%d * J%d);" % (row, col, row, col)

print """
        }

        /* add_mJ for v error */ {
            // Calculate Jv.
            float const J0 = (0              );
            float const J1 = (q1             );
            float const J2 = (-v1 * q1       );
            float const J3 = (-1.0f - sq(v1) );
            float const J4 = (u1 * v1        );
            float const J5 = (u1             );

            // Calculate error in v.
            float const dv = (v2 - v1);

            // Update vector."""

for row in range(6):
    print "            b%d   += (J%d * dv);" % (row, row)

print
print "            // Update matrix."

for row in range(6):
    for col in range(row, 6):
        print "            r%dc%d += (J%d * J%d);" % (row, col, row, col)

print """        }
    }
"""

print "    // Copy top-right triangle to bottom-left."

for row in range(6):
    for col in range(row):
        print "    float const r%dc%d = r%dc%d;" % (row, col, col, row)

print """
    // Write matrix elements."""

for row in range(6):
    for col in range(6):
        factor = ((row * 6) + col)
        print "    As[mad24(%2d, nsets, iset)] = r%dc%d;" % (factor, row, col)

print """
    // Write vector elements."""

for row in range(6):
    print "    bs[mad24(%2d, nsets, iset)] = b%d;" % (row, row)

print "}"

