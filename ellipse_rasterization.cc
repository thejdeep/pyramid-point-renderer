/*
** ellipse_rasterization.cc 
**
** Author : Ricardo Marroquim
**
**   history:	created  29-Feb-08
*/

#include "ellipse_rasterization.h"

/**
 * Default constructor.
 **/
EllipseRasterization::EllipseRasterization() : PointBasedRender(),
					     canvas_border_width(32),
					     canvas_border_height(32),
					     render_state(RS_BUFFER0) {
  createFBO();
  createShaders();

}

EllipseRasterization::EllipseRasterization(int w, int h) : PointBasedRender(w, h),
					   canvas_border_width(w/32),
					   canvas_border_height(h/32),
					   render_state(RS_BUFFER0) {
  createFBO();
  createShaders();
}

EllipseRasterization::~EllipseRasterization() {
  delete shader_projection;
  delete shader_evaluate;
  delete shader_phong;

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  glDeleteTextures(FBO_BUFFERS_COUNT, fbo_textures);
  for (int i = 0; i < FBO_BUFFERS_COUNT; ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(FBO_TYPE, 0);
    glDisable(FBO_TYPE);
  }

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);
}

void EllipseRasterization::drawQuad( void ) {

  /* send quad */
  glBegin(GL_QUADS);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
  glTexCoord2d(0.0, 0.0);
  glVertex2f(0.0, 0.0);
  glTexCoord2d(0.0, 1.0);
  glVertex2f(0.0, canvas_height+canvas_border_height);
  glTexCoord2d(1.0, 1.0);
  glVertex2f(canvas_width+canvas_border_width, canvas_height+canvas_border_height);
  glTexCoord2d(1.0, 0.0);
  glVertex2f(canvas_width+canvas_border_width, 0.0);

  glEnd();
}

