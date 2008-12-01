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

  // Debugging output flag
  bool shader_inst_debug = 0;

  // Store texture names to be passed as uniforms
  shader_texture_names = new string[fbo_buffers_count/2];
  shader_texture_names[0] = "textureA";
  shader_texture_names[1] = "textureB";

  shader_projection = new glslKernel();
  shader_projection->vertex_source("pyramid_point_renderer/shader_point_projection.vert");
  shader_projection->fragment_source("pyramid_point_renderer/shader_point_projection.frag");
  shader_projection->install( shader_inst_debug );

  shader_analysis = new glslKernel();
  shader_analysis->vertex_source("pyramid_point_renderer/shader_analysis.vert");
  shader_analysis->fragment_source("pyramid_point_renderer/shader_analysis.frag");
  shader_analysis->install( shader_inst_debug );

  shader_copy = new glslKernel();
  shader_copy->vertex_source("pyramid_point_renderer/shader_copy.vert");  
  shader_copy->fragment_source("pyramid_point_renderer/shader_copy.frag");
  shader_copy->install( shader_inst_debug );

  shader_synthesis = new glslKernel();
  shader_synthesis->vertex_source("pyramid_point_renderer/shader_synthesis.vert");
  shader_synthesis->fragment_source("pyramid_point_renderer/shader_synthesis.frag");
  shader_synthesis->install( shader_inst_debug );

  shader_phong = new glslKernel();
  shader_phong->vertex_source("pyramid_point_renderer/shader_phong.vert");
  shader_phong->fragment_source("pyramid_point_renderer/shader_phong.frag");
  shader_phong->install( shader_inst_debug );
}
