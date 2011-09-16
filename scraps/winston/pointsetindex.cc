#include "pointsetindex.h"

#include <cvd/fast_corner.h>
#include <vector>
#include <cvd/vision.h>
#include <list>
#include <map>


using namespace std;
using namespace CVD;



void PointSetIndex::build_tree(PointSet& ps){
  
  multimap<int,int> unmerged;
  my_ps=&ps;
  index.reserve(ps.size()*2); // reserve enough for the leaves and the tree above (plus one spare)
  index.resize(ps.size());
  for(int i=0; i<ps.size(); i++){
    index[i].children[0]=index[i].children[1]=-1;
    index[i].imagepointindex=i;
    index[i].descriptor=ps[i].descriptor;
    unmerged.insert(pair<int,int>(index[i].descriptor.bitcount(),i));	
  }

  typedef multimap<int,int>::iterator It;

  // keep merging nodes until only one left
  while(unmerged.size()>1){

    // try to merge the first one in the list
    It merger=unmerged.begin();  // the one we're trying to merge
    It mergee;                  // the best one to merge it with

    bool done=false;
    int bestscore=1000000;

    while(!done){
      // cout << "searching for a close match" << endl;
      done = true;
      Descriptor& d_merger = index[merger->second].descriptor;
      for(It scan = unmerged.begin(); scan != unmerged.end(); scan++){
        if(merger==scan) continue;
	int score = (d_merger | index[scan->second].descriptor).bitcount();
	if(score < bestscore){
	  bestscore = score;
	  mergee = merger;
	  merger = scan; // look for a better match from here
	  done = false;
	  break;
	}
      }
    }
    // cout << "merging" << endl;

    // now merge them!
    DescriptorTree dt;
    dt.descriptor = index[merger->second].descriptor | index[mergee->second].descriptor;
    dt.children[0]=merger->second;
    dt.children[1]=mergee->second;
    dt.imagepointindex=-1;
    index.push_back(dt);
    
    unmerged.erase(merger);
    unmerged.erase(mergee);
    unmerged.insert(pair<int,int>(bestscore,index.size()-1)); // push the new node onto the unmerged list
  }
  
  for(It subtreenode = unmerged.begin(); subtreenode != unmerged.end(); subtreenode++)
    {
      index.push_back(index[(*subtreenode).second]);
      }
}

list<pair<int,ImagePoint*> > PointSetIndex::find_match(const Descriptor& d, int dist_thresh){
  list<pair<int,ImagePoint*> > result;
  list<int> candidates;
  candidates.push_back(index.size()-1); // push the root node of the tree onto the candidate list

// ImagePoint* base = &(database[0]);

  while (!candidates.empty()){
    int dtix = *(candidates.begin());
    candidates.pop_front();
    int this_dist= index[dtix].descriptor.error_of(d);
    if(this_dist < dist_thresh){
      if(index[dtix].children[0]==-1) { // we've found a leaf
	result.push_back(pair<int,ImagePoint*>(this_dist,my_ps->get_corner(index[dtix].imagepointindex)));
      } else {
	candidates.push_back(index[dtix].children[0]);
	candidates.push_back(index[dtix].children[1]);
      }
    }
  }
  return result;
}

pair<int,ImagePoint*> PointSetIndex::best_match(const Descriptor& d, int dist_thresh){
  int best_dist = dist_thresh;
  ImagePoint* best_match=0;
  list<int> candidates;
  candidates.push_back(index.size() -1); // push the root node of the tree onto the candidate list

  while (!candidates.empty()){
    int dtix = candidates.front();
    candidates.pop_front();
    int this_dist= index[dtix].descriptor.error_of(d);
   
    if(this_dist < best_dist){
      if(index[dtix].children[0]==-1) { // we've found a leaf
	best_dist = this_dist;
	best_match = my_ps->get_corner(index[dtix].imagepointindex);
      } else {
	candidates.push_front(index[dtix].children[0]);
	candidates.push_front(index[dtix].children[1]);
      }
    }
  }
  return pair<int,ImagePoint*>(best_dist,best_match);
}


