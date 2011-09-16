// -*- C++ -*-


#ifndef FEATUREPATCH_H
#define FEATUREPATCH_H

#include <cvd/image.h>
#include <TooN/TooN.h>

class FeaturePatch {
public:
  void build_from_image(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos);
  

  unsigned char operator()(int r, int c) const{
    return patch[r][c];
  }
private:
  unsigned char patch[8][8];
};





#endif
