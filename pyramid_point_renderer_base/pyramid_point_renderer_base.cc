/*
** pyramid_point_render.cc Pyramid Point Based Rendering.
**
**
**   history:	created  02-Jul-07
*/

#include "pyramid_point_renderer_base.h"

void PyramidPointRendererBase::init ( void ) {
  resetPointers();
  createFBO();
  levels_count = max((int)(log(canvas_width)/log(2.0)), (int)(log(canvas_height)/log(2.0)));
}

/**
 * Default constructor.
 **/
PyramidPointRendererBase::PyramidPointRendererBase() : PointBasedRenderer(),
					   fbo_width(1800),
					   fbo_height(1200),
					   fbo_buffers_count(4),
					   canvas_border_width(32),
					   canvas_border_height(32),
					   render_state(RS_BUFFER0) {
  init();
}

PyramidPointRendererBase::PyramidPointRendererBase(int w, int h) : PointBasedRenderer(w, h),
					   fbo_width(1800),
					   fbo_height(1200),
					   fbo_buffers_count(4),
					   canvas_border_width(w/32),
					   canvas_border_height(h/32),
					   render_state(RS_BUFFER0) {
  init();
}

PyramidPointRendererBase::PyramidPointRendererBase(int w, int h, int fbos) : PointBasedRenderer(w, h),
					   fbo_width(1800),
					   fbo_height(1200),
					   fbo_buffers_count(fbos),
					   canvas_border_width(w/32),
					   canvas_border_height(h/32),
					   render_state(RS_BUFFER0) {
  init();
}

PyramidPointRendererBase::~PyramidPointRendererBase()  {

  
  for (int i = 0; i < fbo_buffers_count; ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(FBO_TYPE, 0);
    glDisable(FBO_TYPE);
  }
  glDeleteTextures(fbo_buffers_count, fbo_textures);


  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);

  glDeleteTextures(1, &fbo_depth);

  glDeleteFramebuffersEXT(1, &fbo);

  delete [] fbo_buffers;
  delete [] fbo_textures;

  delete [] shader_texture_names;
}

/** 
 * Returns the OpenGL texture name of a color attachment buffer (GL_COLOR_ATTACHMENTx_EXT) 
 * of the global framebuffer object or 0 if the color attachment buffer is not bound
 * @param buffer Given buffer to associate texture number.
 * @return Texture id associted to given buffer.
 **/
const GLuint PyramidPointRendererBase::getTextureOfBuffer(const GLuint buffer) const

{
  int i;

  for (i = 0; i < fbo_buffers_count; i++)
    if (fbo_buffers[i] == buffer)
	  break;
  if (i < fbo_buffers_count)
    return fbo_textures[i];
  else
    return (GLuint)0;
}

/** Binds buffers and rasterizes quad;
 * Calls callbackfunc to set the fragment program 
 * @param dest Given buffer to be written to.
 * @param src0 First reading buffer.
 * @param src1 Second reading buffer. (In the case of the synthesis phase for example, must
 * read two levels at the same time. Both src specify the same texture but at different locations
 * associated to the correct level)
 * @param Phase The current phase of the algorithm (Projection, Analysis, Copy, Synthesis, Phong).
 **/
const void PyramidPointRendererBase::rasterizePixels(const pixels_struct dest, const pixels_struct src0,
						 const pixels_struct src1, const int phase)
     
