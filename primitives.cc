#include "primitives.h"

#define PI 3.14159265

// Conversion from radians to degrees
const double rad_to_deg = 180.0/PI;

// GLfloat obj_colors[8][4] = {{0.35, 0.1, 0.1, 0.0},
// 			    {0.35, 0.1, 0.1, 0.4},
// 			    {0.1, 0.5, 0.1, 0.2},
// 			    {0.8, 0.7, 0.2, 0.3},
// 			    {0.35, 0.1, 0.1, 0.5},
// 			    {0.1, 0.5, 0.1, 0.6},
// 			    {0.35, 0.1, 0.1, 0.7},
// 			    {0.1, 0.5, 0.1, 0.8}};

GLfloat obj_colors[8][4] = {{0.3, 0.1, 0.1, 1.0},
			    {0.1, 0.1, 0.3, 1.0},
			    {0.3, 0.1, 0.1, 1.0},
			    {0.8, 0.7, 0.2, 0.3},
			    {0.35, 0.1, 0.1, 0.5},
			    {0.1, 0.5, 0.1, 0.6},
			    {0.35, 0.1, 0.1, 0.7},
			    {0.1, 0.5, 0.1, 0.8}};

/**
 * Render object using designed rendering system.
 **/
void Primitives::render ( void ) {

  if (renderer_type == PYRAMID_POINTS) {
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

    //glCallList(triangleDisplayList);
  }
  if (renderer_type == PYRAMID_LINES) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexPointer(4, GL_FLOAT, 0, NULL); 
    
    glEnableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glColorPointer(4, GL_FLOAT, 0, NULL);

    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glNormalPointer(GL_FLOAT, 0, NULL); 
    
    glDrawElements(GL_LINES, number_triangles*2, GL_UNSIGNED_INT, &indices[0]);

    //glCallList(triangleDisplayList);
  }
  else if ( (renderer_type == PYRAMID_TRIANGLES) 
	    || (renderer_type == PYRAMID_HYBRID)
	    || (renderer_type == PYRAMID_HYBRID_TEST)
	    || (renderer_type == PYRAMID_LINES)) {
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
  else if (renderer_type == LINES) {
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
      
    glShadeModel(GL_SMOOTH);
    
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    glLineWidth(2.0);

    glCallList(triangleDisplayList);

    glLineWidth(1.0);
  }
}

/**
 * Changes the renderer type.
 **/
void Primitives::setRendererType ( int type ) { 
  renderer_type = type; 

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &color_buffer);
  glDeleteBuffers(1, &normal_buffer);
  glDeleteLists(triangleDisplayList, 1);

  if (renderer_type == PYRAMID_POINTS)
    setPyramidPointsArraysColor();
  else if (renderer_type == PYRAMID_POINTS_COLOR)
    setPyramidPointsDisplayList();
  else if (renderer_type == PYRAMID_TRIANGLES)
    setPyramidTrianglesDisplayList();
  else if (renderer_type == PYRAMID_HYBRID)
    setPyramidHybridDisplayList();
  else if (renderer_type == PYRAMID_HYBRID_TEST)
    setPyramidHybridTestDisplayList();
  else if (renderer_type == PYRAMID_LINES)
    setPyramidLinesArraysColor();
  //    setPyramidLinesDisplayList();
  else if (renderer_type == TRIANGLES)
    setTrianglesDisplayList();
  else if (renderer_type == LINES)
    setLinesDisplayList();
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
 * Create arrays and VBO.
 **/
