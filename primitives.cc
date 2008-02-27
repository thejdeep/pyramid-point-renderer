#include "glslKernel.h"

#include "primitives.h"

#include <iostream>
#include <fstream>
#include <assert.h>

#define PI 3.14159265

// Conversion from radians to degrees
const double rad_to_deg = 180.0/PI;

#define NUM_MATERIALS 5

GLfloat obj_colors[8][4] = {{0.0, 0.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0},
			    {0.0, 0.0, 0.0, 1.0}};

// For apple tree
// GLfloat obj_colors[8][4] = {{0.2, 1.0, 0.2, 1.0},
// 			    {0.7, 0.2, 0.2, 0.1},
// 			    {0.7, 0.2, 0.2, 0.1},
// 			    {0.2, 1.0, 0.2, 1.0},
// 			    {0.7, 0.2, 0.2, 1.0},
// 			    {0.7, 0.2, 0.2, 1.0},
// 			    {0.1, 0.5, 0.1, 1.0},
// 			    {0.35, 0.1, 0.1, 0.7}};

// GLfloat obj_colors[8][4] = {{0.3, 0.1, 0.1, 1.0},
// 			    {0.1, 0.1, 0.3, 1.0},
// 			    {0.3, 0.1, 0.1, 1.0},
// 			    {0.8, 0.7, 0.2, 0.3},
// 			    {0.35, 0.1, 0.1, 0.5},
// 			    {0.1, 0.5, 0.1, 0.6},
// 			    {0.35, 0.1, 0.1, 0.7},
// 			    {0.1, 0.5, 0.1, 0.8}};

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
  else if (renderer_type == PYRAMID_POINTS_LOD) {

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, surfels_per_level_patches_buffer);
    glTexCoordPointer(4, GL_INT, 0, NULL);

    glEnableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, normal_patches_buffer);
    glColorPointer(4, GL_FLOAT, 0, NULL);

    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_patches_buffer);
    glVertexPointer(4, GL_FLOAT, 0, NULL);

    glActiveTexture( GL_TEXTURE6 );
    glGenTextures(1, &vertTextBufferObject);
    glBindTexture(GL_TEXTURE_BUFFER_EXT, vertTextBufferObject);
    glTexBufferEXT(GL_TEXTURE_BUFFER_EXT, GL_RGBA32F_ARB, vertex_buffer);

    glActiveTexture( GL_TEXTURE7 );
    glGenTextures(1, &normalTextBufferObject);
    glBindTexture(GL_TEXTURE_BUFFER_EXT, normalTextBufferObject);
    glTexBufferEXT(GL_TEXTURE_BUFFER_EXT, GL_RGBA32F_ARB, normal_buffer);

    glDrawArrays(GL_POINTS, 0, numPatches);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
  }
  else if (renderer_type == PYRAMID_LINES) {

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

  }
  else if (renderer_type == PYRAMID_TRIANGLES) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexPointer(4, GL_FLOAT, 0, NULL); 
    
    glEnableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
    glColorPointer(4, GL_FLOAT, 0, NULL);

    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
    glNormalPointer(GL_FLOAT, 0, NULL); 
    
    glDrawElements(GL_TRIANGLES, number_triangles*3, GL_UNSIGNED_INT, &indices[0]);
  }
  else if ( (renderer_type == PYRAMID_TRIANGLES) 
	    || (renderer_type == PYRAMID_HYBRID)
	    || (renderer_type == PYRAMID_HYBRID_TEST)
	    || (renderer_type == PYRAMID_LINES)) {
    glCallList(triangleDisplayList);

  }
//   else if (renderer_type == TRIANGLES) {
//     glEnableClientState(GL_VERTEX_ARRAY);
//     glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
//     glVertexPointer(4, GL_FLOAT, 0, NULL); 
    
//     glEnableClientState(GL_COLOR_ARRAY);
//     glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
//     glColorPointer(4, GL_FLOAT, 0, NULL);

//     glEnableClientState(GL_NORMAL_ARRAY);
//     glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
//     glNormalPointer(GL_FLOAT, 0, NULL); 
    
//     glDisable(GL_BLEND);
//     glEnable(GL_DEPTH_TEST);
//     glDepthMask(GL_TRUE);
    
//     glDisable(GL_CULL_FACE);

//     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
//     glShadeModel(GL_SMOOTH);
    
//     glColor4f(1.0, 1.0, 1.0, 1.0);

//     glDrawElements(GL_TRIANGLES, number_triangles*3, GL_UNSIGNED_INT, &indices[0]);

//   }
  else if (renderer_type == TRIANGLES) {

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
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
    
    glLineWidth(1.0);

    glCallList(triangleDisplayList);
  }
}

