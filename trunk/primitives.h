/**
 *
 * Author: Ricardo Marroquim
 *
 * Data created: 2007-11-19
 *
 **/

#ifndef __PRIMITIVES_H__
#define __PRIMITIVES_H__

#include "quat.h"

#include <cfloat>

typedef enum 
  {
    PYRAMID_POINTS,
    PYRAMID_POINTS_COLOR,
    PYRAMID_TEMPLATES
  } point_render_type_enum;

using namespace std;

class Primitives
{
 public:
  
  Primitives() { }

  Primitives(int id_num, GLfloat t) : id(id_num) {}
    
  Primitives(int id_num) : id(id_num)  {}
      
  ~Primitives();

  void render ( void ) const;

  vector<Surfeld> * getSurfels ( void ) { return &surfels; }
  vector<Triangle> * getTriangles ( void ) { return &triangles; }

  void clearSurfels ( void );

  int getRendererType ( void ) { return renderer_type; }
  void setRendererType ( int type );

  void setId ( int id_num ) { id = id_num; }
  int getId ( void ) { return id; }

  int numberPoints ( void ) const { return number_points; }

  Point eye;

 private:

  void setPyramidPointsArrays( void );
  void setPyramidPointsArraysColor( void );

  // Primitives group identification number.
  int id;
 
  // Rendering type.
  int renderer_type;

  /// Vertex buffer
  GLuint vertex_buffer;

  /// Color Buffer
  GLuint color_buffer;

  /// Normal Buffer
  GLuint normal_buffer;

  /// Number of samples.
  int number_points;

  // Vector of surfels belonging to this object.
  vector<Surfeld> surfels;

  // Vector of triangles belonging to this object.
  vector<Triangle> triangles;

};

#endif