{
  /* bind framebuffer and renderbuffers */

  /// Render to GL buffer (GL_BACK or GL_FRONT) if there is no associated destination buffer.
  if (0 == dest.fbo)
    {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
      glDrawBuffer(dest.buffers[0]);

      /// If camera properties are not already set (case of Projection),
      /// set it to match screen size.
      if (framebuffer_state != FBS_SYSTEM_PROVIDED)
	{
	  glViewport(0, 0, canvas_width + 2*canvas_border_width, 
		     canvas_height + 2*canvas_border_height);
	  glMatrixMode(GL_PROJECTION);
	  glLoadIdentity();
	  gluOrtho2D(0.0, canvas_width + 2*canvas_border_width, 
		     0.0, canvas_height + 2*canvas_border_height);

	  glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity();
	  framebuffer_state = FBS_SYSTEM_PROVIDED;
	}
    }
  else
    {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, dest.fbo);
      glDrawBuffers(dest.buffersCount, dest.buffers);

      /// If camera properties are not defined, set it to match framebuffer space
      /// associated to current processing level of the pyramid.
      if (framebuffer_state != FBS_APPLICATION_CREATED) 
	{
	  glViewport(0, 0, fbo_width, fbo_height);
	  glMatrixMode(GL_PROJECTION);
	  glLoadIdentity();
	  gluOrtho2D(0.0, fbo_width, 0.0, fbo_height);
	  glMatrixMode(GL_MODELVIEW);
	  glLoadIdentity();
	  framebuffer_state = FBS_APPLICATION_CREATED;
	}
    }

  /* Bind textures: src0 is bound as 0th and 1st texture, src1 as 2nd and 3rd texture (in the case of two buffers) */
  /* For more buffers src0 is associated with the first buffers_num/2 and src1 with the remaining buffers_num/2 */
  for (int i = 0; i < fbo_buffers_count / 2; ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    if (src0.buffersCount > i) {	
      glBindTexture(FBO_TYPE, src0.textures[i]);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    }
    else 
      glBindTexture(FBO_TYPE, (GLuint)0);
    
    glActiveTexture(GL_TEXTURE0 + (fbo_buffers_count/2) + i);      
    if (src1.buffersCount > i) {
      glBindTexture(FBO_TYPE, src1.textures[i]);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    }
    else
      glBindTexture(FBO_TYPE, (GLuint)0);      	
  }

  // Calls the corresponding function depending on the current phase
  if (EMPTY != phase)
    {
      bool ret = true;
      switch(phase) {
      case PROJECTION:
	ret = projectionCallbackFunc();	
	break;
      case ANALYSIS:
	ret = analysisCallbackFunc();
	break;
      case COPY:
	ret = copyCallbackFunc();
	break;
      case SYNTHESIS:
	ret = synthesisCallbackFunc();
	break;
      case PHONG:
	ret = phongShadingCallbackFunc();
	break;
      }
      if (false != ret)
	{
	  // Callback function has done the rendering
	  return; 
	}
    }

  // Set vertex and texture coordinates for current level
  {
    GLfloat texcoors0[4][2];
    GLfloat texcoors1[4][2];
    GLfloat vertices[4][2];
  
    int i;
	  
    vertices[0][0] = (dest.x); 
    vertices[0][1] = (dest.y);
    vertices[1][0] = (dest.x); 
    vertices[1][1] = (dest.y + dest.height);
    vertices[2][0] = (dest.x + dest.width); 
    vertices[2][1] = (dest.y + dest.height);
    vertices[3][0] = (dest.x + dest.width); 
    vertices[3][1] = (dest.y);

    texcoors0[0][0] = (src0.x) / (GLfloat)fbo_width; 
    texcoors0[0][1] = (src0.y) / (GLfloat)fbo_height;
    texcoors0[1][0] = (src0.x) / (GLfloat)fbo_width; 
    texcoors0[1][1] = (src0.y + src0.height) / (GLfloat)fbo_height;
    texcoors0[2][0] = (src0.x + src0.width) / (GLfloat)fbo_width; 
    texcoors0[2][1] = (src0.y + src0.height) / (GLfloat)fbo_height;
    texcoors0[3][0] = (src0.x + src0.width) / (GLfloat)fbo_width; 
    texcoors0[3][1] = (src0.y) / (GLfloat)fbo_height;

    texcoors1[0][0] = (src1.x) / (GLfloat)fbo_width; 
    texcoors1[0][1] = (src1.y) / (GLfloat)fbo_height;
    texcoors1[1][0] = (src1.x) / (GLfloat)fbo_width; 
    texcoors1[1][1] = (src1.y + src1.height) / (GLfloat)fbo_height;
    texcoors1[2][0] = (src1.x + src1.width) / (GLfloat)fbo_width; 
    texcoors1[2][1] = (src1.y + src1.height) / (GLfloat)fbo_height;
    texcoors1[3][0] = (src1.x + src1.width) / (GLfloat)fbo_width; 
    texcoors1[3][1] = (src1.y) / (GLfloat)fbo_height;


    // Rasterize texture as quads.
    // Uses multiple tex coords in the case of reading from multiple levels
    // at the same time. (ex. synthesis phase)
    glBegin(GL_QUADS);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    for(i = 0; i < 4; i++) {	
      for (int buf = 0; buf < fbo_buffers_count / 2; ++buf) {
	if (src0.buffersCount > buf)
	  glMultiTexCoord2fARB(GL_TEXTURE0 + buf, texcoors0[i][0], texcoors0[i][1]);
	if (src1.buffersCount > buf)
	  glMultiTexCoord2fARB(GL_TEXTURE0 + (fbo_buffers_count/2) + buf, texcoors1[i][0], texcoors1[i][1]);
      }
      glVertex2f(vertices[i][0], vertices[i][1]);
    }
    glEnd();
  }
}

