// Copyright (C) 2011  Dmitri Nikulin, Monash University
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

// Specify a threshold for pixel difference.
#define THRESH  60

// Generate bitwise mask of n bits.
#define MASK(n) ((1 << (n)) - 1)

// Create a bitwise mask of n bits, rotated by r bits, in a ring of w bits.
#define MASK_TURN(n, w, r) (((MASK(n) << (r)) | (MASK(n) >> ((w) - (r)))) & MASK(w))

// Test a value x against a bitwise mask of n bits, rotated by r bits, in a ring of w bits.
#define MASK_TEST(x, n, w, r) (((x) & MASK_TURN(n, w, r)) == MASK_TURN(n, w, r))

#define MASK_TEST_9_16(x, r) MASK_TEST((x), 9, 16, (r))

kernel void fast_gray_9(
    read_only image2d_t   image,
    global    int2      * corners,
    global    int       * icorner
) {

    // Prepare a suitable OpenCL image sampler.
    sampler_t const sampler = CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    // Use global work item as 2D image coordinates.
    int  const x   = get_global_id(0);
    int  const y   = get_global_id(1);
    int2 const xy  = (int2)(x, y);

    // Read the candidate pixel.
    int  const p00 = read_imagei(image, sampler, xy).x;

    // Read other pixels in a circle around the candidate pixel.
    int  const p01 = read_imagei(image, sampler, xy + (int2)( 0,  3)).x;
    int  const p02 = read_imagei(image, sampler, xy + (int2)( 1,  3)).x;
    int  const p03 = read_imagei(image, sampler, xy + (int2)( 2,  2)).x;
    int  const p04 = read_imagei(image, sampler, xy + (int2)( 3,  1)).x;
    int  const p05 = read_imagei(image, sampler, xy + (int2)( 3,  0)).x;
    int  const p06 = read_imagei(image, sampler, xy + (int2)( 3, -1)).x;
    int  const p07 = read_imagei(image, sampler, xy + (int2)( 2, -2)).x;
    int  const p08 = read_imagei(image, sampler, xy + (int2)( 1, -3)).x;
    int  const p09 = read_imagei(image, sampler, xy + (int2)( 0, -3)).x;
    int  const p10 = read_imagei(image, sampler, xy + (int2)(-1, -3)).x;
    int  const p11 = read_imagei(image, sampler, xy + (int2)(-2, -2)).x;
    int  const p12 = read_imagei(image, sampler, xy + (int2)(-3, -1)).x;
    int  const p13 = read_imagei(image, sampler, xy + (int2)(-3,  0)).x;
    int  const p14 = read_imagei(image, sampler, xy + (int2)(-3,  1)).x;
    int  const p15 = read_imagei(image, sampler, xy + (int2)(-2,  2)).x;
    int  const p16 = read_imagei(image, sampler, xy + (int2)(-1,  3)).x;

    // Threshold the absolute difference of each circle pixel.
    int  const sum = (
        ((abs(p01 - p00) > THRESH) <<  0) |
        ((abs(p02 - p00) > THRESH) <<  1) |
        ((abs(p03 - p00) > THRESH) <<  2) |
        ((abs(p04 - p00) > THRESH) <<  3) |
        ((abs(p05 - p00) > THRESH) <<  4) |
        ((abs(p06 - p00) > THRESH) <<  5) |
        ((abs(p07 - p00) > THRESH) <<  6) |
        ((abs(p08 - p00) > THRESH) <<  7) |
        ((abs(p09 - p00) > THRESH) <<  8) |
        ((abs(p10 - p00) > THRESH) <<  9) |
        ((abs(p11 - p00) > THRESH) << 10) |
        ((abs(p12 - p00) > THRESH) << 11) |
        ((abs(p13 - p00) > THRESH) << 12) |
        ((abs(p14 - p00) > THRESH) << 13) |
        ((abs(p15 - p00) > THRESH) << 14) |
        ((abs(p16 - p00) > THRESH) << 15)
    );

    // Check if at least one mask applies entirely.
    int  const yes = (
        MASK_TEST_9_16(sum,  0) ||
        MASK_TEST_9_16(sum,  1) ||
        MASK_TEST_9_16(sum,  2) ||
        MASK_TEST_9_16(sum,  3) ||
        MASK_TEST_9_16(sum,  4) ||
        MASK_TEST_9_16(sum,  5) ||
        MASK_TEST_9_16(sum,  6) ||
        MASK_TEST_9_16(sum,  7) ||
        MASK_TEST_9_16(sum,  8) ||
        MASK_TEST_9_16(sum,  9) ||
        MASK_TEST_9_16(sum, 10) ||
        MASK_TEST_9_16(sum, 11) ||
        MASK_TEST_9_16(sum, 12) ||
        MASK_TEST_9_16(sum, 13) ||
        MASK_TEST_9_16(sum, 14) ||
        MASK_TEST_9_16(sum, 15)
    );

    if (yes) {
        // Atomically append to corner buffer.
        corners[atom_inc(icorner)] = xy;
    }
}

