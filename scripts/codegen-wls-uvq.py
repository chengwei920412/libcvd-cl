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

float sq(float x) {
    return (x * x);
}

kernel void wls_uvq(
    global float const * u1s,
    global float const * v1s,
    global float const * q1s,
    global float const * u2s,
    global float const * v2s,
    global float       * As,
    global float       * bs
) {

    // Use global work item as correspondence set index.
    int const iset   = get_global_id(0);
    int const nsets  = get_global_size(0);

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
        float const u1 = u1s[of];
        float const v1 = v1s[of];
        float const u2 = u2s[of];
        float const v2 = v2s[of];
        float const q1 = q1s[of];

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

