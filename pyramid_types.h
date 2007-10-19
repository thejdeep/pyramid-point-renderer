/* types */

/** A pixels_struct specifies a rectangle of pixels 
 * in one or two buffers of one framebuffer object 
 **/
typedef struct 
{
  GLfloat x;
  GLfloat y;
  GLfloat width;
  GLfloat height;
  GLuint fbo; /* framebuffer object handle */
  GLuint buffers[2]; /* e.g. GL_BACK or GL_COLOR_ATTACHMENT0_EXT */
  GLuint textures[2]; /* texture handle */
  int buffersCount;
} pixels_struct;

typedef enum 
  {
    RS_BUFFER0,
    RS_BUFFER1,
    RS_BUFFER2,
    RS_BUFFER3,
    RS_TIME
  } render_state_enum;

typedef enum 
  {
    EMPTY,
    PROJECTION,
    ANALYSIS,
    COPY,
    SYNTHESIS,
    PHONG,
    SHOW
  } phase_enum;

typedef enum 
  {
    FBS_SYSTEM_PROVIDED,
    FBS_APPLICATION_CREATED,
    FBS_UNDEFINED
  } framebuffer_state_enum;
