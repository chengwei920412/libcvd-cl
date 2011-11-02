// Copyright (C) 2011  Monash University
// Copyright (C) 2011  Dmitri Nikulin
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

// Undo namespace damage done by CUDA's nvcc.
#undef isfinite
#undef isnan

#include <cvd/fast_corner.h>
#include <cvd/image_io.h>

#include <cuda.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

// Use time() because nvcc isn't compatible with boost.
#include <ctime>

#define X_OFF            8
#define Y_OFF            7
#define FAST_RING        9
#define FAST_THRESH     40
#define REPEAT       10000

// Maximum number of corners.
#define FAST_COUNT (1 << 18)

// Number of threads per 1D group.
#define NTHREADS 512

// Declare 1-byte read-only texture object.
texture<uchar1, 2, cudaReadModeElementType> static testImage;

__device__ int mask_test(uint x16) {
    // Duplicate bit pattern to simulate barrel shift.
    uint const x = (x16 | (x16 << 16));

    // Accumulator.
    uint x1 = x;

    // AND against down-shifts.
    #pragma unroll
    for (uint i = 1; i < FAST_RING; i++)
        x1 &= (x >> i);

    // Return of 1 here proves that FAST_RING
    // consecutive bits were 1.
    return (x1 != 0);
}

__global__ void fast1_kernel(
    int2 * corners,
    int  * icorner
) {

    // Calculate (x,y) of center pixel.
    int const x = ((blockIdx.x * blockDim.x) + threadIdx.x + X_OFF);
    int const y = ((blockIdx.y * blockDim.y) + threadIdx.y + Y_OFF);

    // Read center pixel, upcast to int.
    int const p00 = tex2D(testImage, x, y).x;

    // Include generated code here.
    // Checks ring of pixels, and populates the boolean "isCorner".
    #include "fast1_gen.cu"

    if (isCorner) {
        // Atomically append to corner buffer.
        int const icorn = atomicAdd(icorner, 1);
        if ((icorn >= 0) && (icorn < FAST_COUNT))
            corners[icorn] = make_int2(x, y);
    }
}

__global__ void fast2_kernel(
    int2 const * i_corners,
    int  const * i_ncorners,
    int2       * o_corners,
    int        * o_ncorners
) {

    // Find input offset.
    int const idx = ((blockIdx.x * blockDim.x) + threadIdx.x);
    if ((idx < 0) || (idx >= i_ncorners[0]))
        return;

    // Read (x,y) of center pixel.
    int2 const xy = i_corners[idx];
    int const x = xy.x;
    int const y = xy.y;

    // Read center pixel, upcast to int.
    int const p00 = tex2D(testImage, x, y).x;

    // Include generated code here.
    // Checks ring of pixels, and populates "pattern".
    #include "fast2_gen.cu"

    if (mask_test(pattern)) {
        // Atomically append to corner buffer.
        int const icorn = atomicAdd(o_ncorners, 1);
        if (icorn < FAST_COUNT)
            o_corners[icorn] = xy;
    }
}

