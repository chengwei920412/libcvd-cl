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

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <cvd/camera.h>
#include <cvd/fast_corner.h>
#include <cvd/gl_helpers.h>
#include <cvd/image_io.h>
#include <cvd/videodisplay.h>

#include <cvd-cl/steps/PreFastGrayStep.hh>
#include <cvd-cl/steps/ClipDepthStep.hh>
#include <cvd-cl/steps/FastGrayStep.hh>
#include <cvd-cl/steps/FastBestStep.hh>
#include <cvd-cl/steps/HipsGrayStep.hh>
#include <cvd-cl/steps/HipsBlendGrayStep.hh>
#include <cvd-cl/steps/HipsFindStep.hh>
#include <cvd-cl/steps/ToUvqUvStep.hh>
#include <cvd-cl/steps/MixUvqUvStep.hh>
#include <cvd-cl/steps/PoseUvqWlsStep.hh>
#include <cvd-cl/steps/CholeskyStep.hh>
#include <cvd-cl/steps/MatIdentStep.hh>
#include <cvd-cl/steps/MatMulStep.hh>
#include <cvd-cl/steps/SE3ExpStep.hh>
#include <cvd-cl/steps/SE3ScoreStep.hh>
#include <cvd-cl/steps/SE3Run1Step.hh>


// Typedefs for image format.
typedef CVD::byte                 GrayPixel;
typedef uint16_t                 DepthPixel;
typedef CVD::Image<GrayPixel >    GrayImage;
typedef CVD::Image<DepthPixel>   DepthImage;

// Size constants.
size_t const static KiB = 1024;
size_t const static MiB = KiB * KiB;

// Cropped image size.
CVD::ImageRef const static   ref0(  0, 80);
CVD::ImageRef const static ref512(512, 256);

// Maximum corners processed.
size_t const static ncorners = 2048;

// Number of hypotheses to generate.
size_t const static nhypos   = 8192;

static void readCamera(Camera::Linear * camera, char const * path) {
    // Open parameter file.
    std::ifstream file(path);
    file.exceptions(~std::ios::goodbit);

    // Consume parameters.
    camera->load(file);
}

static void learnCamera(
    Camera::Linear const      & camera,
    CVD::BasicImage<cl_float>   uimage,
    CVD::BasicImage<cl_float>   vimage
) {

    // Extract image dimensions.
    CVD::ImageRef const size = uimage.size();
    int           const nx   = size.x;
    int           const ny   = size.y;

    // Loop over all coordinates.
    for (int x = 0; x < nx; x++) {
        for (int y = 0; y < ny; y++) {
            // Construct image reference.
            CVD::ImageRef const ref(x, y);

            // Construct (x, y) vector.
            // NB: The camera size does not match the image size,
            // however, the offset is 0, so use (x, y) directly.
            TooN::Vector<2> const xy = TooN::makeVector(x, y);

            // Translate from (x, y) to (u, v).
            TooN::Vector<2> const uv = camera.unproject(xy);

//            TooN::Vector<2> const xy2 = camera.project(uv);
//            std::cerr << "x = " << xy[0] << " -> " << uv[0] << " -> " << xy2[0] << std::endl;
//            std::cerr << "y = " << xy[1] << " -> " << uv[1] << " -> " << xy2[1] << std::endl;

            // Record (u, v) pair.
            uimage[ref] = uv[0];
            vimage[ref] = uv[1];
        }
    }
}

static void translateDepth(
    DepthImage const          & idimage,
    CVD::BasicImage<cl_float>   fdimage
) {

    // Extract image dimensions.
    CVD::ImageRef const size = idimage.size();
    int           const nx   = size.x;
    int           const ny   = size.y;

    // Loop over all coordinates.
    for (int x = 0; x < nx; x++) {
        for (int y = 0; y < ny; y++) {
            // Construct image reference.
            CVD::ImageRef const ref(x, y);

            // Extract integer depth.
            DepthPixel const depth = idimage[ref];

            // Record as corrected depth.
            // TODO: Work out actual translation.
            fdimage[ref] = depth;
        }
    }
}

