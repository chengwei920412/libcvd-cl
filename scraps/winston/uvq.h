// -*- c++ -*-

#include <TooN/TooN.h>
#include <TooN/se3.h>

struct UVQ {
  TooN::Vector<3> uvq;  // uvq is public in struct UVQ
};


UVQ operator*(const TooN::SE3<>& pose, const UVQ& rhs){
  UVQ result;
  
  const TooN::Matrix<3>& R = pose.get_rotation().get_matrix();
  const TooN::Vector<3>& t = pose.get_translation();

  double qfac = 1.0/(R(2,0)*rhs.uvq[0] + R(2,1)*rhs.uvq[1] + R(2,2) + t[2]*rhs.uvq[2]);

  result.uvq[0] = (R(0,0)*rhs.uvq[0] + R(0,1)*rhs.uvq[1] + R(0,2) + t[0]*rhs.uvq[2]) * qfac;
  result.uvq[1] = (R(1,0)*rhs.uvq[0] + R(1,1)*rhs.uvq[1] + R(1,2) + t[1]*rhs.uvq[2]) * qfac;
  result.uvq[2] = rhs.uvq[2] * qfac;

  return result;
}
