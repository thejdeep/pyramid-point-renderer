#include "glslKernel/glslKernel.h"

#include "primitives.h"
#include "point_based_renderer.h"

#include <iostream>
#include <fstream>
#include <assert.h>


Primitives::~Primitives() {
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &color_buffer);
  glDeleteBuffers(1, &normal_buffer);
}

/**
 * Render object using designed rendering system.
 **/
void Primitives::render ( void ) const {

  if (renderer_type == PYRAMID_POINTS) {

    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexPointer(4, GL_FLOAT, 0, NULL);   

    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glNormalPointer(GL_FLOAT, 0, NULL); 
    
    glDrawArrays(GL_POINTS, 0, number_points);   
  }
  else if ((renderer_type == PYRAMID_POINTS_COLOR) ||
      (renderer_type == PYRAMID_TEMPLATES)) {

    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexPointer(4, GL_FLOAT, 0, NULL); 
    
    glEnableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, NULL);

    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glNormalPointer(GL_FLOAT, 0, NULL); 
    
    glDrawArrays(GL_POINTS, 0, number_points);   
  }
  
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  check_for_ogl_error("Pritives render");

}

/**
 * Changes the renderer type.
 * @param rtype Given renderer type.
 **/
void Primitives::setRendererType ( int rtype ) {

  renderer_type = rtype;

//   glDisableClientState(GL_VERTEX_ARRAY);
//   glDisableClientState(GL_COLOR_ARRAY);
//   glDisableClientState(GL_NORMAL_ARRAY);

//   glDeleteBuffers(1, &vertex_buffer);
//   glDeleteBuffers(1, &color_buffer);
//   glDeleteBuffers(1, &normal_buffer);

  if (renderer_type == PYRAMID_POINTS)
    setPyramidPointsArrays();
  else if (renderer_type == PYRAMID_POINTS_COLOR)
    setPyramidPointsArraysColor();
  else if (renderer_type == PYRAMID_TEMPLATES)
    setPyramidPointsArraysColor();
}

/**
 * Create arrays and VBO.
 **/
void Primitives::setPyramidPointsArrays ( void ) {

  GLfloat *vertex_array, *normal_array;
  number_points = surfels.size();
  vertex_array = new GLfloat[number_points * 4];
  normal_array = new GLfloat[number_points * 3];

  int pos = 0;
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {

    vertex_array[pos*4 + 0] = (GLfloat)(it->Center().x);
    vertex_array[pos*4 + 1] = (GLfloat)(it->Center().y);
    vertex_array[pos*4 + 2] = (GLfloat)(it->Center().z);
    vertex_array[pos*4 + 3] = (GLfloat)(it->Radius());

    normal_array[pos*3 + 0] = (GLfloat)(it->Normal().x);
    normal_array[pos*3 + 1] = (GLfloat)(it->Normal().y);
    normal_array[pos*3 + 2] = (GLfloat)(it->Normal().z);

    ++pos;
  }
  
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 4 * sizeof(float), (const void*)vertex_array, GL_STATIC_DRAW);
  delete [] vertex_array;

  glGenBuffers(1, &normal_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 3 * sizeof(float), (const void*)normal_array, GL_STATIC_DRAW);
  delete [] normal_array;
}

/**
 * Create arrays and VBO for pyramid point renderer color class.
 **/
void Primitives::setPyramidPointsArraysColor ( void ) {

  GLfloat *vertex_array, *normal_array;
  GLubyte *color_array;
  number_points = surfels.size();
  vertex_array = new GLfloat[number_points * 4];
  normal_array = new GLfloat[number_points * 3];
  color_array = new GLubyte[number_points * 4];

  int pos = 0;
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {

    vertex_array[pos*4 + 0] = (GLfloat)(it->Center().x);
    vertex_array[pos*4 + 1] = (GLfloat)(it->Center().y);
    vertex_array[pos*4 + 2] = (GLfloat)(it->Center().z);
    vertex_array[pos*4 + 3] = (GLfloat)(it->Radius());

	color_array[pos*4 + 0] = (GLubyte)(it->color()[0] * 255.0);
	color_array[pos*4 + 1] = (GLubyte)(it->color()[1] * 255.0);
	color_array[pos*4 + 2] = (GLubyte)(it->color()[2] * 255.0);

// 	color_array[pos*4 + 0] = (GLfloat)(it->color()[0]);
// 	color_array[pos*4 + 1] = (GLfloat)(it->color()[1]);
// 	color_array[pos*4 + 2] = (GLfloat)(it->color()[2]);

	color_array[pos*4 + 3] = 1.0;

    normal_array[pos*3 + 0] = (GLfloat)(it->Normal().x);
    normal_array[pos*3 + 1] = (GLfloat)(it->Normal().y);
    normal_array[pos*3 + 2] = (GLfloat)(it->Normal().z);

    ++pos;
  }

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 4 * sizeof(GLfloat), (const void*)vertex_array, GL_STATIC_DRAW);

  glGenBuffers(1, &color_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 4 * sizeof(GLubyte), (const void*)color_array, GL_STATIC_DRAW);

  glGenBuffers(1, &normal_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 3 * sizeof(GLfloat), (const void*)normal_array, GL_STATIC_DRAW);

  delete [] vertex_array;
  delete [] color_array;
  delete [] normal_array;

  check_for_ogl_error("Array Buffers Creation");
}

void Primitives::clearSurfels ( void ) {  
  surfels.clear();
}
