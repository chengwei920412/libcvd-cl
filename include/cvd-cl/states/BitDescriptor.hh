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

#ifndef __CVD_CL_BIT_DESCRIPTOR_HH__
#define __CVD_CL_BIT_DESCRIPTOR_HH__

#include <cvd-cl/core/Bits.hh>

namespace CVD {
namespace CL  {

// A bit descriptor can be of any type.
// Recommended types are cl_ulong2, cl_ulong4, cl_ulong8, etc.

// The following functions must be defined for any valid descriptor.
// The defaults are likely fine for most POD types, but will not be
// nearly as fast as specialised code.

/// \brief Clear a bit descriptor to its zero state.
///
/// \param desc  Pointer to descriptor.
template<class BDT>
void clearBitDescriptor(BDT * desc) {
    memZero(desc, sizeof(BDT));
}

/// \brief Combine two descriptors, usually by bitwise OR.
///
/// \param idesc1  Pointer to descriptor to read FROM.
/// \param idesc2  Pointer to descriptor to read FROM.
/// \param odesc   Pointer to descriptor to write TO.
template<class BDT>
void mergeBitDescriptors(BDT * odesc, BDT const & idesc1, BDT const & idesc2) {
    memOR(odesc, &idesc1, &idesc2, sizeof(BDT));
}

/// \brief Calculate the "difference" between two descriptors.
///
/// Must be symmetrical and non-negative.
/// Not necessarily the same as the "error" of a test descriptor
/// against a reference descriptor, which must also be
/// non-negative but does not have to be symmetrical.
///
/// \param idesc1  Pointer to descriptor.
/// \param idesc2  Pointer to descriptor.
/// \return Difference between the two descriptors.
///
/// \see ::errorBitDescriptors
template<class BDT>
int diffBitDescriptors(BDT const & idesc1, BDT const & idesc2) {
    BDT bitxor;
    memXOR(&bitxor, &idesc1, &idesc2, sizeof(BDT));
    return memBitCount(&bitxor, sizeof(BDT));
}

/// \brief Calculate the "error" of a test descriptor with
/// respect to a reference descriptor.
///
/// Must be non-negative but not necessarily symmetrical.
///
/// \param tdesc  Pointer to test descriptor.
/// \param rdesc  Pointer to reference descriptor.
/// \return Error of the test descriptor with respect to the reference descriptor.
///
/// \see ::diffBitDescriptors
template<class BDT>
int errorBitDescriptors(BDT const & tdesc, BDT const & rdesc) {
    // Compute (~rdesc).
    BDT rcomp;
    memCopy(&rcomp, &rdesc, sizeof(BDT));
    memFlip(&rcomp, sizeof(BDT));

    // Compute (tdesc & ~rdesc);
    BDT trand;
    memAND(&trand, &tdesc, &rcomp, sizeof(BDT));

    // Compute bit count.
    return memBitCount(&trand, sizeof(BDT));
}

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_BIT_DESCRIPTOR_HH__ */
