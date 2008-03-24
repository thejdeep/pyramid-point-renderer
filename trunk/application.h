/**
 * Header file for application.cc
 *
 * Application interface independent layer
 * 
 * Author: Ricardo Marroquim
 *
 * Data created: 20-12-07
 *
 **/

#ifndef __APPLICATION__
#define __APPLICATION__

//OpenGL and GLUT includes
#include <cassert>

//IO includes
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <cmath>

#include <list>
#include <vector>

//#include "ewa_surface_splatting.h"

#include "pyramid_point_render.h"
#include "pyramid_point_render_nearest.h"
#include "pyramid_point_render_color.h"
#include "pyramid_point_render_lod.h"
#include "pyramid_point_render_trees.h"
#include "pyramid_triangle_renderer.h"
#include "ellipse_rasterization.h"
#include "jfa_splatting.h"
#include "triangle_renderer.h"

#include "camera.h"

#define PI 3.14159265
#define HALF_PI 1.570796325
#define QUARTER_PI 0.785398163
#define E 2.71828183

#define CANVAS_WIDTH  768
#define CANVAS_HEIGHT 768

class Application
{
 private :

  void createPointRender( void );
  void glVertex ( const Surfel * s );
  void glVertex ( surfelVectorIter it );
  void glVertex ( Point p );

  void changeMaterial ( void );
  void changePrimitivesRendererType ( point_render_type_enum type );

 public :

  Application();
  ~Application();

  void setColorBars ( bool c );
  void renderLODColorBars( void );

  int writeLodFile ( void );
  int writeSceneFile ( void );
  int readFile ( const char * filename );
  int readLodFile ( const char * filename );
  int readPolFile (const char * filename, vector<int> * ids);
  int readSceneFile (const char * filename, vector<int> *objs_ids);
  void drawPoints ( void );
  void draw ( void );
  void reshape ( int w, int h );

  void changeRendererType ( int type );
  void changeMaterial( int mat );
  void changeSelectedObjsMaterial( int mat );

  int getRendererType ( int object_id );
  int getNumberPoints ( int object_id );
  int getNumberTriangles ( int object_id );
  double getReconstructionFilter ( void ) const { return reconstruction_filter_size; }
  double getPrefilter ( void ) const { return prefilter_size; }
  int getMaterial ( void ) const { return material_id; }

  void resetMaxValues ( void );

  void setCpuMask ( int m );
  void setGpuMask ( int m );
  void setSampleSubdivision ( int s );
  void setPerVertexColor ( bool b, int object_id );
  void setAutoRotate ( bool r );
  void useLOD( bool l );
  void setLodColors( bool l );
  void switchLodsPerc ( void );

  void setReconstructionFilter ( double s );
  void setPrefilter ( double s );
  void setDepthTest ( bool b );

  void clearSelectedObjects ( void );
  void setSelectedObject ( int id );

  void mouseLeftButton( int x, int y );
  void mouseMiddleButton(int x, int y);
  void mouseRightButton(int x, int y);
  void mouseReleaseButton( void );
  void mouseLeftMotion( int x, int y );
  void mouseMiddleMotion( int x, int y );
  void mouseMiddleMotionShift( int x, int y );
  void mouseRightMotion( int x, int y );

  void createKeyFrame( void ) {
    if (camera) camera->createKeyFrame(reconstruction_filter_size, prefilter_size);
  }
  void clearFrames( void ) {
    if (camera) camera->clearFrames();
  }
  void writeKeyFrames( void ) {
    if (camera) camera->writeKeyFrames("camera.frames");
  }
  void loadKeyFrames( const char* filename ) {
    if (camera) camera->loadKeyFrames(filename);
  }
  void runFrames( void ) {
    if (camera) camera->runFrames();
  }
  bool runningFrames( void ) {
    return (camera) ? camera->runningFrames() : false;
  }

 private :

  unsigned int number_surfels;

  PointBasedRender *point_based_render;

  Camera *camera;

  int material_id;
  vector< int > selected_objs;
  int num_objects;

  vector<Object> objects;
  vector<Primitives> primitives;

  // Vector of point samples
  //std::vector<Surfel> surfels;
  // Vector of triangles with indices to surfels
  //std::vector<Triangle> triangles;

  // Determines which rendering class to use (Pyramid points, pyramid lines for tree rendering, ellipse rasterization ...)
  // see primitives.h for the complete list (point_render_type_enum).
  GLint render_mode;

  /*****Visual interface global vars*****/
  surfelVectorIter selected_surfel;
  int selected_point_id;

  double reconstruction_filter_size;
  double prefilter_size;

  bool show_points;
  bool show_color_bars;
  int show_splats;
  bool lods_perc;

  bool elliptical_weight;
  bool depth_culling;
  bool rotating;
  bool color_model;

  // Frames per sencond and Surfels per second
  double sps, fps;
  int fps_loop;
  double start_time, end_time;
  int timing_profile;

  int surfs_per_level[5];
  int max_surfs_per_level[4];

  /*************************************/

};

#endif
