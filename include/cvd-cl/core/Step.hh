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

#ifndef __CVD_CL_STEP_HH__
#define __CVD_CL_STEP_HH__

#include <boost/cstdint.hpp>
#include <boost/date_time.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/thread_time.hpp>

namespace CVD {
namespace CL  {

/// \brief A completely abstract step of computation.
///
/// A Step should be constructed with references to input and output State objects,
/// possibly none.
///
/// \see WorkerStep
class Step : public boost::noncopyable {
public:

    /// \brief Construct the Step (does nothing).
    explicit Step();

    /// \brief De-construct the Step (does nothing).
    virtual ~Step();

    /// \brief Execute the Step.
    ///
    /// Override this method in sub-classes.
    virtual void execute() = 0;

    /// \brief Call execute() repeatedly and estimate average runtime.
    ///
    /// \param Number of times to call execute().
    virtual int64_t measure(int repeat=10);
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_STEP_HH__ */