/** Returns an pixels_struct specifying the pixels of the specified level in the
 * specified fbo (0 or g.fbo) and one or two of its buffers (GL_BACK or g.fbo_buffers[i])
 * @param level The pyramid level currently processed.
 * @param curr_fbo The current framebuffer object.
 * @param buffersCount How many buffers are being used.
 * @param buffers Pointer to the buffers.
 * @return Pixel structure containint information to access the pixels at the current level.
 **/
const pixels_struct PyramidPointRendererBase::generatePixels(const int level, const GLuint curr_fbo, const int buffersCount, 
							 const GLuint* buffers) const {
  pixels_struct result;
  
  result.width = (canvas_width >> level);
  if (result.width < 1)
    result.width = 1;
  result.height = (canvas_height >> level);
  if (result.height < 1)
    result.height = 1;
  if (0 == level)
    {
      result.x = canvas_border_width;
      result.y = canvas_border_height;	  
    }
  else if (1 == level)
    {
      result.x = canvas_width + 2 * canvas_border_width + canvas_border_width / 2;
      result.y = canvas_border_height / 2;
    } 
  else if (2 == level)
    {
      result.x = canvas_width + 2 * canvas_border_width + canvas_border_width / 2;
      result.y = (canvas_height + 2 * canvas_border_height) / 2 + canvas_border_height / 4;
    }
  else if (3 == level)
    {
      result.x = canvas_width + 2 * canvas_border_width + 
	(canvas_width + 2 * canvas_border_width) / 8 - 
	(canvas_width + 2 * canvas_border_width) / 16;
      result.y = (canvas_height + 2 * canvas_border_height) / 2 +
	(canvas_height + 2 * canvas_border_height) / 4 +
	canvas_border_height / 4;
    }
  else
    {
      int i;

      result.x = (canvas_width + 2 * canvas_border_width) + 
	(canvas_width + 2 * canvas_border_width) / 4 +
	(canvas_width + 2 * canvas_border_width) / 8 -
	((canvas_width + 2 * canvas_border_width) >> (level + 1));
      result.y = (canvas_height + 2 * canvas_border_height) / 2;
      for (i = 4; i < level; i++)
	{
	  result.y += ((canvas_height + 2 * canvas_border_height) >> i) + 2;
	}
      result.y += (canvas_border_height >> level) + 2; 
    }

  result.fbo = curr_fbo;
  result.buffersCount = buffersCount;
  for (int i = 0; i < buffersCount; ++i) {
    result.buffers[i] = buffers[i];
    result.textures[i] = getTextureOfBuffer(buffers[i]);
  }

  return result;
}

/**
 * Setting of variables for shader program of the projection phase
 * @return True if done with rasterization, False if still needs to
 * render textures.
 **/
