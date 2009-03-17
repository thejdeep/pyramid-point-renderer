/*
** pyramid_point_render.cc Pyramid Point Based Rendering.
**
**
**   history:	created  02-Jul-07
*/

#include "pyramid_point_renderer_color.h"

/**
 * Default constructor.
 **/
PyramidPointRendererColor::PyramidPointRendererColor(int w, int h) : PyramidPointRendererBase(w, h, 6) {
}

void PyramidPointRendererColor::createShaders ( void ) {

  // Store texture names to be passed as uniforms
  shader_texture_names = new string[fbo_buffers_count/2];
  shader_texture_names[0] = "textureA";
  shader_texture_names[1] = "textureB";
  shader_texture_names[2] = "textureC";

  mShaderProjection.LoadSources("pyramid_point_renderer_color/shader_point_projection_color.vert", "pyramid_point_renderer_color/shader_point_projection_color.frag");
  mShaderProjection.prog.Link();

  mShaderAnalysis.LoadSources("pyramid_point_renderer_color/shader_analysis_color.vert", "pyramid_point_renderer_color/shader_analysis_color.frag");
  mShaderAnalysis.prog.Link();

  mShaderCopy.LoadSources("pyramid_point_renderer_color/shader_copy_color.vert", "pyramid_point_renderer_color/shader_copy_color.frag");
  mShaderCopy.prog.Link();

  mShaderSynthesis.LoadSources("pyramid_point_renderer_color/shader_synthesis_color.vert", "pyramid_point_renderer_color/shader_synthesis_color.frag");
  mShaderSynthesis.prog.Link();

  mShaderPhong.LoadSources("pyramid_point_renderer_color/shader_phong_color.vert", "pyramid_point_renderer_color/shader_phong_color.frag");
  mShaderPhong.prog.Link();

}
