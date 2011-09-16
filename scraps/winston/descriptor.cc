#include "descriptor.h"

#include <iomanip>

using namespace std;
using namespace CVD;

void Descriptor::clear(){
  for(int i=0; i<Num_Q_Intensities; i++){
    descriptor[i]=0;
  }
}
    

void Descriptor::build_from_patch(const FeaturePatch& fp){
  double total=0;
  double total_sq=0;

  for(int y=0; y<8; y++){
    for(int x=0; x<8; x++){
      int val= fp(y,x);
      total += val;
      total_sq += val*val;
    }
  }

  double mean = total/64;
  double sigma = sqrt(total_sq/64 - total*total/4096);


  // assume 5 levels as per Simon; these values give 20% in each bin if distribution is Gaussian
  double near = 0.253*sigma;
  double far = 0.842*sigma;


  for(int i=0; i<Num_Q_Intensities; i++){
    descriptor[i]=0;
  }

  long long int bit=1;
  for(int y=0; y<8; y++){
    for(int x=0; x<8; x++){
      int val= fp(y,x);
      if(val < mean - far){
	descriptor[0] |= bit;
      } else if (val < mean - near) {
	descriptor[1] |= bit;
      } else if (val < mean + near) {
	descriptor[2] |= bit;
      } else if (val < mean + far) {
	descriptor[3] |= bit;
      } else {
	descriptor[4] |= bit;
      }

      bit <<=1;
    }
  }
}

void Descriptor::build_from_image(const BasicImage<unsigned char>& im, ImageRef pos){
  FeaturePatch fp;
  fp.build_from_image(im,pos);
  build_from_patch(fp);
}




unsigned int Descriptor::error_of(const Descriptor& d) const {
  unsigned int result=0;
  for(int i=0; i<Num_Q_Intensities; i++){
    result += ::bitcount(d.descriptor[i] & ~ descriptor[i]);
  }
  return result;
}

unsigned int Descriptor::bitcount () const{
  unsigned int result=0;
  for(int i=0; i<Num_Q_Intensities; i++){
    result += ::bitcount(descriptor[i]);
  }
  return result;
}


std::ostream& operator << (std::ostream& os, const Descriptor& d){
  for(int i=0; i<Descriptor::Num_Q_Intensities; i++){
    os << std::hex << std::setw(16) << setfill('0') << d.descriptor[i] << std::endl;
  }
  return os;
}
