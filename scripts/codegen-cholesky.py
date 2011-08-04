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

import sys
size   = int(sys.argv[1])
square = size * size

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

// Solve b=Ax for x given A and b.

kernel void cholesky%d(
    global float const * As,
    global float const * bs,
    global float       * xs
) {

    // Use global work item as matrix index.
    int const imatrix   = get_global_id(0);
    int const nmatrices = get_global_size(0);

    // Read matrix elements.
    // Note that matrices are NOT contiguous in memory,
    // so that memory access can be coalesced for multiple threads.""" % size

for row in range(size):
    for col in range(size):
        factor = ((row * size) + col)
        print "    float r%dc%d = As[mad24(%3d, nmatrices, imatrix)];" % (row, col, factor)

print """
    // Read vector elements.
    // Like matrices, they are NOT contiguous in memory, but are only 1D."""

for col in range(size):
    print "    float v%d   = bs[mad24(%3d, nmatrices, imatrix)];" % (col, col)

print

for col in range(size):
    print "    /* Column %d */ {" % col
    print "        float inv     = 1;"
    for row in range(col, size):
        print "        /* Row %d */ {" % row
        print "            float val = r%dc%d;" % (row, col)
        print "            // Correct for the parts of Cholesky already computed."
        for col2 in range(col):
            print "            val      -= (r%dc%d * r%dc%d);" % (col2, col, row,col2)
        if row == col:
            print "            // Diagonal element, don't divide."
            print "            r%dc%d      = val;" % (row, col)
            print "            inv       = 1.0f / val;"
        else:
            print "            // Cache the value, without division, in the upper half."
            print "            r%dc%d      = val;" % (col, row)
            print "            // Divide by the diagonal element."
            print "            r%dc%d      = (val * inv);" % (row, col)
        print "        }"
    print "    }"
    print

print "    // Back-substitute through L."
for col in range(1, size):
    for row in range(col):
        print "    v%d -= (r%dc%d * v%d);" % (col, col, row, row)

print

print "    // Back-substitute through diagonal."
for col in range(size):
    print "    v%d /= r%dc%d;" % (col, col, col)

print

print "    // Back-substitute through L.T."
for col in reversed(range(size - 1)):
    for row in range(col + 1, size):
        print "    v%d -= (r%dc%d * v%d);" % (col, row, col, row)

print """
    // Write vector elements."""

for col in range(size):
    print "    xs[mad24(%3d, nmatrices, imatrix)] = v%d;" % (col, col)

print "}"
