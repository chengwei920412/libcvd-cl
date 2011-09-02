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

// This program implements part of MurmurHash3.
// http://code.google.com/p/smhasher/wiki/MurmurHash3

uint rotl32(uint x, uint r) {
    return ((x << r) | (x >> (32 - r)));
}

uint hmix(uint x) {
    x *= 0xcc9e2d51;
    x  = rotl32(x, 15);
    x *= 0x1b873593;
    return x;
}

uint hash(uint h, uint x) {
    h ^= x;
    h  = rotl32(h, 13);
    h  = ((h * 5) + 0xe6546b64);
    return h;
}

kernel void random_int(
    global uint const * pmod,
    global uint       * out,
           uint         seed1,
           uint         seed2
) {

    // Read modulo.
    uint const mod = (pmod[0] & 0xffffff);

    // Use global work item as integer index.
    uint const i   = get_global_id(0);

    // Mix the global work item, modulo, and seeds.
    uint const k1 = hmix(i);
    uint const k2 = hmix(mod);
    uint const k3 = hmix(seed1);
    uint const k4 = hmix(seed2);

    // Hash all values.
    uint h  = 0xbcaa747;
         h  = hash(h, k1);
         h  = hash(h, k2);
         h  = hash(h, k3);
         h  = hash(h, k4);
         h ^= (h >> 16);
         h *= 0x85ebca6b;
         h ^= (h >> 13);
         h *= 0xc2b2ae35;
         h ^= (h >> 16);

    // Write hash after modulo.
    out[i] = ((h & 0xffffff) % mod);
}
"""
