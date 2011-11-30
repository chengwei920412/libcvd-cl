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

#ifndef __CVD_CL_BITS_HH__
#define __CVD_CL_BITS_HH__

#include <cvd-cl/worker/Worker.hh>

namespace CVD {
namespace CL  {

/// \brief Reset memory to zero.
///
/// Equivalent to \c memset(data, 0, nbytes)
///
/// \param data    Pointer to memory that will be zeroed.
/// \param nbytes  Number of bytes in \a data to zero.
void    memZero     (void * data, size_t nbytes);

/// \brief Invert all bits in memory.
///
/// \param data    Pointer to memory that will be flipped.
/// \param nbytes  Number of bytes in \a data to flip.
void    memFlip     (void * data, size_t nbytes);

/// \brief Copy memory between two pointers.
///
/// Equivalent to \c memcpy(odata, idata, nbytes)
///
/// \param idata   Pointer to memory that will be copied FROM.
/// \param odata   Pointer to memory that will be copied TO.
/// \param nbytes  Number of bytes to copy.
void    memCopy     (void * odata, void const * idata, size_t nbytes);

/// \brief OR memory between two pointers.
///
/// \code
/// odata = (idata1 | idata2);
/// \endcode
///
/// \param idata1  Pointer to memory that will be read FROM.
/// \param idata2  Pointer to memory that will be read FROM.
/// \param odata   Pointer to memory that will be written TO.
/// \param nbytes  Number of bytes to OR.
void    memOR       (void * odata, void const * idata1, void const * idata2, size_t nbytes);

/// \brief AND memory between two pointers.
///
/// \code
/// odata = (idata1 & idata2);
/// \endcode
///
/// \param idata1  Pointer to memory that will be read FROM.
/// \param idata2  Pointer to memory that will be read FROM.
/// \param odata   Pointer to memory that will be written TO.
/// \param nbytes  Number of bytes to AND.
void    memAND      (void * odata, void const * idata1, void const * idata2, size_t nbytes);

/// \brief XOR memory between two pointers.
///
/// \code
/// odata = (idata1 ^ idata2);
/// \endcode
///
/// \param idata1  Pointer to memory that will be read FROM.
/// \param idata2  Pointer to memory that will be read FROM.
/// \param odata   Pointer to memory that will be written TO.
/// \param nbytes  Number of bytes to XOR.
void    memXOR      (void * odata, void const * idata1, void const * idata2, size_t nbytes);

/// \brief Count 1 bits in memory.
///
/// \param data    Pointer to memory that will be read.
/// \param nbytes  Number of bytes for which to count bits.
size_t  memBitCount (void const * data, size_t nbytes);

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_BITS_HH__ */