// Copyright (C) 2011  Dmitri Nikulin, Monash University
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

// Square an integer, for standard deviation calculation.
int sq(int x) {
    return (x * x);
}

// Shorthand for ulong cast.
#define L(x) ((ulong)(x))

kernel void hips_gray(
    read_only image2d_t   image,
    global    int2      * corners,
    global    ulong4    * bins
) {

    // Prepare a suitable OpenCL image sampler.
    sampler_t const sampler = CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;

    // Use global work item as corner index.
    int  const ic  = get_global_id(0);
    int2 const xy  = corners[ic];

    // Read pixels in a grid around the corner pixel.
    int  const p01 = read_imagei(image, sampler, xy + (int2)(-7, -7)).x;
    int  const p02 = read_imagei(image, sampler, xy + (int2)(-5, -7)).x;
    int  const p03 = read_imagei(image, sampler, xy + (int2)(-3, -7)).x;
    int  const p04 = read_imagei(image, sampler, xy + (int2)(-1, -7)).x;
    int  const p05 = read_imagei(image, sampler, xy + (int2)( 1, -7)).x;
    int  const p06 = read_imagei(image, sampler, xy + (int2)( 3, -7)).x;
    int  const p07 = read_imagei(image, sampler, xy + (int2)( 5, -7)).x;
    int  const p08 = read_imagei(image, sampler, xy + (int2)( 7, -7)).x;
    int  const p09 = read_imagei(image, sampler, xy + (int2)(-7, -5)).x;
    int  const p10 = read_imagei(image, sampler, xy + (int2)(-5, -5)).x;
    int  const p11 = read_imagei(image, sampler, xy + (int2)(-3, -5)).x;
    int  const p12 = read_imagei(image, sampler, xy + (int2)(-1, -5)).x;
    int  const p13 = read_imagei(image, sampler, xy + (int2)( 1, -5)).x;
    int  const p14 = read_imagei(image, sampler, xy + (int2)( 3, -5)).x;
    int  const p15 = read_imagei(image, sampler, xy + (int2)( 5, -5)).x;
    int  const p16 = read_imagei(image, sampler, xy + (int2)( 7, -5)).x;
    int  const p17 = read_imagei(image, sampler, xy + (int2)(-7, -3)).x;
    int  const p18 = read_imagei(image, sampler, xy + (int2)(-5, -3)).x;
    int  const p19 = read_imagei(image, sampler, xy + (int2)(-3, -3)).x;
    int  const p20 = read_imagei(image, sampler, xy + (int2)(-1, -3)).x;
    int  const p21 = read_imagei(image, sampler, xy + (int2)( 1, -3)).x;
    int  const p22 = read_imagei(image, sampler, xy + (int2)( 3, -3)).x;
    int  const p23 = read_imagei(image, sampler, xy + (int2)( 5, -3)).x;
    int  const p24 = read_imagei(image, sampler, xy + (int2)( 7, -3)).x;
    int  const p25 = read_imagei(image, sampler, xy + (int2)(-7, -1)).x;
    int  const p26 = read_imagei(image, sampler, xy + (int2)(-5, -1)).x;
    int  const p27 = read_imagei(image, sampler, xy + (int2)(-3, -1)).x;
    int  const p28 = read_imagei(image, sampler, xy + (int2)(-1, -1)).x;
    int  const p29 = read_imagei(image, sampler, xy + (int2)( 1, -1)).x;
    int  const p30 = read_imagei(image, sampler, xy + (int2)( 3, -1)).x;
    int  const p31 = read_imagei(image, sampler, xy + (int2)( 5, -1)).x;
    int  const p32 = read_imagei(image, sampler, xy + (int2)( 7, -1)).x;
    int  const p33 = read_imagei(image, sampler, xy + (int2)(-7,  1)).x;
    int  const p34 = read_imagei(image, sampler, xy + (int2)(-5,  1)).x;
    int  const p35 = read_imagei(image, sampler, xy + (int2)(-3,  1)).x;
    int  const p36 = read_imagei(image, sampler, xy + (int2)(-1,  1)).x;
    int  const p37 = read_imagei(image, sampler, xy + (int2)( 1,  1)).x;
    int  const p38 = read_imagei(image, sampler, xy + (int2)( 3,  1)).x;
    int  const p39 = read_imagei(image, sampler, xy + (int2)( 5,  1)).x;
    int  const p40 = read_imagei(image, sampler, xy + (int2)( 7,  1)).x;
    int  const p41 = read_imagei(image, sampler, xy + (int2)(-7,  3)).x;
    int  const p42 = read_imagei(image, sampler, xy + (int2)(-5,  3)).x;
    int  const p43 = read_imagei(image, sampler, xy + (int2)(-3,  3)).x;
    int  const p44 = read_imagei(image, sampler, xy + (int2)(-1,  3)).x;
    int  const p45 = read_imagei(image, sampler, xy + (int2)( 1,  3)).x;
    int  const p46 = read_imagei(image, sampler, xy + (int2)( 3,  3)).x;
    int  const p47 = read_imagei(image, sampler, xy + (int2)( 5,  3)).x;
    int  const p48 = read_imagei(image, sampler, xy + (int2)( 7,  3)).x;
    int  const p49 = read_imagei(image, sampler, xy + (int2)(-7,  5)).x;
    int  const p50 = read_imagei(image, sampler, xy + (int2)(-5,  5)).x;
    int  const p51 = read_imagei(image, sampler, xy + (int2)(-3,  5)).x;
    int  const p52 = read_imagei(image, sampler, xy + (int2)(-1,  5)).x;
    int  const p53 = read_imagei(image, sampler, xy + (int2)( 1,  5)).x;
    int  const p54 = read_imagei(image, sampler, xy + (int2)( 3,  5)).x;
    int  const p55 = read_imagei(image, sampler, xy + (int2)( 5,  5)).x;
    int  const p56 = read_imagei(image, sampler, xy + (int2)( 7,  5)).x;
    int  const p57 = read_imagei(image, sampler, xy + (int2)(-7,  7)).x;
    int  const p58 = read_imagei(image, sampler, xy + (int2)(-5,  7)).x;
    int  const p59 = read_imagei(image, sampler, xy + (int2)(-3,  7)).x;
    int  const p60 = read_imagei(image, sampler, xy + (int2)(-1,  7)).x;
    int  const p61 = read_imagei(image, sampler, xy + (int2)( 1,  7)).x;
    int  const p62 = read_imagei(image, sampler, xy + (int2)( 3,  7)).x;
    int  const p63 = read_imagei(image, sampler, xy + (int2)( 5,  7)).x;
    int  const p64 = read_imagei(image, sampler, xy + (int2)( 7,  7)).x;

    // Calculate the sum of the pixel values.
    int  const sum1 = (
        p01 +
        p02 +
        p03 +
        p04 +
        p05 +
        p06 +
        p07 +
        p08 +
        p09 +
        p10 +
        p11 +
        p12 +
        p13 +
        p14 +
        p15 +
        p16 +
        p17 +
        p18 +
        p19 +
        p20 +
        p21 +
        p22 +
        p23 +
        p24 +
        p25 +
        p26 +
        p27 +
        p28 +
        p29 +
        p30 +
        p31 +
        p32 +
        p33 +
        p34 +
        p35 +
        p36 +
        p37 +
        p38 +
        p39 +
        p40 +
        p41 +
        p42 +
        p43 +
        p44 +
        p45 +
        p46 +
        p47 +
        p48 +
        p49 +
        p50 +
        p51 +
        p52 +
        p53 +
        p54 +
        p55 +
        p56 +
        p57 +
        p58 +
        p59 +
        p60 +
        p61 +
        p62 +
        p63 +
        p64
    );

    // Calculate the mean of the pixel values.
    int  const mean = (sum1 / 64);
    // Calculate the sum of squares of differences of the pixel values.
    int  const sum2 = (
        sq(p01 - mean) +
        sq(p02 - mean) +
        sq(p03 - mean) +
        sq(p04 - mean) +
        sq(p05 - mean) +
        sq(p06 - mean) +
        sq(p07 - mean) +
        sq(p08 - mean) +
        sq(p09 - mean) +
        sq(p10 - mean) +
        sq(p11 - mean) +
        sq(p12 - mean) +
        sq(p13 - mean) +
        sq(p14 - mean) +
        sq(p15 - mean) +
        sq(p16 - mean) +
        sq(p17 - mean) +
        sq(p18 - mean) +
        sq(p19 - mean) +
        sq(p20 - mean) +
        sq(p21 - mean) +
        sq(p22 - mean) +
        sq(p23 - mean) +
        sq(p24 - mean) +
        sq(p25 - mean) +
        sq(p26 - mean) +
        sq(p27 - mean) +
        sq(p28 - mean) +
        sq(p29 - mean) +
        sq(p30 - mean) +
        sq(p31 - mean) +
        sq(p32 - mean) +
        sq(p33 - mean) +
        sq(p34 - mean) +
        sq(p35 - mean) +
        sq(p36 - mean) +
        sq(p37 - mean) +
        sq(p38 - mean) +
        sq(p39 - mean) +
        sq(p40 - mean) +
        sq(p41 - mean) +
        sq(p42 - mean) +
        sq(p43 - mean) +
        sq(p44 - mean) +
        sq(p45 - mean) +
        sq(p46 - mean) +
        sq(p47 - mean) +
        sq(p48 - mean) +
        sq(p49 - mean) +
        sq(p50 - mean) +
        sq(p51 - mean) +
        sq(p52 - mean) +
        sq(p53 - mean) +
        sq(p54 - mean) +
        sq(p55 - mean) +
        sq(p56 - mean) +
        sq(p57 - mean) +
        sq(p58 - mean) +
        sq(p59 - mean) +
        sq(p60 - mean) +
        sq(p61 - mean) +
        sq(p62 - mean) +
        sq(p63 - mean) +
        sq(p64 - mean)
    );

    // Calculate the standard deviation of the pixel values.
    int  const dev  = (int) sqrt((float) (sum2 / 64));

    // Calculate thresholds for standard deviation bins.
    // TODO: Size factors.
    int  const dev1 = (mean - dev);
    int  const dev2 = (mean + dev);

    // Bin all values lower than a standard deviation from the mean.
    ulong const b1  = (
        (L(p01 < dev1) << L( 0)) |
        (L(p02 < dev1) << L( 1)) |
        (L(p03 < dev1) << L( 2)) |
        (L(p04 < dev1) << L( 3)) |
        (L(p05 < dev1) << L( 4)) |
        (L(p06 < dev1) << L( 5)) |
        (L(p07 < dev1) << L( 6)) |
        (L(p08 < dev1) << L( 7)) |
        (L(p09 < dev1) << L( 8)) |
        (L(p10 < dev1) << L( 9)) |
        (L(p11 < dev1) << L(10)) |
        (L(p12 < dev1) << L(11)) |
        (L(p13 < dev1) << L(12)) |
        (L(p14 < dev1) << L(13)) |
        (L(p15 < dev1) << L(14)) |
        (L(p16 < dev1) << L(15)) |
        (L(p17 < dev1) << L(16)) |
        (L(p18 < dev1) << L(17)) |
        (L(p19 < dev1) << L(18)) |
        (L(p20 < dev1) << L(19)) |
        (L(p21 < dev1) << L(20)) |
        (L(p22 < dev1) << L(21)) |
        (L(p23 < dev1) << L(22)) |
        (L(p24 < dev1) << L(23)) |
        (L(p25 < dev1) << L(24)) |
        (L(p26 < dev1) << L(25)) |
        (L(p27 < dev1) << L(26)) |
        (L(p28 < dev1) << L(27)) |
        (L(p29 < dev1) << L(28)) |
        (L(p30 < dev1) << L(29)) |
        (L(p31 < dev1) << L(30)) |
        (L(p32 < dev1) << L(31)) |
        (L(p33 < dev1) << L(32)) |
        (L(p34 < dev1) << L(33)) |
        (L(p35 < dev1) << L(34)) |
        (L(p36 < dev1) << L(35)) |
        (L(p37 < dev1) << L(36)) |
        (L(p38 < dev1) << L(37)) |
        (L(p39 < dev1) << L(38)) |
        (L(p40 < dev1) << L(39)) |
        (L(p41 < dev1) << L(40)) |
        (L(p42 < dev1) << L(41)) |
        (L(p43 < dev1) << L(42)) |
        (L(p44 < dev1) << L(43)) |
        (L(p45 < dev1) << L(44)) |
        (L(p46 < dev1) << L(45)) |
        (L(p47 < dev1) << L(46)) |
        (L(p48 < dev1) << L(47)) |
        (L(p49 < dev1) << L(48)) |
        (L(p50 < dev1) << L(49)) |
        (L(p51 < dev1) << L(50)) |
        (L(p52 < dev1) << L(51)) |
        (L(p53 < dev1) << L(52)) |
        (L(p54 < dev1) << L(53)) |
        (L(p55 < dev1) << L(54)) |
        (L(p56 < dev1) << L(55)) |
        (L(p57 < dev1) << L(56)) |
        (L(p58 < dev1) << L(57)) |
        (L(p59 < dev1) << L(58)) |
        (L(p60 < dev1) << L(59)) |
        (L(p61 < dev1) << L(60)) |
        (L(p62 < dev1) << L(61)) |
        (L(p63 < dev1) << L(62)) |
        (L(p64 < dev1) << L(63))
    );

    // Bin all values higher than a standard deviation from the mean.
    ulong const b4  = (
        (L(p01 > dev2) << L( 0)) |
        (L(p02 > dev2) << L( 1)) |
        (L(p03 > dev2) << L( 2)) |
        (L(p04 > dev2) << L( 3)) |
        (L(p05 > dev2) << L( 4)) |
        (L(p06 > dev2) << L( 5)) |
        (L(p07 > dev2) << L( 6)) |
        (L(p08 > dev2) << L( 7)) |
        (L(p09 > dev2) << L( 8)) |
        (L(p10 > dev2) << L( 9)) |
        (L(p11 > dev2) << L(10)) |
        (L(p12 > dev2) << L(11)) |
        (L(p13 > dev2) << L(12)) |
        (L(p14 > dev2) << L(13)) |
        (L(p15 > dev2) << L(14)) |
        (L(p16 > dev2) << L(15)) |
        (L(p17 > dev2) << L(16)) |
        (L(p18 > dev2) << L(17)) |
        (L(p19 > dev2) << L(18)) |
        (L(p20 > dev2) << L(19)) |
        (L(p21 > dev2) << L(20)) |
        (L(p22 > dev2) << L(21)) |
        (L(p23 > dev2) << L(22)) |
        (L(p24 > dev2) << L(23)) |
        (L(p25 > dev2) << L(24)) |
        (L(p26 > dev2) << L(25)) |
        (L(p27 > dev2) << L(26)) |
        (L(p28 > dev2) << L(27)) |
        (L(p29 > dev2) << L(28)) |
        (L(p30 > dev2) << L(29)) |
        (L(p31 > dev2) << L(30)) |
        (L(p32 > dev2) << L(31)) |
        (L(p33 > dev2) << L(32)) |
        (L(p34 > dev2) << L(33)) |
        (L(p35 > dev2) << L(34)) |
        (L(p36 > dev2) << L(35)) |
        (L(p37 > dev2) << L(36)) |
        (L(p38 > dev2) << L(37)) |
        (L(p39 > dev2) << L(38)) |
        (L(p40 > dev2) << L(39)) |
        (L(p41 > dev2) << L(40)) |
        (L(p42 > dev2) << L(41)) |
        (L(p43 > dev2) << L(42)) |
        (L(p44 > dev2) << L(43)) |
        (L(p45 > dev2) << L(44)) |
        (L(p46 > dev2) << L(45)) |
        (L(p47 > dev2) << L(46)) |
        (L(p48 > dev2) << L(47)) |
        (L(p49 > dev2) << L(48)) |
        (L(p50 > dev2) << L(49)) |
        (L(p51 > dev2) << L(50)) |
        (L(p52 > dev2) << L(51)) |
        (L(p53 > dev2) << L(52)) |
        (L(p54 > dev2) << L(53)) |
        (L(p55 > dev2) << L(54)) |
        (L(p56 > dev2) << L(55)) |
        (L(p57 > dev2) << L(56)) |
        (L(p58 > dev2) << L(57)) |
        (L(p59 > dev2) << L(58)) |
        (L(p60 > dev2) << L(59)) |
        (L(p61 > dev2) << L(60)) |
        (L(p62 > dev2) << L(61)) |
        (L(p63 > dev2) << L(62)) |
        (L(p64 > dev2) << L(63))
    );

    // Bin all values lower than the mean but not a standard deviation.
    ulong const b2  = (
        (L(p01 < mean) << L( 0)) |
        (L(p02 < mean) << L( 1)) |
        (L(p03 < mean) << L( 2)) |
        (L(p04 < mean) << L( 3)) |
        (L(p05 < mean) << L( 4)) |
        (L(p06 < mean) << L( 5)) |
        (L(p07 < mean) << L( 6)) |
        (L(p08 < mean) << L( 7)) |
        (L(p09 < mean) << L( 8)) |
        (L(p10 < mean) << L( 9)) |
        (L(p11 < mean) << L(10)) |
        (L(p12 < mean) << L(11)) |
        (L(p13 < mean) << L(12)) |
        (L(p14 < mean) << L(13)) |
        (L(p15 < mean) << L(14)) |
        (L(p16 < mean) << L(15)) |
        (L(p17 < mean) << L(16)) |
        (L(p18 < mean) << L(17)) |
        (L(p19 < mean) << L(18)) |
        (L(p20 < mean) << L(19)) |
        (L(p21 < mean) << L(20)) |
        (L(p22 < mean) << L(21)) |
        (L(p23 < mean) << L(22)) |
        (L(p24 < mean) << L(23)) |
        (L(p25 < mean) << L(24)) |
        (L(p26 < mean) << L(25)) |
        (L(p27 < mean) << L(26)) |
        (L(p28 < mean) << L(27)) |
        (L(p29 < mean) << L(28)) |
        (L(p30 < mean) << L(29)) |
        (L(p31 < mean) << L(30)) |
        (L(p32 < mean) << L(31)) |
        (L(p33 < mean) << L(32)) |
        (L(p34 < mean) << L(33)) |
        (L(p35 < mean) << L(34)) |
        (L(p36 < mean) << L(35)) |
        (L(p37 < mean) << L(36)) |
        (L(p38 < mean) << L(37)) |
        (L(p39 < mean) << L(38)) |
        (L(p40 < mean) << L(39)) |
        (L(p41 < mean) << L(40)) |
        (L(p42 < mean) << L(41)) |
        (L(p43 < mean) << L(42)) |
        (L(p44 < mean) << L(43)) |
        (L(p45 < mean) << L(44)) |
        (L(p46 < mean) << L(45)) |
        (L(p47 < mean) << L(46)) |
        (L(p48 < mean) << L(47)) |
        (L(p49 < mean) << L(48)) |
        (L(p50 < mean) << L(49)) |
        (L(p51 < mean) << L(50)) |
        (L(p52 < mean) << L(51)) |
        (L(p53 < mean) << L(52)) |
        (L(p54 < mean) << L(53)) |
        (L(p55 < mean) << L(54)) |
        (L(p56 < mean) << L(55)) |
        (L(p57 < mean) << L(56)) |
        (L(p58 < mean) << L(57)) |
        (L(p59 < mean) << L(58)) |
        (L(p60 < mean) << L(59)) |
        (L(p61 < mean) << L(60)) |
        (L(p62 < mean) << L(61)) |
        (L(p63 < mean) << L(62)) |
        (L(p64 < mean) << L(63))
    );

    // Bin all values higher than the mean but not a standard deviation.
    ulong const b3  = (
        (L(p01 > mean) << L( 0)) |
        (L(p02 > mean) << L( 1)) |
        (L(p03 > mean) << L( 2)) |
        (L(p04 > mean) << L( 3)) |
        (L(p05 > mean) << L( 4)) |
        (L(p06 > mean) << L( 5)) |
        (L(p07 > mean) << L( 6)) |
        (L(p08 > mean) << L( 7)) |
        (L(p09 > mean) << L( 8)) |
        (L(p10 > mean) << L( 9)) |
        (L(p11 > mean) << L(10)) |
        (L(p12 > mean) << L(11)) |
        (L(p13 > mean) << L(12)) |
        (L(p14 > mean) << L(13)) |
        (L(p15 > mean) << L(14)) |
        (L(p16 > mean) << L(15)) |
        (L(p17 > mean) << L(16)) |
        (L(p18 > mean) << L(17)) |
        (L(p19 > mean) << L(18)) |
        (L(p20 > mean) << L(19)) |
        (L(p21 > mean) << L(20)) |
        (L(p22 > mean) << L(21)) |
        (L(p23 > mean) << L(22)) |
        (L(p24 > mean) << L(23)) |
        (L(p25 > mean) << L(24)) |
        (L(p26 > mean) << L(25)) |
        (L(p27 > mean) << L(26)) |
        (L(p28 > mean) << L(27)) |
        (L(p29 > mean) << L(28)) |
        (L(p30 > mean) << L(29)) |
        (L(p31 > mean) << L(30)) |
        (L(p32 > mean) << L(31)) |
        (L(p33 > mean) << L(32)) |
        (L(p34 > mean) << L(33)) |
        (L(p35 > mean) << L(34)) |
        (L(p36 > mean) << L(35)) |
        (L(p37 > mean) << L(36)) |
        (L(p38 > mean) << L(37)) |
        (L(p39 > mean) << L(38)) |
        (L(p40 > mean) << L(39)) |
        (L(p41 > mean) << L(40)) |
        (L(p42 > mean) << L(41)) |
        (L(p43 > mean) << L(42)) |
        (L(p44 > mean) << L(43)) |
        (L(p45 > mean) << L(44)) |
        (L(p46 > mean) << L(45)) |
        (L(p47 > mean) << L(46)) |
        (L(p48 > mean) << L(47)) |
        (L(p49 > mean) << L(48)) |
        (L(p50 > mean) << L(49)) |
        (L(p51 > mean) << L(50)) |
        (L(p52 > mean) << L(51)) |
        (L(p53 > mean) << L(52)) |
        (L(p54 > mean) << L(53)) |
        (L(p55 > mean) << L(54)) |
        (L(p56 > mean) << L(55)) |
        (L(p57 > mean) << L(56)) |
        (L(p58 > mean) << L(57)) |
        (L(p59 > mean) << L(58)) |
        (L(p60 > mean) << L(59)) |
        (L(p61 > mean) << L(60)) |
        (L(p62 > mean) << L(61)) |
        (L(p63 > mean) << L(62)) |
        (L(p64 > mean) << L(63))
    );

    // Record in output buffer.
    // Use and-not to exclude known overlaps.
    bins[ic] = (ulong4)(b1, b2 & ~b1, b3 & ~b4, b4);
}

