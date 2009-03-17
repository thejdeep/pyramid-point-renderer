/**
 *
 * Author: Ricardo Marroquim
 *
 * Data created: 2007-10-17
 *
 **/

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "primitives.h"

using namespace std;

class Object
{
 public:
  
  Object() { }

 Object(int id_num, double x, double y, double z) : scale_factor(1.0), _id(id_num) {
    center = Point3f(x, y, z);
  }

 Object(int id_num) : scale_factor(1.0), _id(id_num) {
    center = Point3f(0.0, 0.0, 0.0);
  }

  ~Object() {
	centers.clear();
	primitives_ids_list.clear();
  }

  void render ( void );
  void translate ( void );
  void rotate ( void );
  void scale ( void );

  void setId ( int id_num ) { _id = id_num; }
  int id ( void ) const { return _id; }

  char* filename( void ) { return _filename; }
  void setFilename ( const char* name ) {
    strcpy (&_filename[0], name);
  }

  double* getScaleFactor ( void ) { return &scale_factor; }


  Point3f* getCenter ( void ) { return &center; }

  void setCenter ( Point3f c ) { 
    center = c;
  }

  void addPrimitives( int p_id) { primitives_ids_list.push_back(p_id); }
  vector< int > * getPrimitivesList( void ) { return &primitives_ids_list; }

 private:

  // Center position (for individual translation)
  Point3f center;
  vector<Point3f> centers;

  // Scale factor for zooming
  double scale_factor;

  // Number of instances of this object
  int instances;

  // Pointer to instance of class cointaing primitives (verts, lines, triangles)
  vector< int > primitives_ids_list;

  // Object filename
  char _filename[200];

  // Object identification number.
  int _id;

};

#endif
