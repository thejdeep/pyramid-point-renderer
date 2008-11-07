#include "glslKernel/glslKernel.h"

#include "primitives.h"

#include <iostream>
#include <fstream>
#include <assert.h>

#define NUM_MATERIALS 6

GLfloat obj_colors[8][4] = {{0.0, 1.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0}};


/**
 * Render object using designed rendering system.
 **/
void Primitives::render ( void ) const {

  if ((renderer_type == PYRAMID_POINTS) ||
      (renderer_type == PYRAMID_POINTS_ER)) {

    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexPointer(4, GL_FLOAT, 0, NULL);   

    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glNormalPointer(GL_FLOAT, 0, NULL); 
    
    glDrawArrays(GL_POINTS, 0, number_points);   
  }
  else if ((renderer_type == PYRAMID_POINTS_COLOR)) {

    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexPointer(4, GL_FLOAT, 0, NULL); 
    
    glEnableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glColorPointer(4, GL_FLOAT, 0, NULL);

    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glNormalPointer(GL_FLOAT, 0, NULL); 
    
    glDrawArrays(GL_POINTS, 0, number_points);   
  }

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
}

/**
 * Changes the renderer type.
 **/
//void Primitives::setRendererType ( point_render_type_enum type ) {
void Primitives::setRendererType ( int rtype ) {

  number_points = surfels.size();
  number_triangles = triangles.size();

  renderer_type = rtype;

  if (renderer_type == NONE)
    return;

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &color_buffer);
  glDeleteBuffers(1, &normal_buffer);

  if (renderer_type == PYRAMID_POINTS)
    setPyramidPointsArrays();
  else if (renderer_type == PYRAMID_POINTS_COLOR)
    setPyramidPointsArraysColor();
  else if (renderer_type == PYRAMID_POINTS_ER)
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

  glGenBuffers(1, &normal_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 3 * sizeof(float), (const void*)normal_array, GL_STATIC_DRAW);

  delete(vertex_array);
  delete(normal_array);

  glEnableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glVertexPointer(4, GL_FLOAT, 0, NULL); 
  
  glEnableClientState(GL_NORMAL_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
  glNormalPointer(GL_FLOAT, 0, NULL); 
}

/**
 * Create arrays and VBO for pyramid point renderer color class.
 **/
void Primitives::setPyramidPointsArraysColor ( void ) {

  GLfloat *vertex_array, *normal_array, *color_array;
  number_points = surfels.size();
  vertex_array = new GLfloat[number_points * 4];
  normal_array = new GLfloat[number_points * 3];
  color_array = new GLfloat[number_points * 4];

  int pos = 0;
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {

    vertex_array[pos*4 + 0] = (GLfloat)(it->Center().x);
    vertex_array[pos*4 + 1] = (GLfloat)(it->Center().y);
    vertex_array[pos*4 + 2] = (GLfloat)(it->Center().z);
    vertex_array[pos*4 + 3] = (GLfloat)(it->Radius());
    
    if (color_model) {
      color_array[pos*4 + 0] = (GLfloat)(it->color()[0]);
      color_array[pos*4 + 1] = (GLfloat)(it->color()[1]);
      color_array[pos*4 + 2] = (GLfloat)(it->color()[2]);
    }
    else {
      color_array[pos*4 + 0] = obj_colors[id][0];
      color_array[pos*4 + 1] = obj_colors[id][1];
      color_array[pos*4 + 2] = obj_colors[id][2];
    }
    //color_array[pos*4 + 3] = type;
    color_array[pos*4 + 3] = material / (GLfloat)NUM_MATERIALS;

    normal_array[pos*3 + 0] = (GLfloat)(it->Normal().x);
    normal_array[pos*3 + 1] = (GLfloat)(it->Normal().y);
    normal_array[pos*3 + 2] = (GLfloat)(it->Normal().z);

    ++pos;
  }
  
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 4 * sizeof(float), (const void*)vertex_array, GL_STATIC_DRAW);

  glGenBuffers(1, &color_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 4 * sizeof(float), (const void*)color_array, GL_STATIC_DRAW);

  glGenBuffers(1, &normal_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
  glBufferData(GL_ARRAY_BUFFER, number_points * 3 * sizeof(float), (const void*)normal_array, GL_STATIC_DRAW);

  delete(vertex_array);
  delete(normal_array);
  delete(color_array);

  glEnableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glVertexPointer(4, GL_FLOAT, 0, NULL); 

  glEnableClientState(GL_COLOR_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
  glColorPointer(4, GL_FLOAT, 0, NULL); 

  glEnableClientState(GL_NORMAL_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
  glNormalPointer(GL_FLOAT, 0, NULL); 
}