/**
 * Changes the renderer type.
 **/
//void Primitives::setRendererType ( point_render_type_enum type ) {
void Primitives::setRendererType ( int rtype ) {

  number_points = surfels[0].size();
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
  glDeleteLists(triangleDisplayList, 1);

  if (renderer_type == PYRAMID_POINTS)
    setPyramidPointsArraysColor();
  if (renderer_type == PYRAMID_POINTS_LOD) {
    setPyramidPointsArraysLOD();
    setPatchesArray();
  }
  else if (renderer_type == PYRAMID_POINTS_COLOR)
    setPyramidPointsDisplayList();
  else if (renderer_type == PYRAMID_TRIANGLES)
    setPyramidTrianglesArrays();
    //setPyramidTrianglesDisplayList();
  else if (renderer_type == PYRAMID_HYBRID)
    setPyramidHybridDisplayList();
  else if (renderer_type == PYRAMID_HYBRID_TEST)
    setPyramidHybridTestDisplayList();
  else if (renderer_type == PYRAMID_LINES)
    setPyramidLinesArraysColor();
    //setPyramidLinesDisplayList();
  else if (renderer_type == TRIANGLES)
    //setTrianglesArrays();
    setTrianglesDisplayList();
  else if (renderer_type == LINES)
    setLinesDisplayList();

}

/**
 * Create arrays and VBO.
 **/
