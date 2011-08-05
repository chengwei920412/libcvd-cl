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

#ifndef __CVD_CL_STEP_HH__
#define __CVD_CL_STEP_HH__

#include <boost/cstdint.hpp>
#include <boost/date_time.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/thread_time.hpp>

namespace CVD {
namespace CL  {

template<class IState, class OState>
class Step : public boost::noncopyable {
public:

    Step() {}
    virtual ~Step() {}

    virtual void execute(IState & istate, OState & ostate) = 0;

    virtual boost::system_time::time_rep_type measure(IState & istate, OState & ostate, int repeat=10) {
        boost::system_time const t1 = boost::get_system_time();
        for (int i = 0; i < repeat; i++)
            execute(istate, ostate);
        boost::system_time const t2 = boost::get_system_time();
        return ((t2 - t1).total_microseconds() / repeat);
    }
};

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_STEP_HH__ */
