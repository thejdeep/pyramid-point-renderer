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
#include <assert.h>

//IO includes
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <math.h>

#include <list>
#include <vector>


#include "camera.h"
#include "materials.h"

//#include "ewa_surface_splatting.h"
//#include "pyramid_triangle_renderer.h"
//#include "triangle_renderer.h"

#include "pyramid_point_render.h"
#include "pyramid_point_render_color.h"
#include "pyramid_point_render_trees.h"

#include "object.h"

#define PI 3.14159265
#define HALF_PI 1.570796325
#define QUARTER_PI 0.785398163
#define E 2.71828183

class Application
{
 private :

  void createPointRender( int type );
  //void screenText( int w, int h );
  Point unproject ( const Point& p );
  void glVertex ( const Surfel * s );
  void glVertex ( surfelVectorIter it );
  void glVertex ( Point p );

 public :

  Application();
  ~Application() {};

  int readFile ( char * filename );
  void drawPoints ( void );
  void draw ( void );
  void reshape ( int w, int h );
  void changeMaterial ( void );
  void changeRendererType ( point_render_type_enum type );

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

 private :

  unsigned int number_surfels;

  PointBasedRender *point_based_render;

  Camera * camera;

  int material_id;
  vector< int > selected_objs;
  int num_objects;

  vector<Object> objects;
  vector<Primitives> primitives;

  // Vector of point samples
  std::vector<Surfel> surfels;
  // Vector of triangles with indices to surfels
  std::vector<Triangle> triangles;

  /*****Visual interface global vars*****/
  surfelVectorIter selected_surfel;
  int selected_point_id;

  double reconstruction_filter_size;
  double prefilter_size;

  GLint render_mode;
  bool show_kd_tree;
  bool show_points;
  int show_splats;
  bool show_screen_info;

  bool elliptical_weight;
  bool depth_culling;
  bool rotating;
  bool color_model;

  bool active_shift;

  int analysis_filter_size;

  double max_radius;

  // Frames per sencond and Surfels per second
  double sps, fps;
  int fps_loop;
  double start_time, end_time;
  int timing_profile;

  /*************************************/

};

#endif