void Primitives::setPyramidPointsArrays ( void ) {

  GLfloat *vertex_array, *normal_array;
  number_points = surfels[0].size();
  vertex_array = new GLfloat[number_points * 4];
  normal_array = new GLfloat[number_points * 3];

  int pos = 0;
  for (surfelVectorIter it = surfels[0].begin(); it != surfels[0].end(); ++it) {

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

  GLfloat *vertex_array, *normal_array, *color_array;
  number_points = surfels[0].size();
  vertex_array = new GLfloat[number_points * 4];
  normal_array = new GLfloat[number_points * 3];
  color_array = new GLfloat[number_points * 4];

  int pos = 0;
  for (surfelVectorIter it = surfels[0].begin(); it != surfels[0].end(); ++it) {

    vertex_array[pos*4 + 0] = (GLfloat)(it->position().x());
    vertex_array[pos*4 + 1] = (GLfloat)(it->position().y());
    vertex_array[pos*4 + 2] = (GLfloat)(it->position().z());
    vertex_array[pos*4 + 3] = (GLfloat)(it->radius());

    if (color_model) {
      color_array[pos*4 + 0] = (GLfloat)(it->color().x());
      color_array[pos*4 + 1] = (GLfloat)(it->color().y());
      color_array[pos*4 + 2] = (GLfloat)(it->color().z());
    }
    else {
      color_array[pos*4 + 0] = obj_colors[id][0];
      color_array[pos*4 + 1] = obj_colors[id][1];
      color_array[pos*4 + 2] = obj_colors[id][2];
    }
    //    color_array[pos*4 + 3] = type;
    color_array[pos*4 + 3] = material / (GLfloat)NUM_MATERIALS;

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

void Primitives::countNumVertsLOD ( int *spl ) {

  double error;
  double sin_alpha, cos_alpha;
  Point p;
  Vector n;
  double d;
  double epsilon = 0.006;
  //double epsilon = 0.02;

  if (renderer_type != PYRAMID_POINTS_LOD) {
    spl[3] += surfels[0].size();
    spl[4] += spl[3];
  }
  else {

    for (uint i = 0; i < numPatches; ++i) {
      error = surfels[LOD_LEVELS-1][i].perpendicularError();
      p = surfels[LOD_LEVELS-1][i].position();
      n = surfels[LOD_LEVELS-1][i].normal();
      
      cos_alpha = (p - eye).normalize() * n;
      d = (p - eye).length();
      sin_alpha = sqrt( 1.0 - cos_alpha*cos_alpha);
      error *= sin_alpha / d;
      
      if (error < epsilon)
	spl[0] += 1;
      else if (error < 4.0 * epsilon)
	spl[1] += surfels_per_level[i*4 + 1];
      else if (error < 8.0 * epsilon)
	spl[2] += surfels_per_level[i*4 + 2];
      else
	spl[3] += surfels_per_level[i*4 + 3];
    }
    spl[4] += numPatches + numVertsArray;
  }

  

}

/**
 * Create arrays and VBO.
 **/
void Primitives::setPyramidPointsArraysLOD ( void ) {

  GLfloat *vertex_array, *normal_array;

  vertex_array = new GLfloat[numVertsArray * 4];
  normal_array = new GLfloat[numVertsArray * 4];

  uint pos = 0;
  for (surfelVectorIter it = surfels_lod.begin(); it != surfels_lod.end(); ++it) {

    vertex_array[pos*4 + 0] = (GLfloat)(it->position().x());
    vertex_array[pos*4 + 1] = (GLfloat)(it->position().y());
    vertex_array[pos*4 + 2] = (GLfloat)(it->position().z());
    vertex_array[pos*4 + 3] = (GLfloat)(it->radius());

    normal_array[pos*4 + 0] = (GLfloat)(it->normal().x());
    normal_array[pos*4 + 1] = (GLfloat)(it->normal().y());
    normal_array[pos*4 + 2] = (GLfloat)(it->normal().z());
    normal_array[pos*4 + 3] = 1.0;

    ++pos;
  }
  
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, numVertsArray * 4 * sizeof(GLfloat), (const void*)vertex_array, GL_STATIC_DRAW);

  glEnableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glVertexPointer(4, GL_FLOAT, 0, NULL); 

  glActiveTexture( GL_TEXTURE6 );
  glGenTextures(1, &vertTextBufferObject);
  glBindTexture(GL_TEXTURE_BUFFER_EXT, vertTextBufferObject);
  glTexBufferEXT(GL_TEXTURE_BUFFER_EXT, GL_RGBA32F_ARB, vertex_buffer);

  glGenBuffers(1, &normal_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
  glBufferData(GL_ARRAY_BUFFER, numVertsArray * 4 * sizeof(GLfloat), (const void*)normal_array, GL_STATIC_DRAW);
  
  glEnableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
  glVertexPointer(4, GL_FLOAT, 0, NULL);

  glActiveTexture( GL_TEXTURE7 );
  glGenTextures(1, &normalTextBufferObject);
  glBindTexture(GL_TEXTURE_BUFFER_EXT, normalTextBufferObject);
  glTexBufferEXT(GL_TEXTURE_BUFFER_EXT, GL_RGBA32F_ARB, normal_buffer);

  delete [] vertex_array;
  delete [] normal_array;
}

void Primitives::setPatchesArray ( void ) {

  GLfloat *vertex_array, *normal_array;
  GLint *surfels_per_level_array;
  vertex_array = new GLfloat[numPatches*4];
  normal_array = new GLfloat[numPatches*4];
  surfels_per_level_array = new GLint[numPatches*4];

  for (uint i = 0; i < numPatches; ++i) {
    Point p = surfels[LOD_LEVELS-1][i].position();
    vertex_array[i*4 + 0] = (GLfloat)p.x();
    vertex_array[i*4 + 1] = (GLfloat)p.y();
    vertex_array[i*4 + 2] = (GLfloat)p.z();
    vertex_array[i*4 + 3] = (GLfloat)surfels[LOD_LEVELS-1][i].radius();

    Vector n = surfels[LOD_LEVELS-1][i].normal();
    normal_array[i*4 + 0] = (GLfloat)n.x();
    normal_array[i*4 + 1] = (GLfloat)n.y();
    normal_array[i*4 + 2] = (GLfloat)n.z();
    normal_array[i*4 + 3] = (GLfloat)surfels[LOD_LEVELS-1][i].perpendicularError();

    surfels_per_level_array[i*4 + 0] = (GLint)surfels_per_level[i*4 + 0];
    surfels_per_level_array[i*4 + 1] = (GLint)surfels_per_level[i*4 + 1];
    surfels_per_level_array[i*4 + 2] = (GLint)surfels_per_level[i*4 + 2];
    surfels_per_level_array[i*4 + 3] = (GLint)surfels_per_level[i*4 + 3];
  }

  glGenBuffers(1, &vertex_patches_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_patches_buffer);
  glBufferData(GL_ARRAY_BUFFER, numPatches * 4 * sizeof(GLfloat), (const void*)vertex_array, GL_STATIC_DRAW);

  glGenBuffers(1, &normal_patches_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, normal_patches_buffer);
  glBufferData(GL_ARRAY_BUFFER, numPatches * 4 * sizeof(GLfloat), (const void*)normal_array, GL_STATIC_DRAW);

  glGenBuffers(1, &surfels_per_level_patches_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, surfels_per_level_patches_buffer);
  glBufferData(GL_ARRAY_BUFFER, numPatches * 4 * sizeof(GLint), (const void*)surfels_per_level_array, GL_STATIC_DRAW);

  delete [] vertex_array;
  delete [] normal_array;
  delete [] surfels_per_level_array;

  // Using color arrays instead of normal to pass perpendicular error as fourth component
  // Normal array has fixed sized of 3 elements
  glEnableClientState(GL_COLOR_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, normal_patches_buffer);
  glColorPointer(4, GL_FLOAT, 0, NULL);

  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, surfels_per_level_patches_buffer);
  glTexCoordPointer(4, GL_INT, 0, NULL);

  glEnableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_patches_buffer);
  glVertexPointer(4, GL_FLOAT, 0, NULL);
}

void Primitives::setPyramidPointsDisplayList ( void ) {

  triangleDisplayList = glGenLists(1);

  glNewList(triangleDisplayList, GL_COMPILE);

  glBegin(GL_POINTS);
  for (surfelVectorIter it = surfels[0].begin(); it != surfels[0].end(); ++it) {
    glColor4fv(obj_colors[id]);
    glNormal3f(it->normal().x(), it->normal().y(), it->normal().z());
    glVertex4f(it->position().x(), it->position().y(), it->position().z(), it->radius());
  }
  glEnd();

  glEndList();
}

/**
 * Create arrays and VBO for pyramid line rendering.
 **/
void Primitives::setPyramidTrianglesArrays ( void ) {

  GLfloat *vertex_array, *normal_array, *color_array;
  number_points = surfels[0].size();
  vertex_array = new GLfloat[number_points * 4];
  normal_array = new GLfloat[number_points * 3];
  color_array = new GLfloat[number_points * 4];

  // triangles are actually lines in this representation
  number_triangles = triangles.size();
  indices = new GLuint[number_triangles*3];

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  obj_colors[id][3] = type;
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, obj_colors[id]);

//   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
//   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
//   glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 0);

  int pos = 0;
  for (surfelVectorIter it = surfels[0].begin(); it != surfels[0].end(); ++it) {

    vertex_array[pos*4 + 0] = (GLfloat)(it->position().x());
    vertex_array[pos*4 + 1] = (GLfloat)(it->position().y());
    vertex_array[pos*4 + 2] = (GLfloat)(it->position().z());
    vertex_array[pos*4 + 3] = 0.0001;

    if (color_model) {
      color_array[pos*4 + 0] = (GLfloat)(it->color().x());
      color_array[pos*4 + 1] = (GLfloat)(it->color().y());
      color_array[pos*4 + 2] = (GLfloat)(it->color().z());
    }
    else {
      color_array[pos*4 + 0] = obj_colors[id][0];
      color_array[pos*4 + 1] = obj_colors[id][1];
      color_array[pos*4 + 2] = obj_colors[id][2];
    }
    //    color_array[pos*4 + 3] = type;
    color_array[pos*4 + 3] = material / (GLfloat)NUM_MATERIALS;

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
    indices[cnt] = (GLuint)it->verts[2];
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
 * Sets the pyramid triangles list.
 **/
void Primitives::setPyramidTrianglesDisplayList( void ) {

  triangleDisplayList = glGenLists(1);

  Point p[3];
  Vector n[3];
  glNewList(triangleDisplayList, GL_COMPILE);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  obj_colors[id][3] = type;
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, obj_colors[id]);

  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    p[0] = surfels[0].at( it->verts[0] ).position();
    p[1] = surfels[0].at( it->verts[1] ).position();
    p[2] = surfels[0].at( it->verts[2] ).position();
    n[0] = surfels[0].at( it->verts[0] ).normal();
    n[1] = surfels[0].at( it->verts[1] ).normal();
    n[2] = surfels[0].at( it->verts[2] ).normal();

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 3; ++i) {
      //      glColor4fv(obj_colors[id]);
      //      glColor4f(obj_colors[id][0], obj_colors[id][1], obj_colors[id][2], type);
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
  number_points = surfels[0].size();
  vertex_array = new GLfloat[number_points * 4];
  normal_array = new GLfloat[number_points * 3];
  color_array = new GLfloat[number_points * 4];

  // triangles are actually lines in this representation
  number_triangles = triangles.size();
  indices = new GLuint[number_triangles*2];

  int pos = 0;
  for (surfelVectorIter it = surfels[0].begin(); it != surfels[0].end(); ++it) {

    vertex_array[pos*4 + 0] = (GLfloat)(it->position().x());
    vertex_array[pos*4 + 1] = (GLfloat)(it->position().y());
    vertex_array[pos*4 + 2] = (GLfloat)(it->position().z());
    vertex_array[pos*4 + 3] = (GLfloat)(it->radius());

    if (color_model) {
      color_array[pos*4 + 0] = (GLfloat)(it->color().x());
      color_array[pos*4 + 1] = (GLfloat)(it->color().y());
      color_array[pos*4 + 2] = (GLfloat)(it->color().z());
    }
    else {
      color_array[pos*4 + 0] = obj_colors[id][0];
      color_array[pos*4 + 1] = obj_colors[id][1];
      color_array[pos*4 + 2] = obj_colors[id][2];
    }
    color_array[pos*4 + 3] = type;

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

  number_triangles = triangles.size();

  glNewList(triangleDisplayList, GL_COMPILE);
  GLfloat pos = 0.0;

  obj_colors[id][3] = type;
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, obj_colors[id]);

  glLineWidth(1.0);
  glDisable(GL_LINE_SMOOTH);
  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    r[0] = surfels[0].at( it->verts[0] ).radius();
    p[0] = surfels[0].at( it->verts[0] ).position();
    n[0] = surfels[0].at( it->verts[0] ).normal();
    r[1] = surfels[0].at( it->verts[1] ).radius();
    p[1] = surfels[0].at( it->verts[1] ).position();
    n[1] = surfels[0].at( it->verts[1] ).normal();
    p[2] = surfels[0].at( it->verts[0] ).position() + Vector(0.0000001, 0.0000001, 0.0000001);
    n[2] = surfels[0].at( it->verts[0] ).normal();
    r[2] = surfels[0].at( it->verts[0] ).radius();
//     p[2] = Point ((p[0].x() + p[1].x())*0.5, (p[0].y() + p[1].y())*0.5, (p[0].z() + p[1].z())*0.5);
//     n[2] = Vector((n[0].x() + n[1].x())*0.5, (n[0].y() + n[1].y())*0.5, (n[0].z() + n[1].z())*0.5);
//     r[2] = (r[0] + r[1]) * 0.5;

    glBegin(GL_LINES);
    //    glBegin(GL_TRIANGLES);

    for (int i = 0; i < 2; ++i) {
      //glColor4fv(obj_colors[id]);
      if (type == 1.0)
	glColor4f(obj_colors[id][0], obj_colors[id][1], obj_colors[id][2], 0.9 + ((pos / number_triangles)/10.0));
      else
	glColor4f(obj_colors[id][0], obj_colors[id][1], obj_colors[id][2], type);
      glNormal3f(n[i].x(), n[i].y(), n[i].z());
      glVertex4f(p[i].x(), p[i].y(), p[i].z(), r[i]);
    }
    glEnd();
    ++pos;
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

  double radius = 0.00001;
  double type_id;

//   GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
//   GLfloat white[] = {1.0, 1.0, 1.0, 1.0};

//   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);

//   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);  
//   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
//   glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 128);

  // Half as triangles.
  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    p[0] = surfels[0].at( it->verts[0] ).position();
    p[1] = surfels[0].at( it->verts[1] ).position();
    p[2] = surfels[0].at( it->verts[2] ).position();
    n[0] = surfels[0].at( it->verts[0] ).normal();
    n[1] = surfels[0].at( it->verts[1] ).normal();
    n[2] = surfels[0].at( it->verts[2] ).normal();

    if ((p[0].x() > 0.0) || (p[1].x() > 0.0) || (p[2].x() > 0.0)) {

      if ((p[0].x() > 0.005) || (p[1].x() > 0.005) || (p[2].x() > 0.005)) {
	radius = 0.00001;
	type_id = 0.1;
      }
      else {
	radius = 0.0015;
	type_id = 0.6;
      }

      glBegin(GL_TRIANGLES);
      for (int i = 0; i < 3; ++i) {
	//glColor4fv(obj_colors[0]);
	glColor4f(obj_colors[1][0], obj_colors[1][1], obj_colors[1][2], type_id);
	glNormal3f(n[i].x(), n[i].y(), n[i].z());
	glVertex4f(p[i].x(), p[i].y(), p[i].z(), radius);
      }
      glEnd();
    }
  }

  glBegin(GL_POINTS);  
  for (surfelVectorIter it = surfels[0].begin(); it != surfels[0].end(); ++it) {
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

   glEnable(GL_LINE_SMOOTH);

   double radius;
   double type_id;

  // Half as triangles.
  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    p[0] = surfels[0].at( it->verts[0] ).position();
    p[1] = surfels[0].at( it->verts[1] ).position();
    p[2] = surfels[0].at( it->verts[2] ).position();
    n[0] = surfels[0].at( it->verts[0] ).normal();
    n[1] = surfels[0].at( it->verts[1] ).normal();
    n[2] = surfels[0].at( it->verts[2] ).normal();

    if ((p[0].x() > 0.0) || (p[1].x() > 0.0) || (p[2].x() > 0.0)) {


      if ((p[0].x() > 0.005) || (p[1].x() > 0.005) || (p[2].x() > 0.005)) {
	radius = 0.00001;
	type_id = 0.1;
      }
      else {
	radius = 0.0015;
	type_id = 0.6;
      }

      glLineWidth(5.0);
      glBegin(GL_LINE_LOOP);
//      glBegin(GL_TRIANGLES);
      for (int i = 0; i < 3; ++i) {
	//glColor4fv(obj_colors[0]);
	glColor4f(obj_colors[1][0], obj_colors[1][1], obj_colors[1][2], type_id);
	glNormal3f(n[i].x(), n[i].y(), n[i].z());
	glVertex4f(p[i].x(), p[i].y(), p[i].z(), radius);
      }
      glEnd();
    }
  }

  glBegin(GL_POINTS);  
  for (surfelVectorIter it = surfels[0].begin(); it != surfels[0].end(); ++it) {
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
 * Create arrays and VBO for pyramid line rendering.
 **/
void Primitives::setTrianglesArrays ( void ) {
  GLfloat *vertex_array, *normal_array, *color_array;
  number_points = surfels[0].size();
  vertex_array = new GLfloat[number_points * 4];
  normal_array = new GLfloat[number_points * 3];
  color_array = new GLfloat[number_points * 4];

  // triangles are actually lines in this representation
  number_triangles = triangles.size();
  indices = new GLuint[number_triangles*3];

  GLfloat black[] = {0.0, 0.0, 0.0, 1.0};
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, obj_colors[id]);

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);  
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
  glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 0);

  int pos = 0;
  for (surfelVectorIter it = surfels[0].begin(); it != surfels[0].end(); ++it) {

    vertex_array[pos*4 + 0] = (GLfloat)(it->position().x());
    vertex_array[pos*4 + 1] = (GLfloat)(it->position().y());
    vertex_array[pos*4 + 2] = (GLfloat)(it->position().z());
    vertex_array[pos*4 + 3] = 1.0;

    if (color_model) {
      color_array[pos*4 + 0] = (GLfloat)(it->color().x());
      color_array[pos*4 + 1] = (GLfloat)(it->color().y());
      color_array[pos*4 + 2] = (GLfloat)(it->color().z());
    }
    else {
      color_array[pos*4 + 0] = obj_colors[id][0];
      color_array[pos*4 + 1] = obj_colors[id][1];
      color_array[pos*4 + 2] = obj_colors[id][2];
    }
    color_array[pos*4 + 3] = type;

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
    indices[cnt] = (GLuint)it->verts[2];
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
 * Sets the triangles list.
 **/
void Primitives::setTrianglesDisplayList( void ) {

  triangleDisplayList = glGenLists(1);

  Point p[3];
  Vector n[3];
  glNewList(triangleDisplayList, GL_COMPILE);

  GLfloat black[] = {0.0, 0.0, 0.0, 1.0};

  surfelVectorIter it_s = surfels[0].begin();

  GLfloat color[] = {(GLfloat)(it_s->color().x()),
		     (GLfloat)(it_s->color().y()), 
		     (GLfloat)(it_s->color().z())};

  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);  
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
  glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 0);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  for (triangleVectorIter it = triangles.begin(); it != triangles.end(); ++it) {
    p[0] = surfels[0].at( it->verts[0] ).position();
    p[1] = surfels[0].at( it->verts[1] ).position();
    p[2] = surfels[0].at( it->verts[2] ).position();
    n[0] = surfels[0].at( it->verts[0] ).normal();
    n[1] = surfels[0].at( it->verts[1] ).normal();
    n[2] = surfels[0].at( it->verts[2] ).normal();

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 3; ++i) {
      //      glColor4fv(obj_colors[id]);
      glNormal3f(-n[i].x(), -n[i].y(), -n[i].z());
      glVertex4f(p[i].x(), p[i].y(), p[i].z(), 1.0);
    }
    glEnd();
  }
 
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, black);

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
    p[0] = surfels[0].at( it->verts[0] ).position();
    p[1] = surfels[0].at( it->verts[1] ).position();
    p[2] = surfels[0].at( it->verts[2] ).position();
    n[0] = surfels[0].at( it->verts[0] ).normal();
    n[1] = surfels[0].at( it->verts[1] ).normal();
    n[2] = surfels[0].at( it->verts[2] ).normal();

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


