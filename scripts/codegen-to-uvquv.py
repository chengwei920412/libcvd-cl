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

kernel void to_uvquv(
    read_only  image2d_t     umap,
    read_only  image2d_t     vmap,
    read_only  image2d_t     qmap,
    global     int2  const * xy1s,
    global     int2  const * xy2s,
    global     int2  const * pairs,
    global     float       * u1s,
    global     float       * v1s,
    global     float       * q1s,
    global     float       * u2s,
    global     float       * v2s
) {

    // Prepare a suitable OpenCL image sampler.
    sampler_t const sampler = CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    // Use global work item as pair index.
    int  const idx  = get_global_id(0);

    // Read pair.
    int2 const pair = pairs[idx];

    // Read integer (x,y) coordinates.
    int2 const xy1  = xy1s[pair.x];
    int2 const xy2  = xy2s[pair.y];

    // Translate with floating image maps.
    u1s[idx] = read_imagef(umap, sampler, xy1).x;
    u2s[idx] = read_imagef(umap, sampler, xy2).x;
    v1s[idx] = read_imagef(vmap, sampler, xy1).x;
    v2s[idx] = read_imagef(vmap, sampler, xy2).x;
    q1s[idx] = read_imagef(qmap, sampler, xy1).x;
}
"""
