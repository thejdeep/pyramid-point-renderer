/**
 *
 * Author: Ricardo Marroquim
 *
 * Data created: 2007-10-17
 *
 **/

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "lal/Matrix4x4.hpp"
#include "primitives.h"

using namespace std;

typedef LAL::Matrix4x4<double> Matrix4;

class Object
{
 public:
  
  Object() { }

  Object(int id_num, double x, double y, double z, Quat q) : _id(id_num) {
    center = Point(x, y, z);
    q_rot = q;
  }

  Object(int id_num, double x, double y, double z) : _id(id_num) {
    center = Point(x, y, z);
    q_rot.a = 1.0; 
    q_rot.x = 0.0; 
    q_rot.y = 0.0; 
    q_rot.z = 0.0;
  }

  Object(int id_num) : _id(id_num) {
    center = Point(0.0, 0.0, 0.0);
    q_rot.a = 1.0; q_rot.x = 0.0; q_rot.y = 0.0; q_rot.z = 0.0;
  }

  ~Object() {}

  void render ( void );
  void render ( Point camera_pos );
  void translate ( void );
  void rotate ( void );

  void multMatrix ( const Matrix4 mat) {stateMatrix = stateMatrix * mat;}
  const Matrix4* getMatrix ( void ) const {return &stateMatrix;}

  void setId ( int id_num ) { _id = id_num; }
  int id ( void ) const { return _id; }

  char* filename( void ) { return _filename; }
  void setFilename ( const char* name ) {
    strcpy (&_filename[0], name);
  }

  Point* getCenter ( void ) { return &center; }

  void setCenter ( Point c ) { 
    center = c;
  }

  Quat* getRotationQuat ( void ) { return &q_rot; }
  void setRotationQuat ( Quat* q ) { q_rot = *q; }

  void addPrimitives( int p_id) { primitives_ids_list.push_back(p_id); }
  vector< int > * getPrimitivesList( void ) { return &primitives_ids_list; }

 private:

  // Center position (for individual translation)
  Point center;
  vector<Point> centers;

  // Rotation quaternion (for individual rotation)
  Quat q_rot;
  vector<Quat> rotations;

  // Number of instances of this object
  int instances;

  // Pointer to instance of class cointaing primitives (verts, lines, triangles)
  vector< int > primitives_ids_list;

  // Object filename
  char _filename[200];

  // Object identification number.
  int _id;

  // State matrix with objects current translation and rotation
  Matrix4 stateMatrix;
};

#endif