void Primitives::createLOD ( int lod ) {

  /** Create KdTree **/
  if (kdTree)
    delete kdTree;

  Point min (-1.0, -1.0, -1.0);
  Point max ( 1.0,  1.0, 1.0);
  Box worldCoords(min, max);
  kdTree = new KdTree3D(worldCoords);

  vector<Surfel> s = surfels[lod-1];

  cout << "creating kd-tree " << lod << endl;

  for (surfelVectorIter it = s.begin(); it != s.end(); ++it) {
    kdTree->insert ( &(*it) );
  }

  cout << "created kd-tree " << lod << endl;

  KdTree3DNode* node = kdTree->begin();
  int id = 0;
  while (node != NULL) {

    if ( node->isLeaf() )
      {
	if (node->itemPtrCount() > 0) {
	  Surfel s = *(node->element(0));
	  s.setId(id);
	  surfels[lod].push_back( s );
	  ++id;
	}
      }

    node = (KdTree3DNode*)node->next();
  }

  cout << "created verts " << lod << endl;

  merged_ids[lod] = new GLint[surfels[lod].size()*4];

  node = kdTree->begin();
  int id_merge = 0;
  id = 0;
  double max_error = DBL_MIN;
  while (node != NULL) {

    if ( node->isLeaf() )
      {
      if (node->itemPtrCount() > 0) {
	
	Surfel *son;
	Surfel *s = &surfels[lod][id];
	double max = DBL_MIN, min = DBL_MAX;
	double max_son_error = DBL_MIN;
	vector<int> *ids = node->mergedElements();

	// Iterate over all merged sons
	for (vector<int>::iterator it = ids->begin(); it != ids->end(); ++it) {
	  merged_ids[lod][id_merge] = *it;
	  ++id_merge;

	  // Compute perpendicular error between merged node and this son
	  son = &surfels[lod-1][*it];
	  double normal_dot = son->normal().normalize() * s->normal().normalize();
	  if (normal_dot*normal_dot > 1.0) 
	    normal_dot = 1.0;
	  double di = son->radius() * sqrt(1.0 - normal_dot*normal_dot);
	  Vector v = son->position() - s->position();
	  double ei = v * s->normal();
	  if (ei + di > max)
	    max = ei + di;
	  if (ei - di < min)
	    min = ei - di;

	  double son_error = son->perpendicularError();
	  if (son_error > max_son_error)
	    max_son_error = son_error;
	}
	// The perpendicular error of this node is the difference between
	// the max and min error between all sons -- PBG pg 335
	s->ep = (max - min) + max_son_error;

	if (lod == LOD_LEVELS-1)
	  if (s->ep > max_error)
	    max_error = s->ep;

	assert (max >= min);

	for (unsigned int i = 0; i < 4 - ids->size(); ++i) {
	  merged_ids[lod][id_merge] = -1;
	  ++id_merge;
	}
	++id;
      }
    }

    node = (KdTree3DNode*)node->next();

  }

  //normalize error for highest level
  if (lod == LOD_LEVELS-1) {
    for (vector<Surfel>::iterator it = surfels[LOD_LEVELS-1].begin(); it != surfels[LOD_LEVELS-1].end(); ++it)
      it->ep /= max_error;
  }

  cout << "created ids " << lod << endl;

}

