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

#ifndef __CVD_CL_WORKER_HH__
#define __CVD_CL_WORKER_HH__

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>

// Include official OpenCL C++ wrapper, with exceptions enabled.
#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

namespace CVD {
namespace CL  {

class Worker : public boost::noncopyable {
public:

    explicit Worker(cl::Device & device);
    virtual ~Worker();

    void compile(cl::Program * program, cl::Kernel * kernel,
                 char const  * source,  char const * name,
                 char const  * options = "");

    void barrier();
    void finish();

    // Leave public for direct access by steps and states.
    cl::Device               device;
    std::vector<cl::Device>  devices;
    cl::Context              context;
    cl::CommandQueue         queue;
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_WORKER_HH__ */