void Primitives::setPyramidPointsArraysColor ( void ) {

  type = 1.0;

  GLfloat *vertex_array, *normal_array, *color_array;
  number_points = surfels.size();
  vertex_array = new GLfloat[number_points * 4];
  normal_array = new GLfloat[number_points * 3];
  color_array = new GLfloat[number_points * 4];

  int pos = 0;
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {

    vertex_array[pos*4 + 0] = (GLfloat)(it->position().x());
    vertex_array[pos*4 + 1] = (GLfloat)(it->position().y());
    vertex_array[pos*4 + 2] = (GLfloat)(it->position().z());
    vertex_array[pos*4 + 3] = (GLfloat)(it->radius());

//     color_array[pos*4 + 0] = (GLfloat)(it->color().x());
//     color_array[pos*4 + 1] = (GLfloat)(it->color().y());
//     color_array[pos*4 + 2] = (GLfloat)(it->color().z());
//     color_array[pos*4 + 3] = (GLfloat)(it->color().w());


    color_array[pos*4 + 0] = obj_colors[id][0];
    color_array[pos*4 + 1] = obj_colors[id][1];
    color_array[pos*4 + 2] = obj_colors[id][2];
    color_array[pos*4 + 3] = type;


    normal_array[pos*3 + 0] = (GLfloat)(it->normal().x());
    normal_array[pos*3 + 1] = (GLfloat)(it->normal().y());
    normal_array[pos*3 + 2] = (GLfloat)(it->normal().z());

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

void Primitives::setPyramidPointsDisplayList ( void ) {

  triangleDisplayList = glGenLists(1);

  glNewList(triangleDisplayList, GL_COMPILE);

  glBegin(GL_POINTS);
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {
    glColor4fv(obj_colors[id]);
    glNormal3f(it->normal().x(), it->normal().y(), it->normal().z());
    glVertex4f(it->position().x(), it->position().y(), it->position().z(), it->radius());
  }
  glEnd();

  glEndList();
}

/**
 * Sets the pyramid triangles list.
 **/
void Primitives::setPyramidTrianglesDisplayList( void ) {

  type = 0.1;

  triangleDisplayList = glGenLists(1);

  Point p[3];
  Vector n[3];
  glNewList(triangleDisplayList, GL_COMPILE);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    p[0] = surfels.at( it->verts[0] ).position();
    p[1] = surfels.at( it->verts[1] ).position();
    p[2] = surfels.at( it->verts[2] ).position();
    n[0] = surfels.at( it->verts[0] ).normal();
    n[1] = surfels.at( it->verts[1] ).normal();
    n[2] = surfels.at( it->verts[2] ).normal();

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 3; ++i) {
      //      glColor4fv(obj_colors[id]);
      glColor4f(obj_colors[id][0], obj_colors[id][1], obj_colors[id][2], type);
      glNormal3f(n[i].x(), n[i].y(), n[i].z());
      glVertex4f(p[i].x(), p[i].y(), p[i].z(), 0.0001);
    }
    glEnd();
  }

  glEndList();
}

/**
 * Create arrays and VBO for pyramid line rendering.
 **/
void Primitives::setPyramidLinesArraysColor ( void ) {

  GLfloat *vertex_array, *normal_array, *color_array;
  number_points = surfels.size();
  vertex_array = new GLfloat[number_points * 4];
  normal_array = new GLfloat[number_points * 3];
  color_array = new GLfloat[number_points * 4];

  number_triangles = triangles.size();
  indices = new GLuint[number_triangles*2];

  int pos = 0;
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {

    vertex_array[pos*4 + 0] = (GLfloat)(it->position().x());
    vertex_array[pos*4 + 1] = (GLfloat)(it->position().y());
    vertex_array[pos*4 + 2] = (GLfloat)(it->position().z());
    vertex_array[pos*4 + 3] = (GLfloat)(it->radius());

    color_array[pos*4 + 0] = obj_colors[id][0];
    color_array[pos*4 + 1] = obj_colors[id][1];
    color_array[pos*4 + 2] = obj_colors[id][2];
    color_array[pos*4 + 3] = obj_colors[id][0];

    normal_array[pos*3 + 0] = (GLfloat)(it->normal().x());
    normal_array[pos*3 + 1] = (GLfloat)(it->normal().y());
    normal_array[pos*3 + 2] = (GLfloat)(it->normal().z());

    ++pos;
  }

  int cnt = 0;
  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    indices[cnt] = (GLuint)it->verts[0];
    ++cnt;
    indices[cnt] = (GLuint)it->verts[1];
    ++cnt;
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

/**
 * Sets the pyramid lines list.
 **/
void Primitives::setPyramidLinesDisplayList( void ) {

  triangleDisplayList = glGenLists(1);

  Point p[3];
  Vector n[3];
  double r[3];
  glNewList(triangleDisplayList, GL_COMPILE);
//   GLfloat obj_id = 0.0;
//   GLfloat max_tris = triangles.size() + 1.0;

  glLineWidth(1.0);
  glDisable(GL_LINE_SMOOTH);
  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    r[0] = surfels.at( it->verts[0] ).radius();
    p[0] = surfels.at( it->verts[0] ).position();
    n[0] = surfels.at( it->verts[0] ).normal();
    r[1] = surfels.at( it->verts[1] ).radius();
    p[1] = surfels.at( it->verts[1] ).position();
    n[1] = surfels.at( it->verts[1] ).normal();
//     p[2] = surfels.at( it->verts[2] ).position();
//     n[2] = surfels.at( it->verts[2] ).normal();
//     r[2] = surfels.at( it->verts[2] ).radius();
//     p[2] = Point ((p[0].x() + p[1].x())*0.5, (p[0].y() + p[1].y())*0.5, (p[0].z() + p[1].z())*0.5);
//     n[2] = Vector((n[0].x() + n[1].x())*0.5, (n[0].y() + n[1].y())*0.5, (n[0].z() + n[1].z())*0.5);
//     r[2] = (r[0] + r[1]) * 0.5;

    glBegin(GL_LINES);
    for (int i = 0; i < 2; ++i) {
      //glColor4fv(obj_colors[id]);
      glColor4f(obj_colors[id][0], obj_colors[id][1], obj_colors[id][2], type);
      glNormal3f(n[i].x(), n[i].y(), n[i].z());
      glVertex4f(p[i].x(), p[i].y(), p[i].z(), r[i]);
    }
    glEnd();
  }

  glEndList();
}

