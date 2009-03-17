/*
** pyramid_point_render.cc Pyramid Point Based Rendering.
**
**
**   history:	created  02-Jul-07
*/

#include "pyramid_point_renderer.h"

/**
 * Default constructor.
 **/
PyramidPointRenderer::PyramidPointRenderer(int w, int h) : PyramidPointRendererBase(w, h, 4) {
}

/**
 * Installs the shaders using the GLSL Kernel class.
 **/
void PyramidPointRenderer::createShaders ( void ) {

  // Store texture names to be passed as uniforms
  shader_texture_names = new string[fbo_buffers_count/2];
  shader_texture_names[0] = "textureA";
  shader_texture_names[1] = "textureB";

  mShaderProjection.LoadSources("pyramid_point_renderer/shader_point_projection.vert", "pyramid_point_renderer/shader_point_projection.frag");
  mShaderProjection.prog.Link();

  mShaderAnalysis.LoadSources("pyramid_point_renderer/shader_analysis.vert", "pyramid_point_renderer/shader_analysis.frag");
  mShaderAnalysis.prog.Link();

  mShaderCopy.LoadSources("pyramid_point_renderer/shader_copy.vert", "pyramid_point_renderer/shader_copy.frag");
  mShaderCopy.prog.Link();

  mShaderSynthesis.LoadSources("pyramid_point_renderer/shader_synthesis.vert", "pyramid_point_renderer/shader_synthesis.frag");
  mShaderSynthesis.prog.Link();

  mShaderPhong.LoadSources("pyramid_point_renderer/shader_phong.vert", "pyramid_point_renderer/shader_phong.frag");
  mShaderPhong.prog.Link();

}
