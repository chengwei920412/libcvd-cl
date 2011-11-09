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

#include "cvd-cl/states/PointGalaxy.hh"

#include <cmath>
#include <cstdio>

#include <algorithm>

#include <blitz/array.h>

namespace CVD {
namespace CL  {

static double sq(double const x) {
    assert(std::isfinite(x));
    return (x * x);
}

static double squareDistance(Point3D const & p1, Point3D const & p2) {
    double const x1 = p1(0);
    double const y1 = p1(1);
    double const z1 = p1(2);
    double const x2 = p2(0);
    double const y2 = p2(1);
    double const z2 = p2(2);

    assert(std::isfinite(x1));
    assert(std::isfinite(y1));
    assert(std::isfinite(z1));
    assert(std::isfinite(x2));
    assert(std::isfinite(y2));
    assert(std::isfinite(z2));

    double const dx = sq(p1(0) - p2(0));
    double const dy = sq(p1(1) - p2(1));
    double const dz = sq(p1(2) - p2(2));

    assert(dx >= 0);
    assert(dy >= 0);
    assert(dz >= 0);

    return (dx + dy + dz);
}

void makePointGalaxy(PointGalaxy & galaxy) {
    size_t const nstars = galaxy.size();

    // Calculate total gravity applied to each star.
    for (size_t istar1 = 0; istar1 < nstars; istar1++) {
        PointStar & star1 = galaxy.at(istar1);

        // Reset star gravity and proximity.
        star1.gravity   = 0;
        star1.proximity = 1.0e9;

        for (size_t istar2 = 0; istar2 < nstars; istar2++) {
            // Ignore same star.
            if (istar1 == istar2)
                continue;

            PointStar const & star2 = galaxy.at(istar2);

            // Verify star mass.
            assert(star2.mass > 0);

            // Calculate star squared distance.
            double const distance = squareDistance(star1.position, star2.position);
            assert(distance >= 0);

            // Contribute to star gravity.
            star1.gravity += (star2.mass / distance);
            assert(star1.gravity >= 0);

            // Check against proximity.
            if (star1.proximity > distance)
                star1.proximity = distance;
        }

        // TODO: Decide whether to have the following line.
        // star1.gravity *= star1.mass;
    }

    // Order stars by total gravity, descending.
    std::sort(galaxy.begin(), galaxy.end());
}

static cl_int score(PointStar const & p1, PointStar const & p2) {
    double const lo = (std::min(p1.proximity, p2.proximity) + (1e-6));
    double const hi = (std::max(p1.proximity, p2.proximity) + (1e-6));
    double const de = (hi / lo);
    assert(de > 0);
    return (de < 1.1);
}

void matchPointGalaxies(std::vector<cl_int2> & pairs, PointGalaxy const & galaxy1, PointGalaxy const & galaxy2) {
    // Calculates longest common subsequence by matching each PointStar.

    pairs.clear();

    // Extend each galaxy with a duplicate of itself.
    PointGalaxy egalaxy1;
    PointGalaxy egalaxy2;
    egalaxy1.insert(egalaxy1.end(), galaxy1.begin(), galaxy1.end());
    egalaxy1.insert(egalaxy1.end(), galaxy1.begin(), galaxy1.end());
    egalaxy2.insert(egalaxy2.end(), galaxy2.begin(), galaxy2.end());
    egalaxy2.insert(egalaxy2.end(), galaxy2.begin(), galaxy2.end());
    assert(egalaxy1.size() == (2 * galaxy1.size()));
    assert(egalaxy2.size() == (2 * galaxy2.size()));

    cl_int const n1 = egalaxy1.size();
    cl_int const n2 = egalaxy2.size();

    // Create longest common subsequence grid.
    blitz::Array<cl_int, 2> grid(n1 + 1, n2 + 1);
    grid = 0;

    for (cl_int i1 = 1; i1 <= n1; i1++) {
        PointStar const & p1 = egalaxy1.at(i1 - 1);

        for (cl_int i2 = 1; i2 <= n2; i2++) {
            PointStar const & p2 = egalaxy2.at(i2 - 1);

            // Start with no length.
            cl_int length = 0;

            // Extend with each recursion.
            length = std::max(length, grid(i1 - 1, i2 - 1));
            length = std::max(length, grid(i1    , i2 - 1));
            length = std::max(length, grid(i1 - 1, i2    ));

            // Extend with match.
            length = std::max(length, grid(i1 - 1, i2 - 1) + score(p1, p2));

            // Record in grid.
            grid(i1, i2) = length;
        }
    }

    // Back-track through grid.
    cl_int i1 = n1;
    cl_int i2 = n2;

    while ((i1 > 0) && (i2 > 0)) {
        PointStar const & p1 = egalaxy1.at(i1 - 1);
        PointStar const & p2 = egalaxy2.at(i2 - 1);

        // Recall all grid possibilities.
        cl_int const gridSkip = grid(i1 - 1, i2 - 1);
        cl_int const gridHit1 = grid(i1    , i2 - 1);
        cl_int const gridHit2 = grid(i1 - 1, i2    );
        cl_int const gridTake = gridSkip + score(p1, p2);

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
            cl_int  const i1b = ((i1 - 1) % galaxy1.size());
            cl_int  const i2b = ((i2 - 1) % galaxy2.size());

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
