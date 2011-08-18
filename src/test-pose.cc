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

#include <cvd-cl/steps/PreFastRichStep.hh>
#include <cvd-cl/steps/FastRichStep.hh>
#include <cvd-cl/steps/FastBestStep.hh>
#include <cvd-cl/steps/HipsRichStep.hh>
#include <cvd-cl/steps/HipsFindStep.hh>
#include <cvd-cl/steps/ToUvqUvStep.hh>
#include <cvd-cl/steps/MixUvqUvStep.hh>
#include <cvd-cl/steps/PoseUvqWlsStep.hh>
#include <cvd-cl/steps/CholeskyStep.hh>
#include <cvd-cl/steps/SE3ExpStep.hh>


// Typedefs for image format.
typedef CVD::Rgba<CVD::byte>    ColourPixel;
typedef uint16_t                 DepthPixel;
typedef CVD::Image<ColourPixel> ColourImage;
typedef CVD::Image<DepthPixel>   DepthImage;

// Size constants.
size_t const static KiB = 1024;
size_t const static MiB = KiB * KiB;

// Cropped image size.
CVD::ImageRef const static   ref0(  0,   0);
CVD::ImageRef const static ref512(512, 256);

// Maximum corners processed.
size_t const static ncorners = 256;

// Number of hypotheses to generate.
size_t const static nhypos   = 2048;

