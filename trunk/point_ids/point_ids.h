/*
** point_ids.h Pyramid Point Based Rendering header.
**
**
**   history:	created  16-Jul-08
*/


#ifndef __POINT_IDS_H__
#define __POINT_IDS_H__

#include <cmath>
#include <cassert>

#include "point_based_renderer.h"

class PointIds : public PointBasedRenderer
{
 private:

  void createFBO( void );
  void createShaders ( void );
  void projectSurfels( Primitives * );
  void prepareBuffer( void );

 public:
  PointIds();
  PointIds(int w, int h);
  ~PointIds();

  void drawPointIdsBuffer ( GLfloat* data, int w, int h );
  void clearBuffers (void);
  void projectSamples ( Primitives* prim );

 private:

  /// Canvas border width.
  int canvas_border_width;
  /// Canvas border height.
  int canvas_border_height;

  /// Projection shader.
  glslKernel *shader_projection;

  /// The application-created framebuffer object.
  GLuint fbo;

  GLuint fbo_depth;

  /// usually fboBuffers[i] == GL_COLOR_ATTACHMENT0_EXT + i, 
  /// but we don't rely on this assumption
  GLuint fbo_buffer;

  /// Textures bound to the framebuffer object; 
  GLuint fbo_texture;
  
  /// total number of points to be projected (for normalization purposes)
  int number_points;

};

#endif
