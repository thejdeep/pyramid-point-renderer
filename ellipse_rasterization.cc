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

/**
 * Constructor with screen size.
 **/
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

/** 
 * Draws a quad for texture rendering.
 * The texture coordinates are relative to the full texture size,
 * since some frameBuffers except only power of two sizes and the
 * screen size is arbitrary.
 * The vertices coordinates do not include the bordes.
 **/
void EllipseRasterization::drawQuad( void ) {

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
 * Project surfels os a single primitive to screen space.
 * Projection is written on third color attachment (number 2).
 * Each sample is projected to one single pixel with information
 * of its normal, radius, and depth.
 * @param prim Given primitive to be projected.
 **/
void EllipseRasterization::projectSurfels ( Primitives* prim )
{
//   GLenum outputBuffers[3] = {fbo_buffers[2], fbo_buffers[3], fbo_buffers[4]};
//   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
//   glDrawBuffers(3, outputBuffers);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
  glDrawBuffer(fbo_buffers[2]);
 
  shader_projection->use();
  shader_projection->set_uniform("eye", (GLfloat)eye[0], (GLfloat)eye[1], (GLfloat)eye[2]);
  shader_projection->set_uniform("num_subdivisions", NUM_CIRCLE_SUBDIVISIONS);

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
 * Reconstructs surface by rasterizing ellipses.
 * A fixed sized mask is set on top of each pixel.
 * Each iteration sums the contribution of a fixed
 * displacement on the mask for every pixel.
 * Thus n iterations is necessary where n is the size of
 * the mask in pixels.
 **/
void EllipseRasterization::evaluatePixels( void )
{  

  //shader_evaluate->set_uniform("depth_test", depth_test);

  // Activate projected surfels texture
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(FBO_TYPE, fbo_textures[2]);
//   glActiveTexture(GL_TEXTURE3);
//   glBindTexture(FBO_TYPE, fbo_textures[3]);

  shader_evaluate->use();

  shader_evaluate->set_uniform("prefilter_size", (GLfloat)(prefilter_size / (GLfloat)(canvas_width)));
  shader_evaluate->set_uniform("reconstruction_filter_size", (GLfloat)(reconstruction_filter_size));

  // pass texture with original normals and radius from projected pixel
  // textureA [n.x, n.y, n.z, radius]
  shader_evaluate->set_uniform("textureA", 2);
// textureA [z, deltaZ, material_id, 0.0]
//   shader_evaluate->set_uniform("textureB", 3);

  GLfloat size[2] = {(GLfloat)(fbo_width),
		     (GLfloat)(fbo_height)};

  for (int j = -MAX_DISPLACEMENT; j < MAX_DISPLACEMENT; ++j)
    for (int i = -MAX_DISPLACEMENT; i < MAX_DISPLACEMENT; ++i) {
      switchBuffers();
      shader_evaluate->set_uniform("displacement", i/size[0], j/size[1]);
      shader_evaluate->set_uniform("textureB", (GLint)read_buffer);
      drawQuad();
    }

  shader_evaluate->use(0);
}

/**
 * Computes phong illumination for reconstructed surface.
 * Each pixel contains normal information.
 **/
void EllipseRasterization::rasterizePhongShading( void )
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

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(FBO_TYPE, fbo_textures[1]);
//   glActiveTexture(GL_TEXTURE4);
//   glBindTexture(FBO_TYPE, fbo_textures[4]);

  shader_phong->use();
  shader_phong->set_uniform("textureA", 1);
//   shader_phong->set_uniform("textureB", 4);

  drawQuad();

  shader_phong->use(0);
}

/**
 * Clear all framebuffers, depthbuffer and screen buffer.
 **/
void EllipseRasterization::clearBuffers( void ) {
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
void EllipseRasterization::projectSamples(Primitives* prim) {
  projectSurfels( prim );
  CHECK_FOR_OGL_ERROR();
}

/**
 * Reconstructs surface.
 **/
void EllipseRasterization::interpolate( void ) {
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
 * Initialize frameBuffer object and textures.
 **/
void EllipseRasterization::createFBO( void ) {
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
void EllipseRasterization::createShaders ( void ) {

  bool shader_inst_debug = 1;

  shader_projection = new glslKernel();
  shader_projection->vertex_source("shader_point_projection_er.vert");
  shader_projection->geometry_source("shader_point_projection_er.geom");
  shader_projection->set_geom_max_output_vertices( NUM_CIRCLE_SUBDIVISIONS );
  shader_projection->set_geom_input_type(GL_POINTS);
  shader_projection->set_geom_output_type(GL_POINTS);
  shader_projection->fragment_source("shader_point_projection_er.frag");
  shader_projection->install( shader_inst_debug );

  shader_evaluate = new glslKernel();
  shader_evaluate->vertex_source("shader_evaluate_er.vert");
  shader_evaluate->fragment_source("shader_evaluate_er.frag");
  shader_evaluate->install( shader_inst_debug );

  shader_phong = new glslKernel();
  shader_phong->vertex_source("shader_phong_er.vert");
  shader_phong->fragment_source("shader_phong_er.frag");
  shader_phong->install( shader_inst_debug );

}