const int PyramidPointRendererBase::projectionCallbackFunc( void )
{
  mShaderProjection.prog.Bind();
  mShaderProjection.prog.Uniform("eye", (GLfloat)eye[0], (GLfloat)eye[1], (GLfloat)eye[2]);
  mShaderProjection.prog.Uniform("back_face_culling", (GLint)back_face_culling);
  mShaderProjection.prog.Uniform("scale", (GLfloat)scale_factor);

  // Projection phase takes care of rasterizing the pixels by projecting surfels, 
  // no need to send textures.
  return true;
}

/** 
 * Project point sized samples to screen space.
 * @param obj Pointer to object for rendering.
 **/
void PyramidPointRendererBase::projectSurfels ( const Object* const obj )
{
  pixels_struct nullPixels;
  pixels_struct destinationPixels;

  nullPixels = generatePixels(0, 0, 0, 0);

  // Camera state has already been set by application.
  framebuffer_state = FBS_APPLICATION_CREATED;

  GLuint buffers[fbo_buffers_count/2];
  for (int i = 0; i < fbo_buffers_count/2; ++i)
    buffers[i] = fbo_buffers[i*2];

  // Renders to first half of buffers in ping-pong scheme.
  destinationPixels = generatePixels(0, fbo, fbo_buffers_count/2, &buffers[0]);
  // No need to read from texture, only sets destination target.
  rasterizePixels(destinationPixels, nullPixels, nullPixels, PROJECTION);

  glPointSize(1.0);

  // Render vertices using the vertex buffer object.
  obj->render();

  mShaderProjection.prog.Unbind();
}

/**
 * Computes the pixel size of the current processing pyramid level.
 * @return Half the current pixel dimension.
 **/
const double PyramidPointRendererBase::computeHalfPixelSize( void ) const {
  double d = pow(2.0, (double)cur_level) / (double)(canvas_width);
  d *= 0.5;
  return d;
}

/**
 * Setting of variables for shader program of the analysis phase
 * @return True if done with rasterization, False if still needs to
 * render textures.
 **/
const int PyramidPointRendererBase::analysisCallbackFunc( void )  {

  mShaderAnalysis.prog.Bind();
  mShaderAnalysis.prog.Uniform("oo_2fbo_size", (GLfloat)(0.5 / fbo_width), (GLfloat)(0.5 / fbo_height));
  mShaderAnalysis.prog.Uniform("half_pixel_size", (GLfloat)computeHalfPixelSize());
  mShaderAnalysis.prog.Uniform("prefilter_size", (GLfloat)(prefilter_size / (GLfloat)(canvas_width)));
  mShaderAnalysis.prog.Uniform("reconstruction_filter_size", (GLfloat)(reconstruction_filter_size));
  mShaderAnalysis.prog.Uniform("depth_test", depth_test);
  
  // Loads the textures ids as uniforms for shader access
  for (int i = 0; i < fbo_buffers_count/2; ++i)
     mShaderAnalysis.prog.Uniform(shader_texture_names[i].c_str(), i);


  // Not done, still has to rasterize quads with textures.
  return false; 
}

/** 
 * Using ping-pong rasterization between color attachment pairs 0-2 and 1-3.
 * Each pair 0-2 and 1-3 stores all resolution levels in a compact way.
 * Analysis phases (pull phase) constructs the lower resolution levels
 * starting from the original image.
 **/
void PyramidPointRendererBase::rasterizeAnalysisPyramid( void ) {
  int level;
  pixels_struct nullPixels;
  pixels_struct sourcePixels;
  pixels_struct destinationPixels;

  nullPixels = generatePixels(0, 0, 0, 0);

  GLuint buffers[fbo_buffers_count/2];

  // Reconstructs all lower resolution levels bottom-up fashion
  for (level = 1; level < levels_count; level++)
    {
      cur_level = level-1;

      // Source texture with corresponding level position, starts with original image and moves up
      for (int i = 0; i < fbo_buffers_count/2; ++i)
	buffers[i] = fbo_buffers[i*2 + ((level - 1) % 2)];
      sourcePixels = generatePixels(level - 1, fbo, fbo_buffers_count/2, &buffers[0]);
      
      // Destination texture with correspondig level position, always one above the source texture level
      for (int i = 0; i < fbo_buffers_count/2; ++i)
	buffers[i] = fbo_buffers[i*2 + ((level) % 2)];
      destinationPixels = generatePixels(level, fbo, fbo_buffers_count/2, &buffers[0]);
      
      rasterizePixels(destinationPixels, sourcePixels, nullPixels, ANALYSIS);
	  mShaderAnalysis.prog.Unbind();
    }
}

