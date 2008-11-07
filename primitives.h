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
    TRIANGLES = 0,
    LINES,
    PYRAMID_POINTS,
    PYRAMID_TRIANGLES,
    PYRAMID_LINES,
    PYRAMID_HYBRID,
    PYRAMID_POINTS_LOD,
    PYRAMID_POINTS_UPSAMPLING,
    PYRAMID_POINTS_JFA,
    PYRAMID_POINTS_ER,
    RASTERIZE_ELLIPSES = 10,
    JFA_SPLATTING,
    PYRAMID_POINTS_TEXTURE,
    EWA_SPLATTING,
    NONE,
    PYRAMID_POINTS_COLOR,
    EWA_SPLATTING_INTERPOLATE_NORMALS,

  } point_render_type_enum;

using namespace std;

class Primitives
{
 public:
  
  Primitives() : color_model(0), material(0) { }

  Primitives(int id_num, GLfloat t) : id(id_num), type(t), color_model(0), material(0) {}
    
  Primitives(int id_num) : id(id_num), color_model(0), material(0) {}
      
  ~Primitives() {}

  void render ( void );

  vector<Surfeld> * getSurfels ( void ) { return &surfels; }
  vector<Triangle> * getTriangles( void ) { return &triangles; }

  int getRendererType ( void ) { return renderer_type; }
  void setRendererType ( int type );

  void setType ( GLfloat t ) {type = t;}
  GLfloat getType ( void ) {return type;}

  void setMaterial ( int m ) {material = m;}
  int getMaterial ( void ) const {return material;}

  void setId ( int id_num ) { id = id_num; }
  int getId ( void ) { return id; }

  int numberPoints ( void ) const { return number_points; }
  int numberTriangles ( void ) const { return number_triangles; }

  void setPerVertexColor( bool c ) { color_model = c; }
  bool getPerVertexColor( void ) const { return color_model; }

  Point eye;

 private:

  void setPyramidPointsArraysLOD( void );
  void setPatchesArray ( void );

  void setPyramidPointsArrays( void );
  void setPyramidPointsArraysColor( void );
  void setPyramidPointsArraysTexture ( void );

  // Primitives group identification number.
  int id;

  // Type of primitives (Cylinder, other)
  GLfloat type;
  
  // Flag to indicate per vertex color component
  bool color_model;

  // Material id
  int material;

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

  /// Number of triangles or lines
  int number_triangles;

  /// Indices vector for vertex arrays for lines
  GLuint *indices;

  // Vector of triangles belonging to this object.
  vector<Triangle> triangles;

  // Vector of surfels belonging to this object.
  vector<Surfeld> surfels;

  vector<Surfeld> surfels_lod;
  vector<GLuint> surfels_per_level;

  // Flag indicating if surfels are circles or ellipses
  bool elliptical_surfels;

};

#endif
