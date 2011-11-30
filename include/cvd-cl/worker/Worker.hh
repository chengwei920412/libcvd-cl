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

#ifndef __CVD_CL_WORKER_HH__
#define __CVD_CL_WORKER_HH__

#include <boost/cstdint.hpp>
#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>

// Include official OpenCL C++ wrapper, with exceptions enabled.
#define __CL_ENABLE_EXCEPTIONS
#include "CL/cl.hpp"

namespace CVD {
namespace CL  {

/// A single OpenCL device that can maintain states and execute steps.
/// \see WorkerState
/// \see WorkerStep
class Worker : public boost::noncopyable {
public:

    /// \brief Construct a worker for a given OpenCL device.
    ///
    /// A context and command queue are created automatically.
    explicit Worker(cl::Device & device);

    /// \brief Release resources allocated for this worker.
    virtual ~Worker();

    /// \brief Convenience method to compile an OpenCL program with a single kernel.
    ///
    /// Consider using embed.py to generate headers with string constants.
    ///
    /// \param program  OpenCL program object that will receive the compiled program (given by \a source and \a options).
    /// \param kernel   OpenCL kernel object that will receive the program's kernel (given by \a name).
    /// \param source   OpenCL program source code.
    /// \param name     Kernel name as found in \a source.
    /// \param options  OpenCL compiler options.
    void compile(cl::Program * program, cl::Kernel * kernel,
                 char const  * source,  char const * name,
                 char const  * options = "");

    /// \brief Issue an OpenCL command queue barrier using \c clEnqueueBarrier.
    void barrier();

    /// \brief Wait for the OpenCL command queue to finish using \c clFinish.
    void finish();


    // --- Leave members public for direct access by steps and states ---


    /// \brief OpenCL device used by this worker.
    cl::Device               device;

    /// \brief Vector of OpenCL devices, containing only #device.
    std::vector<cl::Device>  devices;

    /// \brief OpenCL context containing only #device.
    cl::Context              context;

    /// \brief OpenCL command queue related to #device and #context.
    cl::CommandQueue         queue;

    /// \brief Maximum local work group size for 1 dimension, as determined from \c clGetInfo.
    size_t      const        maxLocalSize;

    /// \brief Default local work group size for 1 dimension, based on
    /// #maxLocalSize and an implicit limit (currently 512) that is
    /// necessary for unusual implementations.
    size_t      const        defaultLocalSize;

    /// \brief #defaultLocalSize wrapped in a \c cl::NDRange.
    cl::NDRange const        defaultLocal;

    /// \brief Round global size up to a multiple of #defaultLocalSize.
    /// \param items  Number of items actually in the global work size.
    /// \return An integer at least as large as \a items and a
    ///         multiple of #defaultLocalSize.
    size_t                   padGlobalSize(size_t items);

    /// \brief Round global size up to a multiple of #defaultLocalSize
    /// and wrap in a \c cl::NDRange.
    /// \return A \c cl::NDRange with 1 dimension at least as large as \a items and a
    ///         multiple of #defaultLocalSize.
    cl::NDRange              padGlobal(size_t items);
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_WORKER_HH__ */