/**
 * Setting of variables for shader program of the copy phase
 * @return True if done with rasterization, False if still needs to
 * render textures.
 **/
const int PyramidPointRendererBase::copyCallbackFunc( void ) 
{
  mShaderCopy.prog.Bind();

  // Loads the textures ids as uniforms for shader access
  for (int i = 0; i < fbo_buffers_count/2; ++i)
    mShaderCopy.prog.Uniform(shader_texture_names[i].c_str(), i);

  return false; /* not done, rasterize quad */
}

/** 
 * Copies odd levels from color attachment pair 1-3 to buffer pair 0-2 and 
 * even levels from 0-2 to 1-3.
 **/
void PyramidPointRendererBase::copyAnalysisPyramid( void ) {
  int level;
  pixels_struct nullPixels;
  pixels_struct sourcePixels;
  pixels_struct destinationPixels;

  nullPixels = generatePixels(0, 0, 0, 0);
  GLuint buffers[fbo_buffers_count/2];

  for (level = 0; level < levels_count; level++)
    {
      for (int i = 0; i < fbo_buffers_count/2; ++i)
	buffers[i] = fbo_buffers[i*2 + ((level) % 2)];
      sourcePixels = generatePixels(level, fbo, fbo_buffers_count/2, &buffers[0]);

      for (int i = 0; i < fbo_buffers_count/2; ++i)
	buffers[i] = fbo_buffers[i*2 + ((level + 1) % 2)];
      destinationPixels = generatePixels(level, fbo, fbo_buffers_count/2, &buffers[0]); 

      rasterizePixels(destinationPixels, sourcePixels, nullPixels, COPY);
	  mShaderCopy.prog.Unbind();
    }
}

const int PyramidPointRendererBase::synthesisCallbackFunc( void ) 
{
  mShaderSynthesis.prog.Bind();

  mShaderSynthesis.prog.Uniform("fbo_size", (GLfloat)fbo_width, (GLfloat)fbo_height);
  mShaderSynthesis.prog.Uniform("oo_fbo_size", (GLfloat)(1.0/fbo_width), (GLfloat)(1.0/fbo_height));
  mShaderSynthesis.prog.Uniform("half_pixel_size", (GLfloat)computeHalfPixelSize());
  mShaderSynthesis.prog.Uniform("prefilter_size", (GLfloat)(prefilter_size / (GLfloat)(canvas_width)));
  mShaderSynthesis.prog.Uniform("reconstruction_filter_size", (GLfloat)(reconstruction_filter_size));
  mShaderSynthesis.prog.Uniform("depth_test", depth_test);
  mShaderSynthesis.prog.Uniform("elliptical_weight", elliptical_weight);

  for (int i = 0; i < fbo_buffers_count/2; ++i)
	mShaderSynthesis.prog.Uniform(shader_texture_names[i].c_str(), i);

  return false; /* not done, rasterize quad */
}

