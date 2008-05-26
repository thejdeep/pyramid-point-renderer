/**
 * Quaternion class
 *
 * Author : Ricardo Marroquim
 * Date Created ; 28-01-07
 *
 **/

#ifndef __QUAT_H__
#define __QUAT_H__

#include <iostream>
#include <cmath>
#include "surfels.h"

using namespace std;

class Quat {
public:
  double x,y,z,a;

  Quat() {
    a = x = y = z = 0.0;
  }

  Quat(const Quat& q) {
    a = q.a;
    x = q.x;
    y = q.y;
    z = q.z;
  }

  Quat(double _x, double _y, double _z, double _a) {
    a = _a;
    x = _x;
    y = _y;
    z = _z;
  }
   
  // Constructor with an angle and an axis
  Quat(const double axis[3], double rot) {
    float c = cos(0.5 * rot);
    float s = sin(0.5 * rot);
    float t = s / sqrt(axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2]);
    a = c;
    x = axis[0] * t;
    y = axis[1] * t;
    z = axis[2] * t;
  }


  // Rotate a vector using this quaternion
  Vector rotate(Vector vec) {
    double len = vec.length();

/*     if (len == 0.0) */
/*       return; */

    Quat v(vec.x(), vec.y(), vec.z(), 1.0);
    Quat qbar(-x, -y, -z, a); // complement
    Quat qtmp;
 
    qtmp = composeWith(v);
    qtmp = qtmp.composeWith(qbar);
    qtmp.normalize();

    //rescale to original size
    return Vector(qtmp.x, qtmp.y, qtmp.z) * len;
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

  // addition with another quaternion
  Quat operator +(const Quat r) const {
    return Quat (this->x + r.x, this->y +r.y, this->z + r.z, this->a + r.a);
  }

  //  multiplication with another quaternion
  Quat operator *(const Quat r) const {
    return Quat ( (this->y*r.z - this->z*r.y) + this->x*r.a + r.x*this->a,
		  (this->z*r.x - this->x*r.z) + this->y*r.a + r.y*this->a,
		  (this->x*r.y - this->y*r.x) + this->z*r.a + r.z*this->a,
		  (this->a*r.a - (this->x*r.x + this->y*r.y + this->z*r.z)) );
  }

  // multiply by scalar
  Quat operator *(const double mult) const {
    return Quat (this->x * mult, this->y * mult, this->z * mult, this->a * mult);
  }

  // multiply by scalar
  inline friend Quat operator *(const double mult, const Quat q) {
    return q * mult;
  }

  // divide by scalar
  Quat operator /(const double div) const {
    return Quat (this->x / div, this->y / div, this->z / div, this->a / div);
  }

  // returns the norm of the quaternion
  double norm ( void ) const {
    return this->x*this->x + this->y*this->y + this->z*this->z + this->a*this->a;
  }

  // logarithm
  inline friend Quat log( const Quat q ) {
    double len = sqrt(q.x*q.x + q.y*q.y + q.z*q.z);
    if (len <= 0)
      return Quat(q);
    double angle = acos(q.a);
    double c = angle / sin(angle);
    
    return Quat(q.x*c, q.y*c, q.z*c, 0);
  }



  // quaternion to the power of t
  Quat power ( double t ) const {
    double len = sqrt(this->x*this->x + this->y*this->y + this->z*this->z);
    if (len <= 0)
      return Quat(*this);
    double phi = len;//acos(this->a);
    double exs = sin(phi*t)/len;
    return Quat (this->x*exs, this->y*exs, this->z*exs, cos(phi*t));
  }

  // exponential function
  inline friend Quat exp( const Quat q ) {
    return q.power(1.0);
  }

  // returns the conjugate of the quaternion
  Quat conjugate( void ) const{
    return Quat(this->x * -1.0, this->y * -1.0, this->z * -1.0, this->a);
  }

  // returns the inverse of the quaternion
  Quat inverse( void ) const {
    return conjugate() / norm();       
  }

  // Spherical linear interpolation between two quaternions
  // with interpolation parameter t
  inline friend Quat slerp (Quat q, Quat r, double t) {
    double phi = acos ( q.x*r.x + q.y*r.y + q.z*r.z + q.a*r.a);
    double sin_phi = sin(phi);
    if ((phi == 0.0) || (sin_phi == 0.0) || (isnan(phi)))
      return q;
    return (q * (sin ( phi * (1.0 - t) ) / sin_phi)) + 
      (r * (sin ( phi * t ) / sin_phi));
  }

  // Smooth cubic spline interpolation
  inline friend Quat squad (Quat q, Quat r, Quat a, Quat b, double t) {
    return slerp ( slerp (q, r, t), slerp (a, b, t), 2.0*t*(1.0 - t) );
  }

  // Computes a new quaternion between q and r
  inline friend Quat intermediate ( Quat p, Quat q, Quat r ) {
    return q * exp ( -0.25 * (log( q.inverse() * p) + log( q.inverse() * r)));
  }

  // Cubic interpolation between quaternions 
  inline friend Quat cubicInterpolation ( Quat p, Quat q, Quat r, Quat s, double t ) {
    Quat a = intermediate( p, q, r );
    Quat b = intermediate( q, r, s );

    return squad (q, r, a, b, t);
  }

  /// I/O operator - output
  inline friend ostream& operator << (ostream& out, const Quat &q) {
    out << q.x << " " << q.y << " " << q.z << " " << q.a;
    return out;
  }

};

#endif
