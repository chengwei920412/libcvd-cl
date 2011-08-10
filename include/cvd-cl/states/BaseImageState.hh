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

#ifndef __CVD_CL_BASE_IMAGE_STATE_HH__
#define __CVD_CL_BASE_IMAGE_STATE_HH__

#include <cvd-cl/worker/WorkerState.hh>

#include <cvd/image_ref.h>

namespace CVD {
namespace CL  {

class BaseImageState : public WorkerState {
public:

    explicit BaseImageState(Worker & worker, CVD::ImageRef const & size,
            ::cl_channel_order order, ::cl_channel_type type, size_t pbytes);
    virtual ~BaseImageState();

    void copyToWorker();
    void copyFromWorker();
    void zero();

    // Public immutable members.
    CVD::ImageRef const   size;
    size_t        const   pbytes;
    size_t        const   nbytes;

    // Members left public for WorkerStep access.
    cl::Image2D           image;
    cl::size_t<3>         origin;
    cl::size_t<3>         region;
    void                * mapping;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_BASE_IMAGE_STATE_HH__ */
