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

#include "surfels.h"
#include "quat.h"

#include "GL/glut.h"



typedef enum 
  {
    TRIANGLES,
    PYRAMID_POINTS,
    PYRAMID_TRIANGLES,
    PYRAMID_HYBRID,
    EWA_SPLATTING,
    EWA_SPLATTING_INTERPOLATE_NORMALS
  } point_render_type_enum;

using namespace std;

class Object
{
 public:
  
  Object() { }

  Object(int id_num) : id(id_num) {}

  ~Object() {}

  void render ( void );

  vector<Surfel> * getSurfels ( void ) { return &surfels; }
  vector<Triangle> * getTriangles( void ) { return &triangles; }

  int getRendererType ( void ) { return renderer_type; }

  void setRendererType ( int type );

  void setId ( int id_num ) { id = id_num; }


 private:

  void setArrays( void );
  void setDisplayList( void );
  void setHybridDisplayList( void );
  void setTriangleDisplayList( void );

  // Center position (for individual translation)

  // Rotation quaternion (for individual rotation)
  Quat q_rotation;

  // Rendering type.
  int renderer_type;

  /// Vertex buffer
  GLuint vertex_buffer;
  /// Normal Buffer
  GLuint normal_buffer;
  /// Triangle Display List
  GLuint triangleDisplayList;

  /// Number of samples.
  int number_points;

  // Vector of surfels belonging to this object.
  vector<Surfel> surfels;

  // Vector of triangles belonging to this object.
  vector<Triangle> triangles;

  // Object identification number.
  int id;
};

#endif
