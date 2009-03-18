/*
** pyramid_point_renderer_base.h Pyramid Point Based Rendering Base header.
**
**
**   history:	created  02-Jul-07
*/


#ifndef __PYRAMID_POINT_RENDERER_BASE_H__
#define __PYRAMID_POINT_RENDERER_BASE_H__

#include <cmath>
#include <cassert>

#include "point_based_renderer.h"

/**
 * Pyramid point renderer algorithm as described in: <br>
 * Efficient Point-Based Rendering Using Image Reconstruction. <br>
 * Ricardo Marroquim, Martin Kraus, Paulo Roma Cavalcanti <br>
 * IEEE/Eurographics Symposium on Point-Based Graphics (PBG), Sep-2007
 **/
class PyramidPointRendererBase : public PointBasedRenderer
{
 private:

  virtual const int analysisCallbackFunc( void ) ;
  virtual const int projectionCallbackFunc( void ) ;
  virtual const int synthesisCallbackFunc( void ) ;
  virtual const int phongShadingCallbackFunc( void ) ;
  virtual void rasterizeAnalysisPyramid( void );
  virtual void rasterizeSynthesisPyramid( void );
  virtual void rasterizePhongShading(int bufferIndex);

 protected:

  void createFBO( void );
  void copyAnalysisPyramid();
  const int copyCallbackFunc( void ) ;
  void projectSurfels( const Object * const );

  const pixels_struct generatePixels(const int level, const GLuint fbo, const int buffersCount, const GLuint* buffers) const;
  const void rasterizePixels(const pixels_struct dest, const pixels_struct src0, const pixels_struct src1, const int phase);

  const GLuint getTextureOfBuffer(const GLuint buffer) const;

  const double computeHalfPixelSize( void ) const;

  void resetPointers ( void ) {   
	fbo_buffers = NULL;
	fbo_textures = NULL;
	shader_texture_names = NULL;
  }

 public:
  PyramidPointRendererBase();
  PyramidPointRendererBase(int w, int h);
  PyramidPointRendererBase(int w, int h, int fbos);
  ~PyramidPointRendererBase();
  
  void init ( void );

  virtual void createShaders ( void ) = 0;

  void draw();
  void clearBuffers (void);
  void projectSamples (Object* const obj );
  void interpolate ( void );

 protected:
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

  /// Shaders using VCG lib
  ProgramVF mShaderProjection;
  ProgramVF mShaderCopy;
  ProgramVF mShaderAnalysis;
  ProgramVF mShaderSynthesis;
  ProgramVF mShaderPhong;

/*   /// Projection shader. */
/*   glslKernel *shader_projection; */
/*   /// Pyramid copy phase shader. */
/*   glslKernel *shader_copy; */
/*   /// Pyramid analysis phase shader. */
/*   glslKernel *shader_analysis; */
/*   /// Pyramid synthesis phase shader. */
/*   glslKernel *shader_synthesis; */
/*   /// Phong shading shader. */
/*   glslKernel *shader_phong; */

  /// Textures names to pass as uniform to shaders
  string *shader_texture_names;

  /// The application-created framebuffer object.
  GLuint fbo;

  /// Framebuffer for depth test.
  GLuint fbo_depth;

  /// usually fboBuffers[i] == GL_COLOR_ATTACHMENT0_EXT + i, 
  /// but we don't rely on this assumption
  GLuint *fbo_buffers;

  /** Textures bound to the framebuffer object; 
   * the ping-pong rendering switches between pairs 0-2 and 1-3
   * because this can be easily generalized to triples 0-2-4 and 1-3-5 etc.
   * (pairs 0-1 and 2-3 would have to be rearranged to 0-1-2 and 3-4-5).
   * use getTextureOfBuffer to find the texture name of a buffer
   **/
  GLuint *fbo_textures;
  
  /// Number of pyramid levels.
  int levels_count;

  /// Current rasterize level
  int cur_level;

  /// Type of most recently used framebuffer.
  framebuffer_state_enum framebuffer_state; 
  
  /// Current rendering mode.
  render_state_enum render_state;

};

#endif
