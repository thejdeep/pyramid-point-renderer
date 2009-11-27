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

	mShaderProjection.LoadSources("pyramid_point_renderer_elipse/shader_point_projection.vert", "pyramid_point_renderer/shader_point_projection.frag");
	link = mShaderProjection.prog.Link();
	compileinfo = mShaderProjection.vshd.InfoLog();
	std::cout << "Projection Vertex shader info : " << compileinfo << "\n";
	compileinfo = mShaderProjection.fshd.InfoLog();
	std::cout << "Projection Fragment shader info : " << compileinfo << "\n";
	assert (link == 1);

	mShaderAnalysis.LoadSources("pyramid_point_renderer_elipse/shader_analysis.vert", "pyramid_point_renderer/shader_analysis.frag");
	link =  mShaderAnalysis.prog.Link();
	compileinfo = mShaderAnalysis.vshd.InfoLog();
	std::cout << "Analysis Vertex shader info : " << compileinfo << "\n";
	compileinfo = mShaderAnalysis.fshd.InfoLog();
	std::cout << "Analysis Fragment shader info : " << compileinfo << "\n";
	assert (link == 1);

	mShaderCopy.LoadSources("pyramid_point_renderer_elipse/shader_copy.vert", "pyramid_point_renderer/shader_copy.frag");
	link = mShaderCopy.prog.Link();
	compileinfo = mShaderCopy.vshd.InfoLog();
	std::cout << "Copy Vertex shader info : " << compileinfo << "\n";
	compileinfo = mShaderCopy.fshd.InfoLog();
	std::cout << "Copy Fragment shader info : " << compileinfo << "\n";
	assert (link == 1);

	mShaderSynthesis.LoadSources("pyramid_point_renderer_elipse/shader_synthesis.vert", "pyramid_point_renderer/shader_synthesis.frag");
	link = mShaderSynthesis.prog.Link();
	compileinfo = mShaderSynthesis.vshd.InfoLog();
	std::cout << "Synthesis Vertex shader info : " << compileinfo << "\n";
	compileinfo = mShaderSynthesis.fshd.InfoLog();
	std::cout << "Synthesis Fragment shader info : " << compileinfo << "\n";
	assert (link == 1);

	mShaderPhong.LoadSources("pyramid_point_renderer_elipse/shader_phong.vert", "pyramid_point_renderer/shader_phong.frag");
	link = mShaderPhong.prog.Link();
	compileinfo = mShaderPhong.vshd.InfoLog();
	std::cout << "Phong Vertex shader info : " << compileinfo << "\n";
	compileinfo = mShaderPhong.fshd.InfoLog();
	std::cout << "Phong Fragment shader info : " << compileinfo << "\n";
	assert (link == 1);

}
