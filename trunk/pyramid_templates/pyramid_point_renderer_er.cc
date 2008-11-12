/*
** pyramid_point_renderer_er.cc Pyramid Point Based Rendering with Ellipse Rasterization.
**
**
**   history:	created  24-Apr-08
*/

#include "pyramid_point_renderer_er.h"


PyramidPointRendererER::PyramidPointRendererER(int w, int h) : PyramidPointRenderer(w, h, 6),
							       gpu_mask_size(1) {
  createShaders();
}

const int PyramidPointRendererER::projectionCallbackFunc( void ) const {
  shader_projection->use();
  shader_projection->set_uniform("eye", (GLfloat)eye[0], (GLfloat)eye[1], (GLfloat)eye[2]);
  shader_projection->set_uniform("back_face_culling", (GLint)back_face_culling);

  shader_projection->set_uniform("oo_fbo_size", (GLfloat)(1.0/(GLfloat)fbo_width), (GLfloat)(1.0/(GLfloat)fbo_height));

  shader_projection->set_uniform("min_size", (GLfloat) (((gpu_mask_size*2.0)+1.0) / (2.0 * canvas_width)));

  return true;
}

const int PyramidPointRendererER::analysisCallbackFunc( void ) const {
  shader_analysis->use();
  shader_analysis->set_uniform("oo_2fbo_size", (GLfloat)(0.5 / (GLfloat)fbo_width), (GLfloat)(0.5 / (GLfloat)fbo_height));

  shader_analysis->set_uniform("level", cur_level);
  shader_analysis->set_uniform("canvas_width", (GLfloat)canvas_width);

  shader_analysis->set_uniform("reconstruction_filter_size", (GLfloat)(reconstruction_filter_size));
 
  shader_analysis->set_uniform("depth_test", depth_test);
  shader_analysis->set_uniform("mask_size", gpu_mask_size);

  // Loads the textures ids as uniforms for shader access
  for (int i = 0; i < fbo_buffers_count/2; ++i)
    shader_analysis->set_uniform(shader_texture_names[i].c_str(), i);

  return false; /* not done, rasterize quad */
}


const int PyramidPointRendererER::synthesisCallbackFunc( void ) const {
  shader_synthesis->use();
  shader_synthesis->set_uniform("fbo_size", (GLfloat)fbo_width, (GLfloat)fbo_height);
  shader_synthesis->set_uniform("oo_fbo_size", (GLfloat)(1.0/(GLfloat)fbo_width), (GLfloat)(1.0/(GLfloat)fbo_height));
  shader_synthesis->set_uniform("oo_canvas_size", 1.0/(GLfloat)canvas_width, 1.0/(GLfloat)canvas_height);

  shader_synthesis->set_uniform("prefilter_size", (GLfloat)(prefilter_size / (GLfloat)(canvas_width)));
  shader_synthesis->set_uniform("reconstruction_filter_size", (GLfloat)(reconstruction_filter_size));

  shader_synthesis->set_uniform("elliptical_weight", elliptical_weight);
  shader_synthesis->set_uniform("level", cur_level);

  shader_synthesis->set_uniform("mask_size", gpu_mask_size);
  shader_synthesis->set_uniform("depth_test", depth_test);  



  // Loads the textures ids as uniforms for shader access
  for (int i = 0; i < fbo_buffers_count/2; ++i)
    shader_synthesis->set_uniform(shader_texture_names[i].c_str(), i);

  return false; /* not done, rasterize quad */
}

void PyramidPointRendererER::rasterizeSynthesisPyramid( void )
/* using ping-pong rasterization between color attachment pairs 0-2 and 1-3 */
{
  int level;
  pixels_struct source0Pixels; /* same level as destination */
  pixels_struct source1Pixels; /* coarser level than destination */
  pixels_struct destinationPixels;

  GLuint buffers[fbo_buffers_count/2];

  //  for (level = levels_count - 2; level >= 0; level--)
  //  for (level = 0; level <= 10; level++)
  for (level = 0; level < levels_count; level++) {
    cur_level = level;
    
    for (int i = 0; i < fbo_buffers_count/2; ++i)
      buffers[i] = fbo_buffers[i*2 + ((level+1) % 2)];   
    source0Pixels = generatePixels(0, fbo, fbo_buffers_count/2, &buffers[0]);
    source1Pixels = generatePixels(level, fbo, fbo_buffers_count/2, &buffers[0]);

    for (int i = 0; i < (fbo_buffers_count/2) - 1; ++i)
      buffers[i] = fbo_buffers[i*2 + ((level) % 2)];
    destinationPixels = generatePixels(0, fbo, (fbo_buffers_count/2) - 1, &buffers[0]);
    
    rasterizePixels(destinationPixels, source0Pixels, source1Pixels, SYNTHESIS);

    shader_synthesis->use(0);
  }
}

