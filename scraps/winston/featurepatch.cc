#include "featurepatch.h"

#include <cvd/fast_corner.h>
#include <cvd/vector_image_ref.h>
#include <TooN/TooN.h>
#include <TooN/helpers.h>

#include <iostream>
using namespace std;
using namespace CVD;
using namespace TooN;

void FeaturePatch::build_from_image(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos){
  for(int x=0; x<8; x++){
    for(int y=0; y<8; y++){
      ImageRef offset;
      offset.x = 2*x-7;
      offset.y = 2*y-7;
      patch[y][x] = im[pos+offset];
    }
  }
}
