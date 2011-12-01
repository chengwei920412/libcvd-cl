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

#ifndef __CVD_CL_POINT_RADAR_HH__
#define __CVD_CL_POINT_RADAR_HH__

#include <cvd-cl/worker/Worker.hh>

namespace CVD {
namespace CL  {

/// \brief A single feature point with calculations for radar matching.
struct RadarPoint {
    /// \brief Position in pixel coordinates.
    cl_int2 position;

    /// \brief Discriminating integer, such as quantised FAST score.
    cl_int  score;

    /// \brief Distance from radar center.
    double  distance;

    /// \brief Angle from radar axis.
    double  angle;

    /// \brief Compare radar points by angle.
    ///
    /// \param b RadarPoint to compare to.
    inline bool operator <(RadarPoint const & b) const {
        return (angle < b.angle);
    }
};

/// \brief A vector of RadarPoint structures.
typedef std::vector<RadarPoint> RadarPoints;

/// \brief Construct a radar report from \a positions, \a scores and \a center.
///
/// \pre \code
/// positions.size() == scores.size()
/// \endcode
///
/// \param radar       RadarPoints that will be filled and calculated.
/// \param positions   Feature point pixel coordinates.
/// \param scores      Scores corresponding to feature points.
/// \param center      Radar center point.
void makePointRadar(RadarPoints & radar, std::vector<cl_int2> const & positions, std::vector<int> const & scores, cl_int2 const center);

/// \brief Attempt to match features between two radar reports.
///
/// \param pairs       Buffer for matched pairs.
/// \param radar1      Radar report to match.
/// \param radar2      Radar report to match.
void matchPointRadars(std::vector<cl_int2> & pairs, RadarPoints const & radar1, RadarPoints const & radar2);

} // namespace CL
} // namespace CVD

#endif /* __CVD_CL_POINT_RADAR_HH__ */
