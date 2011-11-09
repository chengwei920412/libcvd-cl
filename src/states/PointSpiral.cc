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

#include "cvd-cl/states/PointSpiral.hh"

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

void makePointSpiral(SpiralPoints & spiral, std::vector<cl_int2> const & positions, std::vector<int> const & scores) {
    // Allocate spiral points for each position.
    cl_int const npoints = positions.size();
    spiral.resize(npoints);

    // Start with center estimate at (0,0).
    cl_int2 center = {{0, 0}};

    for (cl_int i = 0; i < npoints; i++) {
        SpiralPoint & point = spiral.at(i);
        cl_int2 const & position = positions.at(i);
        int const score = scores.at(i);

        // Record spiral point position and quantized score.
        point.position = position;
        point.score = (score / 10);

        // Count towards total x and y.
        center.x += position.x;
        center.y += position.y;
    }

    // Estimate center as average of x and y.
    if (npoints > 0) {
        center.x /= npoints;
        center.y /= npoints;
    }

    // Calculate distance and angle from center for each spiral point.
    for (cl_int i = 0; i < npoints; i++) {
        SpiralPoint & point = spiral.at(i);

        // Calculate Euclidian distance.
        point.distance = cl_int(std::ceil(dist(point.position, center)));

        // Calculate signed (x,y) difference.
        double const dx = (point.position.x - center.x);
        double const dy = (point.position.y - center.y);

        // Calculate point angle.
        point.angle = std::atan2(dy, dx);
    }

    // Order spiral points by angle.
    std::sort(spiral.begin(), spiral.end());
}

static cl_int cost(SpiralPoint const & p1, SpiralPoint const & p2) {
    return ((p1.score == p2.score) ? 1 : 0);
}

void matchPointSpirals(std::vector<cl_int2> & pairs, SpiralPoints const & spiral1, SpiralPoints const & spiral2) {
    // Calculates longest common subsequence by matching each 'Point'.

    pairs.clear();

    // Extend each spiral with a duplicate of itself.
    SpiralPoints espiral1;
    SpiralPoints espiral2;
    espiral1.insert(espiral1.end(), spiral1.begin(), spiral1.end());
    espiral1.insert(espiral1.end(), spiral1.begin(), spiral1.end());
    espiral2.insert(espiral2.end(), spiral2.begin(), spiral2.end());
    espiral2.insert(espiral2.end(), spiral2.begin(), spiral2.end());
    assert(espiral1.size() == (2 * spiral1.size()));
    assert(espiral2.size() == (2 * spiral2.size()));

    cl_int const n1 = espiral1.size();
    cl_int const n2 = espiral2.size();

    // Create longest common subsequence grid.
    blitz::Array<cl_int, 2> grid(n1 + 1, n2 + 1);
    grid = 0;

    for (cl_int i1 = 1; i1 <= n1; i1++) {
        SpiralPoint const & p1 = espiral1.at(i1 - 1);

        for (cl_int i2 = 1; i2 <= n2; i2++) {
            SpiralPoint const & p2 = espiral2.at(i2 - 1);

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
        SpiralPoint const & p1 = espiral1.at(i1 - 1);
        SpiralPoint const & p2 = espiral2.at(i2 - 1);

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
        } else if (gridPlan == gridTake) {
            // Modulo indices back into original point list.
            cl_int  const i1b = ((i1 - 1) % spiral1.size());
            cl_int  const i2b = ((i2 - 1) % spiral2.size());

            // Record the match.
            cl_int2 const pair = {{i1b, i2b}};
            pairs.push_back(pair);

            // Back-track one spot in each list.
            i1--;
            i2--;
        } else if (gridPlan == gridHit1) {
            // Back-track one spot in second list.
            i2--;
        } else if (gridPlan == gridHit2) {
            // Back-track one spot in first list.
            i1--;
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