/* rasterize level 0 of pyramid with per pixel shading */

const int PyramidPointRendererER::phongShadingCallbackFunc( void ) const
{
  shader_phong->use();
  shader_phong->set_uniform("textureA", 0);
  //  shader_phong->set_uniform("textureB", 1);

  shader_phong->set_uniform("color_ambient", Mats[material_id][0], Mats[material_id][1], Mats[material_id][2], Mats[material_id][3]);
  shader_phong->set_uniform("color_diffuse", Mats[material_id][4], Mats[material_id][5], Mats[material_id][6], Mats[material_id][7]);
  shader_phong->set_uniform("color_specular", Mats[material_id][8], Mats[material_id][9], Mats[material_id][10], Mats[material_id][11]);
  shader_phong->set_uniform("shininess", Mats[material_id][12]);

  return false; /* not done, rasterize quad */
}

void PyramidPointRendererER::rasterizePhongShading(int bufferIndex)
     /* using ping-pong rasterization between color attachment pairs 0-2 and 1-3 */
{
  pixels_struct nullPixels;
  pixels_struct sourcePixels;
  pixels_struct destinationPixels;

  nullPixels = generatePixels(0, 0, 0, 0);

  shader_projection->use(0);
  shader_analysis->use(0);
  shader_copy->use(0);
  shader_synthesis->use(0);

//   GLuint buffers[(fbo_buffers_count/2) - 1];

//   for (int i = 0; i < (fbo_buffers_count/2)-1; ++i)
//     buffers[i] = fbo_buffers[bufferIndex + i*2];
//   sourcePixels = generatePixels(level, fbo, (fbo_buffers_count/2) - 1, &buffers[0]);

  GLuint buffers[1] = {fbo_buffers[0]};
  sourcePixels = generatePixels(0, fbo, 1, &buffers[0]);

  buffers[0] = GL_BACK;
  destinationPixels = generatePixels(0, 0, 1, &buffers[0]);
  rasterizePixels(destinationPixels, sourcePixels, nullPixels, PHONG);

  shader_phong->use(0);
}


/**
 * Installs the shaders using the GLSL Kernel class.
 **/
void PyramidPointRendererER::createShaders ( void ) {

  bool shader_inst_debug = 0;

  shader_texture_names = new string[fbo_buffers_count/2];
  shader_texture_names[0] = "textureA";
  shader_texture_names[1] = "textureB";
  shader_texture_names[2] = "textureC";

  shader_projection = new glslKernel();
  shader_projection->vertex_source("pyramid_templates/shader_point_projection_color_er.vert");                                    
  shader_projection->fragment_source("pyramid_templates/shader_point_projection_color_er.frag");
  shader_projection->install( shader_inst_debug );

  shader_analysis = new glslKernel();
  shader_analysis->vertex_source("pyramid_templates/shader_analysis_er.vert");
  shader_analysis->fragment_source("pyramid_templates/shader_analysis_er.frag");
  shader_analysis->install( shader_inst_debug );

  shader_copy = new glslKernel();
  shader_copy->vertex_source("pyramid_templates/shader_copy_er.vert");
  shader_copy->fragment_source("pyramid_templates/shader_copy_er.frag");
  shader_copy->install( shader_inst_debug );

  shader_synthesis = new glslKernel();
  shader_synthesis->vertex_source("pyramid_templates/shader_synthesis_er.vert");
  shader_synthesis->fragment_source("pyramid_templates/shader_synthesis_er.frag");
  shader_synthesis->install( shader_inst_debug );

  shader_phong = new glslKernel();
  shader_phong->vertex_source("pyramid_templates/shader_phong_er.vert");
  shader_phong->fragment_source("pyramid_templates/shader_phong_er.frag");
  shader_phong->install( shader_inst_debug );

}
