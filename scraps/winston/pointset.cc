#include "pointset.h"

#include <cvd/fast_corner.h>
#include <vector>
#include <cvd/vision.h>
#include <list>
#include <map>



using namespace std;
using namespace CVD;

void PointSet::build_from_image(const CVD::BasicImage<unsigned char>& im1, Camera::Linear& cam, bool sample){
  database.clear();
  // powers of two scale (1, 2, 4, ...)

  add_layer(im1,cam,1,sample);
  Image<unsigned char> im2 = halfSample(im1); // must use a temporary because of lazy eval ?? really?
  add_layer(im2,cam,2,sample);
  Image<unsigned char> im4 = halfSample(im2); // must use a temporary because of lazy eval ?? really?
  add_layer(im4,cam,4,sample);
  // intermediate 1.5 * powers of two scale (1.5, 3, 6, ...)
  Image<unsigned char>im1_5 = twoThirdsSample(im1);
  add_layer(im1_5,cam,1.5,sample);
  Image<unsigned char>im3 = halfSample(im1_5);
  add_layer(im3,cam,3,sample);

  //cout << "found " << database.size() << "corners across all scales" << endl;
}


void PointSet::add_layer(const CVD::BasicImage<unsigned char>& im, Camera::Linear& cam, double scale, bool sample){
  vector<ImageRef> corners;
  fast_corner_detect_9(im,corners, FAST_THRESHOLD);
  // cerr << "detected corners" << endl;
  vector<ImageRef> max_corners;
  fast_nonmax(im,corners, FAST_THRESHOLD, max_corners);
  // cerr << "found max corners" << endl;
  vector<ImageRef> useable_corners;
  for(unsigned int i=0; i<max_corners.size(); i++){
    if(im.in_image_with_border(max_corners[i],15)){ // 8 pixels from edge would do (7 for patch corner and 1 for offset)
      useable_corners.push_back(max_corners[i]);
    }
  }
  //cerr << "found " << useable_corners.size() << " useable corners" << endl;
  for(unsigned int i=0; i<useable_corners.size(); i++){
    // cerr << i << endl;
    ImagePoint ip;
    if(sample) {
      ip.sample_from_image(im,useable_corners[i],cam,scale);
    } else {
      ip.build_from_image(im,useable_corners[i],cam,scale);
    }
    database.push_back(ip);
  }
  // cerr << "built database layer" << endl;
}



ImagePoint& PointSet::operator[](int i){
  return database[i];
}
