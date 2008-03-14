/*
** triangle_renderer.cc OpenGL triagle renderer.
**
**
**   history:	created  16-Oct-07
*/

#include "triangle_renderer.h"

TriangleRenderer::TriangleRenderer() {}

TriangleRenderer::~TriangleRenderer() {
  glDeleteLists(triangleDisplayList, 1);
}

void TriangleRenderer::draw ( double[3] ){
  draw();
}

/**
 * Render the triangle soup.
 **/
void TriangleRenderer::draw() {

  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

//   glEnable(GL_CULL_FACE);

  glShadeModel(GL_SMOOTH);

  glColor4f(1.0, 1.0, 1.0, 1.0);

  glCallList(triangleDisplayList);
}

/**
 * Sets the triangles list.
 **/
void TriangleRenderer::setTriangles( vector<Triangle> *t ) {
  triangles = t;

  CHECK_FOR_OGL_ERROR();


  triangleDisplayList = glGenLists(1);

  Point p[3];
  Vector n[3];
  glNewList(triangleDisplayList, GL_COMPILE);

  for (triangleVectorIter it = triangles->begin(); it != triangles->end(); ++it) {
    p[0] = surfels->at( it->verts[0] ).position();
    p[1] = surfels->at( it->verts[1] ).position();
    p[2] = surfels->at( it->verts[2] ).position();
    n[0] = surfels->at( it->verts[0] ).normal();
    n[1] = surfels->at( it->verts[1] ).normal();
    n[2] = surfels->at( it->verts[2] ).normal();

    glBegin(GL_TRIANGLES);  
    for (int i = 0; i < 3; ++i) {
      glNormal3f(n[i].x(), n[i].y(), n[i].z());
      glVertex4f(p[i].x(), p[i].y(), p[i].z(), 1.0);
    }
    glEnd();
  }

  glEndList();

  CHECK_FOR_OGL_ERROR();
}

/**
 * Sets the vertices vector.
 **/
void TriangleRenderer::setVertices( vector<Surfel> *s ) {
  surfels = s;
}
