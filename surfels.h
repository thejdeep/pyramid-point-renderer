/**
 *
 * 
 * Author: Ricardo Marroquim
 *
 * Data created: 02-07-07
 *
 **/

#ifndef __SURFELS__
#define __SURFELS__

#include <list.h>
#include <vector.h>
#include <math.h>



struct Vector
{
  /// Constructor
  Vector() : _x(0.0), _y(0.0), _z(0.0), _w(0.0) { }

  /// Constructor
  Vector(double _nx, double _ny, double _nz) : _x(_nx), _y(_ny), _z(_nz), _w(0.0) { }

  /// Destructor
  ~Vector() { }


  /// Computes length of vectorextern double start_projection;

  const double length ( void ) const {
    return sqrt(_x*_x + _y*_y + _z*_z);
  }

  /// Computes length of vector
  void normalize ( void ) {
    double n = 1.0/length();
    _x *= n;
    _y *= n;
    _z *= n;
  }

  /// Operator to grant write/read access to the color values
  double& operator [](const uint i) {
    return (i==0) ? this->_x : ( (i==1) ? this->_y : ( (i==2) ? this->_z : ( (i==3) ? this->_w : (this->_w) ) ) );
  }

  // dot product
  double operator *(const Vector v) const {
    return (this->_x * v.x() + this->_y * v.y() + this->_z * v.z());
  }

  // cross product
  const Vector cross(const Vector v) {
    return Vector((this->_y * v.z()) - (this->_z * v.y()),
		  (this->_z * v.x()) - (this->_x * v.z()),
		  (this->_x * v.y()) - (this->_y * v.x()));
  }

  Vector operator /= (const double d) {
    *this = *this / d;
    return *this;
  }

  Vector operator += (const Vector v) {
    *this = *this + v;
    return *this;
  }

  // add by scalar, vector (s,s,s)
  const Vector operator +(const Vector v) {
    return Vector (this->_x + v.x(), this->_y + v.y(), this->_z + v.z());
  }

  // divide by scalar
  const Vector operator /(const double div) {
    return Vector (this->_x / div, this->_y / div, this->_z / div);
  }

  // multiply by scalar
  Vector operator *(const double mult) const {
    return Vector (this->_x * mult, this->_y * mult, this->_z * mult);
  }

  // multiply by scalar
  inline friend Vector operator *(const double mult, const Vector v) {
    return v * mult;
  }

  // subtract by scalar, vector (s,s,s)
  const Vector operator -(const double s) {
    return Vector (this->_x - s, this->_y - s, this->_z - s);
  }

  // add by scalar, vector (s,s,s)
  const Vector operator +(const double s) {
    return Vector (this->_x + s, this->_y + s, this->_z + s);
  }

  const double x ( void) const { return _x; }
  const double y ( void) const { return _y; }
  const double z ( void) const { return _z; }
  const double w ( void) const { return _w; }
 
  double _x, _y, _z, _w;
};

struct Point
{
  /// Constructor
  Point() : _x(0.0), _y(0.0), _z(0.0), _w(1.0) { }

  /// Constructor
  Point(double _nx, double _ny, double _nz) : _x(_nx), _y(_ny), _z(_nz), _w(1.0) { }

  /// Destructor
  ~Point() { }

  /// Operator to grant write/read access to values
/*   const double& operator [](const uint i) { */
/*     return (i==0) ? this->_x : ( (i==1) ? this->_y : ( (i==2) ? this->_z : ( (i==3) ? this->_w : (this->_w) ) ) ); */
/*   } */

  double squared_distance ( const Point p ) const {
    return pow(p.x() - this->_x, 2) + pow(p.y() - this->_y, 2) + pow(p.z() - this->_z, 2);
  }

  double& operator [](const uint i) {
    return (i==0) ? this->_x : ( (i==1) ? this->_y : ( (i==2) ? this->_z : ( (i==3) ? this->_w : (this->_w) ) ) );
  }


  Point operator /=(const double d) {
    this->_x /= d;
    this->_y /= d; 
    this->_z /= d;
    return *this;
  }


  const Vector operator -(const Point p) {
    return Vector (this->_x - p.x(), this->_y - p.y(), this->_z - p.z());
  }

  // add vector
  inline friend const Point operator +(const Point p, const Vector v) {
    return Point (p.x() + v.x(), p.y() + v.y(), p.z() + v.z());
  }

