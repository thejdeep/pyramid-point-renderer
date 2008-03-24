/*
** jfa_splatting.cc 
**
** Author : Ricardo Marroquim
**
**   history:	created  24-Mar-08
*/

#include "jfa_splatting.h"

/**
 * Default constructor.
 **/
JFASplatting::JFASplatting() : PointBasedRender(),
					       canvas_border_width(32),
					       canvas_border_height(32) {
  createFBO();
  createShaders();
}

/**
 * Constructor with screen size.
 **/
JFASplatting::JFASplatting(int w, int h) : PointBasedRender(w, h),
							   canvas_border_width(w/32),
							   canvas_border_height(h/32) {
  createFBO();
  createShaders();
}

JFASplatting::~JFASplatting() {
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

/** 
 * Draws a quad for texture rendering.
 * The texture coordinates are relative to the full texture size,
 * since some frameBuffers except only power of two sizes and the
 * screen size is arbitrary.
 * The vertices coordinates do not include the bordes.
 **/
void JFASplatting::drawQuad( void ) {

  /* send quad */
  glBegin(GL_QUADS);
  glColor4f(0.0f, 0.0f, 0.0f, 1.0f);

  GLfloat vertices[4][2];
  GLfloat texcoords[4][2];

  vertices[0][0] = (GLfloat)canvas_border_width;
  vertices[0][1] = (GLfloat)canvas_border_height;
  vertices[1][0] = (GLfloat)canvas_border_width;
  vertices[1][1] = (GLfloat)(canvas_height+canvas_border_height);
  vertices[2][0] = (GLfloat)(canvas_width+canvas_border_width);
  vertices[2][1] = (GLfloat)(canvas_height+canvas_border_height);
  vertices[3][0] = (GLfloat)(canvas_width+canvas_border_width);
  vertices[3][1] = (GLfloat)canvas_border_height;

  texcoords[0][0] = (GLfloat)canvas_border_width / (GLfloat)fbo_width;
  texcoords[0][1] = (GLfloat)canvas_border_height / (GLfloat)fbo_height;
  texcoords[1][0] = (GLfloat)canvas_border_width / (GLfloat)fbo_width;
  texcoords[1][1] = (GLfloat)(canvas_height+canvas_border_height) / (GLfloat)fbo_height;
  texcoords[2][0] = (GLfloat)(canvas_width+canvas_border_width) / (GLfloat)fbo_width;
  texcoords[2][1] = (GLfloat)(canvas_height+canvas_border_height) / (GLfloat)fbo_height;
  texcoords[3][0] = (GLfloat)(canvas_width+canvas_border_width) / (GLfloat)fbo_width;
  texcoords[3][1] = (GLfloat)canvas_border_height / (GLfloat)fbo_height;

  for (int i = 0; i < 4; ++i) {
    glTexCoord2f(texcoords[i][0], texcoords[i][1]);
    glVertex2f(vertices[i][0], vertices[i][1]);
  }

  glEnd();
}

/**
 * Extracts debugging information from the reconstructed buffer.
 **/
void JFASplatting::getDataReconstructedPixels ( int buffer ) {
  
  GLfloat *outputBuffer = new GLfloat[fbo_width * fbo_height * 4];
  glReadBuffer(fbo_buffers[buffer]);
  glReadPixels(0, 0, fbo_width, fbo_height, GL_RGBA, GL_FLOAT, &outputBuffer[0]);

  int pixels = 0;

  for (int i = 0; i < fbo_width * fbo_height * 4; i+=4) {
    if (outputBuffer[i + 3] > 0)
    ++pixels;
  }
  
  cout << pixels << endl;
//   cout << sqrt((double)pixels) / 2.0 << endl;

  delete outputBuffer;
}

/**
 * Extracts debugging information from the projected samples.
 **/
void JFASplatting::getDataProjectedPixels ( int* data ) {
  
  GLfloat *outputBuffer = new GLfloat[fbo_width * fbo_height * 4];
  glReadBuffer(fbo_buffers[2]);
  glReadPixels(0, 0, fbo_width, fbo_height, GL_RGBA, GL_FLOAT, &outputBuffer[0]);

  GLfloat radius = 0.0;
  int splats = 0;

  for (int i = 0; i < fbo_width * fbo_height * 4; i+=4) {
    radius = outputBuffer[i + 3];
    if (radius > 0.0)
      ++splats;
  }

  data[0] += splats;
  data[1] += splats;
  data[2] += 0;
  data[3] += 0;
  data[4] = splats;

  delete outputBuffer;
}

/**
 * Project surfels os a single primitive to screen space.
 * Projection is written on third color attachment (number 2).
 * Each sample is projected to one single pixel with information
 * of its normal, radius, and depth.
 * @param prim Given primitive to be projected.
 **/
void JFASplatting::projectSurfels ( Primitives* prim )
{
  GLenum outputBuffers[3] = {fbo_buffers[2], fbo_buffers[0], fbo_buffers[1]};
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
  glDrawBuffers(3, outputBuffers);

//   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
//   glDrawBuffer(fbo_buffers[2]);
 
  shader_projection->use();
  shader_projection->set_uniform("eye", (GLfloat)eye[0], (GLfloat)eye[1], (GLfloat)eye[2]);

  // Render vertices using the vertex buffer object.
  prim->render();

  shader_projection->use(0);
}

/**
 * Switchs the render source and target.
 * Ping-pong scheme, reads from one texture and writes to 
 * another of identical size. During the next interation the
 * source and destination are switched around.
 **/
void JFASplatting::switchBuffers( void ) {
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

  glViewport(0, 0, canvas_width+2*canvas_border_width,
	     canvas_height+2*canvas_border_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, canvas_width+2*canvas_border_width,
	     0.0, canvas_height+2*canvas_border_height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glActiveTexture(GL_TEXTURE0 + read_buffer);
  glBindTexture(FBO_TYPE, fbo_textures[read_buffer]);
  //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/**

 **/
void JFASplatting::evaluatePixels( void )
{  

  // Activate projected surfels texture
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(FBO_TYPE, fbo_textures[2]);
//   glActiveTexture(GL_TEXTURE3);
//   glBindTexture(FBO_TYPE, fbo_textures[3]);

  shader_evaluate->use();
  //shader_evaluate->set_uniform("depth_test", depth_test);
  shader_evaluate->set_uniform("prefilter_size", (GLfloat)(prefilter_size / (GLfloat)(canvas_width)));
  shader_evaluate->set_uniform("reconstruction_filter_size", (GLfloat)(reconstruction_filter_size));

  // pass texture with original normals and radius from projected pixel
  // textureA [n.x, n.y, n.z, radius]
  shader_evaluate->set_uniform("textureA", 2);

  read_buffer = 1;
  
  for (int l = fbo_width/2; l >= 1; l=l/2) {
    switchBuffers();
    shader_evaluate->set_uniform("step_length", (GLint)l);
    shader_evaluate->set_uniform("textureB", (GLint)read_buffer);
    drawQuad();
  }

  shader_evaluate->use(0);

  //getDataReconstructedPixels ( dest_buffer );
}

/**
 * Computes phong illumination for reconstructed surface.
 * Each pixel contains normal information.
 **/
void JFASplatting::rasterizePhongShading( void )
{
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);
  CHECK_FOR_OGL_ERROR();

  glViewport(0, 0, canvas_width+2*canvas_border_width,
	     canvas_height+2*canvas_border_height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0.0, canvas_width+2*canvas_border_width,
	     0.0, canvas_height+2*canvas_border_height);

//   glViewport(canvas_border_width, canvas_border_height, 
// 	     canvas_width+canvas_border_width,
// 	     canvas_height+canvas_border_height);
//   CHECK_FOR_OGL_ERROR();
//   glMatrixMode(GL_PROJECTION);
//   glLoadIdentity();
//   gluOrtho2D(canvas_border_width, canvas_width+canvas_border_width,
// 	     canvas_border_height, canvas_height+canvas_border_height);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glActiveTexture(GL_TEXTURE0+dest_buffer);
  glBindTexture(FBO_TYPE, fbo_textures[dest_buffer]);
//   glActiveTexture(GL_TEXTURE4);
//   glBindTexture(FBO_TYPE, fbo_textures[4]);
  
  shader_phong->use();
  shader_phong->set_uniform("textureA", 1);
  shader_phong->set_uniform("textureB", 2);

  drawQuad();

  shader_phong->use(0);
}

/**
 * Clear all framebuffers, depthbuffer and screen buffer.
 **/
void JFASplatting::clearBuffers( void ) {
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
 * Project points of a primitive to framebuffer with depth test on.
 * @param prim Given primitive.
 **/
void JFASplatting::projectSamples(Primitives* prim) {

  projectSurfels( prim );
  CHECK_FOR_OGL_ERROR();
}

/**
 * Reconstructs surface.
 **/
void JFASplatting::interpolate( void ) {
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // Interpolate scattered data using pyramid algorithm
  evaluatePixels();
}

/**
 * Renders reconstructed model on screen with per pixel shading.
 **/
void JFASplatting::draw( void ) {
  // Deffered shading of the final image containing normal map
  rasterizePhongShading();

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);
  glDisable(FBO_TYPE);

  CHECK_FOR_OGL_ERROR();
}

/**
 * Initialize frameBuffer object and textures.
 **/
void JFASplatting::createFBO( void ) {
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
  fbo_width = fbo_height = 1024;

  for (i = 0; i < FBO_BUFFERS_COUNT; i++) {
    fbo_buffers[i] = attachments[i];
    glBindTexture(FBO_TYPE, fbo_textures[i]);

    glTexImage2D(FBO_TYPE, 0, FBO_FORMAT, fbo_width, fbo_height,
		 0, GL_RGBA, GL_FLOAT, NULL);

    CHECK_FOR_OGL_ERROR();
    
    glTexParameteri(FBO_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(FBO_TYPE, GL_TEXTURE_WRAP_T, GL_CLAMP);
  }

  //for creating and binding a depth buffer:
  glGenTextures(1, &fbo_depth);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo_depth);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT32, fbo_width, fbo_height);

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
void JFASplatting::createShaders ( void ) {

  bool shader_inst_debug = 1;

  shader_projection = new glslKernel();
  shader_projection->vertex_source("shader_point_projection_jfa.vert");
  shader_projection->fragment_source("shader_point_projection_jfa.frag");
  shader_projection->install( shader_inst_debug );

  shader_evaluate = new glslKernel();
  shader_evaluate->vertex_source("shader_evaluate_jfa.vert");
  shader_evaluate->fragment_source("shader_evaluate_jfa.frag");
  shader_evaluate->install( shader_inst_debug );

  shader_phong = new glslKernel();
  shader_phong->vertex_source("shader_phong_jfa.vert");
  shader_phong->fragment_source("shader_phong_jfa.frag");
  shader_phong->install( shader_inst_debug );

}
