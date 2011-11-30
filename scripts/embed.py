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

bytes  = sys.stdin.read()
name   = sys.argv[1]
perRow = 16

macro  = ("__CVD_CL_EMBED_%s_HH__" % name)

print("#ifndef %s" % macro)
print("#define %s" % macro)
print
print("namespace CVD {")
print("namespace CL  {")
print
print("/// \\brief Embedded OpenCL program source.")
print("///")
print("/// \\see Worker::compile")
print("char static const %s [] = {" % name)

lines  = []
while len(bytes) > 0:
    row   = bytes[:perRow]
    bytes = bytes[ perRow:]
    cells = [(" %3d," % ord(byte)) for byte in row]
    print("   " + ("".join(cells)))

print("      0")
print("};")
print
print("} // namespace CL")
print("} // namespace CVD")
print
print("#endif /* %s */" % macro)
