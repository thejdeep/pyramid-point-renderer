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

#include "pyramid_point_renderer.h"
#include "pyramid_point_renderer_color.h"
#include "pyramid_point_renderer_er.h"

#include "camera.h"

/* #define CANVAS_WIDTH  1024 */
/* #define CANVAS_HEIGHT 1024 */
#define CANVAS_WIDTH  768
#define CANVAS_HEIGHT 768
/* #define CANVAS_WIDTH  512 */
/* #define CANVAS_HEIGHT 512 */

class Application
{
 private :

  void createPointRenderer( void );
  void glVertex ( const Surfeld * s ) const;
  void glVertex ( const surfelVectorIter it ) const;
  void glVertex ( const Point p ) const;

  void changePrimitivesRendererType ( point_render_type_enum type );

  void drawPoints ( void );

 public :

  Application( GLint default_mode = PYRAMID_POINTS );
  ~Application();

  int readFile ( const char * filename );
  int readNormalsFile ( const char * filename );
  void draw ( void );
  void reshape ( int w, int h );

  void changeRendererType ( int type );
  void changeMaterial( int mat );

  int getNumberPoints ( void );
  double getReconstructionFilter ( void ) const { return reconstruction_filter_size; }
  double getPrefilter ( void ) const { return prefilter_size; }
  Camera* getCamera ( void ) { return camera; }
  vector<Surfeld>* getSurfelsList ( void );

  void setGpuMask ( int m );
  void setPerVertexColor ( bool b );
  void setAutoRotate ( bool r );

  void toggleBackFaceCulling ( void );
  void setEllipticalWeight ( bool b );

  void setReconstructionFilter ( double s );
  void setPrefilter ( double s );
  void toggleDepthTest ( void );
  
  void mouseLeftButton( int x, int y );
  void mouseMiddleButton(int x, int y);
  void mouseRightButton(int x, int y);
  void mouseReleaseButton( void );
  void mouseLeftMotion( int x, int y );
  void mouseMiddleMotion( int x, int y );
  void mouseMiddleMotionShift( int x, int y );
  void mouseRightMotion( int x, int y );

 private :

  // Generic class, is instanced as one of the inherited classes (rendering algorithms)
  PointBasedRenderer *point_based_render;

  // Camera class
  Camera *camera;

  // Lists of objects and primitives
  // In this simple application only one object associated to one primitive is used.
  vector<Object> objects;
  vector<Primitives> primitives;

  // Determines which rendering class to use (Pyramid points, pyramid lines for tree rendering, ellipse rasterization ...)
  // see primitives.h for the complete list (point_render_type_enum).
  GLint render_mode;

  /*****Visual interface vars*****/

  double reconstruction_filter_size;
  double prefilter_size;
  int mask_size;

  int material;

  // Flags on/off
  bool show_points;
  bool elliptical_weight;
  bool depth_culling;
  bool back_face_culling;
  bool rotating;

  /***** Frames per second and Surfels per second vars ******/
  double sps, fps;
  int fps_loop;
  double start_time, end_time;
  int timing_profile;

  /*************************************/

};

#endif
