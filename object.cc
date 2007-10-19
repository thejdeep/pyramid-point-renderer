#include "object.h"

/**
 * Render object using designed rendering system.
 **/
void Object::render ( void ) {

  if (renderer_type == PYRAMID_POINTS) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexPointer(4, GL_FLOAT, 0, NULL); 
    
    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glNormalPointer(GL_FLOAT, 0, NULL); 
    
    glDrawArrays(GL_POINTS, 0, number_points);

  }
  else if ( (renderer_type == PYRAMID_TRIANGLES) 
	    || (renderer_type == PYRAMID_HYBRID)) {
    glCallList(triangleDisplayList);

  }
  else if (renderer_type == TRIANGLES) {
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    
    //    glEnable(GL_CULL_FACE);
    
    glShadeModel(GL_SMOOTH);
    
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    glCallList(triangleDisplayList);
  }
}

/**
 * Changes the renderer type.
 **/
void Object::setRendererType ( int type ) { 
  renderer_type = type; 

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &normal_buffer);
  glDeleteLists(triangleDisplayList, 1);

  if (renderer_type == PYRAMID_POINTS)
    setArrays();
  else if (renderer_type == PYRAMID_TRIANGLES)
    setDisplayList();
  else if (renderer_type == PYRAMID_HYBRID)
    setHybridDisplayList();
  else if (renderer_type == TRIANGLES)
    setTriangleDisplayList();
}

/**
 * Create arrays and VBO.
 **/
void Object::setArrays ( void ) {

  GLfloat *vertex_array, *normal_array;
  number_points = surfels.size();
  vertex_array = new GLfloat[number_points * 4];
  normal_array = new GLfloat[number_points * 3];

  int pos = 0;
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {

    vertex_array[pos*4 + 0] = (GLfloat)(it->position().x());
    vertex_array[pos*4 + 1] = (GLfloat)(it->position().y());
    vertex_array[pos*4 + 2] = (GLfloat)(it->position().z());
    vertex_array[pos*4 + 3] = (GLfloat)(it->radius());

    normal_array[pos*3 + 0] = (GLfloat)(it->normal().x());
    normal_array[pos*3 + 1] = (GLfloat)(it->normal().y());
    normal_array[pos*3 + 2] = (GLfloat)(it->normal().z());

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
 * Sets the triangles list.
 **/
void Object::setTriangleDisplayList( void ) {

  triangleDisplayList = glGenLists(1);

  Point p[3];
  Vector n[3];
  glNewList(triangleDisplayList, GL_COMPILE);
  
  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    p[0] = surfels.at( it->verts[0] ).position();
    p[1] = surfels.at( it->verts[1] ).position();
    p[2] = surfels.at( it->verts[2] ).position();
    n[0] = surfels.at( it->verts[0] ).normal();
    n[1] = surfels.at( it->verts[1] ).normal();
    n[2] = surfels.at( it->verts[2] ).normal();

    glBegin(GL_TRIANGLES);  
    for (int i = 0; i < 3; ++i) {
      glNormal3f(n[i].x(), n[i].y(), n[i].z());
      glVertex4f(p[i].x(), p[i].y(), p[i].z(), 1.0);
    }
    glEnd();
  }

  glEndList();
}

/**
 * Sets the triangles list.
 **/
void Object::setDisplayList( void ) {

  triangleDisplayList = glGenLists(1);

  Point p[3];
  Vector n[3];
  glNewList(triangleDisplayList, GL_COMPILE);
  
  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    p[0] = surfels.at( it->verts[0] ).position();
    p[1] = surfels.at( it->verts[1] ).position();
    p[2] = surfels.at( it->verts[2] ).position();
    n[0] = surfels.at( it->verts[0] ).normal();
    n[1] = surfels.at( it->verts[1] ).normal();
    n[2] = surfels.at( it->verts[2] ).normal();

    glBegin(GL_TRIANGLES);  
    for (int i = 0; i < 3; ++i) {
      glNormal3f(n[i].x(), n[i].y(), n[i].z());
      glVertex4f(p[i].x(), p[i].y(), p[i].z(), 0.000001);
    }
    glEnd();
  }

  glEndList();
}

/**
 * Sets the display list, half rendered as triangles
 * and the other half as points.
 **/
void Object::setHybridDisplayList( void ) {


  triangleDisplayList = glGenLists(1);

  Point p[3];
  Vector n[3];
  glNewList(triangleDisplayList, GL_COMPILE);
  
  // Half as triangles.
  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    p[0] = surfels.at( it->verts[0] ).position();
    p[1] = surfels.at( it->verts[1] ).position();
    p[2] = surfels.at( it->verts[2] ).position();
    n[0] = surfels.at( it->verts[0] ).normal();
    n[1] = surfels.at( it->verts[1] ).normal();
    n[2] = surfels.at( it->verts[2] ).normal();

    if ((p[0].y() > 0.0) || (p[1].y() > 0.0) || (p[2].y() > 0.0)) {

      glBegin(GL_TRIANGLES);  
      for (int i = 0; i < 3; ++i) {
	glNormal3f(n[i].x(), n[i].y(), n[i].z());
	glVertex4f(p[i].x(), p[i].y(), p[i].z(), 0.000001);
      }
      glEnd();
    }
  }
  glBegin(GL_POINTS);  
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {
    if (it->position().y() <= 0.001) {    
      glNormal3f(it->normal().x(), it->normal().y(), it->normal().z());
      glVertex4f(it->position().x(), it->position().y(), it->position().z(), it->radius());
    }
  }

  
  glEnd();

  glEndList();
}
