#include "epipolar.h"

#define TOON_DEBUG 1


#include <TooN/TooN.h>
#include <TooN/wls.h>
#include <TooN/irls.h>

#include <iomanip>

#include <cvd/random.h>

using namespace TooN;
using namespace CVD;
using namespace std;


void Hypothesis<Epipolar>::generate(const std::vector<Ransac<Epipolar>* >& gen_set){
  Vector<3> X = makeVector(1,0,0);
  // Vector<3> trans = unit(makeVector(rand_g(), rand_g(), rand_g()));
  Vector<3> trans = makeVector(1,0,0);

  // guess rotation between cameras is nothing
  // Rt = Identity; // not needed because default initialiser is identity.
  Rn = SO3<> (X,trans);

  int count = 0;
  double old_err =  HUGE_VAL;
  WLS<5, double> wls;
  do {
    // cout << "count " << count << endl;
    wls.clear();
    double err=0;
    const Matrix<3> C = cross_product_matrix(Rn * X);
    for(unsigned int i = 0; i < gen_set.size(); ++i){
      // cout << "gen set point " << i << endl;
      Vector<5> J;

      // cout << gen_set[i]->data.im1 << endl;
      // cout << gen_set[i]->data.im2 << endl;


      Vector<3> v1 = unproject(gen_set[i]->data.im1->normalized_cam_coords);
      Vector<3> v2 = unproject(gen_set[i]->data.im2->normalized_cam_coords);

      // cout << "copied v1 and v2 from gen_set[" << i << "]" << endl;

      const Vector<3> LEFT = v2 * C;
      const Vector<3> RIGHT = Rt * v1;

      // cout << "about to build Jacobian" << endl;

      J[0] = LEFT * Rt.generator_field(0, RIGHT);
      J[1] = LEFT * Rt.generator_field(1, RIGHT);
      J[2] = LEFT * Rt.generator_field(2, RIGHT);
      J[3] = v2 * cross_product_matrix(Rn * Rn.generator_field(1, X)) * RIGHT;
      J[4] = v2 * cross_product_matrix(Rn * Rn.generator_field(2, X)) * RIGHT;

      // cout << "jacobian built" << endl;

      const double e = 0 - LEFT * RIGHT;
      err += e * e;
      wls.add_mJ(e, J);

      // cout << "added to wls" << endl;
    }

    // cout << "points added to wls" << endl;

    if(err>old_err)
      break;
    old_err = err;
    wls.add_prior(1e-6);
    wls.compute();

    // cout << "wls computed" << endl;

    Rt = SO3<>::exp(wls.get_mu().slice<0,3>()) * Rt;
    Rn = Rn * SO3<>::exp(makeVector(0, wls.get_mu()[3], wls.get_mu()[4]));

    ++count;
  } while(norm_sq(wls.get_mu()) > 1e-6 && count < 6);
}

double Hypothesis<Epipolar>::is_inlier(const Ransac<Epipolar>& test, double threshold){
  Vector<3> X = makeVector(1,0,0);
  Matrix<3> E = cross_product_matrix(Rn * X) * Rt;

  Vector<3> v1 = unproject(test.data.im1->normalized_cam_coords);
  Vector<3> v2 = unproject(test.data.im2->normalized_cam_coords);

  Vector<3> rline = v2*E;
  rline /= sqrt(rline[0]*rline[0] + rline[1]*rline[1]);

  Vector<3> lline = E*v1;
  lline /= sqrt(lline[0]*lline[0] + lline[1]*lline[1]);

  double lerr = v2*lline;
  double rerr = rline*v1;

  double errsq = lerr*lerr + rerr*rerr;

  double score = 1 - errsq/(threshold*threshold);
  if(score < 0){
    score=0;
  }
  return score;
}

void  Hypothesis<Epipolar>::save(std::ostream& os){
  os << setprecision(10) << Rt << endl << Rn << endl;
}

void  Hypothesis<Epipolar>::load(std::istream& is){
  is >> Rt >> Rn;
}



