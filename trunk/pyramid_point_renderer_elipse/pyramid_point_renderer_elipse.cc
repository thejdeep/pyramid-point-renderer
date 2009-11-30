/*
** pyramid_point_render.cc Pyramid Point Based Rendering.
**
**
**   history:	created  20-Nov-09
*/

#include "pyramid_point_renderer_elipse.h"

/**
 * Default constructor.
 **/
PyramidPointRendererElipse::PyramidPointRendererElipse(int w, int h) : PyramidPointRendererBase(w, h, 6) {
}

/* rasterize level 0 of pyramid with per pixel shading */

const int PyramidPointRendererElipse::phongShadingCallbackFunc( void )
{
	mShaderPhong.prog.Bind();

	mShaderPhong.prog.Uniform("textureB", 0);
	mShaderPhong.prog.Uniform("textureC", 1);

	mShaderPhong.prog.Uniform("color_ambient", Mats[material_id][0], Mats[material_id][1], Mats[material_id][2], Mats[material_id][3]);
	mShaderPhong.prog.Uniform("color_diffuse", Mats[material_id][4], Mats[material_id][5], Mats[material_id][6], Mats[material_id][7]);
	mShaderPhong.prog.Uniform("color_specular", Mats[material_id][8], Mats[material_id][9], Mats[material_id][10], Mats[material_id][11]);
	mShaderPhong.prog.Uniform("shininess", Mats[material_id][12]);

	return false; /* not done, rasterize quad */
}

void PyramidPointRendererElipse::rasterizePhongShading(int bufferIndex)
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

  GLuint buffers[2] = {fbo_buffers[2], fbo_buffers[4]};
  sourcePixels = generatePixels(0, fbo, 2, &buffers[0]);

  buffers[0] = GL_BACK;
  destinationPixels = generatePixels(0, 0, 1, &buffers[0]);
  rasterizePixels(destinationPixels, sourcePixels, nullPixels, PHONG);

  mShaderPhong.prog.Unbind();
}


/**
 * Installs the shaders using the GLSL Kernel class.
 **/
void PyramidPointRendererElipse::createShaders ( void ) {

	// Store texture names to be passed as uniforms
	shader_texture_names = new string[fbo_buffers_count/2];
	shader_texture_names[0] = "textureA";
	shader_texture_names[1] = "textureB";
	shader_texture_names[2] = "textureC";

	bool link;
	std::string compileinfo;

	mShaderProjection.LoadSources("pyramid_point_renderer_elipse/shader_point_projection.vert", "pyramid_point_renderer_elipse/shader_point_projection.frag");
	link = mShaderProjection.prog.Link();
	compileinfo = mShaderProjection.vshd.InfoLog();
	std::cout << "Projection Vertex shader info : " << compileinfo << "\n";
	compileinfo = mShaderProjection.fshd.InfoLog();
	std::cout << "Projection Fragment shader info : " << compileinfo << "\n";
	assert (link == 1);

	mShaderAnalysis.LoadSources("pyramid_point_renderer_elipse/shader_analysis.vert", "pyramid_point_renderer_elipse/shader_analysis.frag");
	link =  mShaderAnalysis.prog.Link();
	compileinfo = mShaderAnalysis.vshd.InfoLog();
	std::cout << "Analysis Vertex shader info : " << compileinfo << "\n";
	compileinfo = mShaderAnalysis.fshd.InfoLog();
	std::cout << "Analysis Fragment shader info : " << compileinfo << "\n";
	assert (link == 1);

	mShaderCopy.LoadSources("pyramid_point_renderer_elipse/shader_copy.vert", "pyramid_point_renderer_elipse/shader_copy.frag");
	link = mShaderCopy.prog.Link();
	compileinfo = mShaderCopy.vshd.InfoLog();
	std::cout << "Copy Vertex shader info : " << compileinfo << "\n";
	compileinfo = mShaderCopy.fshd.InfoLog();
	std::cout << "Copy Fragment shader info : " << compileinfo << "\n";
	assert (link == 1);

	mShaderSynthesis.LoadSources("pyramid_point_renderer_elipse/shader_synthesis.vert", "pyramid_point_renderer_elipse/shader_synthesis.frag");
	link = mShaderSynthesis.prog.Link();
	compileinfo = mShaderSynthesis.vshd.InfoLog();
	std::cout << "Synthesis Vertex shader info : " << compileinfo << "\n";
	compileinfo = mShaderSynthesis.fshd.InfoLog();
	std::cout << "Synthesis Fragment shader info : " << compileinfo << "\n";
	assert (link == 1);

	mShaderPhong.LoadSources("pyramid_point_renderer_elipse/shader_phong.vert", "pyramid_point_renderer_elipse/shader_phong.frag");
	link = mShaderPhong.prog.Link();
	compileinfo = mShaderPhong.vshd.InfoLog();
	std::cout << "Phong Vertex shader info : " << compileinfo << "\n";
	compileinfo = mShaderPhong.fshd.InfoLog();
	std::cout << "Phong Fragment shader info : " << compileinfo << "\n";
	assert (link == 1);

}
