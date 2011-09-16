// -*- c++ -*-

#ifndef POINTSETINDEX_H
#define POINTSETINDEX_H

#include <fstream>
#include <list>

#include "descriptor.h"
#include "imagepoint.h"
#include "pointset.h"

#include <sys/time.h> //*************
#include <iostream> //*************

struct DescriptorTree {
  Descriptor descriptor;
  int children[2];
  int imagepointindex;
};


class PointSetIndex {
public:
  PointSetIndex(){my_ps=0;}
  PointSetIndex(PointSet& ps){
    build_tree(ps);
  }

  void build_tree(PointSet& ps);

  std::list<std::pair<int,ImagePoint*> > find_match(const Descriptor& d, int dist_thresh);
  std::pair<int,ImagePoint*> best_match(const Descriptor& d, int dist_thresh);

private:
  PointSet* my_ps;
  std::vector<DescriptorTree> index;
  
};


#endif
