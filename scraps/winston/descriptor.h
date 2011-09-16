// -*- c++ -*-

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <iostream>
#include <limits.h>
#include "featurepatch.h"

template <typename T>
int bitcount (T v){
  v = v - ((v >> 1) & (T)~(T)0/3);                           // temp
  v = (v & (T)~(T)0/15*3) + ((v >> 2) & (T)~(T)0/15*3);      // temp
  v = (v + (v >> 4)) & (T)~(T)0/255*15;                      // temp
  int c = (T)(v * ((T)~(T)0/255)) >> (sizeof(T) - 1) * CHAR_BIT; // count
  return c;
}

class Descriptor{
public:

  void clear();
  void build_from_patch(const FeaturePatch& fp);
  void build_from_image(const CVD::BasicImage<unsigned char>& im, CVD::ImageRef pos);

  unsigned int error_of(const Descriptor& d) const; // how many bits are set in d that are not set in me
  unsigned int bitcount() const; // how many bits are set in me


  Descriptor operator|(const Descriptor& d) const{
    Descriptor result;
    for(int i=0; i<Num_Q_Intensities; i++){
      result.descriptor[i] = descriptor[i] | d.descriptor[i];
    }
    return result;
  }

  Descriptor& operator|=(const Descriptor& d) {
    for(int i=0; i<Num_Q_Intensities; i++){
      descriptor[i] |= d.descriptor[i];
    }
    return *this;
  }

  static const int Num_Q_Intensities=5; // Simon says there should be 5 intensities
  unsigned long long int descriptor[Num_Q_Intensities];
 
};

std::ostream& operator << (std::ostream& os, const Descriptor& d);

#endif
