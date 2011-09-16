// -*- c++ -*-

#ifndef POINTSET_H
#define POINTSET_H

#include <fstream>
#include <list>

#include "imagepoint.h"

class PointSet {
public:
  void build_from_image(const CVD::BasicImage<unsigned char>& im, Camera::Linear& cam, bool sample);
  void add_layer(const CVD::BasicImage<unsigned char>& im, Camera::Linear& cam, double scale, bool sample);

  void save(std::ofstream& ofs);

  ImagePoint& operator[](int i);
  ImagePoint* get_corner(int i){return &(database[i]);}

  int size(){return database.size();}

  void erase(){database.clear();}

  void push_back(ImagePoint& ip){
    database.push_back(ip);
  }

private:
  std::vector<ImagePoint> database;
  static const int FAST_THRESHOLD = 40;


};


#endif
