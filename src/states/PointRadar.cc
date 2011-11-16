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

#include "cvd-cl/states/PointRadar.hh"

#include <cmath>
#include <cstdio>

#include <algorithm>

#include <blitz/array.h>

namespace CVD {
namespace CL  {

static double sq(double const x) {
    return (x * x);
}

static double dist(cl_int2 const p1, cl_int2 const p2) {
    double const dx = sq(p1.x - p2.x);
    double const dy = sq(p1.y - p2.y);
    assert(dx >= 0);
    assert(dy >= 0);
    return std::sqrt(dx + dy);
}

void makePointRadar(RadarPoints & radar, std::vector<cl_int2> const & positions, std::vector<int> const & scores, cl_int2 const center) {
    // Allocate radar points for each position.
    cl_int const npoints = positions.size();
    radar.resize(npoints);

    // Calculate distance and angle from center for each radar point.
    for (cl_int i = 0; i < npoints; i++) {
        RadarPoint & point = radar.at(i);

        // Copy position vector and quantised score.
        point.position = positions.at(i);
        point.score    = (scores.at(i) / 5);

        // Calculate Euclidian distance.
        point.distance = dist(point.position, center);

        // Calculate signed (x,y) difference.
        double const dx = (point.position.x - center.x);
        double const dy = (point.position.y - center.y);

        // Calculate point angle.
        point.angle = std::atan2(dy, dx);
    }

    // Order radar points by angle.
    std::sort(radar.begin(), radar.end());
}

static cl_int cost(RadarPoint const & p1, RadarPoint const & p2) {
    return (std::abs(p1.distance - p2.distance) < 20);
}

void matchPointRadars(std::vector<cl_int2> & pairs, RadarPoints const & radar1, RadarPoints const & radar2) {
    // Calculates longest common subsequence by matching each 'Point'.

    pairs.clear();

    // Extend each radar with a duplicate of itself.
    RadarPoints eradar1;
    RadarPoints eradar2;
    eradar1.insert(eradar1.end(), radar1.begin(), radar1.end());
    eradar1.insert(eradar1.end(), radar1.begin(), radar1.end());
    eradar2.insert(eradar2.end(), radar2.begin(), radar2.end());
    eradar2.insert(eradar2.end(), radar2.begin(), radar2.end());
    assert(eradar1.size() == (2 * radar1.size()));
    assert(eradar2.size() == (2 * radar2.size()));

    cl_int const n1 = eradar1.size();
    cl_int const n2 = eradar2.size();

    // Create longest common subsequence grid.
    blitz::Array<cl_int, 2> grid(n1 + 1, n2 + 1);
    grid = 0;

    for (cl_int i1 = 1; i1 <= n1; i1++) {
        RadarPoint const & p1 = eradar1.at(i1 - 1);

        for (cl_int i2 = 1; i2 <= n2; i2++) {
            RadarPoint const & p2 = eradar2.at(i2 - 1);

            // Start with no length.
            cl_int length = 0;

            // Extend with each recursion.
            length = std::max(length, grid(i1 - 1, i2 - 1));
            length = std::max(length, grid(i1    , i2 - 1));
            length = std::max(length, grid(i1 - 1, i2    ));

            // Extend with match.
            length = std::max(length, grid(i1 - 1, i2 - 1) + cost(p1, p2));

            // Record in grid.
            grid(i1, i2) = length;
        }
    }

    // Back-track through grid.
    cl_int i1 = n1;
    cl_int i2 = n2;

    while ((i1 > 0) && (i2 > 0)) {
        RadarPoint const & p1 = eradar1.at(i1 - 1);
        RadarPoint const & p2 = eradar2.at(i2 - 1);

        // Recall all grid possibilities.
        cl_int const gridSkip = grid(i1 - 1, i2 - 1);
        cl_int const gridHit1 = grid(i1    , i2 - 1);
        cl_int const gridHit2 = grid(i1 - 1, i2    );
        cl_int const gridTake = gridSkip + cost(p1, p2);

        // Recall chosen path.
        cl_int const gridPlan = grid(i1    , i2    );

        // Check against recorded value.
        if (gridPlan == gridSkip) {
            // Back-track one spot in each list.
            i1--;
            i2--;
        } else if (gridPlan == gridHit1) {
            // Back-track one spot in second list.
            i2--;
        } else if (gridPlan == gridHit2) {
            // Back-track one spot in first list.
            i1--;
        } else if (gridPlan == gridTake) {
            // Modulo indices back into original point list.
            cl_int  const i1b = ((i1 - 1) % radar1.size());
            cl_int  const i2b = ((i2 - 1) % radar2.size());

            // Record the match.
            cl_int2 const pair = {{i1b, i2b}};
            pairs.push_back(pair);

            // Back-track one spot in each list.
            i1--;
            i2--;
        } else {
            // Weird, abort.
            assert(false);
            i1 = 0;
            i2 = 0;
        }
    }
}

} // namespace CL
} // namespace CVD
