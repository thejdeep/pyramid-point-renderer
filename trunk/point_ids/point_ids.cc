/*
** point_ids.cc Pyramid Point Based Rendering.
**
** Project only points with ids to buffer.
**
**   history:	created  16-Jul-08
*/

#include "point_ids.h"

/**
 * Default constructor.
 **/
PointIds::PointIds() : PointBasedRenderer(),
		       canvas_border_width(32),
		       canvas_border_height(32) {
  createFBO();
  createShaders();
}

PointIds::PointIds(int w, int h) : PointBasedRenderer(w, h),
				   canvas_border_width(w/32),
				   canvas_border_height(h/32) {
  createFBO();
  createShaders();
}

PointIds::~PointIds() {
  delete shader_projection;

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  glDeleteTextures(1, &fbo_texture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(FBO_TYPE, 0);
  glDisable(FBO_TYPE);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);
}



void PointIds::prepareBuffer( void )
     /* binds buffers and rasterizes quad; calls callbackfunc to set the fragment program */
{
  /* bind framebuffer and renderbuffers */

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
  glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(FBO_TYPE, fbo_texture);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

}

/// Project point sized samples to screen space
void PointIds::projectSurfels ( Primitives* prim )
{
  prepareBuffer();

  shader_projection->use();
  shader_projection->set_uniform("eye", (GLfloat)eye[0], (GLfloat)eye[1], (GLfloat)eye[2]);

  // Render vertices using the vertex buffer object.
  glPointSize(1.0);

  prim->render();

  shader_projection->use(0);
}

/**
 * Clear all framebuffers and screen buffer.
 **/
void PointIds::clearBuffers() {

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
  glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Clears the back buffer
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  CHECK_FOR_OGL_ERROR();
}

/**
 * Project points to framebuffer with depth test on.
 **/
void PointIds::projectSamples(Primitives* prim) {
  projectSurfels( prim );

  // keep size to renormalize after retrieving buffer
  number_points = prim->numberPoints();

  CHECK_FOR_OGL_ERROR();
}

/**
 * Renders points with ids on given buffer.
 **/
void PointIds::drawPointIdsBuffer ( GLfloat* data, int w, int h ){

  GLfloat *buffer_data = new GLfloat[w*h*4];

  glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
  glReadPixels(0, 0, w, h, GL_RGBA, GL_FLOAT, &buffer_data[0]);

  // id is retrieved normalized [0, 1], rescale to [0, number_points]
  for (int i = 0; i < w*h; ++i) {
    if (buffer_data[i*4 + 1] == 1.0)
      data[i] = floor(buffer_data[i*4] * number_points + 0.5);    
    else
      data[i] = -1.0;
  }

  delete buffer_data;

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);
  glDisable(FBO_TYPE);

  CHECK_FOR_OGL_ERROR();
}

/**
 * Initialize OpenGL state variables.
 **/
void PointIds::createFBO() {

  glGenTextures(1, &fbo_texture);
  
  glBindTexture(FBO_TYPE, fbo_texture);
  glTexImage2D(FBO_TYPE, 0, FBO_FORMAT,
	       canvas_width+2*canvas_border_width, 
	       canvas_height+2*canvas_border_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(FBO_TYPE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(FBO_TYPE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(FBO_TYPE, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(FBO_TYPE, GL_TEXTURE_WRAP_T, GL_CLAMP);

  //for creating and binding a depth buffer:
  glGenTextures(1, &fbo_depth);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo_depth);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT32, canvas_width+2*canvas_border_width,
			   canvas_height+2*canvas_border_height);

  CHECK_FOR_OGL_ERROR();

  glGenFramebuffersEXT(1, &fbo);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

  CHECK_FOR_OGL_ERROR();

  glBindTexture(FBO_TYPE, fbo_texture);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
			   GL_COLOR_ATTACHMENT0_EXT, FBO_TYPE, fbo_texture, 0);
  CHECK_FOR_OGL_ERROR();
    

  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
			       GL_RENDERBUFFER_EXT, fbo_depth);
 
  CHECK_FOR_OGL_ERROR();

  GLenum framebuffer_status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
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

  glShadeModel(GL_FLAT);
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  CHECK_FOR_OGL_ERROR();

}		

/**
 * Installs the shaders using the GLSL Kernel class.
 **/
void PointIds::createShaders ( void ) {

  bool shader_inst_debug = 0;

  shader_projection = new glslKernel();
  shader_projection->vertex_source("point_ids/shader_point_projection.vert");
  shader_projection->fragment_source("point_ids/shader_point_projection.frag");
  shader_projection->install( shader_inst_debug );
}
