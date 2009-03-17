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
// Standard headers
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stack>
#include <cassert>

using namespace std;

#include <vcg/simplex/vertex/base.h>
#include <vcg/simplex/vertex/component.h>

#include <vcg/simplex/face/base.h>
#include <vcg/simplex/face/component.h>

#include <vcg/complex/trimesh/base.h>
#include <vcg/complex/trimesh/update/normal.h>
#include <vcg/complex/trimesh/update/bounding.h>
#include <vcg/complex/trimesh/update/flag.h>

// input output
#include <wrap/io_trimesh/import.h>

//IO includes
#include <sstream>
#include <iomanip>
#include <cmath>

#include <list>
#include <vector>

#include "pyramid_point_renderer_base.h"
#include "pyramid_point_renderer.h"
#include "pyramid_point_renderer_color.h"
#include "pyramid_point_renderer_er.h"

#include <wrap/gui/trackball.h>

using namespace vcg;

class CFace;
class CEdge;
class CVertex  : public VertexSimp2< CVertex, CEdge, CFace, vertex::Coord3f, vertex::Normal3f, vertex::Color4b, vertex::Qualityf > {};
class CFace    : public FaceSimp2< CVertex, CEdge, CFace, face::VertexRef > {};
class CMesh    : public vcg::tri::TriMesh< vector<CVertex>, vector<CFace> > {};

class Application
{
 private :

  void createPointRenderer( void );
  void glVertex ( const Surfeld * s ) const;
  void glVertex ( const surfelVectorIter it ) const;
  void glVertex ( const Point3f p ) const;

  void changePrimitivesRendererType ( point_render_type_enum type );

  void drawPoints ( void );

 public :

  Application( GLint default_mode = PYRAMID_POINTS );
  ~Application();
  
  int readFile ( const char * filename );
  int appendFile ( const char * filename );

  int startFileReading ( void );
  int finishFileReading ( void );

  void draw ( void );
  void reshape ( int w, int h );

  void setView( void );

  void changeRendererType ( int type );
  void changeMaterial( int mat );

  int getNumberPoints ( void );
  vector<Surfeld>* getSurfelsList ( void );

  void setGpuMask ( int m );
  void setPerVertexColor ( bool b );
  void setAutoRotate ( bool r );

  void setBackFaceCulling ( bool c );
  void setEllipticalWeight ( bool b );
  void setQualityPerVertex ( bool c );

  void setQualityThreshold ( double q );
  void setReconstructionFilter ( double s );
  void setPrefilter ( double s );
  void setDepthTest ( bool d );
  
  void mouseLeftButton( int x, int y, bool shift, bool ctrl, bool alt );
  void mouseMiddleButton(int x, int y, bool shift, bool ctrl, bool alt );
  void mouseRightButton(int x, int y, bool shift, bool ctrl, bool alt );

  void mouseLeftMotion( int x, int y, bool shift, bool ctrl, bool alt );
  void mouseMiddleMotion( int x, int y, bool shift, bool ctrl, bool alt );
  void mouseRightMotion( int x, int y, bool shift, bool ctrl, bool alt );

  void mouseReleaseLeftButton( int x, int y, bool shift, bool ctrl, bool alt );
  void mouseReleaseMiddleButton( int x, int y, bool shift, bool ctrl, bool alt );
  void mouseReleaseRightButton( int x, int y, bool shift, bool ctrl, bool alt );

  void mouseWheel( int step, bool shift, bool ctrl, bool alt );

  void increaseSelected ( void );
  void decreaseSelected ( void );

 private :

  int readFile ( const char * filename, vector<Surfeld> *surfels );

  Trackball trackball;
  Trackball trackball_light;

  Box3f FullBBox;

  // Generic class, is instanced as one of the inherited classes (rendering algorithms)
  PointBasedRenderer *point_based_render;

  int canvas_width, canvas_height;
  int windows_width, windows_height;

  float clipRatioNear, clipRatioFar;
  float fov;
  float scale_factor;

  // Lists of objects and primitives
  // In this simple application only one object associated to one primitive is used.
  vector<Object> objects;
  vector<Primitives> primitives;

  // Determines which rendering class to use (Pyramid points, pyramid lines for tree rendering, ellipse rasterization ...)
  // see primitives.h for the complete list (point_render_type_enum).
  GLint render_mode;

  // Flags on/off
  bool show_points;
  bool rotating;

  int selected;

  /***** Frames per second and Surfels per second vars ******/
  double sps, fps;
  int fps_loop;
  double start_time, end_time;
  int timing_profile;

  /*************************************/

};

#endif
