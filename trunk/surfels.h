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

#include <list>
#include <vector>
#include <iostream>
#include <cmath>
#include "surfel.hpp"

using namespace std;
using namespace LAL;

typedef Point3<double> Point;
typedef Vector3<double> Vector;

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

typedef Surfel<double> Surfeld;

typedef vector<Surfeld>::iterator surfelVectorIter;
typedef vector<Triangle>::iterator triangleVectorIter;

#endif
