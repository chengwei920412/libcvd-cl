// Copyright (C) 2011  Dmitri Nikulin
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

#ifndef __CL_CVD_TEST_CUDA_COMMON_H__
#define __CL_CVD_TEST_CUDA_COMMON_H__

#include <cvd/fast_corner.h>
#include <cvd/image_io.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

// Use time() because nvcc isn't compatible with boost.
#include <ctime>

#define X_OFF            8
#define Y_OFF            8
#define FAST_RING        9
#define FAST_THRESH     40
#define REPEAT       10000

// Maximum number of corners.
#define FAST_COUNT (1 << 18)

#endif /* __CL_CVD_TEST_CUDA_COMMON_H__ */
