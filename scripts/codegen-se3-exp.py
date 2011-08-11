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

float len(float x, float y, float z) {
    return sqrt(sq(x) + sq(y) + sq(z));
}

#define ONE_6th  (1.0f /  6.0f)
#define ONE_20th (1.0f / 20.0f)
#define SMALL_8  (1.0e-8f)
#define SMALL_6  (1.0e-6f)

kernel void se3_exp(
    global float const * mus,
    global float       * mats
) {

    // Use global work item as vector index.
    int const ivector   = get_global_id(0);
    int const nvectors  = get_global_size(0);

    // Read vector elements.
    // Note that vectors are NOT contiguous in memory,
    // so that memory access can be coalesced for multiple threads.
    float const mu0 = mus[mad24(0, nvectors, ivector)];
    float const mu1 = mus[mad24(1, nvectors, ivector)];
    float const mu2 = mus[mad24(2, nvectors, ivector)];
    float const  w0 = mus[mad24(3, nvectors, ivector)];
    float const  w1 = mus[mad24(4, nvectors, ivector)];
    float const  w2 = mus[mad24(5, nvectors, ivector)];

    float const theta_sq = (sq(w0) + sq(w1) + sq(w2));
    float const theta    = sqrt(theta_sq);

    float const cross0 = ((w1 * mu2) - (w2 * mu1));
    float const cross1 = ((w2 * mu0) - (w0 * mu2));
    float const cross2 = ((w0 * mu1) - (w1 * mu0));

    // Prepare matrix elements.
    float r0c0;
    float r0c1;
    float r0c2;
    float r0c3;
    float r1c0;
    float r1c1;
    float r1c2;
    float r1c3;
    float r2c0;
    float r2c1;
    float r2c2;
    float r2c3;

    float A;
    float B;

    if (theta_sq < SMALL_8) {
        A = (1.0f - (ONE_6th * theta_sq));
        B = (0.5f);

        // Assign translation matrix.
        r0c3 = (mu0 + (cross0 * 0.5f));
        r1c3 = (mu1 + (cross1 * 0.5f));
        r2c3 = (mu2 + (cross2 * 0.5f));
    } else {
        float C;

        if (theta_sq < SMALL_6) {
            C = (ONE_6th * (1.0f - (ONE_20th * theta_sq)));
            A = (1.0f - (theta_sq * C));
            B = (0.5f - (0.25f * ONE_6th * theta_sq));
        } else {
            float const inv_theta  = (1.0f / theta);
            float const inv_theta2 = sq(inv_theta);

            A = (sin(theta) * inv_theta);
            B = ((1.0f - cos(theta)) * inv_theta2);
            C = ((1.0f - A) * inv_theta2);
        }

        float const wc0 = ((w1 * cross2) - (w2 * cross1));
        float const wc1 = ((w2 * cross0) - (w0 * cross2));
        float const wc2 = ((w0 * cross1) - (w1 * cross0));

        // Assign translation matrix.
        r0c3 = (mu0 + (cross0 * B) + (wc0 * C));
        r1c3 = (mu1 + (cross1 * B) + (wc1 * C));
        r2c3 = (mu2 + (cross2 * B) + (wc2 * C));
    }

    {
        float const wx2 = sq(w0);
        float const wy2 = sq(w1);
        float const wz2 = sq(w2);

        r0c0 = (1.0f - (B * (wy2 + wz2)));
        r1c1 = (1.0f - (B * (wx2 + wz2)));
        r2c2 = (1.0f - (B * (wx2 + wy2)));
    }
    {
        float const a   = (A * w2);
        float const b   = (B * w0 * w1);

        r0c1 = (b - a);
        r1c0 = (b + a);
    }
    {
        float const a   = (A * w1);
        float const b   = (B * w0 * w2);

        r0c2 = (b + a);
        r2c0 = (b - a);
    }
    {
        float const a   = (A * w0);
        float const b   = (B * w1 * w2);

        r1c2 = (b - a);
        r2c1 = (b + a);
    }

    // Coerce matrix.

    // my_matrix[0] = unit(my_matrix[0]);
    float const l0 = len(r0c0, r0c1, r0c2);
    r0c0 /= l0;
    r0c1 /= l0;
    r0c2 /= l0;

    // my_matrix[1] -= my_matrix[0] * (my_matrix[0]*my_matrix[1]);
    float const r0 = ((r0c0 * r1c0) + (r0c1 * r1c1) + (r0c2 * r1c2));
    r1c0 -= (r0c0 * r0);
    r1c1 -= (r0c1 * r0);
    r1c2 -= (r0c2 * r0);

    // my_matrix[1] = unit(my_matrix[1]);
    float const l1 = len(r1c0, r1c1, r1c2);
    r1c0 /= l1;
    r1c1 /= l1;
    r1c2 /= l1;

    // my_matrix[2] -= my_matrix[0] * (my_matrix[0]*my_matrix[2]);
    float const r1 = ((r0c0 * r2c0) + (r0c1 * r2c1) + (r0c2 * r2c2));
    r2c0 -= (r0c0 * r1);
    r2c1 -= (r0c1 * r1);
    r2c2 -= (r0c2 * r1);

    // my_matrix[2] -= my_matrix[1] * (my_matrix[1]*my_matrix[2]);
    float const r2 = ((r1c0 * r2c0) + (r1c1 * r2c1) + (r1c2 * r2c2));
    r2c0 -= (r1c0 * r2);
    r2c1 -= (r1c1 * r2);
    r2c2 -= (r1c2 * r2);

    // my_matrix[2] = unit(my_matrix[2]);
    float const l2 = len(r2c0, r2c1, r2c2);
    r2c0 /= l2;
    r2c1 /= l2;
    r2c2 /= l2;

    // Write matrix elements.
    mats[mad24( 0, nvectors, ivector)] = r0c0;
    mats[mad24( 1, nvectors, ivector)] = r0c1;
    mats[mad24( 2, nvectors, ivector)] = r0c2;
    mats[mad24( 3, nvectors, ivector)] = r0c3;
    mats[mad24( 4, nvectors, ivector)] = r1c0;
    mats[mad24( 5, nvectors, ivector)] = r1c1;
    mats[mad24( 6, nvectors, ivector)] = r1c2;
    mats[mad24( 7, nvectors, ivector)] = r1c3;
    mats[mad24( 8, nvectors, ivector)] = r2c0;
    mats[mad24( 9, nvectors, ivector)] = r2c1;
    mats[mad24(10, nvectors, ivector)] = r2c2;
    mats[mad24(11, nvectors, ivector)] = r2c3;
    mats[mad24(12, nvectors, ivector)] =    0;
    mats[mad24(13, nvectors, ivector)] =    0;
    mats[mad24(14, nvectors, ivector)] =    0;
    mats[mad24(15, nvectors, ivector)] =    1;
}
"""