  // subtract vector
  inline friend const Point operator -(const Point p, const Vector v) {
    return Point (p.x() - v.x(), p.y() - v.y(), p.z() - v.z());
  }
 

  bool operator ==(const Point p) const {
    return ((this->_x == p.x()) && (this->_y == p.y()) && (this->_z == p.z()));
  }

  bool operator !=(const Point p) const {
    return !(*this == p);
  }


  inline friend double squared_distance (const Point p, const Point q) {
    return p.squared_distance(q);
  }

  const double pos ( int i ) const {
    return (i==0) ? this->_x : ( (i==1) ? this->_y : ( (i==2) ? this->_z : ( (i==3) ? this->_w : (this->_w) ) ) );
  }
  const double x ( void) const { return _x; }
  const double y ( void) const { return _y; }
  const double z ( void) const { return _z; }
  const double w ( void) const { return _w; }
 
  double _x, _y, _z, _w;
};


struct Box
{
  Box() {}

  Box(Point a, Point b) : _min(a), _max(b) {}


  const Point min ( void ) const { return _min; }
  const Point max ( void ) const { return _max; }

  const double xmin ( void ) const { return _min.x(); }
  const double ymin ( void ) const { return _min.y(); }
  const double zmin ( void ) const { return _min.z(); }
  const double xmax ( void ) const { return _max.x(); }
  const double ymax ( void ) const { return _max.y(); }
  const double zmax ( void ) const { return _max.z(); }

  Point _min, _max;

};


struct Triangle {
  int verts[3];
  int id;
};


static const Point default_color (0.8, 0.4, 0.2);
static const Point bg_color (1.0, 1.0, 1.0);
static const Point black_color (0.0, 0.0, 0.0);

/**
 * Surfel class.
 * A surface element (surfel) is a sample point of the surface.
 * Also known as splats in some implementations.
 * The splat has an elliptical or circular form, and an estimated
 * surface normal at it's center.
 **/
class Surfel
{
 public:

  Surfel(Point _p, Vector _n, Point _c, double _r, unsigned int _id) {
    p = Point(_p);
    id_num = _id;
    n = _n;
    c = _c;
    r = _r;
  }

  Surfel(Point _p, Vector _n, double _r, unsigned int _id) {
    p = Point(_p);
    id_num = _id;
    n = _n;
    c = default_color;
    r = _r;
  }

  Surfel(Point _p, unsigned int _id) {
    p = Point(_p);
    id_num = _id;
    n = Vector(0.0, 0.0, 0.0);
    c = default_color;
    r = 0;
  }

  Surfel(Point _p) {
    p = Point(_p);
    n = Vector(0.0, 0.0, 0.0);
    c = default_color;
    id_num = 0;
    r = 0;
  }

  Surfel() {
  }
  
  ~Surfel() {};

  const Point position(void) const { return p; }
  const double position(const int axis) { return p[axis]; }
  void setPosition(const Point _p) { p = Point(_p); }

  const Vector normal(void) const { return n; }
  const double normal(int axis) { return n[axis]; }
  void setNormal(Vector _n) { n = Vector(_n); }

  const Point color(void) const { return c; }
  const double color(int spectrum) { return c[spectrum]; }
  void setColor(Point _c) { c = Point(_c); }

  const unsigned int id (void) const { return id_num; }
  void setId (unsigned int _id) {id_num = _id; }

  const double radius (void) const { return r; }
  void setRadius (double _r) { r = _r; }
  
  /// I/O operator - output
  inline friend ostream& operator << (ostream& out, const Surfel &s) {
    out << s.id_num << " " << s.p.x() << " " << s.p.y() << " " << s.p.z() << " " << s.r << " "
	<< s.n.x() << " " << s.n.y() << " " << s.n.z();
    return out;
  }

  /// Point coordinates
  Point p;

  /// Estimated surface normal at point sample
  Vector n;

  /// Surface color at point sample
  Point c;

  /// Splat radius
  double r;

 private:

  /// An identification number for the surfel
  unsigned int id_num;

};

//typedef list<Surfel>::const_iterator surfelListIter;
typedef vector<Surfel>::iterator surfelVectorIter;
typedef vector<Triangle>::iterator triangleVectorIter;

#endif
