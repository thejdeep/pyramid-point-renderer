/**
 * Quaternion class
 *
 * Author : Ricardo Marroquim
 * Date Created ; 28-01-07
 *
 **/

#ifndef __QUAT_H__
#define __QUAT_H__

#include "math.h"

class Quat {
public:
  double x,y,z,a;

  Quat() {}

  Quat(double _a, double _x, double _y, double _z) {
    a = _a;
    x = _x;
    y = _y;
    z = _z;
  }
   
  // Constructor with an angle and an axis
  Quat(double rot, const double axis[3]) {
    float c = cos(0.5 * rot);
    float s = sin(0.5 * rot);
    float t = s / sqrt(axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2]);
    a = c;
    x = axis[0] * t;
    y = axis[1] * t;
    z = axis[2] * t;
  }


  // Rotate a vector using this quaternion
  void rotate(double vec[]) {
    double len = sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);

    if (len == 0.0)
      return;

    Quat v(0, vec[0], vec[1], vec[2]);
    Quat qbar(a, -x, -y, -z); // complement
    Quat qtmp;
 
    qtmp = composeWith(v);
    qtmp = qtmp.composeWith(qbar);
    qtmp.normalize();

    //rescale to original size
    vec[0] = qtmp.x * len;
    vec[1] = qtmp.y * len;
    vec[2] = qtmp.z * len;
  }

  // Multiply by given quaternion
  // @param q Given quaternion
  // @return Multiplication result
  Quat composeWith(const Quat & q) {
    Quat result;
    result.a = a*q.a - x*q.x - y*q.y - z*q.z;
    result.x = a*q.x + x*q.a + y*q.z - z*q.y;
    result.y = a*q.y + y*q.a + z*q.x - x*q.z;
    result.z = a*q.z + z*q.a + x*q.y - y*q.x;
    //result.normalize();
    return result;
  }
      
  void normalize() {
    double d = sqrt(a*a + x*x + y*y + z*z);
    if (d == 0.0) {
      a = 1.0;
      x = y = z = 0.0;
      return;
    }
    d = 1.0 / d;
    a *= d;
    x *= d;
    y *= d;
    z *= d;
  }

  void toRotAxis(double &rot, double axis[]) {
    rot = 2.0 * acos(a);
    if (rot == 0) {
      axis[0] = axis[1] = 0; axis[2] = 1;
      return;
    }
    double d = 1.0 / sqrt(x*x + y*y + z*z);
    axis[0] = x * d;
    axis[1] = y * d;
    axis[2] = z * d;
  }
      
};

#endif
