/**
 * ewa_surface_splatting.h EWA Surface Splatting header.
 *
 *
 * history:	created  05-Jul-07
 **/

#ifndef __EWA_SURFACE_SPLATTING_H__
#define __EWA_SURFACE_SPLATTING_H__

#define GL_GLEXT_PROTOTYPES

/* extern "C" { */
/* #include <GL/glut.h> */
/* #include <GL/glext.h> */
/* } */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector.h>

#include "point_based_render.h"

#define EPS 0.00000001

/// A pixel structure for the accumulation buffer (A-Buffer)
struct zBufferPixel {
  double r, g, b;
  double weight;
  double z;
  double radius;
  double zmax;
  double dx, dy;
  double w;
};

class EWASurfaceSplatting : public PointBasedRender
{
 private:

  Vector perpendicular( const Vector &v );
  void splatShader ( surfelListIter s, double mv[][4], double c[]);
  void phongShader ( double n[3], double c[]);

 public:
  EWASurfaceSplatting();
  EWASurfaceSplatting(int w, int h, bool i);
  ~EWASurfaceSplatting();

  void draw();
  void draw(int) {}
  
  void setVertices( list<Surfel> *surfels );
  void setPrefilterSize(double s);
  void setReconstructionFilterSize(double s);
  void setZoomFactor (double z);
  void setEye (double e[3]);
  void setLight (double l[3]);

 private:

  /// Number of samples.
  int number_points;

  /// Pointer to surfel list.
  list<Surfel> *surfels;

  /// Vector of pixels
  std::vector<zBufferPixel> zbuffer;

  /// Final image containing RBG values
  GLfloat *fb;

  // Flag to indicate normal or color interpolation
  bool interpolate_normals;
};

#endif
