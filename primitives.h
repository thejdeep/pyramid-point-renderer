/**
 *
 * Author: Ricardo Marroquim
 *
 * Data created: 2007-11-19
 *
 **/

#ifndef __PRIMITIVES_H__
#define __PRIMITIVES_H__

//#define GL_GLEXT_PROTOTYPES

#include "surfels.h"
#include "quat.h"
#include "kd-tree.h"

#include "GL/glu.h"

#define LOD_LEVELS 4
#define MAX_LEAF_SURFELS 4 // the code expects this to be 4

typedef enum 
  {
    TRIANGLES,
    LINES,
    PYRAMID_POINTS,
    PYRAMID_TRIANGLES,
    PYRAMID_LINES,
    PYRAMID_HYBRID,
    PYRAMID_POINTS_LOD,
    NONE,
    PYRAMID_POINTS_COLOR,
    EWA_SPLATTING,
    EWA_SPLATTING_INTERPOLATE_NORMALS,
    PYRAMID_HYBRID_TEST

  } point_render_type_enum;

using namespace std;

typedef KdTree<Surfel*, OverflowRefine <Surfel*, MAX_LEAF_SURFELS> > KdTree3D;
typedef KdTree3D::Node KdTree3DNode;

class Primitives
{
 public:
  
  Primitives() : color_model(0) { }

  Primitives(int id_num, GLfloat t) : id(id_num), type(t), color_model(0) {}
      
  Primitives(int id_num) : id(id_num), color_model(0) {}
      
  ~Primitives() {}

  void render ( void );

  vector<Surfel> * getSurfels ( void ) { return &surfels[0]; }
  vector<Triangle> * getTriangles( void ) { return &triangles; }

  int getRendererType ( void ) { return renderer_type; }
  void setRendererType ( int type );

  void setType ( GLfloat t ) {type = t;}
  GLfloat getType ( void ) {return type;}

  void setId ( int id_num ) { id = id_num; }
  int getId ( void ) { return id; }

  int numberPoints ( void ) const { return number_points; }
  int numberTriangles ( void ) const { return number_triangles; }

  void setPerVertexColor( bool c ) { color_model = c; }
  bool getPerVertexColor( void ) const { return color_model; }

  void createLOD ( void );

  void writeFileLOD ( const char* fn );
  void readFileLOD ( const char* fn );

  inline uint numPrimitivesIn(void) { return numPatches; }
  inline uint numPrimitivesLOD(void) { return numVertsArray; }
  inline uint numPrimitivesInLevel( int l ) { return surfels[l].size(); }

 private:

  void reorderSurfels ( void );
  void createLOD ( int lod );

  void setPyramidPointsArraysLOD( void );
  void setPatchesArray ( void );

  void setPyramidPointsArrays( void );
  void setPyramidPointsArraysColor( void );
  void setPyramidPointsDisplayList( void );
  void setPyramidTrianglesDisplayList( void );
  void setPyramidTrianglesArrays( void );
  void setPyramidHybridDisplayList( void );
  void setPyramidHybridTestDisplayList( void );
  void setPyramidLinesDisplayList( void );
  void setPyramidLinesArraysColor ( void );
  void setTrianglesDisplayList( void );
  void setTrianglesArrays( void );
  void setLinesDisplayList( void );

  // Primitives group identification number.
  int id;

  // Type of primitives (Cylinder, other)
  GLfloat type;
  
  // Flag to indicate per vertex color component
  bool color_model;

  // Rendering type.
  int renderer_type;

  /// Vertex buffer
  GLuint vertex_buffer;

  /// Color Buffer
  GLuint color_buffer;

  /// Normal Buffer
  GLuint normal_buffer;

  /// Triangle Display List
  GLuint triangleDisplayList;

  /// Number of samples.
  int number_points;

  /// Number of triangles or lines
  int number_triangles;

  /// Indices vector for vertex arrays for lines
  GLuint *indices;

  // Vector of surfels belonging to this object.
  //  vector<Surfel> surfels;

  // Vector of triangles belonging to this object.
  vector<Triangle> triangles;

  GLuint vertex_patches_buffer;
  GLuint normal_patches_buffer;
  GLuint surfels_per_level_patches_buffer;

  GLuint vertTextBufferObject; ///< Texture buffer object
  GLuint normalTextBufferObject; ///< Texture buffer object

  uint numPatches, numVertsArray;

  // Vector of surfels belonging to this object.
  vector<Surfel> surfels[LOD_LEVELS];

  GLint * merged_ids[LOD_LEVELS];

  vector<Surfel> surfels_lod;
  vector<GLuint> surfels_per_level;

  KdTree3D * kdTree;

};

#endif
