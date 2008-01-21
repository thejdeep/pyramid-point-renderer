/*
** pyramid_point_render.h Pyramid Point Based Rendering header.
**
**
**   history:	created  02-Jul-07
*/

#ifndef __PYRAMID_POINT_RENDER_COLOR_H__
#define __PYRAMID_POINT_RENDER_COLOR_H__

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "point_based_render.h"

#define FBO_BUFFERS_COUNT_6 6

class PyramidPointRenderColor : public PointBasedRender
{
 private:

  void createFBO( void );
  void createShaders ( void );
  int showCallbackFunc(pixels_struct dest, pixels_struct src0, pixels_struct src1);
  void showPixels(int bufferIndex);
  void rasterizePhongShading(int bufferIndex);
  int phongShadingCallbackFunc(pixels_struct dest, pixels_struct src0, pixels_struct src1);
  void rasterizeSynthesisPyramid();
  int synthesisCallbackFunc(pixels_struct dest, pixels_struct src0, pixels_struct src1);
  void copyAnalysisPyramid();
  int copyCallbackFunc(pixels_struct dest, pixels_struct src0, pixels_struct src1);
  void rasterizeAnalysisPyramid( void );
  int analysisCallbackFunc(pixels_struct dest, pixels_struct src0, pixels_struct src1);
  int projectionCallbackFunc(pixels_struct dest, pixels_struct src0, pixels_struct src1);
  void projectSurfels( Primitives* prim );

  pixels_struct generatePixels(int level, GLuint fbo, int buffersCount, GLuint buffer0, GLuint buffer1, GLuint buffer2);
  void rasterizePixels(pixels_struct dest, pixels_struct src0, pixels_struct src1, int phase);
  GLuint getTextureOfBuffer(GLuint buffer);

  double computeHalfPixelSize(int level);

 public:
  PyramidPointRenderColor();
  PyramidPointRenderColor(int w, int h);
  ~PyramidPointRenderColor();

  void draw();
  
  void clearBuffers (void);
  void projectSamples ( Primitives* prim );
  void interpolate ( void );

  void setVertices( vector<Surfel> *surfels );
  void setPrefilterSize(double s);
  void setReconstructionFilterSize(double s);
  void setZoomFactor (double z);
  void setEye (double e[3]);
  void setLight (double l[3]);

  void useLOD( bool l ) {
    use_lod = l;
    if (l)
      shader_projection = shader_projection_lod;
    else
      shader_projection = shader_projection_no_lod;
  }

 private:
  /// Frame buffer object width.
  int fbo_width;
  /// Frame buffer object height.
  int fbo_height;
  /// Number of frame buffer object attachments.
  int fbo_buffers_count;
  /// Canvas border width.
  int canvas_border_width;
  /// Canvas border height.
  int canvas_border_height;

  /// Pointer to the projection shader being used (no_lod or lod)
  glslKernel *shader_projection;
  /// Projection shader without LOD rendering
  glslKernel *shader_projection_no_lod;
  /// Projection shader with LOD rendering
  glslKernel *shader_projection_lod;
  /// Pyramid copy phase shader.
  glslKernel *shader_copy;
  /// Pyramid analysis phase shader.
  glslKernel *shader_analysis;
  /// Pyramid synthesis phase shader.
  glslKernel *shader_synthesis;
  /// Phong shading shader.
  glslKernel *shader_phong;
  /// Pixel shader.
  glslKernel *shader_show;

  /// The application-created framebuffer object.
  GLuint fbo;

  GLuint fbo_depth;

  /// usually fboBuffers[i] == GL_COLOR_ATTACHMENT0_EXT + i, 
  /// but we don't rely on this assumption
  GLuint fbo_buffers[FBO_BUFFERS_COUNT_6];

  /** Textures bound to the framebuffer object; 
   * the ping-pong rendering switches between pairs 0-2 and 1-3
   * because this can be easily generalized to triples 0-2-4 and 1-3-5 etc.
   * (pairs 0-1 and 2-3 would have to be rearranged to 0-1-2 and 3-4-5).
   * use getTextureOfBuffer to find the texture name of a buffer
   **/
  GLuint fbo_textures[FBO_BUFFERS_COUNT_6];
  
  /// Number of pyramid levels.
  int levels_count;

  /// Current rasterize level
  int cur_level;

  /// Type of most recently used framebuffer.
  framebuffer_state_enum framebuffer_state; 
  
  /// Current rendering mode.
  render_state_enum render_state;

  /// Num of primitives to be passed to the geom shader when using LOD
  int num_primitives;

};

#define CHECK_FOR_OGL_ERROR()	  									 \
	do {															 \
		GLenum err;													 \
		err = glGetError();											 \
		if (err != GL_NO_ERROR)										 \
		{															 \
			fprintf(stderr, "%s(%d) glError: %s\n",					 \
					__FILE__, __LINE__, gluErrorString(err));		 \
		}															 \
	} while(0)

#if !defined NULL
#	define NULL 0
#endif

#if !defined TRUE
#	define TRUE 1
#endif

#if !defined FALSE
#	define FALSE 0
#endif


 /* macros */

#if !defined MAX
#	define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#endif
