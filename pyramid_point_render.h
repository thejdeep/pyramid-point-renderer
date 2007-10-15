/*
** pyramid_point_render.h Pyramid Point Based Rendering header.
**
**
**   history:	created  02-Jul-07
*/


#ifndef __PYRAMID_POINT_RENDER_H__
#define __PYRAMID_POINT_RENDER_H__

#define GL_GLEXT_PROTOTYPES

extern "C" {
/* #include <GL/glut.h> */
/* #include <GL/glext.h> */
#include "timer.h"
}

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "point_based_render.h"
#include "glslkernel.h"

#define FBO_TYPE GL_TEXTURE_2D
#define FBO_FORMAT GL_RGBA16F_ARB
#define FBO_BUFFERS_COUNT 4

/* types */

/** A pixels_struct specifies a rectangle of pixels 
 * in one or two buffers of one framebuffer object 
 **/
typedef struct 
{
  GLfloat x;
  GLfloat y;
  GLfloat width;
  GLfloat height;
  GLuint fbo; /* framebuffer object handle */
  GLuint buffers[2]; /* e.g. GL_BACK or GL_COLOR_ATTACHMENT0_EXT */
  GLuint textures[2]; /* texture handle */
  int buffersCount;
} pixels_struct;

typedef enum 
  {
    RS_BUFFER0,
    RS_BUFFER1,
    RS_BUFFER2,
    RS_BUFFER3,
    RS_TIME
  } render_state_enum;

typedef enum 
  {
    EMPTY,
    PROJECTION,
    ANALYSIS,
    COPY,
    SYNTHESIS,
    PHONG,
    SHOW
  } phase_enum;

typedef enum 
  {
    FBS_SYSTEM_PROVIDED,
    FBS_APPLICATION_CREATED,
    FBS_UNDEFINED
  } framebuffer_state_enum;


class PyramidPointRender : public PointBasedRender
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
  void projectPoints( void );
  pixels_struct generatePixels(int level, GLuint fbo, int buffersCount, GLuint buffer0, GLuint buffer1);
  void rasterizePixels(pixels_struct dest, pixels_struct src0, pixels_struct src1, int phase);
  GLuint getTextureOfBuffer(GLuint buffer);

  double computeHalfPixelSize(int level);

 public:
  PyramidPointRender();
  PyramidPointRender(int w, int h);
  ~PyramidPointRender();

  void draw();
  void draw(int timing_profile);
  
  void setVertices( list<Surfel> *surfels );
  void setPrefilterSize(double s);
  void setReconstructionFilterSize(double s);
  void setZoomFactor (double z);
  void setEye (double e[3]);
  void setLight (double l[3]);

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

  /// Vertex buffer
  GLuint vertex_buffer;
  /// Normal Buffer
  GLuint normal_buffer;
  /// Vertex List
  //GLuint vertex_list;

  /// Number of samples.
  int number_points;

  /// Point projection shader.
  GLSLKernel *shader_projection;
  /// Pyramid copy phase shader.
  GLSLKernel *shader_copy;
  /// Pyramid analysis phase shader.
  GLSLKernel *shader_analysis;
  /// Pyramid synthesis phase shader.
  GLSLKernel *shader_synthesis;
  /// Phong shading shader.
  GLSLKernel *shader_phong;
  /// Pixel shader.
  GLSLKernel *shader_show;

  /// The application-created framebuffer object.
  GLuint fbo;

  GLuint fbo_depth;

  /// usually fboBuffers[i] == GL_COLOR_ATTACHMENT0_EXT + i, 
  /// but we don't rely on this assumption
  GLuint fbo_buffers[FBO_BUFFERS_COUNT];

  /** Textures bound to the framebuffer object; 
   * the ping-pong rendering switches between pairs 0-2 and 1-3
   * because this can be easily generalized to triples 0-2-4 and 1-3-5 etc.
   * (pairs 0-1 and 2-3 would have to be rearranged to 0-1-2 and 3-4-5).
   * use getTextureOfBuffer to find the texture name of a buffer
   **/
  GLuint fbo_textures[FBO_BUFFERS_COUNT];
  
  /// Number of pyramid levels.
  int levels_count;

  /// Current rasterize level
  int cur_level;

  /// Type of most recently used framebuffer.
  framebuffer_state_enum framebuffer_state; 
  
  //// Current rendering mode.
  render_state_enum render_state;

  list<Surfel> *surfels;
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
