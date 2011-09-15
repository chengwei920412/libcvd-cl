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

#include "cvd-cl/steps/HipsTurnStep.hh"
#include "kernels/hips-find.hh"
#include "kernels/hips-turn.hh"

#ifdef CVD_CL_VERBOSE
#include <iomanip>
#include <iostream>
#endif

namespace CVD {
namespace CL  {

HipsTurnStep::HipsTurnStep(HipsListState & i_hips1, HipsListState & i_hips2, PointListState & o_matches, cl_int maxerr) :
    WorkerStep (i_hips1.worker),
    i_hips1    (i_hips1),
    i_hips2    (i_hips2),
    o_matches  (o_matches),
    m_hips1    (worker, i_hips1.size),
    m_hips2    (worker, i_hips1.size),
    m_best     (worker, o_matches.size),
    maxerr     (maxerr)
{
    // Compile turning kernel.
    worker.compile(&program_turn, &kernel_turn, OCL_HIPS_TURN, "hips_turn");

    // Compile finding kernel.
    char opt[256] = {0,};
    snprintf(opt, sizeof(opt) - 1, "-DHIPS_MAX_ERROR=%d", int(maxerr));
    worker.compile(&program_find, &kernel_find, OCL_HIPS_FIND, "hips_find", opt);
}

HipsTurnStep::~HipsTurnStep() {
    // Do nothing.
}

void HipsTurnStep::execute() {
    // Read number of descriptors.
    size_t const np1 = i_hips1.getCount();
    size_t const np2 = i_hips2.getCount();

    // Round down number of descriptors.
    cl_int const np1_16 = (np1 / 16) * 16;
    cl_int const np2_16 = (np2 / 16) * 16;

    // Create 1D work size (turn kernel).
    cl::NDRange const global1D(np1_16);

    // Create 2D work size (find kernel).
    cl::NDRange const global2D(np1_16, np2_16);
    cl::NDRange const local2D(16, 16);

    // Assign find kernel parameters except first list.
    kernel_find.setArg(1, i_hips2.buffer);
    kernel_find.setArg(2, o_matches.buffer);
    kernel_find.setArg(3, o_matches.count);
    kernel_find.setArg(4, o_matches.size);

    // Keep note of best results.
    cl_uint bestCount = 0;
    m_best.setCount(0);

    // Keep pointers to buffers, to swap them.
    HipsListState * list1 = &m_hips1;
    HipsListState * list2 = &m_hips2;

    // Copy HIPS descriptors to first internal buffer.
    m_hips1.copyFrom(i_hips1);

    // Try 16 rotations.
    for (int rot = 0; rot < 16; rot++) {
        // Reset number of output pairs.
        o_matches.setCount(0);

        // Match rotated first list against second list.
        kernel_find.setArg(0, list1->buffer);
        worker.queue.enqueueNDRangeKernel(kernel_find, cl::NullRange, global2D, local2D);

        // Note number of matches.
        cl_uint const thisCount = o_matches.getCount();

#ifdef CVD_CL_VERBOSE
        std::cerr << "Rotation " << std::setw(3) << rot << " produces " << std::setw(8) << thisCount << " matches" << std::endl;
#endif

        // Check if this number is an improvement.
        if (thisCount > bestCount) {
            bestCount = thisCount;
            // Save the best list.
            m_best.copyFrom(o_matches);
        }

        if (rot < 15) {
            // Rotate the HIPS descriptors in list 1 to list 2.
            kernel_turn.setArg(0, list1->buffer);
            kernel_turn.setArg(1, list2->buffer);
            worker.queue.enqueueNDRangeKernel(kernel_turn, cl::NullRange, global1D, cl::NullRange);

            // Swap list pointers.
            std::swap(list1, list2);
        }
    }

    // Restore best list.
    o_matches.copyFrom(m_best);
}

} // namespace CL
} // namespace CVD
