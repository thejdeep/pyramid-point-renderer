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
using namespace vcg;

/* typedef vcg::Point3f Point3; */
/* typedef vcg::Point4f Point4; */

struct Box
{
  Box() {}

  Box(Point3 a, Point3 b) : _min(a), _max(b) {}


  const Point3 min ( void ) const { return _min; }
  const Point3 max ( void ) const { return _max; }

  const double xmin ( void ) const { return _min[0]; }
  const double ymin ( void ) const { return _min[1]; }
  const double zmin ( void ) const { return _min[2]; }
  const double xmax ( void ) const { return _max[0]; }
  const double ymax ( void ) const { return _max[1]; }
  const double zmax ( void ) const { return _max[2]; }

  Point _min, _max;

};

struct Triangle {
  int verts[3];
  int id;
};

static const Point3 default_color (0.8, 0.4, 0.2);
static const Point3 bg_color (1.0, 1.0, 1.0);
static const Point3 black_color (0.0, 0.0, 0.0);

typedef Surfel<double> Surfeld;

typedef vector<Surfeld>::iterator surfelVectorIter;
typedef vector<Triangle>::iterator triangleVectorIter;

#endif
