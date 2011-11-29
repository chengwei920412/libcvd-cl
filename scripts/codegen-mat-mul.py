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

import sys
nrows = int(sys.argv[1])

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

kernel void mat_mul_%d(
    global float const * As,
    global float       * Bs
) {

    // Use global work item as matrix pair index.
    int   const im    = get_global_id(0);
    int   const nm    = get_global_size(0);

    // Calculate matrix cell offsets.""" % nrows

for row in range(nrows):
    for col in range(nrows):
        factor = ((row * nrows) + col)
        core   = ("r%dc%d" % (row, col))
        print "    int   const %si = ((%2d * nm) + im);" % (core, factor)

print """
    // Read matrix A."""

for row in range(nrows):
    for col in range(nrows):
        factor = ((row * nrows) + col)
        core   = ("r%dc%d" % (row, col))
        print "    float const %sa = As[%si];" % (core, core)

print """
    // Read matrix B."""

for row in range(nrows):
    for col in range(nrows):
        factor = ((row * nrows) + col)
        core   = ("r%dc%d" % (row, col))
        print "    float const %sb = Bs[%si];" % (core, core)

print """
    // Calculate output matrix."""

for row in range(nrows):
    for col in range(nrows):
        core1  = ("r%dc%d" % (row, col))
        sums = []
        for idx in range(nrows):
            core2 = ("r%dc%d" % (row, idx))
            core3 = ("r%dc%d" % (idx, col))
            sums.append("(%sa * %sb)" % (core2, core3))
        print "    float const %sc = (%s);" % (core1, " + ".join(sums))

print """
    // Write output matrix in place of B."""

for row in range(nrows):
    for col in range(nrows):
        core = ("r%dc%d" % (row, col))
        print "    Bs[%si] = %sb;" % (core, core)

print "}"