void Primitives::reorderSurfels ( void ) {

  cout << "ordering surfels " <<  endl;

  int num_surfels = 0;
  for (int i = 0; i < LOD_LEVELS-1; ++i)
    num_surfels += surfels[i].size();

  surfels_per_level.reserve( surfels[LOD_LEVELS-1].size()*4 );
  surfels_lod.reserve( num_surfels );

  int num_surfels_per_level[LOD_LEVELS];
  num_surfels_per_level[LOD_LEVELS-1] = 1;

  vector<Surfel>::iterator curr_surfel = surfels_lod.begin();

  int patch_id = 0;
  for (vector<Surfel>::iterator it = surfels[LOD_LEVELS-1].begin(); it != surfels[LOD_LEVELS-1].end(); ++it) {

    surfels_per_level.push_back( surfels_lod.size() );

    patch_id = it->id();

    for (int i = LOD_LEVELS-2; i >= 0; --i) {

      num_surfels_per_level[i] = 0;
      
      for (int k = 0; k < num_surfels_per_level[i+1]; ++k) {

	for (int j = 0; j < 4; ++j) {
	  int merged_id = merged_ids[i+1][patch_id*4 + j];
	  if (merged_id != -1) {
	    surfels_lod.push_back(surfels[i][merged_id]);
	    num_surfels_per_level[i]++;
	  }
	}
      
	if ( (it == surfels[LOD_LEVELS-1].begin()) && (i == LOD_LEVELS-2))
	  curr_surfel = surfels_lod.begin();
	else
	  curr_surfel++;

	patch_id = curr_surfel->id();

      }
    }


    for (int i = LOD_LEVELS-2; i >= 0; --i) {
      surfels_per_level.push_back( (GLuint)num_surfels_per_level[i] );
    }  


    curr_surfel = surfels_lod.end();
    curr_surfel--;
  } // it


  int cnt = 0;
  int cnt_lods[3] = {0, 0, 0};

  for (unsigned int i = 0; i < surfels[3].size(); ++i) {
    cnt_lods[0] += surfels_per_level[i*4 + 1];
    cnt_lods[1] += surfels_per_level[i*4 + 2];
    cnt_lods[2] += surfels_per_level[i*4 + 3];
  }

  for (int i = 0; i < LOD_LEVELS-1; ++i) {
    cnt += surfels[i].size();
    cout << "surfels " << i << " : " << surfels[i].size() << " = " << cnt_lods[i] << endl;
    surfels[i].clear();
    if (i > 0)
      delete [] merged_ids[i];
  }

  cout << "size of s per level " << surfels_per_level.size() << endl;
  cout << "surfels " <<  surfels_lod.size() << " = " << cnt << endl;

  numPatches = surfels[LOD_LEVELS-1].size();

  numVertsArray = surfels_lod.size();

  cout << "reorderer surfels " << endl;
}

