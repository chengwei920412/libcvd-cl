#include "featurepatch.h"

#include <cvd/fast_corner.h>
#include <cvd/vector_image_ref.h>
#include <TooN/TooN.h>
#include <TooN/helpers.h>

#include <iostream>
using namespace std;
using namespace CVD;
using namespace TooN;

#include "interpolate.h"

void FeaturePatch::build_from_image(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos, TooN::Matrix<2>& warp){
  Vector<2> vpos = vec(pos);
  Vector<2> offset;
  for(int x=0; x<8; x++){
    offset[0] = 2*x-7;
    for(int y=0; y<8; y++){
      offset[1] = 2*y-7;
      double val = interpolate(im, vpos + warp*offset);
      patch[y*8+x] = val;
    }
  }
}

/**
void FeaturePatch::build_from_image(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos){
  for(int x=0; x<8; x++){
    for(int y=0; y<8; y++){
      ImageRef offset;
      offset.x = 2*x-7;
      offset.y = 2*y-7;
      patch[y*8+x] = im[pos+offset];
    }
  }
}
**/

ImageRef rhips[64]={
  ImageRef(3,0),  ImageRef(6,-1), ImageRef(8,0),  ImageRef(9,-2),
  ImageRef(3,-1), ImageRef(5,-3), ImageRef(7,-3), ImageRef(8,-5),
  ImageRef(2,-2), ImageRef(3,-5), ImageRef(5,-5), ImageRef(5,-8),
  ImageRef(1,-3), ImageRef(1,-6), ImageRef(3,-7), ImageRef(2,-9),

  ImageRef(0,-3), ImageRef(-1,-6), ImageRef(0,-8), ImageRef(-2,-9),
  ImageRef(-1,-3), ImageRef(-3,-5), ImageRef(-3,-7), ImageRef(-5,-8),
  ImageRef(-2,-2), ImageRef(-5,-3), ImageRef(-5,-5), ImageRef(-8,-5),
  ImageRef(-3,-1), ImageRef(-6,-1), ImageRef(-7,-3), ImageRef(-9,-2),

  ImageRef(-3,0),  ImageRef(-6,1), ImageRef(-8,0),  ImageRef(-9,2),
  ImageRef(-3,1), ImageRef(-5,3), ImageRef(-7,3), ImageRef(-8,5),
  ImageRef(-2,2), ImageRef(-3,5), ImageRef(-5,5), ImageRef(-5,8),
  ImageRef(-1,3), ImageRef(-1,6), ImageRef(-3,7), ImageRef(-2,9),

  ImageRef(0,3), ImageRef(1,6), ImageRef(0,8), ImageRef(2,9),
  ImageRef(1,3), ImageRef(3,5), ImageRef(3,7), ImageRef(5,8),
  ImageRef(2,2), ImageRef(5,3), ImageRef(5,5), ImageRef(8,5),
  ImageRef(3,1), ImageRef(6,1), ImageRef(7,3), ImageRef(9,2)
};


void FeaturePatch::build_from_image(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos){
  for(int i=0; i<64; i++){
    ImageRef offset = rhips[i];
    patch[i] = im[pos+offset];
  }
}