static void testPose(
    cl::Device        & device,
    GrayImage   const & g1image,
    GrayImage   const & g2image,
    DepthImage  const & d1image
) {

    // Extract image dimensions.
    CVD::ImageRef const size = g1image.size();
    int           const nx   = size.x;
    int           const ny   = size.y;
    int           const nxy  = nx * ny;

    // Create OpenCL worker.
    CVD::CL::Worker          worker      (device);

    // Create FAST and HIPS states.
    CVD::CL::GrayImageState  imageNeat   (worker, size);
    CVD::CL::GrayImageState  scores      (worker, size);
    CVD::CL::PointListState  corners1    (worker, nxy);
    CVD::CL::PointListState  corners2    (worker, nxy);
    CVD::CL::PointListState  corners3    (worker, nxy);

    // Create states specific to image1 (colour + depth).
    CVD::CL::PointListState  im1corners  (worker, ncorners);
    CVD::CL::HipsListState   im1hips     (worker, ncorners);
    CVD::CL::PointListState  im1im2      (worker, ncorners);
    CVD::CL::GrayImageState  im1depth    (worker, size);

    // Create states specific to image2 (colour only).
    CVD::CL::PointListState  im2corners  (worker, ncorners);
    CVD::CL::HipsListState   im2hips     (worker, ncorners);

    // Create camera translation states.
    CVD::CL::CameraState     camera      (worker, size);

    // Create states for RANSAC.
    CVD::CL::PointListState  matches     (worker, ncorners);
    CVD::CL::UvqUvState      uvquv       (worker, ncorners, 1);
    CVD::CL::UvqUvState      uvquv_mix   (worker, nhypos, 3);
    CVD::CL::MatrixState     hypo_m      (worker, nhypos, 4, 4);
    CVD::CL::MatrixState     hypo_a      (worker, nhypos, 6, 6);
    CVD::CL::MatrixState     hypo_b      (worker, nhypos, 6, 1);
    CVD::CL::MatrixState     hypo_x      (worker, nhypos, 6, 1);
    CVD::CL::MatrixState     hypo_cam    (worker, nhypos, 4, 4);
    CVD::CL::FloatListState  hypo_scores (worker, nhypos);
    CVD::CL::CountState      hypo_best   (worker, nhypos);
    CVD::CL::Float2ListState test_uvs    (worker, ncorners);

    // Create steps specific to image1.
    CVD::CL::PreFastGrayStep runPreFast1 (imageNeat, corners1);
    CVD::CL::ClipDepthStep   runClip1    (camera.qmap,  corners1, corners2);
    CVD::CL::FastGrayStep    runFast1    (imageNeat, corners2, scores, im1corners);
    CVD::CL::FastBestStep    runMaxFast1 (                     scores, corners3, im1corners);
    CVD::CL::HipsGrayStep    runHips1    (imageNeat,                             im1corners, im1hips);

    // Create steps specific to image2.
    CVD::CL::PreFastGrayStep runPreFast2 (imageNeat, corners1);
    CVD::CL::FastGrayStep    runFast2    (imageNeat, corners1, scores, im2corners);
    CVD::CL::FastBestStep    runMaxFast2 (                     scores, corners2,                      im2corners);
    CVD::CL::HipsBlendGrayStep    runHips2    (imageNeat,                                                  im2corners, im2hips);

    // Create steps for RANSAC.
    CVD::CL::HipsFindStep    runMatch    (im1hips, im2hips, matches);
    CVD::CL::ToUvqUvStep     runToUvqUv  (camera, im1corners, im2corners, matches, uvquv);
    CVD::CL::MixUvqUvStep    runMix      (uvquv, uvquv_mix);
    CVD::CL::MatIdentStep    runIdent    (hypo_m);
    CVD::CL::PoseUvqWlsStep  runWls      (uvquv_mix.uvq, uvquv_mix.uv, hypo_m, hypo_a, hypo_b);
    CVD::CL::CholeskyStep    runCholesky (hypo_a, hypo_b, hypo_x);
    CVD::CL::SE3ExpStep      runSe3Exp   (hypo_x, hypo_cam);
    CVD::CL::MatMulStep      runMul      (hypo_cam, hypo_m);
    CVD::CL::SE3ScoreStep    runSe3Score (uvquv, hypo_cam, hypo_scores);
    CVD::CL::SE3Run1Step     runSe3One   (uvquv, hypo_cam, hypo_best, test_uvs);


    // Populate camera states.
    Camera::Linear cvdcamera;
    readCamera(&cvdcamera, "./etc/kinect.conf");
    learnCamera(cvdcamera, camera.umap.asImage(), camera.vmap.asImage());
    translateDepth(d1image, camera.qmap.asImage());
    camera.copyToWorker();

    boost::system_time const t1 = boost::get_system_time();

    // Write image 1 to device.
    int64_t const timeCopy1 = imageNeat.measure(g1image);

    // Zero FAST scores.
    scores.zero();
    corners1.zero();
    corners2.zero();
    corners3.zero();

    // Run image 1 pipeline.
    int64_t const timePreFast1 = runPreFast1.measure();
    size_t const ncull1 = corners1.getCount();
    int64_t const timeClip1 = runClip1.measure();
    size_t const nclip1 = corners2.getCount();
    int64_t const timeFast1 = runFast1.measure();
    size_t const nfast1 = im1corners.getCount();
    // runMaxFast1.measure();
    size_t const nbest1 = im1corners.getCount();
    int64_t const timeHips1 = runHips1.measure();

    // Write image 2 to device.
    int64_t const timeCopy2 = imageNeat.measure(g2image);

    // Zero FAST scores.
    scores.zero();
    corners1.zero();
    corners2.zero();

    // Run image 2 pipeline.
    int64_t const timePreFast2 = runPreFast2.measure();
    size_t const ncull2 = corners1.getCount();
    int64_t const timeFast2 = runFast2.measure();
    size_t const nfast2 = im2corners.getCount();
    // runMaxFast2.measure();
    size_t const nbest2 = im2corners.getCount();
    int64_t const timeHips2 = runHips2.measure();

    // Finish any outstanding work.
    worker.finish();

    // Run RANSAC steps.
    int64_t const timeMatch    = runMatch.measure();
    size_t  const nmatch       = matches.getCount();
    int64_t const timeToUvqUv  = runToUvqUv.measure();
    int64_t const timeMix      = runMix.measure();
    int64_t const timeIdent    = runIdent.measure();

    std::cerr << std::endl;
    std::cerr << std::setw(8) << nxy    << std::setw(8) << nxy    << " corner candidates in image" << std::endl;
    std::cerr << std::setw(8) << ncull1 << std::setw(8) << ncull2 << " corners after culling" << std::endl;
    std::cerr << std::setw(8) << nclip1 << std::setw(8) << ncull2 << " corners after depth" << std::endl;
    std::cerr << std::setw(8) << nfast1 << std::setw(8) << nfast2 << " corners after FAST" << std::endl;
    // std::cerr << std::setw(8) << nbest1 << std::setw(8) << nbest2 << " corners after filtering" << std::endl;
    std::cerr << std::setw(8) << nmatch << std::setw(8) << nmatch << " corners after HIPS" << std::endl;
    std::cerr << std::endl;
    std::cerr << std::setw(8) << timeCopy1       << std::setw(8) << timeCopy2      << " us writing image" << std::endl;
    std::cerr << std::setw(8) << timePreFast1    << std::setw(8) << timePreFast2   << " us culling corners" << std::endl;
    std::cerr << std::setw(8) << timeClip1       << std::setw(8) << 0              << " us filtering by depth" << std::endl;
    std::cerr << std::setw(8) << timeFast1       << std::setw(8) << timeFast2      << " us running FAST" << std::endl;
    std::cerr << std::setw(8) << timeHips1       << std::setw(8) << timeHips2      << " us making HIPS" << std::endl;
    std::cerr << std::endl;
    std::cerr << std::setw(8) << timeMatch       << " us finding HIPS matches" << std::endl;
    std::cerr << std::setw(8) << timeToUvqUv     << " us converting matches to ((u,v,q),(u,v))" << std::endl;
    std::cerr << std::setw(8) << timeMix         << " us selecting matches for 3-point attempts" << std::endl;
    std::cerr << std::setw(8) << timeIdent       << " us assigning identity matrix" << std::endl;
    std::cerr << std::endl;

    for (int i = 0; i < 10; i++) {
        int64_t const timeWls      = runWls.measure();
        int64_t const timeCholesky = runCholesky.measure();
        int64_t const timeSe3Exp   = runSe3Exp.measure();
        int64_t const timeMul      = runMul.measure(1); // Do not repeat!

        std::cerr << std::setw(8) << timeWls         << " us differentiating matrix" << std::endl;
        std::cerr << std::setw(8) << timeCholesky    << " us decomposing matrix and back-substituting vector" << std::endl;
        std::cerr << std::setw(8) << timeSe3Exp      << " us exponentiating matrix" << std::endl;
        std::cerr << std::setw(8) << timeMul         << " us multiplying matrix" << std::endl;
        std::cerr << std::endl;
    }

    int64_t const timeSe3Score = runSe3Score.measure();
    std::cerr << std::setw(8) << timeSe3Score    << " us scoring matrix" << std::endl;
    std::cerr << std::endl;

    // Read out score list.
    std::vector<cl_float> hyposcores;
    hypo_scores.get(&hyposcores);

    /* Calculate score statistics. */ {
        cl_float total = 0;
        cl_float best  = 0;
        cl_int   non0  = 0;
        cl_int   ibest = 0;

        boost::system_time const t1 = boost::get_system_time();

        for (size_t i = 0; i < hyposcores.size(); i++) {
            cl_float const score = hyposcores.at(i);

            total += score;
            non0  += (score > 0);

            if (score > best) {
                best  = score;
                ibest = i;
            }
        }

        cl_float avg = (total / hyposcores.size());

        boost::system_time const t2 = boost::get_system_time();

        std::cerr << std::setw(8) << non0  << " non-zero scores" << std::endl;
        std::cerr << std::setw(8) << total << " total score" << std::endl;
        std::cerr << std::setw(8) << avg   << " average score" << std::endl;
        std::cerr << std::setw(8) << best  << " best score" << std::endl;
        std::cerr << std::setw(8) << ibest << " best matrix index" << std::endl;

        int64_t const bestTime = ((t2 - t1).total_microseconds() / 1);

        std::cerr << std::setw(8) << bestTime         << " us finding best matrix" << std::endl;

        // Assign and run best matrix.
        worker.finish();
        hypo_best.setCount(ibest);
        runSe3One.measure();
    }

    // Read out final corner lists.
    std::vector<cl_int2> points1;
    std::vector<cl_int2> points2;
    im1corners.get(&points1);
    im2corners.get(&points2);

    // Read out pair lists.
    std::vector<cl_int2> pairs;
    matches.get(&pairs);

    // Read out transformed coordinate list.
    std::vector<cl_float2> uv2s;
    test_uvs.get(&uv2s);

    boost::system_time const t2 = boost::get_system_time();

    int64_t const approxTime = ((t2 - t1).total_microseconds() / 10);

    std::cerr << std::setw(8) << approxTime      << " us approximate total" << std::endl;

    CVD::ImageRef const size2(nx * 2, ny * 2);
    CVD::VideoDisplay window(size2);
    CVD::glDrawPixels(g1image);
    CVD::glRasterPos(CVD::ImageRef(nx,  0));
    CVD::glDrawPixels(g2image);
    CVD::glRasterPos(CVD::ImageRef( 0, ny));
    CVD::glDrawPixels(g1image);
    CVD::glRasterPos(CVD::ImageRef(nx, ny));
    CVD::glDrawPixels(g2image);

    glBegin(GL_LINES);
    for (size_t ip = 0; ip < pairs.size(); ip++) {
        try {
            cl_int2         const pair = pairs.at(ip);

            cl_int2         const xy1  = points1.at(pair.x);
            cl_int2         const xy2  = points2.at(pair.y);

            cl_float2       const uv3  = uv2s.at(ip);
            TooN::Vector<2> const uv3t = TooN::makeVector(uv3.x, uv3.y);
            TooN::Vector<2> const xy3t = cvdcamera.project(uv3t);
            cl_int2         const xy3  = {{cl_int(xy3t[0]), cl_int(xy3t[1])}};

            // Blue: HIPS match.
            glColor3f(0, 0, 1);
            glVertex2i(xy1.x,      xy1.y);
            glVertex2i(xy2.x + nx, xy2.y);

            // Red: RANSAC match.
            glColor3f(1, 0, 0);
            glVertex2i(xy1.x,      xy1.y + ny);
            glVertex2i(xy3.x + nx, xy3.y + ny);
        } catch (...) {
            std::cerr << "Bad pair " << ip << " of " << pairs.size() << std::endl;
        }
    }
    glEnd();
    glFlush();

    // Green: corners.
    glColor3f(0, 1, 0);
    glBegin(GL_POINTS);
    for (size_t i = 0; i < points1.size(); i++) {
        cl_int2 const & xy = points1.at(i);
        glVertex2i(xy.x, xy.y);
    }
    for (size_t i = 0; i < points2.size(); i++) {
        cl_int2 const & xy = points2.at(i);
        glVertex2i(xy.x + nx, xy.y);
    }
    glEnd();
    glFlush();

    sleep(30);
}

