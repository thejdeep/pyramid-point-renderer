#include "object.h"

#define PI 3.14159265

// Conversion from radians to degrees
const double rad_to_deg = 180.0/PI;


GLfloat obj_colors[8][4] = {{0.35, 0.1, 0.1, 0.0},
			    {0.35, 0.1, 0.1, 0.4},
			    {0.1, 0.5, 0.1, 0.2},
			    {0.1, 0.5, 0.1, 0.3},
			    {0.35, 0.1, 0.1, 0.5},
			    {0.1, 0.5, 0.1, 0.6},
			    {0.35, 0.1, 0.1, 0.7},
			    {0.1, 0.5, 0.1, 0.8}};

/**
 * Render object using designed rendering system.
 **/
void Object::render ( void ) {

  glTranslatef(center[0], center[1], center[2]);

    // Convert from quaternion to angle+axis
  double s = 1.0 / sqrt(1 - q_rot.a*q_rot.a);
  double rot[4] = {acos(q_rot.a) * 2.0 * rad_to_deg,
		   q_rot.x * s, q_rot.y * s, q_rot.z * s};
  if (q_rot.a == 1) {
    rot[1] = rot[2] = 0.0; rot[3] = 0.0;
  }
  glRotatef(rot[0], rot[1], rot[2], rot[3]);


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
  else if ( (renderer_type == PYRAMID_TRIANGLES) 
	    || (renderer_type == PYRAMID_HYBRID)
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
void Object::setRendererType ( int type ) { 
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
  else if (renderer_type == PYRAMID_LINES)
    setPyramidLinesDisplayList();
  else if (renderer_type == TRIANGLES)
    setTrianglesDisplayList();
  else if (renderer_type == LINES)
    setLinesDisplayList();
}

/**
 * Create arrays and VBO.
 **/
void Object::setPyramidPointsArrays ( void ) {

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
void Object::setPyramidPointsArraysColor ( void ) {

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
    color_array[pos*4 + 3] = obj_colors[id][0];


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

void Object::setPyramidPointsDisplayList ( void ) {

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
void Object::setPyramidTrianglesDisplayList( void ) {

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
      glVertex4f(p[i].x(), p[i].y(), p[i].z(), 0.000001);
    }
    glEnd();
  }

  glEndList();
}

/**
 * Sets the pyramid lines list.
 **/
void Object::setPyramidLinesDisplayList( void ) {

  triangleDisplayList = glGenLists(1);

  Point p[3];
  Vector n[3];
  double r[3];
  glNewList(triangleDisplayList, GL_COMPILE);
  
  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    p[0] = surfels.at( it->verts[0] ).position();
    p[1] = surfels.at( it->verts[1] ).position();
    p[2] = surfels.at( it->verts[2] ).position();
    n[0] = surfels.at( it->verts[0] ).normal();
    n[1] = surfels.at( it->verts[1] ).normal();
    n[2] = surfels.at( it->verts[2] ).normal();
    r[0] = surfels.at( it->verts[0] ).radius();
    r[1] = surfels.at( it->verts[1] ).radius();
    r[2] = surfels.at( it->verts[2] ).radius();

//     if (id == 0) 
//       {
// 	//compute tangent : line segment representing cylinder
// 	Vector dir = p[1] - p[0];
// 	double length = dir.length();
// 	p[2] = Point((p[0].x() + p[1].x())*0.5, (p[0].y() + p[1].y())*0.5, (p[0].z() + p[1].z())*0.5);
// 	n[2] = Vector((n[0].x() + n[1].x())*0.5, (n[0].y() + n[1].y())*0.5, (n[0].z() + n[1].z())*0.5);
// 	double radius = (surfels.at( it->verts[0] ).radius() + surfels.at( it->verts[1] ).radius()) * 0.5;
// 	double nz = radius / length;
// 	double nxy = sqrt(fabs( (1.0 - nz) / 2.0));
	
// // 	n[2] = Vector (nxy, nxy, nz);
// // 	cout << length << " " << radius << " " << nxy << " " << nz << " " << n[2].length() << endl;

// 	glBegin(GL_POINTS);
// 	glColor4fv(obj_colors[id]);
// 	glNormal3f(-n[2].y(), n[2].x(), nz );
// 	glVertex4f(p[2].x(), p[2].y(), p[2].z(), length*0.5);
	
// 	glEnd();
//       }
//     if (id == 0) 
//       {
// 	//compute tangent : line segment representing cylinder
// 	Vector dir = p[1] - p[0];
// 	double length = dir.length();
// 	n[0].normalize();
// 	n[0] = n[0] * length;

// 	glBegin(GL_POINTS);
// 	glColor4f(obj_colors[id][0], obj_colors[id][1], obj_colors[id][2], obj_cnt/num_objs);
// 	glNormal3f(n[0].x(), n[0].y(), n[0].z() );
// 	glVertex4f(p[0].x(), p[0].y(), p[0].z(), r[0]);
	
// 	glEnd();
//       }
//     else
      {

	p[2] = Point ((p[0].x() + p[1].x())*0.5, (p[0].y() + p[1].y())*0.5, (p[0].z() + p[1].z())*0.5);
 	n[2] = Vector((n[0].x() + n[1].x())*0.5, (n[0].y() + n[1].y())*0.5, (n[0].z() + n[1].z())*0.5);
	r[2] = (r[0] + r[1]) * 0.5;

	glBegin(GL_LINES);
// 	  glColor4fv(obj_colors[id]);

// 	  glNormal3f(n[0].x(), n[0].y(), n[0].z());
// 	  glVertex4f(p[0].x(), p[0].y(), p[0].z(), r[0]);

// 	  glNormal3f(n[2].x(), n[2].y(), n[2].z());
// 	  glVertex4f(p[2].x(), p[2].y(), p[2].z(), r[2]);

// 	  glNormal3f(n[1].x(), n[1].y(), n[1].z());
// 	  glVertex4f(p[1].x(), p[1].y(), p[1].z(), r[1]);

	for (int i = 0; i < 2; ++i) {
	  glColor4fv(obj_colors[id]);
	  //glColor4f(obj_colors[id][0], obj_colors[id][1], obj_colors[id][2], obj_colors[id][3]);
	  glNormal3f(n[i].x(), n[i].y(), n[i].z());
	  glVertex4f(p[i].x(), p[i].y(), p[i].z(), r[i]);
	}
	glEnd();
      }
	
  }

  glEndList();
}

/**
 * Sets the display list, half rendered as triangles
 * and the other half as points.
 **/
void Object::setPyramidHybridDisplayList( void ) {


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
	glColor4fv(obj_colors[0]);
	glNormal3f(n[i].x(), n[i].y(), n[i].z());
	glVertex4f(p[i].x(), p[i].y(), p[i].z(), 0.001);
      }
      glEnd();
    }
  }

  glBegin(GL_POINTS);  
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {
    if (it->position().x() <= 0.001) {
      glColor4fv(obj_colors[1]);
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
void Object::setTrianglesDisplayList( void ) {

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
void Object::setLinesDisplayList( void ) {

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
