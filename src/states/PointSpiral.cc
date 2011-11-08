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
        point.score = ((score + 10) / 20);

        // Count towards total x and y.
        center.x += position.x;
        center.y += position.y;
    }

    // Estimate center as average of x and y.
    if (npoints > 0) {
        center.x /= npoints;
        center.y /= npoints;
    }

    // Calculate distance from center for each spiral point.
    for (cl_int i = 0; i < npoints; i++) {
        SpiralPoint & point = spiral.at(i);
        point.distance = cl_int(std::ceil(dist(point.position, center)));
    }

    // Order spiral points by distance from center.
    std::sort(spiral.begin(), spiral.end());
}

cl_int2 mki2(cl_int x, cl_int y) {
    cl_int2 out = {{x, y}};
    return out;
}

cl_int cost(SpiralPoint const & p1, SpiralPoint const & p2) {
    return ((p1.score == p2.score) ? 1 : 0);
}

void matchPointSpirals(std::vector<cl_int2> & pairs, SpiralPoints const & spiral1, SpiralPoints const & spiral2) {
    // Calculates longest common subsequence by matching the 'stepback' in each 'Point'.

    pairs.clear();

    cl_int const n1 = spiral1.size();
    cl_int const n2 = spiral2.size();

    // Create longest common subsequence grid.
    blitz::Array<cl_int, 2> grid(n1 + 1, n2 + 1);
    grid = 0;

    for (cl_int i1 = 1; i1 <= n1; i1++) {
        SpiralPoint const & p1 = spiral1.at(i1 - 1);

        for (cl_int i2 = 1; i2 <= n2; i2++) {
            SpiralPoint const & p2 = spiral2.at(i2 - 1);

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
        SpiralPoint const & p1 = spiral1.at(i1 - 1);
        SpiralPoint const & p2 = spiral2.at(i2 - 1);

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
            // Record the match.
            cl_int2 const pair = {{i1 - 1, i2 - 1}};
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
