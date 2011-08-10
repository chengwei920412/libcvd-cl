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

#include "cvd-cl/states/ImageState.hh"
#include "cvd-cl/core/Expect.hh"

namespace CVD {
namespace CL  {

cl::ImageFormat const static FORMAT  (CL_INTENSITY, CL_UNSIGNED_INT8);
cl_mem_flags    const static FLAGS = (CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR);

ImageState::ImageState(Worker & worker, CVD::ImageRef const & size) :
    WorkerState(worker),
    size(size),
    nbytes(size.x * size.y),
    bytes(NULL)
{

    // Allocate image (may throw a CL exception).
    // Most exceptions here are from an unsupported format or a lack of memory.
    image = cl::Image2D(worker.context, FLAGS, FORMAT, size.x, size.y, 0);

    // Create origin at 0, 0, 0.
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;

    // Create 3D region for full 2D plane.
    region[0] = size.y;
    region[1] = size.y;
    region[2] = 1;

    // Calculate image row pitch in bytes.
    size_t rpitch = size.x * sizeof(CVD::byte);

    // Map image memory.
    void * data = worker.queue.enqueueMapImage(image, CL_TRUE, CL_MAP_READ | CL_MAP_WRITE, origin, region, &rpitch, NULL);

    // Re-interpret image memory pointer.
    bytes = reinterpret_cast<CVD::byte *>(data);
}

ImageState::~ImageState() {
    if (bytes != NULL) {
        try {
            worker.queue.enqueueUnmapMemObject(image, bytes);
            worker.finish();
        } catch (...) {
            // Ignore any unmapping error.
        }

        bytes = NULL;
    }
}

ByteImage ImageState::asImage() {
    return ByteImage(bytes, size);
}

void ImageState::copyToWorker() {
    worker.queue.enqueueWriteImage(image, CL_TRUE, origin, region, 0, 0, bytes);
}

void ImageState::copyFromWorker() {
    worker.queue.enqueueReadImage(image, CL_TRUE, origin, region, 0, 0, bytes);
}

void ImageState::zero() {
    ::memset(bytes, 0, nbytes);
    copyToWorker();
}

void ImageState::set(ByteSubImage const & image) {
    CVD::ImageRef const size = image.size();

    expect("ImageState::set() must be given an image of the same size",
            this->size == size);

    asImage().copy_from(image);
    copyToWorker();
}

void ImageState::get(ByteSubImage       * image) {
    CVD::ImageRef const size = image->size();

    expect("ImageState::get() must be given an image of the same size",
            this->size == size);

    copyFromWorker();
    image->copy_from(asImage());
}

} // namespace CL
} // namespace CVD
