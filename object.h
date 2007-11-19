/**
 *
 * Author: Ricardo Marroquim
 *
 * Data created: 2007-10-17
 *
 **/

#ifndef __OBJECT_H__
#define __OBJECT_H__

#define GL_GLEXT_PROTOTYPES

#include "primitives.h"


/* typedef enum  */
/*   { */
/*     TRIANGLES, */
/*     LINES, */
/*     PYRAMID_POINTS, */
/*     PYRAMID_POINTS_COLOR, */
/*     PYRAMID_TRIANGLES, */
/*     PYRAMID_HYBRID, */
/*     PYRAMID_LINES, */
/*     EWA_SPLATTING, */
/*     EWA_SPLATTING_INTERPOLATE_NORMALS, */
/*     NONE */
/*   } point_render_type_enum; */

using namespace std;

class Object
{
 public:
  
  Object() { }

  Object(int id_num, double x, double y, double z) : id(id_num) {
    center[0] = x;
    center[1] = y;
    center[2] = z;
    q_rot.a = 1; 
    q_rot.x = 0.0; 
    q_rot.y = 0.0; 
    q_rot.z = 0.0;
  }

  Object(int id_num) : id(id_num) {
    center[0] = center[1] = center[2] = 0.0;
    q_rot.a = 1; q_rot.x = 0.0; q_rot.y = 0.0; q_rot.z = 0.0;
  }

  ~Object() {}

  void render ( void );

  void setId ( int id_num ) { id = id_num; }

  double* getCenter ( void ) { return &center[0]; }
  void setCenter ( double c[3] ) { 
    for (int i = 0; i < 3; ++i)
      center[i]=c[i]; 
  }

  Quat* getRotationQuat ( void ) { return &q_rot; }
  void setRotationQuat ( Quat* q ) { q_rot = *q; }

  void addPrimitives( Primitives * p) {primitives_list.push_back(p);}
  vector<Primitives*> * getPrimitivesList( void ) {return &primitives_list;}

 private:

  // Center position (for individual translation)
  double center[3];
  vector<Point> centers;

  // Rotation quaternion (for individual rotation)
  Quat q_rot;
  vector<Quat> rotations;

  // Number of instances of this object
  int instances;

  // Pointer to instance of class cointaing primitives (verts, lines, triangles)
  vector<Primitives*> primitives_list;

  // Object identification number.
  int id;
};

#endif
