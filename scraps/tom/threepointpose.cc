// -*- c++ -*-
#include "threepointpose.h"

#include <TooN/TooN.h>
#include <TooN/wls.h>
#include <TooN/irls.h>

using namespace std;
using namespace TooN;

Matrix<2,6> compute_J(const Vector<2>& uv, double q){
  Matrix<2,6> result;
  result[0] = makeVector(q,0,-uv[0]*q,-uv[0]*uv[1],1+uv[0]*uv[0],-uv[1]);
  result[1] = makeVector(0,q,-uv[1]*q,-1-uv[1]*uv[1],uv[0]*uv[1],uv[0]);
  return result;
}

void Hypothesis<ThreePointPose>::generate(const std::vector<Ransac<ThreePointPose>* >& gen_set){
  int size = gen_set.size();
  vector<Vector<2> > uv1(size);
  vector<Vector<2> > uv2(size);
  vector<double> q1(size);

  for(int i=0; i<size; i++){
    uv2[i] = gen_set[i]->data.im2->normalized_cam_coords;
  }

  const int num_iterations=6;

  for(int iteration=1; iteration<num_iterations; iteration++){

    // calculate the current estimate for the projection of the image 1 coordinates into image 2
    // first iteration has pose = Identity
    for(int i=0; i<size; i++){
      Vector<3> xyz = pose.get_rotation()*unproject(gen_set[i]->data.im1->normalized_cam_coords)
	+ pose.get_translation()*gen_set[i]->data.im1->q;
      uv1[i] = project(xyz);
      q1[i] = gen_set[i]->data.im1->q/xyz[2];
    }

    WLS<6> wls;
    for(int i=0; i<size; i++){
      Matrix<2,6> J = compute_J(uv1[i],q1[i]);
      Vector<2> diff = uv2[i]-uv1[i];
      wls.add_mJ(diff[0],J[0]);
      wls.add_mJ(diff[1],J[1]);
    }
    wls.compute();
    SE3<> update = SE3<>::exp(wls.get_mu());
    pose = update*pose;
  }
}

void Hypothesis<ThreePointPose>::generate_irls(const std::vector<Ransac<ThreePointPose> >& gen_set, double inlier_sigma){
  int size = gen_set.size();
  vector<Vector<2> > uv1(size);
  vector<Vector<2> > uv2(size);
  vector<double> q1(size);

  const double inlier_sigma_sq = inlier_sigma*inlier_sigma;

  for(int i=0; i<size; i++){
    uv2[i] = gen_set[i].data.im2->normalized_cam_coords;
  }

  const int num_iterations=6;

  for(int iteration=0; iteration<num_iterations; iteration++){

    // calculate the current estimate for the projection of the image 1 coordinates into image 2
    // first iteration has pose = Identity
    for(int i=0; i<size; i++){
      Vector<3> xyz = pose.get_rotation()*unproject(gen_set[i].data.im1->normalized_cam_coords)
	+ pose.get_translation()*gen_set[i].data.im1->q;
      uv1[i] = project(xyz);
      q1[i] = gen_set[i].data.im1->q/xyz[2];
    }

    WLS<6> wls;
    wls.add_prior(1);
    for(int i=0; i<size; i++){
      Matrix<2,6> J = compute_J(uv1[i],q1[i]);
      Vector<2> diff = uv2[i]-uv1[i];

      double ssq = inlier_sigma_sq*(num_iterations-iteration)*(num_iterations-iteration);
      double weight=1;
      if(iteration>0){	
	weight = ssq/(ssq+diff*diff);// weighting for re-weighting the least squares
      }
      wls.add_mJ(diff[0],J[0],weight);
      wls.add_mJ(diff[1],J[1],weight);
    }
    // cout << wls.get_vector() << endl << endl;
    // cout << wls.get_C_inv() << endl << endl;

    wls.compute();
    SE3<> update = SE3<>::exp(wls.get_mu());
    // cout << wls.get_mu() << endl << endl << endl;
    pose = update*pose;
  }
}



double Hypothesis<ThreePointPose>::is_inlier(const Ransac<ThreePointPose>& test, double threshold){
  Vector<3> xyz = pose.get_rotation()*unproject(test.data.im1->normalized_cam_coords)
    + pose.get_translation()*test.data.im1->q;
  Vector<2> uv1 = project(xyz);
  Vector<2> diff = test.data.im2->normalized_cam_coords - uv1;
  double err = diff*diff;
  double score = 1-err/(threshold*threshold);
  if (score<0){
    score=0;
  }
  return score;
}
