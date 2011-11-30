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

#ifndef __CVD_CL_EXPECT_HH__
#define __CVD_CL_EXPECT_HH__

#include <cassert>
#include <stdexcept>

namespace CVD {
namespace CL  {

/// \brief An exception thrown when a function or method pre-condition is not satisfied.
class ExpectationError : public std::invalid_argument {
public:

    /// \brief Construct the ExpectationError with a given message string.
    ///
    /// Avoid using this directly, use ::expect or a similar wrapper.
    ///
    /// \param message Message string
    ///
    /// \see expect
    explicit ExpectationError(const std::string & message) :
        std::invalid_argument(message) {
        // Do nothing.
    }
};

/// \brief Check that a pre-condition \a state is satisfied, and if not, throw an ExpectationError
/// with the given \a message.
///
/// \param message  Message given to the ExpectationError if the \a state is not true.
/// \param state     State to check before throwing an exception.
static void expect(char const * message, bool state) {
    if (state == false)
        throw ExpectationError(message);
}

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_EXPECT_HH__ */