void PyramidPointRendererBase::rasterizeSynthesisPyramid( void )
     /* using ping-pong rasterization between color attachment pairs 0-2 and 1-3 */
{
  int level;
  pixels_struct source0Pixels; /* same level as destination */
  pixels_struct source1Pixels; /* coarser level than destination */
  pixels_struct destinationPixels;

  GLuint buffers[fbo_buffers_count/2];

  for (level = levels_count - 2; level >= 0; level--)
    {
      cur_level = level;
      for (int i = 0; i < fbo_buffers_count/2; ++i)
		buffers[i] = fbo_buffers[i*2 + ((level+1) % 2)];
      source0Pixels = generatePixels(level, fbo, fbo_buffers_count/2, &buffers[0]);
      source1Pixels = generatePixels(level + 1, fbo, fbo_buffers_count/2, &buffers[0]);

      for (int i = 0; i < fbo_buffers_count/2; ++i)
		buffers[i] = fbo_buffers[i*2 + ((level) % 2)];
      destinationPixels = generatePixels(level, fbo, fbo_buffers_count/2, &buffers[0]);

      rasterizePixels(destinationPixels, source0Pixels, source1Pixels, SYNTHESIS);
	  mShaderSynthesis.prog.Unbind();
    }
}

/**
 * Rasterize level 0 of pyramid with per pixel shading.
 **/
const int PyramidPointRendererBase::phongShadingCallbackFunc( void ) 
{
	mShaderPhong.prog.Bind();

	mShaderPhong.prog.Uniform(shader_texture_names[0].c_str(), 0);
	for (int i = 2; i < fbo_buffers_count/2; ++i)
		mShaderPhong.prog.Uniform(shader_texture_names[i].c_str(), i-1);

	mShaderPhong.prog.Uniform("color_ambient", Mats[material_id][0], Mats[material_id][1], Mats[material_id][2], Mats[material_id][3]);
	mShaderPhong.prog.Uniform("color_diffuse", Mats[material_id][4], Mats[material_id][5], Mats[material_id][6], Mats[material_id][7]);
	mShaderPhong.prog.Uniform("color_specular", Mats[material_id][8], Mats[material_id][9], Mats[material_id][10], Mats[material_id][11]);
	mShaderPhong.prog.Uniform("shininess", Mats[material_id][12]);

	return false; /* not done, rasterize quad */
}

void PyramidPointRendererBase::rasterizePhongShading(int bufferIndex)
     /* using ping-pong rasterization between color attachment pairs 0-2 and 1-3 */
{
	int level = 0;
	pixels_struct nullPixels;
	pixels_struct sourcePixels;
	pixels_struct destinationPixels;

	nullPixels = generatePixels(0, 0, 0, 0);

	mShaderProjection.prog.Unbind();
	mShaderCopy.prog.Unbind();
	mShaderAnalysis.prog.Unbind();
	mShaderSynthesis.prog.Unbind();
	mShaderPhong.prog.Unbind();

	GLuint buffers[fbo_buffers_count/2 - 1];

	buffers[0] = fbo_buffers[bufferIndex];

	for (int i = 1; i < fbo_buffers_count/2; ++i)
		buffers[i] = fbo_buffers[bufferIndex + i*2 + 2];
	sourcePixels = generatePixels(level, fbo, fbo_buffers_count/2 - 1, &buffers[0]);

	GLuint back[1] = {GL_BACK};
	destinationPixels = generatePixels(level, 0, 1, &back[0]);
	rasterizePixels(destinationPixels, sourcePixels, nullPixels, PHONG);

	mShaderPhong.prog.Unbind();
}

/**
 * Clear all framebuffers and screen buffer.
 **/
void PyramidPointRendererBase::clearBuffers( void ) {
  int i;

  glEnable(FBO_TYPE);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);

  GLint currentDrawBuffer;
  glGetIntegerv(GL_DRAW_BUFFER, &currentDrawBuffer);

  // Clear all buffers, including the render buffer
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
  for (i = 0; i < fbo_buffers_count; i++) {
    glDrawBuffer(fbo_buffers[i]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  // Clears the back buffer
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  framebuffer_state = FBS_UNDEFINED;

  check_for_ogl_error("clear buffers");
}

/**
 * Reconstructs the surface for visualization.
 **/
void PyramidPointRendererBase::projectSamples(Object* const obj) {
  // Project points to framebuffer with depth test on.
  projectSurfels( obj );

  check_for_ogl_error("project samples");
}

/**
 * Interpolate projected samples using pyramid interpolation
 * algorithm.
 **/
void PyramidPointRendererBase::interpolate() {
  framebuffer_state = FBS_UNDEFINED;

  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // Interpolate scattered data using pyramid algorithm

  rasterizeAnalysisPyramid();

  copyAnalysisPyramid();

  rasterizeSynthesisPyramid();

}

/**
 * Renders reconstructed model on screen with
 * per pixel shading.
 **/
void PyramidPointRendererBase::draw( void ) {

  // Deffered shading of the final image containing normal map
  rasterizePhongShading(0);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);

  glDisable(FBO_TYPE);

  check_for_ogl_error("draw");
}


