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

#include "cvd-cl/states/cvd/CvdBaseImageState.hh"
#include "cvd-cl/core/Expect.hh"

#include <boost/date_time.hpp>
#include <boost/thread/thread_time.hpp>

namespace CVD {
namespace CL  {

CvdBaseImageState::CvdBaseImageState(Worker & worker, CVD::ImageRef const & size,
        ::cl_channel_order order, ::cl_channel_type type, size_t pbytes) :
    WorkerState(worker),
    size(size),
    pixels(size.x * size.y),
    pbytes(pbytes),
    nbytes(pixels * pbytes),
    mapping(NULL)
{

    // Create image format.
    cl::ImageFormat format(order, type);

    // Allocate image (may throw a CL exception).
    // Most exceptions here are from an unsupported format or a lack of memory.
    image = cl::Image2D(worker.context, CL_MEM_READ_WRITE, format, size.x, size.y, 0);

    // Create origin at 0, 0, 0.
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;

    // Create 3D region for full 2D plane.
    region[0] = size.x;
    region[1] = size.y;
    region[2] = 1;

    // Calculate image row pitch in bytes.
    size_t rpitch = size.x * pbytes;

    // Map image memory.
    mapping = ::calloc(size.x * size.y, pbytes);

    expect("Image memory must be mapped", mapping != NULL);
}

CvdBaseImageState::~CvdBaseImageState() {
    if (mapping != NULL) {
        try {
            ::free(mapping);
        } catch (...) {
            // Ignore any unmapping error.
        }

        mapping = NULL;
    }
}

void CvdBaseImageState::copyToWorker() {
    worker.queue.enqueueWriteImage(image, CL_TRUE, origin, region, 0, 0, mapping);
}

void CvdBaseImageState::copyFromWorker() {
    worker.queue.enqueueReadImage(image, CL_TRUE, origin, region, 0, 0, mapping);
}

void CvdBaseImageState::zero() {
    ::memset(mapping, 0, nbytes);
    copyToWorker();
}

} // namespace CL
} // namespace CVD
