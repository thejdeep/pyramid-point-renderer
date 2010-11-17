#include "object.h"
#include "point_based_renderer.h"


Object::~Object() {

  glDeleteLists(pointsDisplayList, 1);
}

/**
 * Render object using designed rendering system.
 **/
void Object::render ( void ) const{

  glCallList(pointsDisplayList);
  

  /// for rendering directly without display-lists uncomment the code below and comment line above
  /// but performance drops drastically in many cases, for the asian dragon it is around 4 times slower for example

  // glBegin(GL_POINTS);

  // for (surfelVectorIterConst it = surfels.begin(); it != surfels.end(); ++it) {
  // 	glColor4f((GLfloat)(it->Color()[0] / 255.0), (GLfloat)(it->Color()[1] / 255.0), (GLfloat)(it->Color()[2] / 255.0), (GLfloat)(1.0));
  //   glNormal3f((GLfloat)it->Normal()[0], (GLfloat)it->Normal()[1], (GLfloat)it->Normal()[2]);
  //   glVertex4f((GLfloat)it->Center()[0], (GLfloat)it->Center()[1], (GLfloat)it->Center()[2], (GLfloat)it->Radius());
  // }
  // glEnd();

  check_for_ogl_error("Primitives render");

}

/**
 * Changes the renderer type.
 * @param rtype Given renderer type.
 **/
void Object::setRendererType ( int rtype ) {

  renderer_type = rtype;

  if (rtype == PYRAMID_POINTS) {
    setPyramidPointsDisplayList();
  }
  else if (rtype == PYRAMID_POINTS_COLOR) {
    setPyramidPointsColorDisplayList();
  }

}

void Object::setPyramidPointsDisplayList ( void ) {

  pointsDisplayList = glGenLists(1);

  glNewList(pointsDisplayList, GL_COMPILE);

  glPointSize(1.0);

  glBegin(GL_POINTS);
  
  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {
    glNormal3f((GLfloat)it->Normal()[0], (GLfloat)it->Normal()[1], (GLfloat)it->Normal()[2]);
    glVertex4f((GLfloat)it->Center()[0], (GLfloat)it->Center()[1], (GLfloat)it->Center()[2], (GLfloat)it->Radius());
  }
  glEnd();

  glEndList();
}

void Object::setPyramidPointsColorDisplayList ( void ) {

  pointsDisplayList = glGenLists(1);
  glNewList(pointsDisplayList, GL_COMPILE);

  glBegin(GL_POINTS);

  for (surfelVectorIter it = surfels.begin(); it != surfels.end(); ++it) {
	glColor4f((GLfloat)(it->Color()[0] / 255.0), (GLfloat)(it->Color()[1] / 255.0), (GLfloat)(it->Color()[2] / 255.0), (GLfloat)(1.0));
    glNormal3f((GLfloat)it->Normal()[0], (GLfloat)it->Normal()[1], (GLfloat)it->Normal()[2]);
    glVertex4f((GLfloat)it->Center()[0], (GLfloat)it->Center()[1], (GLfloat)it->Center()[2], (GLfloat)it->Radius());
  }
  glEnd();

  glEndList();
}

void Object::clearSurfels ( void ) {  
  surfels.clear();
}
