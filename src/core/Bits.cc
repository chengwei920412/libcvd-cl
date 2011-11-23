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

#include "cvd-cl/core/Bits.hh"

#include <bitset>

namespace CVD {
namespace CL  {

static uint8_t * toBytes(void * data) {
    return reinterpret_cast<uint8_t *>(data);
}

static uint8_t const * toBytes(void const * data) {
    return reinterpret_cast<uint8_t const *>(data);
}

void    memZero     (void * data, size_t nbytes) {
    ::memset(data, 0, nbytes);
}

void    memFlip     (void * data, size_t nbytes) {
    uint8_t * bytes = toBytes(data);
    for (size_t i = 0; i < nbytes; i++)
        bytes[i] = ~bytes[i];
}

void    memCopy     (void * odata, void const * idata, size_t nbytes) {
    ::memcpy(odata, idata, nbytes);
}

void    memOR       (void * odata, void const * idata1, void const * idata2, size_t nbytes) {
    uint8_t       * obytes  = toBytes(odata);
    uint8_t const * ibytes1 = toBytes(idata1);
    uint8_t const * ibytes2 = toBytes(idata2);

    for (size_t i = 0; i < nbytes; i++)
        obytes[i] = (ibytes1[i] | ibytes2[i]);
}

void    memAND      (void * odata, void const * idata1, void const * idata2, size_t nbytes) {
    uint8_t       * obytes  = toBytes(odata);
    uint8_t const * ibytes1 = toBytes(idata1);
    uint8_t const * ibytes2 = toBytes(idata2);

    for (size_t i = 0; i < nbytes; i++)
        obytes[i] = (ibytes1[i] & ibytes2[i]);
}

void    memXOR      (void * odata, void const * idata1, void const * idata2, size_t nbytes) {
    uint8_t       * obytes  = toBytes(odata);
    uint8_t const * ibytes1 = toBytes(idata1);
    uint8_t const * ibytes2 = toBytes(idata2);

    for (size_t i = 0; i < nbytes; i++)
        obytes[i] = (ibytes1[i] ^ ibytes2[i]);
}

size_t  memBitCount (void const * data, size_t nbytes) {
    uint8_t const * bytes = toBytes(data);

    size_t total = 0;

    for (size_t i = 0; i < nbytes; i++)
        total += __builtin_popcountl(bytes[i]);

    return total;
}

} // namespace CL
} // namespace CVD