static void cufast(CVD::Image<CVD::byte> const & image, int nx, int ny) {
    // Re-interpret image pointer.
    uchar1 const * const data = reinterpret_cast<uchar1 const *>(image.data());

    // Configure texture object.
    testImage.addressMode[0] = cudaAddressModeClamp;
    testImage.addressMode[1] = cudaAddressModeClamp;
    testImage.filterMode     = cudaFilterModePoint;
    testImage.normalized     = false;

    // Create channel descriptor.
    cudaChannelFormatDesc const format = cudaCreateChannelDesc(8, 0, 0, 0, cudaChannelFormatKindUnsigned);

    // Allocate texture array.
    cudaArray * buffer = NULL;
    cudaMallocArray(&buffer, &format, nx, ny);

    // Populate texture array.
    cudaMemcpyToArray(buffer, 0, 0, data, nx * ny * sizeof(uchar1), cudaMemcpyHostToDevice);
    cudaBindTextureToArray(testImage, buffer, format);

    // Allocate corner array 1.
    int2 * corners1 = NULL;
    cudaMalloc(&corners1, sizeof(int2) * FAST_COUNT);

    // Allocate corner array 2.
    int2 * corners2 = NULL;
    cudaMalloc(&corners2, sizeof(int2) * FAST_COUNT);

    // Allocate corner cursor 1.
    int * icorner1 = NULL;
    cudaMalloc(&icorner1, sizeof(int));

    // Allocate corner cursor 2.
    int * icorner2 = NULL;
    cudaMalloc(&icorner2, sizeof(int));

    // Reset corner cursors.
    int const zero = 0;
    cudaMemcpy(icorner1, &zero, sizeof(zero), cudaMemcpyHostToDevice);
    cudaMemcpy(icorner2, &zero, sizeof(zero), cudaMemcpyHostToDevice);

    // Create work grid 1.
    dim3 const dimBlock1(16, 16, 1);
    dim3 const dimGrid1((nx - (X_OFF * 2)) / dimBlock1.x, (ny - (Y_OFF * 2)) / dimBlock1.y, 1);

    // Warmup.
    fast1_kernel<<<dimGrid1, dimBlock1, 0>>>(corners1, icorner1);
    fast2_kernel<<<FAST_COUNT / NTHREADS, NTHREADS>>>(corners1, icorner1, corners2, icorner2);

    // Reset corner cursors.
    cudaMemcpy(icorner1, &zero, sizeof(zero), cudaMemcpyHostToDevice);
    cudaMemcpy(icorner2, &zero, sizeof(zero), cudaMemcpyHostToDevice);

    // Read number of corners.
    int ncorners1 = 0;
    int ncorners2 = 0;

    // Run kernels for time.

    long const time1 = time(NULL);

    for (int i = 0; i < REPEAT; i++) {
        cudaMemcpy(icorner1, &zero, sizeof(zero), cudaMemcpyHostToDevice);
        cudaThreadSynchronize();
        fast1_kernel<<<dimGrid1, dimBlock1, 0>>>(corners1, icorner1);
        cudaThreadSynchronize();
    }

    long const time2 = time(NULL);

    for (int i = 0; i < REPEAT; i++) {
        cudaMemcpy(icorner2, &zero, sizeof(zero), cudaMemcpyHostToDevice);
        cudaThreadSynchronize();
        fast2_kernel<<<FAST_COUNT / NTHREADS, NTHREADS>>>(corners1, icorner1, corners2, icorner2);
        cudaThreadSynchronize();
    }

    long const time3 = time(NULL);

    cudaMemcpy(&ncorners1, icorner1, sizeof(ncorners1), cudaMemcpyDeviceToHost);
    cudaMemcpy(&ncorners2, icorner2, sizeof(ncorners2), cudaMemcpyDeviceToHost);

    // Cleanup.
    cudaFree(icorner2);
    cudaFree(icorner1);
    cudaFree(corners2);
    cudaFree(corners1);
    cudaFreeArray(buffer);

    // Calculate microseconds per kernel.
    int const us1 = (((time2 - time1) * 1000000) / REPEAT);
    int const us2 = (((time3 - time2) * 1000000) / REPEAT);

    // Report timing and number of corners.
    std::cerr << std::setw(8) << ncorners1 << " corners 1" << std::endl;
    std::cerr << std::setw(8) << ncorners2 << " corners 2" << std::endl;
    std::cerr << std::setw(8) << us1 << " microseconds 1" << std::endl;
    std::cerr << std::setw(8) << us2 << " microseconds 2" << std::endl;

    // Prepare corner buffer.
    std::vector<CVD::ImageRef> cvd_corners;
    cvd_corners.reserve(FAST_COUNT);

    long const time4 = time(NULL);

    for (int i = 0; i < REPEAT; i++) {
        cvd_corners.clear();
        CVD::fast_corner_detect_9(image, cvd_corners, FAST_THRESH);
    }

    long const time5 = time(NULL);

    // Read number of corners.
    int const ncorners3 = cvd_corners.size();

    // Calculate microseconds per kernel.
    int const us3 = (((time5 - time4) * 1000000) / REPEAT);

    // Report timing and number of corners.
    std::cerr << std::setw(8) << ncorners3 << " corners 3" << std::endl;
    std::cerr << std::setw(8) << us3 << " microseconds 3" << std::endl;
}

int main(int argc, char **argv) {
    CVD::Image<CVD::byte> const fullImage = CVD::img_load("../images/shuttle.jpg");
    CVD::ImageRef const fullSize = fullImage.size();

    // Image size to keep for computation.
    int const nx = 2048;
    int const ny = 2048;
    CVD::ImageRef const keepSize(nx, ny);

    // Crop to sub-image.
    CVD::Image<CVD::byte> keepImage(keepSize);
    keepImage.copy_from(fullImage.sub_image(CVD::ImageRef(0, 0), keepSize));

    // Test and benchmark CUFAST.
    cufast(keepImage, nx, ny);

    return 0;
}
