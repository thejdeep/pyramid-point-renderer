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
PyramidPointRendererColor::PyramidPointRendererColor(int w, int h) : PyramidPointRenderer(w, h, 6) {
  createShaders();
}

void PyramidPointRendererColor::createShaders ( void ) {

  bool shader_inst_debug = 0;

  shader_texture_names = new string[fbo_buffers_count/2];
  shader_texture_names[0] = "textureA";
  shader_texture_names[1] = "textureB";
  shader_texture_names[2] = "textureC";

  shader_projection = new glslKernel();
  shader_projection->vertex_source("pyramid_point_renderer_color/shader_point_projection_color.vert");
  shader_projection->fragment_source("pyramid_point_renderer_color/shader_point_projection_color.frag");
  shader_projection->install( shader_inst_debug );
 
  shader_analysis = new glslKernel();
  shader_analysis->vertex_source("pyramid_point_renderer_color/shader_analysis_color.vert");
  shader_analysis->fragment_source("pyramid_point_renderer_color/shader_analysis_color.frag");
  shader_analysis->install( shader_inst_debug );

  shader_copy = new glslKernel();
  shader_copy->vertex_source("pyramid_point_renderer_color/shader_copy_color.vert");
  shader_copy->fragment_source("pyramid_point_renderer_color/shader_copy_color.frag");
  shader_copy->install( shader_inst_debug );

  shader_synthesis = new glslKernel();
  shader_synthesis->vertex_source("pyramid_point_renderer_color/shader_synthesis_color.vert");
  shader_synthesis->fragment_source("pyramid_point_renderer_color/shader_synthesis_color.frag");
  shader_synthesis->install( shader_inst_debug );

  shader_phong = new glslKernel();
  shader_phong->vertex_source("pyramid_point_renderer_color/shader_phong_color.vert");
  shader_phong->fragment_source("pyramid_point_renderer_color/shader_phong_color.frag");
  shader_phong->install( shader_inst_debug );

}
