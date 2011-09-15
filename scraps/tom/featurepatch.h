// -*- C++ -*-


#ifndef FEATUREPATCH_H
#define FEATUREPATCH_H

#include <cvd/image.h>
#include <TooN/TooN.h>

class FeaturePatch {
public:
  void build_from_image(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos, TooN::Matrix<2>& warp);
  void build_from_image(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos);
  

  unsigned char operator()(int i) const{
    return patch[i];
  }
private:
  unsigned char patch[64];
  // static std::map<int,int[64]> fast_lookup;
};





#endif
