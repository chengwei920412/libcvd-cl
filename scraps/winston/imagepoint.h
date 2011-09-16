// -*- c++ -*-

#ifndef IMAGEPOINT_H
#define IMAGEPOINT_H

#include <cvd/image.h>
#include <TooN/TooN.h>

#include <cvd/image.h>
#include <cvd/camera.h>

#include "descriptor.h"

class ImagePoint {
 public:
  ImagePoint(){
    // children[0]=0;
    // children[1]=0;
    match_error=1000; // a number bigger than the maximum error in matching (currently 64)
  }

  void build_from_image(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos, Camera::Linear& cam, double scale);
  void sample_from_image(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos, Camera::Linear& cam, double scale);

  // obtains image coords and normalized cam coords from the image ref, camera and image scale
  void convert_pos_scale(CVD::ImageRef pos, Camera::Linear& cam, double scale);

  Descriptor descriptor;
  // Descriptor pure_descriptor;
  TooN::Vector<2> image_coords;
  TooN::Vector<2> normalized_cam_coords;
  double q; // 1/z in camera coordinates
  int match_error;

  // ImagePoint* children[2];
};


#endif
