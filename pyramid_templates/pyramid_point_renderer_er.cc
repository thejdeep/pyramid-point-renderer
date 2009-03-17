/*
** pyramid_point_renderer_er.cc Pyramid Point Based Rendering with Ellipse Rasterization.
**
**
**   history:	created  24-Apr-08
*/

#include "pyramid_point_renderer_er.h"


PyramidPointRendererER::PyramidPointRendererER(int w, int h) : PyramidPointRendererBase(w, h, 6),
															   gpu_mask_size(1) {
}

const int PyramidPointRendererER::projectionCallbackFunc( void )  {
  mShaderProjection.prog.Bind();
  mShaderProjection.prog.Uniform("eye", (GLfloat)eye[0], (GLfloat)eye[1], (GLfloat)eye[2]);
  mShaderProjection.prog.Uniform("back_face_culling", (GLint)back_face_culling);
  mShaderProjection.prog.Uniform("oo_fbo_size", (GLfloat)(1.0/(GLfloat)fbo_width), (GLfloat)(1.0/(GLfloat)fbo_height));
  mShaderProjection.prog.Uniform("scale", (GLfloat)scale_factor);
  mShaderProjection.prog.Uniform("canvas_width", (GLfloat)canvas_width);
  mShaderProjection.prog.Uniform("reconstruction_filter_size", (GLfloat)(reconstruction_filter_size));
  mShaderProjection.prog.Uniform("mask_size", gpu_mask_size);


  return true;
}

const int PyramidPointRendererER::analysisCallbackFunc( void )  {

  mShaderAnalysis.prog.Bind();
  mShaderAnalysis.prog.Uniform("oo_2fbo_size", (GLfloat)(0.5 / (GLfloat)fbo_width), (GLfloat)(0.5 / (GLfloat)fbo_height));
  mShaderAnalysis.prog.Uniform("level", (GLint)cur_level);
  mShaderAnalysis.prog.Uniform("canvas_width", (GLfloat)canvas_width);
  mShaderAnalysis.prog.Uniform("reconstruction_filter_size", (GLfloat)(reconstruction_filter_size));
  mShaderAnalysis.prog.Uniform("depth_test", (bool)depth_test);
  mShaderAnalysis.prog.Uniform("mask_size", (GLint)gpu_mask_size);
  mShaderAnalysis.prog.Uniform("quality_threshold", (GLfloat)quality_threshold);
  mShaderAnalysis.prog.Uniform("quality_per_vertex", (bool)quality_per_vertex);

  // Loads the textures ids as uniforms for shader access
  for (int i = 0; i < fbo_buffers_count/2; ++i)
     mShaderAnalysis.prog.Uniform(shader_texture_names[i].c_str(), i);


  return false; /* not done, rasterize quad */
}


const int PyramidPointRendererER::synthesisCallbackFunc( void )  {

  mShaderSynthesis.prog.Bind();

  mShaderSynthesis.prog.Uniform("fbo_size", (GLfloat)fbo_width, (GLfloat)fbo_height);
  mShaderSynthesis.prog.Uniform("oo_fbo_size", (GLfloat)(1.0/fbo_width), (GLfloat)(1.0/fbo_height));
  mShaderSynthesis.prog.Uniform("oo_canvas_size", (GLfloat)1.0/(GLfloat)canvas_width, (GLfloat)1.0/(GLfloat)canvas_height);
  mShaderSynthesis.prog.Uniform("prefilter_size", (GLfloat)(prefilter_size / (GLfloat)(canvas_width)));
  mShaderSynthesis.prog.Uniform("reconstruction_filter_size", (GLfloat)(reconstruction_filter_size));
  mShaderSynthesis.prog.Uniform("depth_test", (bool)depth_test);
  mShaderSynthesis.prog.Uniform("elliptical_weight", (bool)elliptical_weight);
  mShaderSynthesis.prog.Uniform("level", (GLint)cur_level);
  mShaderSynthesis.prog.Uniform("mask_size", (GLint)gpu_mask_size);
  if (quality_per_vertex)
	mShaderSynthesis.prog.Uniform("quality_threshold", (GLfloat)quality_threshold);
  else
	mShaderSynthesis.prog.Uniform("quality_threshold", (GLfloat)1.0);

  for (int i = 0; i < fbo_buffers_count/2; ++i)
	mShaderSynthesis.prog.Uniform(shader_texture_names[i].c_str(), i);


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

	mShaderSynthesis.prog.Unbind();
  }
}

/* rasterize level 0 of pyramid with per pixel shading */

const int PyramidPointRendererER::phongShadingCallbackFunc( void ) 
{
  mShaderPhong.prog.Bind();

  mShaderPhong.prog.Uniform("textureA", 0);
  mShaderPhong.prog.Uniform("textureB", 1);

  mShaderPhong.prog.Uniform("color_ambient", Mats[material_id][0], Mats[material_id][1], Mats[material_id][2], Mats[material_id][3]);
  mShaderPhong.prog.Uniform("color_diffuse", Mats[material_id][4], Mats[material_id][5], Mats[material_id][6], Mats[material_id][7]);
  mShaderPhong.prog.Uniform("color_specular", Mats[material_id][8], Mats[material_id][9], Mats[material_id][10], Mats[material_id][11]);
  mShaderPhong.prog.Uniform("shininess", Mats[material_id][12]);


  return false; /* not done, rasterize quad */
}

void PyramidPointRendererER::rasterizePhongShading(int bufferIndex)
     /* using ping-pong rasterization between color attachment pairs 0-2 and 1-3 */
{
  pixels_struct nullPixels;
  pixels_struct sourcePixels;
  pixels_struct destinationPixels;

  nullPixels = generatePixels(0, 0, 0, 0);

  mShaderProjection.prog.Unbind();
  mShaderCopy.prog.Unbind();
  mShaderAnalysis.prog.Unbind();
  mShaderSynthesis.prog.Unbind();
  mShaderPhong.prog.Unbind();

  GLuint buffers[2] = {fbo_buffers[0], fbo_buffers[2]};
  sourcePixels = generatePixels(0, fbo, 2, &buffers[0]);

  buffers[0] = GL_BACK;
  destinationPixels = generatePixels(0, 0, 1, &buffers[0]);
  rasterizePixels(destinationPixels, sourcePixels, nullPixels, PHONG);

  mShaderPhong.prog.Unbind();
}


/**
 * Installs the shaders using the GLSL Kernel class.
 **/
void PyramidPointRendererER::createShaders ( void ) {
  
  shader_texture_names = new string[fbo_buffers_count/2];
  shader_texture_names[0] = "textureA";
  shader_texture_names[1] = "textureB";
  shader_texture_names[2] = "textureC";
  
  mShaderProjection.LoadSources("pyramid_templates/shader_point_projection_color_er.vert", "pyramid_templates/shader_point_projection_color_er.frag");
  mShaderProjection.prog.Link();

  mShaderAnalysis.LoadSources("pyramid_templates/shader_analysis_er.vert", "pyramid_templates/shader_analysis_er.frag");
  mShaderAnalysis.prog.Link();

  mShaderCopy.LoadSources("pyramid_templates/shader_copy_er.vert", "pyramid_templates/shader_copy_er.frag");
  mShaderCopy.prog.Link();

  mShaderSynthesis.LoadSources("pyramid_templates/shader_synthesis_er.vert", "pyramid_templates/shader_synthesis_er.frag");
  mShaderSynthesis.prog.Link();

  mShaderPhong.LoadSources("pyramid_templates/shader_phong_er.vert", "pyramid_templates/shader_phong_er.frag");
  mShaderPhong.prog.Link();
}