void Primitives::createLOD ( void ) {

  kdTree = NULL;

  for (int i = 1; i < LOD_LEVELS; ++i)
    createLOD( i );

  reorderSurfels( );

  delete kdTree;

}

void Primitives::writeFileLOD ( const char* fn ) {

	ofstream out(fn, ios::trunc);

	out << numVertsArray << endl;

	GLfloat px, py, pz, r, nx, ny, nz;
	for (surfelVectorIter it = surfels_lod.begin(); it != surfels_lod.end(); ++it) {

		px=0; py=0; pz=0; r=0; nx=0; ny=0; nz=0;

		if ( !isnan( (GLfloat)(it->position().x()) ) )
			px = (GLfloat)(it->position().x());
		if ( !isnan( (GLfloat)(it->position().y()) ) )
			py = (GLfloat)(it->position().y());
		if ( !isnan( (GLfloat)(it->position().z()) ) )
			pz = (GLfloat)(it->position().z());
		if ( !isnan( (GLfloat)(it->radius()) ) )
			r = (GLfloat)(it->radius());
		if ( !isnan( (GLfloat)(it->normal().x()) ) )
			nx = (GLfloat)(it->normal().x());
		if ( !isnan( (GLfloat)(it->normal().y()) ) )
			ny = (GLfloat)(it->normal().y());
		if ( !isnan( (GLfloat)(it->normal().z()) ) )
			nz = (GLfloat)(it->normal().z());

		out << px << " " << py << " " << pz << " " << r << " "
		    << nx << " " << ny << " " << nz << endl;

	}

	out << numPatches << endl;
	
	Point p;
	Vector n;
	GLfloat radius, ep, perp_error;
	for (uint i = 0; i < numPatches; ++i) {

	  px=0; py=0; pz=0; r=0; nx=0; ny=0; nz=0; ep=0;

		p = surfels[LOD_LEVELS-1][i].position();
		n = surfels[LOD_LEVELS-1][i].normal();
		radius = (GLfloat)(surfels[LOD_LEVELS-1][i].radius());
		perp_error = (GLfloat)(surfels[LOD_LEVELS-1][i].perpendicularError());

		if ( !isnan( p.x() ) )
			px = p.x();
		if ( !isnan( p.y() ) )
			py = p.y();
		if ( !isnan( p.z() ) )
			pz = p.z();
		if ( !isnan( radius ) )
			r = radius;
		if ( !isnan( n.x() ) )
			nx = n.x();
		if ( !isnan( n.y() ) )
			ny = n.y();
		if ( !isnan( n.z() ) )
			nz = n.z();
		if ( !isnan( perp_error ) )
			ep = perp_error;


		out << px << " " << py << " " << pz << " " << r << " "
		    << nx << " " << ny << " " << nz << " " << ep << endl;

		out << surfels_per_level[i*4 + 0] << " "
		    << surfels_per_level[i*4 + 1] << " "
		    << surfels_per_level[i*4 + 2] << " "
		    << surfels_per_level[i*4 + 3] << endl;

	}

	out.close();

}

void Primitives::readFileLOD ( const char* fn ) {

  ifstream in(fn);

  in >> numVertsArray;

  surfels_lod.clear();

  GLfloat x, y, z, nx, ny, nz, r;
  for (uint i = 0; i < numVertsArray; i++) {
    in >> x >> y >> z >> r >> nx >> ny >> nz;
    surfels_lod.push_back(Surfel(Point(x, y, z), Vector(nx, ny, nz), r, i));
  }

  in >> numPatches;

  surfels[LOD_LEVELS-1].clear();
  surfels_per_level.clear();
	
  GLuint spl[4];
  GLfloat ep;
  for (uint i = 0; i < numPatches; ++i) {
    in >> x >> y >> z >> r >> nx >> ny >> nz >> ep;
    in >> spl[0] >> spl[1] >> spl[2] >> spl[3];
    surfels[LOD_LEVELS-1].push_back(Surfel(Point(x, y, z), Vector(nx, ny, nz), r, i, ep));
    for (uint j = 0; j < 4; ++j)
      surfels_per_level.push_back(spl[j]);
  }

  in.close();
}