static void learnCamera(
    char const                * path,
    CVD::BasicImage<cl_float>   uimage,
    CVD::BasicImage<cl_float>   vimage
) {

    // Create camera.
    Camera::Linear camera;

    // Open parameter file.
    std::ifstream file(path);
    file.exceptions(~std::ios::goodbit);

    // Consume parameters.
    camera.load(file);

    // Close parameter file.
    file.close();

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
    ColourImage const & c1image,
    ColourImage const & c2image,
    DepthImage  const & d1image
) {

    // Extract image dimensions.
    CVD::ImageRef const size = c1image.size();
    int           const nx   = size.x;
    int           const ny   = size.y;
    int           const nxy  = nx * ny;

    // Create OpenCL worker.
    CVD::CL::Worker          worker      (device);

    // Create FAST and HIPS states.
    CVD::CL::RichImageState  imageNeat   (worker, size);
    CVD::CL::GrayImageState  scores      (worker, size);
    CVD::CL::PointListState  corners1    (worker, nxy);
    CVD::CL::PointListState  corners2    (worker, nxy);

    // Create states specific to image1 (colour + depth).
    CVD::CL::PointListState  im1corners  (worker, nxy);
    CVD::CL::HipsListState   im1hips     (worker, nxy);
    CVD::CL::PointListState  im1im2      (worker, nxy);
    CVD::CL::GrayImageState  im1depth    (worker, size);

    // Create states specific to image2 (colour only).
    CVD::CL::PointListState  im2corners  (worker, nxy);
    CVD::CL::HipsListState   im2hips     (worker, nxy);

    // Create camera translation states.
    CVD::CL::CameraState     camera      (worker, size);

    // Create states for RANSAC.
    CVD::CL::UvqUvState      uvquv       (worker, ncorners, 1);
    CVD::CL::UvqUvState      uvquv_mix   (worker, nhypos, 3);
    CVD::CL::MatrixState     hypo_a      (worker, nhypos, 6, 6);
    CVD::CL::MatrixState     hypo_b      (worker, nhypos, 6, 1);
    CVD::CL::MatrixState     hypo_x      (worker, nhypos, 6, 1);
    CVD::CL::MatrixState     hypo_cam    (worker, nhypos, 4, 4);

    // Create reusable steps.
    CVD::CL::PreFastRichStep runPreFast  (imageNeat, corners1);
    CVD::CL::FastRichStep    runFast     (imageNeat, corners1, scores, corners2);

    // Create steps specific to image1.
    CVD::CL::FastBestStep    runMaxFast1 (                     scores, corners2, im1corners);
    CVD::CL::HipsRichStep    runHips1    (imageNeat,                             im1corners, im1hips);

    // Create steps specific to image2.
    CVD::CL::FastBestStep    runMaxFast2 (                     scores, corners2,                      im2corners);
    CVD::CL::HipsRichStep    runHips2    (imageNeat,                                                  im2corners, im2hips);

    // Create steps for RANSAC.
    CVD::CL::HipsFindStep    runMatch    (im1hips, im2hips, im2corners, im1im2);
    CVD::CL::ToUvqUvStep     runToUvqUv  (camera, im1corners, im1im2, uvquv);
    CVD::CL::MixUvqUvStep    runMix      (uvquv, uvquv_mix);
    CVD::CL::PoseUvqWlsStep  runWls      (uvquv_mix.uvq, uvquv_mix.uv, hypo_a, hypo_b);
    CVD::CL::CholeskyStep    runCholesky (hypo_a, hypo_b, hypo_x);
    CVD::CL::SE3ExpStep      runSe3Exp   (hypo_x, hypo_cam);



    // Populate camera states.
    learnCamera("./images/kinectparameters.txt", camera.umap.asImage(), camera.vmap.asImage());
    translateDepth(d1image, camera.qmap.asImage());
    camera.copyToWorker();

    // Write image 1 to device.
    imageNeat.set(c1image);

    // Zero FAST scores.
    scores.zero();
    corners1.zero();
    corners2.zero();

    // Run image 1 pipeline.
    runPreFast.execute();
    runFast.execute();
    runMaxFast1.execute();
    runHips1.execute();

    // Write image 2 to device.
    imageNeat.set(c2image);

    // Zero FAST scores.
    scores.zero();
    corners1.zero();
    corners2.zero();

    // Run image 1 pipeline.
    runPreFast.execute();
    runFast.execute();
    runMaxFast2.execute();
    runHips2.execute();

    // Finish any outstanding work.
    worker.finish();

    // Run RANSAC steps.
    int64_t const timeMatch    = runMatch.measure();
    int64_t const timeToUvqUv  = runToUvqUv.measure();
    int64_t const timeMix      = runMix.measure();
    int64_t const timeWls      = runWls.measure();
    int64_t const timeCholesky = runCholesky.measure();
    int64_t const timeSe3Exp   = runSe3Exp.measure();

    std::cerr << std::endl;
    std::cerr << std::setw(8) << timeMatch       << " us matching HIPS" << std::endl;
    std::cerr << std::setw(8) << timeToUvqUv     << " us converting to ((u,v,q),(u,v))" << std::endl;
    std::cerr << std::setw(8) << timeMix         << " us mixing matches" << std::endl;
    std::cerr << std::setw(8) << timeWls         << " us differentiating matrix" << std::endl;
    std::cerr << std::setw(8) << timeCholesky    << " us running Cholesky" << std::endl;
    std::cerr << std::setw(8) << timeSe3Exp      << " us exponentiating matrix" << std::endl;
    std::cerr << std::endl;

    // Read out final corner list.
    std::vector<cl_int2> points1;
    im1corners.get(&points1);

    // Read out matching corner table.
    std::vector<cl_int2> points2;
    im1im2.get(&points2);

    CVD::ImageRef const size2(nx * 2, ny);
    CVD::VideoDisplay window(size2);
    CVD::glDrawPixels(c1image);
    CVD::glRasterPos(CVD::ImageRef(nx, 0));
    CVD::glDrawPixels(c2image);

    glColor3f(0, 0, 1);
    glBegin(GL_LINES);
    for (size_t ic = 0; (ic < points1.size()) && (ic < points2.size()); ic++) {
        try {
            cl_int2 const xy1 = points1.at(ic);
            cl_int2 const xy2 = points2.at(ic);

            glVertex2i(xy1.x, xy1.y);
            glVertex2i(xy2.x + nx, xy2.y);
        } catch (...) {
            std::cerr << "Bad corner " << ic << " of " << points1.size() << " / " << points2.size() << std::endl;
        }
    }
    glEnd();
    glFlush();

    glColor3f(1, 0, 0);
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

    sleep(5);
}

int main(int argc, char **argv) {
    ColourImage c1image_full = CVD::img_load("images/colour1.bmp");
    ColourImage c2image_full = CVD::img_load("images/colour2.bmp");
    DepthImage  d1image_full = CVD::img_load("images/depth1.png" );

    ColourImage c1image(ref512);
    c1image.copy_from(c1image_full.sub_image(ref0, ref512));

    ColourImage c2image(ref512);
    c2image.copy_from(c2image_full.sub_image(ref0, ref512));

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
                    testPose(dev, c1image, c2image, d1image);
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