/**
 * Sets the display list, half rendered as triangles
 * and the other half as points.
 **/
void Primitives::setPyramidHybridDisplayList( void ) {

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

    if ((p[0].x() > 0.0) || (p[1].x() > 0.0) || (p[2].x() > 0.0)) {


      glBegin(GL_TRIANGLES);
      for (int i = 0; i < 3; ++i) {
	//glColor4fv(obj_colors[0]);
	glColor4f(obj_colors[1][0], obj_colors[1][1], obj_colors[1][2], 0.1);
	glNormal3f(n[i].x(), n[i].y(), n[i].z());
	glVertex4f(p[i].x(), p[i].y(), p[i].z(), 0.00001);
      }
      glEnd();
    }
  }

  glBegin(GL_POINTS);  
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {
    if (it->position().x() <= 0.001) 
      {
      //      glColor4fv(obj_colors[1]);
      glColor4f(obj_colors[0][0], obj_colors[0][1], obj_colors[0][2], 1.0);
      glNormal3f(it->normal().x(), it->normal().y(), it->normal().z());
      glVertex4f(it->position().x(), it->position().y(), it->position().z(), it->radius());
    }
  }
  glEnd();

  glEndList();
}


/**
 * Sets the display list, half rendered as triangles
 * and the other half as points.
 **/
void Primitives::setPyramidHybridTestDisplayList( void ) {

  triangleDisplayList = glGenLists(1);

  Point p[3];
  Vector n[3];
  glNewList(triangleDisplayList, GL_COMPILE);
  
//   glPolygonMode(GL_BACK, GL_LINE);
//   glPolygonMode(GL_FRONT, GL_FILL);

//  glEnable(GL_CULL_FACE);

   glEnable(GL_LINE_SMOOTH);

  // Half as triangles.
  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    p[0] = surfels.at( it->verts[0] ).position();
    p[1] = surfels.at( it->verts[1] ).position();
    p[2] = surfels.at( it->verts[2] ).position();
    n[0] = surfels.at( it->verts[0] ).normal();
    n[1] = surfels.at( it->verts[1] ).normal();
    n[2] = surfels.at( it->verts[2] ).normal();

    if ((p[0].x() > 0.0) || (p[1].x() > 0.0) || (p[2].x() > 0.0)) {

      glLineWidth(5.0);
      glBegin(GL_LINE_LOOP);
//      glBegin(GL_TRIANGLES);
      for (int i = 0; i < 3; ++i) {
	//glColor4fv(obj_colors[0]);
	glColor4f(obj_colors[1][0], obj_colors[1][1], obj_colors[1][2], 0.1);
	glNormal3f(n[i].x(), n[i].y(), n[i].z());
	glVertex4f(p[i].x(), p[i].y(), p[i].z(), 0.0001);
      }
      glEnd();
    }
  }

  glBegin(GL_POINTS);  
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {
    if (it->position().x() <= 0.001) 
      {
      //      glColor4fv(obj_colors[1]);
      glColor4f(obj_colors[0][0], obj_colors[0][1], obj_colors[0][2], 1.0);
      glNormal3f(it->normal().x(), it->normal().y(), it->normal().z());
      glVertex4f(it->position().x(), it->position().y(), it->position().z(), it->radius());
    }
  }
  glEnd();

  glEndList();
}


/**
 * Sets the triangles list.
 **/
void Primitives::setTrianglesDisplayList( void ) {

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
      glColor4fv(obj_colors[id]);
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
void Primitives::setLinesDisplayList( void ) {

  triangleDisplayList = glGenLists(1);

  Point p[3];
  Vector n[3];
  glNewList(triangleDisplayList, GL_COMPILE);

  glPointSize(5.0);
  
  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    p[0] = surfels.at( it->verts[0] ).position();
    p[1] = surfels.at( it->verts[1] ).position();
    p[2] = surfels.at( it->verts[2] ).position();
    n[0] = surfels.at( it->verts[0] ).normal();
    n[1] = surfels.at( it->verts[1] ).normal();
    n[2] = surfels.at( it->verts[2] ).normal();

    glBegin(GL_LINES);
    for (int i = 0; i < 3; ++i) {
      glColor4fv(obj_colors[0]);
      glNormal3f(n[i].x(), n[i].y(), n[i].z());
      glVertex4f(p[i].x(), p[i].y(), p[i].z(), 1.0);
    }
    glEnd();

    glBegin(GL_POINTS);
    for (int i = 0; i < 3; ++i) {
      glColor4fv(obj_colors[1]);
      glNormal3f(n[i].x(), n[i].y(), n[i].z());
      glVertex4f(p[i].x(), p[i].y(), p[i].z(), 1.0);
    }
    glEnd();
  }

  glPointSize(1.0);

  glEndList();
}