void EllipseRasterization::switchBuffers( void ) {
  if (read_buffer == 0) {
    read_buffer = 1;
    dest_buffer = 0;
  }
  else {
    read_buffer = 0;
    dest_buffer = 1;
  }

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
  glDrawBuffer(fbo_buffers[dest_buffer]);

  glViewport(0, 0, canvas_width+canvas_border_width,
	     canvas_height+canvas_border_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, canvas_width+canvas_border_width, 
	     0.0, canvas_height+canvas_border_height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(FBO_TYPE, fbo_textures[read_buffer]);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/// Project point sized samples to screen space
void EllipseRasterization::projectSurfels ( Primitives* prim )
{
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
  glDrawBuffer(fbo_buffers[2]);

  glViewport(0, 0, canvas_width, canvas_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, canvas_width, 0.0, canvas_height);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(FBO_TYPE, (GLuint)0);

  //drawQuad();

  // Render vertices using the vertex buffer object.
  prim->render();

  shader_projection->use(0);
}

void EllipseRasterization::evaluatePixels( void )
{  
  shader_evaluate->use();

  shader_evaluate->set_uniform("prefilter_size", (GLfloat)(prefilter_size / (GLfloat)(canvas_width)));
  shader_evaluate->set_uniform("reconstruction_filter_size", (GLfloat)(reconstruction_filter_size));
  shader_evaluate->set_uniform("depth_test", depth_test);

  shader_evaluate->set_uniform("ellipsesTexture", 0);

  for (int j = 0; j < MAX_DISPLACEMENT; ++j)
    for (int i = 0; i < MAX_DISPLACEMENT; ++i) {
      shader_evaluate->set_uniform("displacement", i, j);
      drawQuad();
      switchBuffers();
    }
  shader_evaluate->use(0);      
}

void EllipseRasterization::rasterizePhongShading( void )
{
  shader_phong->use();
  shader_phong->set_uniform("textureA", 0);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);

  glViewport(0, 0, canvas_width, canvas_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, canvas_width, 0.0, canvas_height);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(FBO_TYPE, fbo_textures[read_buffer]);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  shader_phong->use(0);
}

/**
 * Clear all framebuffers and screen buffer.
 **/
void EllipseRasterization::clearBuffers() {
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
  for (i = 0; i < FBO_BUFFERS_COUNT; i++) {
    glDrawBuffer(fbo_buffers[i]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  // Clears the back buffer
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  CHECK_FOR_OGL_ERROR();
}

/**
 * Project points to framebuffer with depth test on.
 **/
void EllipseRasterization::projectSamples(Primitives* prim) {
  projectSurfels( prim );
  CHECK_FOR_OGL_ERROR();
}

/**
 * Interpolate projected samples by fetching nearest pixels
 **/
void EllipseRasterization::interpolate() {
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // Interpolate scattered data using pyramid algorithm
  evaluatePixels();
}

/**
 * Renders reconstructed model on screen with per pixel shading.
 **/
void EllipseRasterization::draw( void ) {
  // Deffered shading of the final image containing normal map
  rasterizePhongShading();

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);
  glDisable(FBO_TYPE);

  CHECK_FOR_OGL_ERROR();
}

/**
 * Initialize OpenGL state variables.
 **/
void EllipseRasterization::createFBO() {
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
  assert(FBO_BUFFERS_COUNT <= 16);

  glGenTextures(FBO_BUFFERS_COUNT, fbo_textures);
  for (i = 0; i < FBO_BUFFERS_COUNT; i++) {
    fbo_buffers[i] = attachments[i];
    glBindTexture(FBO_TYPE, fbo_textures[i]);
    glTexImage2D(FBO_TYPE, 0, FBO_FORMAT,
		 canvas_width+canvas_border_width, canvas_height+canvas_border_height,
		 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_WRAP_T, GL_CLAMP);
  }

  //for creating and binding a depth buffer:
  glGenTextures(1, &fbo_depth);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo_depth);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT32, canvas_width+canvas_border_width,
			   canvas_height+canvas_border_height);
  glGenFramebuffersEXT(1, &fbo);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

  CHECK_FOR_OGL_ERROR();

  for (i = 0; i < FBO_BUFFERS_COUNT; i++) 
    {
      //fprintf(stderr, "bind fbo buffer %i\n", i);
      glBindTexture(FBO_TYPE, fbo_textures[i]);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				fbo_buffers[i], FBO_TYPE, fbo_textures[i], 0);
      CHECK_FOR_OGL_ERROR();
    }

  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
			       GL_RENDERBUFFER_EXT, fbo_depth);
 
  CHECK_FOR_OGL_ERROR();

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

  CHECK_FOR_OGL_ERROR();

  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &i);
  //  fprintf(stderr, "max color attachments %i\n", i);
}		

/**
 * Installs the shaders using the GLSL Kernel class.
 **/
void EllipseRasterization::createShaders ( void ) {

  bool shader_inst_debug = 0;

  shader_projection = new glslKernel();
  shader_projection->vertex_source("shader_point_projection_er.vert");
  shader_projection->fragment_source("shader_point_projection_er.frag");
  shader_projection->install( shader_inst_debug );

  shader_evaluate = new glslKernel();
  shader_evaluate->vertex_source("shader_evaluate_er.vert");
  shader_evaluate->fragment_source("shader_evaluate_er.frag");
  shader_evaluate->install( shader_inst_debug );

  shader_phong = new glslKernel();
  shader_phong->vertex_source("shader_phong.vert");
  shader_phong->fragment_source("shader_phong.frag");
  shader_phong->install( shader_inst_debug );
}

/**
 * Sets the vertex array.
 **/
void EllipseRasterization::setVertices( vector<Surfel> *s ) {
 

}

void EllipseRasterization::setPrefilterSize(double s) {
  prefilter_size = s;
}

void EllipseRasterization::setReconstructionFilterSize(double s) {
  reconstruction_filter_size = s;
}

void EllipseRasterization::setZoomFactor (double z) {
   zoom_factor = z;
}

void EllipseRasterization::setEye (double e[3]) {
  eye[0] = e[0];
  eye[1] = e[1];
  eye[2] = e[2];
}

void EllipseRasterization::setLight (double l[3]) {
  light_dir[0] = l[0];
  light_dir[1] = l[1];
  light_dir[2] = l[2];
}