int main(int argc, char **argv) {
    GrayImage   g1image_full = CVD::img_load("images/c27.png");
    DepthImage  d1image_full = CVD::img_load("images/d27.png");
    GrayImage   g2image_full = CVD::img_load("images/c28.png");

    GrayImage   g1image(ref512);
    g1image.copy_from(g1image_full.sub_image(ref0, ref512));

    GrayImage   g2image(ref512);
    g2image.copy_from(g2image_full.sub_image(ref0, ref512));

    DepthImage  d1image(ref512);
    d1image.copy_from(d1image_full.sub_image(ref0, ref512));

    try {
        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        for (size_t ip = 0; ip < platforms.size(); ip++) {
            cl::Platform &pf = platforms.at(ip);

            std::cerr
                << pf.getInfo<CL_PLATFORM_NAME   >() << " ("
                << pf.getInfo<CL_PLATFORM_VENDOR >() << ", "
                << pf.getInfo<CL_PLATFORM_VERSION>() << ")"
                << std::endl;

            std::vector<cl::Device> devices;

            try {
                pf.getDevices(CL_DEVICE_TYPE_ALL, &devices);
            } catch (cl::Error & err) {
                std::cerr << err.what() << " (code " << err.err() << ")" << std::endl;
            }

            for (size_t id = 0; id < devices.size(); id++) {
                cl::Device &dev = devices.at(id);

                std::cerr << "  " << dev.getInfo<CL_DEVICE_NAME>() << std::endl;

                std::cerr << "    Compute units:  " << std::setw(8) <<
                        dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;

                std::cerr << "    Global memory:  " << std::setw(8) <<
                        (dev.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() / MiB) <<
                        " MiB" << std::endl;

                try {
                    testPose(dev, g1image, g2image, d1image);
                } catch (cl::Error & err) {
                    std::cerr << err.what() << " (code " << err.err() << ")" << std::endl;
                }
            }
        }
    } catch (cl::Error & err) {
        std::cerr << err.what() << " (code " << err.err() << ")" << std::endl;
    }
    return 0;
}
