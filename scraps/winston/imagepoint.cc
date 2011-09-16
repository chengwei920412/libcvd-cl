#include "imagepoint.h"
#include <cvd/vector_image_ref.h>
#include <iostream>

using namespace TooN;
using namespace CVD;
using namespace std;

void ImagePoint::build_from_image(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos, Camera::Linear& cam, double scale){
  descriptor.build_from_image(im,pos);
  convert_pos_scale(pos,cam,scale);
}

ImageRef OffsetList[8] = {
  ImageRef(0,-1),
  ImageRef(-1,0),
  ImageRef(1,0),
  ImageRef(0,1),
  ImageRef(-1,-1),
  ImageRef(1,-1),
  ImageRef(-1,1),
  ImageRef(1,1)
};

void ImagePoint::sample_from_image(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos, Camera::Linear& cam, double scale){
  //pure_descriptor.build_from_image(im, pos);
  descriptor.build_from_image(im, pos);
  for(int i=0; i<8; i++){ // at the moment only blur with 4 compass directions
    Descriptor d;
    d.build_from_image(im,pos+OffsetList[i]);
    descriptor |= d;
    //cout << "i " << i << " descrip" << descriptor.bitcount() << endl;
  }
  convert_pos_scale(pos,cam,scale);
}

void ImagePoint::convert_pos_scale(CVD::ImageRef pos, Camera::Linear& cam, double scale){
  Vector<2> scale_offset = makeVector(scale/2,scale/2);
  image_coords = vec(pos)*scale + scale_offset;
  normalized_cam_coords = cam.unproject(image_coords);
}