/**
 * Initialize OpenGL state variables.
 **/
void PyramidPointRendererBase::createFBO() {
  int i;
  GLenum framebuffer_status;
  GLenum attachments[16] = {
    GL_COLOR_ATTACHMENT0_EXT,
    GL_COLOR_ATTACHMENT1_EXT,
    GL_COLOR_ATTACHMENT2_EXT,
    GL_COLOR_ATTACHMENT3_EXT,
    GL_COLOR_ATTACHMENT4_EXT,
    GL_COLOR_ATTACHMENT5_EXT,
    GL_COLOR_ATTACHMENT6_EXT,
    GL_COLOR_ATTACHMENT7_EXT,
    GL_COLOR_ATTACHMENT8_EXT,
    GL_COLOR_ATTACHMENT9_EXT,
    GL_COLOR_ATTACHMENT10_EXT,
    GL_COLOR_ATTACHMENT11_EXT,
    GL_COLOR_ATTACHMENT12_EXT,
    GL_COLOR_ATTACHMENT13_EXT,
    GL_COLOR_ATTACHMENT14_EXT,
    GL_COLOR_ATTACHMENT15_EXT
  };

  /* initialize fbos */

  assert(fbo_buffers_count <= 16);

  fbo_buffers = new GLuint[fbo_buffers_count];
  fbo_textures = new GLuint[fbo_buffers_count];

  check_for_ogl_error("new arrays fbo");

  glGenTextures(fbo_buffers_count, fbo_textures);
  for (i = 0; i < fbo_buffers_count; i++) {
    fbo_buffers[i] = attachments[i];
    glBindTexture(FBO_TYPE, fbo_textures[i]);
    glTexImage2D(FBO_TYPE, 0, FBO_FORMAT,
		 fbo_width, fbo_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_WRAP_T, GL_CLAMP);
  }

  check_for_ogl_error("buffers creation");

  //for creating and binding a depth buffer:
  glGenTextures(1, &fbo_depth);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo_depth);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT32, fbo_width,
			   fbo_height);

  check_for_ogl_error("depth buffer creation");

  glGenFramebuffersEXT(1, &fbo);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

  check_for_ogl_error("framebuffer creation");

  for (i = 0; i < fbo_buffers_count; i++) 
    {
      //fprintf(stderr, "bind fbo buffer %i\n", i);
      glBindTexture(FBO_TYPE, fbo_textures[i]);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				fbo_buffers[i], FBO_TYPE, fbo_textures[i], 0);
      check_for_ogl_error("fbo attachment");
    }

  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
			       GL_RENDERBUFFER_EXT, fbo_depth);
 
  check_for_ogl_error("depth attachment");

  framebuffer_status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  switch(framebuffer_status) 
    {
    case 0:
      fprintf(stderr, "Check Framebuffer Status Error\n");
      exit(1);
      break;
    case GL_FRAMEBUFFER_COMPLETE_EXT:
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      fprintf(stderr, "Framebuffer not supported\n");
      exit(1);
      break;
    default:
      fprintf(stderr, "Framebuffer error %x", framebuffer_status);
      exit(1);
      return ;
    }
 
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  framebuffer_state = FBS_UNDEFINED;

  glShadeModel(GL_FLAT);
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  check_for_ogl_error("fbo");

  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &i);
  //  fprintf(stderr, "max color attachments %i\n", i);

}		

