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

#include "cvd-cl/steps/MixUvqUvStep.hh"

#include "kernels/fmix.hh"

namespace CVD {
namespace CL  {

MixUvqUvStep::MixUvqUvStep(UvqUvState<1> & i_uvquv, UvqUvState<3> & o_uvquv) :
    WorkerStep (i_uvquv.worker),
    i_uvquv    (i_uvquv),
    o_uvquv    (o_uvquv),
    m_max      (worker, i_uvquv.maxCount),
    m_ints     (worker, o_uvquv.maxRecords),
    randomise  (m_max, m_ints)
{
    worker.compile(&program, &kernel, OCL_FMIX, "fmix");
}

MixUvqUvStep::~MixUvqUvStep() {
    // Do nothing.
}

void MixUvqUvStep::execute() {
    // Prepare and run random integer generator.
    m_max.setCount(i_uvquv.setCount);
    randomise.execute();

    // Prepare global work size.
    cl::NDRange const global(o_uvquv.maxRecords);

    // Assign output count.
    o_uvquv.setCount = o_uvquv.maxCount;

    // Finish any outstanding work.
    worker.finish();

    // Assign integer offset buffer.
    kernel.setArg(0, m_ints.buffer);

    // Translate u in uvq.
    kernel.setArg(1, i_uvquv.uvq.us.memory);
    kernel.setArg(2, o_uvquv.uvq.us.memory);
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    worker.finish();

    // Translate v in uvq.
    kernel.setArg(1, i_uvquv.uvq.vs.memory);
    kernel.setArg(2, o_uvquv.uvq.vs.memory);
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    worker.finish();

    // Translate q in uvq.
    kernel.setArg(1, i_uvquv.uvq.qs.memory);
    kernel.setArg(2, o_uvquv.uvq.qs.memory);
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    worker.finish();

    // Translate u in uv.
    kernel.setArg(1, i_uvquv.uv.us.memory);
    kernel.setArg(2, o_uvquv.uv.us.memory);
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    worker.finish();

    // Translate v in uv.
    kernel.setArg(1, i_uvquv.uv.vs.memory);
    kernel.setArg(2, o_uvquv.uv.vs.memory);
    worker.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global, cl::NullRange);
    worker.finish();
}

} // namespace CL
} // namespace CVD
