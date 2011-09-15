// -*- c++ -*-

#ifndef THREEPOINTPOSE_H
#define THREEPOINTPOSE_H

#include <TooN/TooN.h>
#include <TooN/se3.h>
#include <vector>
#include "ransac.h"
#include "imagepoint.h"
#include <iostream>

class ThreePointPose {
 public:
  static const int sample_size=3;
  ImagePoint* im1; // has q as well as normalized_cam_coords
  ImagePoint* im2; // only has normalized_cam_coords
};

template<>
class Hypothesis<ThreePointPose> {
 public:
  void generate(const std::vector<Ransac<ThreePointPose>* >& gen_set);
  void generate_irls(const std::vector<Ransac<ThreePointPose> >& gen_set, double inlier_sigma);
  double is_inlier(const Ransac<ThreePointPose>& test, double threshold);

  void save(std::ostream& os);
  void load(std::istream& is);

  TooN::SE3<> pose;
};





#endif
