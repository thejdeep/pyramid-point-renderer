/**
 * Interface independent application class
 * 
 * Author : Ricardo Marroquim
 *
 * Date created : 20-12-2007
 *
 **/

#include "file_io.h"

// Glut was included only because the Qt function to
// write on the screen contains bugs and is not working
// properly. This is necessary for the color bars numbers.
#include <GL/glut.h>

/**
 * Initialize opengl and application state variables.
 **/
Application::Application( GLint default_mode ) {

  // Initialize camera
  camera = new Camera(CANVAS_WIDTH + CANVAS_WIDTH/16,
		      CANVAS_HEIGHT + CANVAS_HEIGHT/16);

  render_mode = default_mode;

  point_based_render = NULL;

  number_surfels = 0;
  fps_loop = 0;

  color_model = false;
  elliptical_weight = true;
  depth_culling = true;
  rotating = 0;

  show_points = false;
  show_splats = 1;

  timing_profile = 0;
  material_id = 0;

  reconstruction_filter_size = 1.0;
  prefilter_size = 1.0;

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  for (int i = 0; i < 8; ++i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
  }

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(GL_BACK);

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  camera->initLight();

  changeMaterial();

  glEnable(GL_NORMALIZE);
  CHECK_FOR_OGL_ERROR()	;

}

Application::~Application( void ) {
  objects.clear();
  primitives.clear();
  delete point_based_render;
  delete camera;
}

/// Renders a surfel as a opengl point primitive
/// @param s Pointer to surfel
void Application::glVertex(const Surfeld * s) {
  Point p = s->Center();
  glVertex3f(p[0], p[1], p[2]);
}

/// Renders a surfel as a opengl point primitive
/// @param s Pointer to surfel
void Application::glVertex(surfelVectorIter it) {
  Point p = it->Center();
  glVertex3f(p[0], p[1], p[2]);
}

/// Renders a vertex
/// @param p Given vertex position
void Application::glVertex(Point p) {
  glVertex3f(p[0], p[1], p[2]);
}

/// Render all points 
void Application::drawPoints(void)
{
  glPointSize(1.0);
  glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

  glBegin(GL_POINTS);
  
  for (surfelVectorIter it = primitives[0].getSurfels()->begin(); it != primitives[0].getSurfels()->end(); ++it)
    glVertex(it);

  glEnd();
}

void Application::projectPoints ( void ) {
  
  if (primitives.size() == 0)
    return;

  // Clear all buffers including pyramid algorithm buffers
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  point_based_render->clearBuffers();
  camera->setView();

  // Render objects primitives with pyramid algorithm
  for (unsigned int i = 0; i < objects.size(); ++i) {

    // Compute rotated eye position for this object for back face culling   
    Point eye = *(objects[i].getCenter());

    glPushMatrix();

    // Translate and rotate object
    objects[i].translate();
    objects[i].rotate();
    point_based_render->setEye( camera->positionVector() );

    // Projects to image plane surfels of all primitives for this object
    vector< int >* prims = objects[i].getPrimitivesList();
    for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it) {
      Primitives * prim = &(primitives[*prim_it]);      
      point_based_render->projectSamples( prim );
    }
    glPopMatrix();
  }
}

/// Display func
void Application::draw(void) {

  if (primitives.size() == 0)
    return;

  // Clear all buffers including pyramid algorithm buffers
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  point_based_render->clearBuffers();

  // Reset camera position and direction
  camera->setView();
  //camera->setTranslation();
  //camera->setRotation();

  // Render objects primitives with pyramid algorithm
  for (unsigned int i = 0; i < objects.size(); ++i) {

    // Compute rotated eye position for this object for back face culling   
    Point eye = *(objects[i].getCenter());

    glPushMatrix();

    // Translate and rotate object
    objects[i].translate();
    objects[i].rotate();

    // objects[i].render( );
    // objects[i].render( camera->positionVector() );

    // Compute the rotated eye (opposite direction) of the camera + object center position
    // camera->computeEyePosition(*(objects[i].getRotationQuat()), &eye);
    //point_based_render->setEye(eye);
 
    point_based_render->setEye( camera->positionVector() );

    // Projects to image plane surfels of all primitives for this object
    vector< int >* prims = objects[i].getPrimitivesList();
    for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it) {
      Primitives * prim = &(primitives[*prim_it]);
      point_based_render->projectSamples( prim );
    }
    glPopMatrix();
  }

  // Interpolates projected surfels using pyramid algorithm
  point_based_render->interpolate();

  // Computes per pixel color with deferred shading
  point_based_render->draw();
  
  glDisable (GL_LIGHTING);
  glDisable (GL_LIGHT0);
  glDisable (GL_COLOR_MATERIAL);

  if (show_points)
    drawPoints();

  if (rotating)
    camera->rotate();

  // necessary to compute correct fps
  glFinish();

}

/// Reshape func
/// @param w New window width
/// @param h New window height
void Application::reshape(int w, int h) {
  camera->reshape(w, h);
}

vector<Surfeld>* Application::getSurfelsList ( void ) {
  vector< int >* prims = objects[0].getPrimitivesList();
  return primitives[*prims->begin()].getSurfels();
}

void Application::changePrimitivesRendererType( point_render_type_enum type ) {
  primitives[0].setRendererType(type);
  // Resets the color material
  changeMaterial();
}


void Application::changeRendererType( int type ) {

  changePrimitivesRendererType ( (point_render_type_enum)type );
  render_mode = type;
  createPointRenderer( );
}

void Application::createPointRenderer( void ) {

  if (render_mode == NONE)
    return;

  if (render_mode == PYRAMID_POINTS) {
    if (color_model)
      point_based_render = new PyramidPointRendererColor(CANVAS_WIDTH, CANVAS_HEIGHT);   
    else
      point_based_render = new PyramidPointRenderer(CANVAS_WIDTH, CANVAS_HEIGHT);
  }
  else if (render_mode == PYRAMID_POINTS_ER)
    point_based_render = new PyramidPointRendererER(CANVAS_WIDTH, CANVAS_HEIGHT);
    //    point_based_render->setVertices(primitives.back().getSurfels());

  assert (point_based_render);

  point_based_render->setBackFaceCulling(1);
  point_based_render->setReconstructionFilterSize(reconstruction_filter_size);
  point_based_render->setPrefilterSize(prefilter_size);
  point_based_render->setDepthTest(depth_culling);
}

/**
 * Reads a ply file, creates an object and
 * loads the vertices and triangles in the associated primitive.
 * It is possible to load multiple objects and multiple primitives
 * per object, but for this simple example only one primitive associate
 * to one object is considered
 * @param filename Given file name.
 * @return Id number of created object.
 **/
int Application::readFile ( const char * filename ) {

  // Create a new primitive from given file
  primitives.push_back( Primitives( primitives.size() ) );

  primitives.back().setPerVertexColor(0);

  // Create a new object with id 0
  objects.push_back( Object( 0 ) );
  objects.back().setFilename( filename );

  readPlyTrianglesColor (filename, (primitives.back()).getSurfels(), (primitives.back()).getTriangles());

  // connect new object to new primitive
  objects[0].addPrimitives( primitives.back().getId() );
  primitives[0].setType( 1.0 );
  // Sets the default rendering algorithm
  primitives[0].setRendererType( render_mode );

  // Count total number of points being rendered
  number_surfels += primitives.back().getSurfels()->size();

  //  if (!point_based_render)
  createPointRenderer( );

  return 0;
}

/**
 * Reads a ply file, creates an object and
 * loads the vertices and triangles in the associated primitive.
 * @param filename Given file name.
 * @return Id number of created object.
 **/
int Application::readNormalsFile ( const char * filename ) {

  // Create a new primitive from given file
  primitives.push_back( Primitives( primitives.size() ) );

  primitives.back().setPerVertexColor(0);

  // Create a new object
  int id = objects.size();
  objects.push_back( Object( id ) );
  objects.back().setFilename( filename );

  readNormals (filename, (primitives.back()).getSurfels());

  // connect new object to new primitive
  objects.back().addPrimitives( primitives.back().getId() );
  primitives.back().setType( 1.0 );
  ;;primitives.back().setRendererType( render_mode );

  // Count total number of points being rendered
  number_surfels += primitives.back().getSurfels()->size();
  

  //  if (!point_based_render)
  createPointRenderer( );
 
  return id;
}

/// Mouse Left Button Function, starts rotation
/// @param x X coordinate of mouse click
/// @param y Y coordinate of mouse click
void Application::mouseLeftButton(int x, int y) {  
  camera->startRotation(x, y);
  //camera->startQuatRotation(x, y, objects[*it].getRotationQuat());
}

/// Mouse Middle Button Function, zoom
/// @param x X coordinate of mouse click
/// @param y Y coordinate of mouse click
void Application::mouseMiddleButton(int x, int y) {
  camera->mouseSetClick(x, y);
}

/// Mouse Right Button Function, light translation
/// @param x X coordinate of mouse click
/// @param y Y coordinate of mouse click
void Application::mouseRightButton(int x, int y) {
  camera->mouseSetClick(x, y);
}

/// Mouse Release Function
void Application::mouseReleaseButton( void ) {
  camera->endRotation();
}

/// Mouse left movement func, rotates the camera or selected object
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void Application::mouseLeftMotion(int x, int y) {
  camera->rotate(x, y);
  //camera->rotateQuat(x, y, objects[*it].getRotationQuat(), objects[*it].getCenter());
}

/// Mouse middle movement func, zooms the camera or selected object
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void Application::mouseMiddleMotion(int x, int y) {
  camera->zooming (x, y);
  //camera->zoomingVec(x, y, objects[*it].getCenter());
  camera->updateMouse();
}

/// Mouse middle movement func, zooms the camera or selected object
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void Application::mouseMiddleMotionShift(int x, int y) {
  camera->translate(x, y);
  //camera->translateVec(x, y, objects[*it].getCenter());
  camera->updateMouse();  
}

/// Mouse right movement func, light translation
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void Application::mouseRightMotion(int x, int y) {
  camera->lightTranslate(x, y);
}

int Application::getNumberPoints ( int object_id ) {
  return primitives[0].numberPoints();
}

int Application::getNumberTriangles ( int object_id ) {
  return primitives[0].numberTriangles();
}

void Application::setReconstructionFilter ( double s ) { 
  reconstruction_filter_size = s;
  if (point_based_render)
    point_based_render->setReconstructionFilterSize(reconstruction_filter_size);
}

void Application::setPrefilter ( double s ) { 
  prefilter_size = s;
  if (point_based_render)
    point_based_render->setPrefilterSize(prefilter_size);
}

void Application::setCpuMask ( int m ) {
  point_based_render->setCpuMaskSize( m );
}

void Application::setGpuMask ( int m ) {
  point_based_render->setGpuMaskSize( m );
}

void Application::setPerVertexColor ( bool b, int object_id ) {
  primitives[0].setPerVertexColor(b);
  // Reset renderer type to load per vertex color or default color in vertex array
  primitives[0].setRendererType( primitives[0].getRendererType() );
}

void Application::setAutoRotate ( bool r ) {
  rotating = r;
}

void Application::setDepthTest ( bool d ) {
  depth_culling = d;
  if (point_based_render)
    point_based_render->setDepthTest(depth_culling);
}

/**
 * Change material properties.
 **/
void Application::changeMaterial(void) {

  GLfloat material_ambient[4] = {Mats[material_id][0], Mats[material_id][1], Mats[material_id][2], Mats[material_id][3]};  
  GLfloat material_diffuse[4] = {Mats[material_id][4], Mats[material_id][5], Mats[material_id][6], Mats[material_id][7]};  
  GLfloat material_specular[4] = {Mats[material_id][8], Mats[material_id][9], Mats[material_id][10], Mats[material_id][11]};
  GLfloat material_shininess = Mats[material_id][12];

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_specular);
  glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, material_shininess);
}

void Application::changeMaterial( int mat ) {
  material_id = mat;
  changeMaterial();
}

void Application::changeSelectedObjsMaterial( int mat ) {  
  primitives[0].setMaterial( mat );
  primitives[0].setRendererType( primitives[0].getRendererType() );
  point_based_render->setMaterial( mat );
  changeMaterial(mat);
}

void Application::setDistanceType ( int n ) {
  point_based_render->setDistanceType(n);
}

void Application::setBackFaceCulling ( bool b ) {
  point_based_render->setBackFaceCulling(b);
}

void Application::setEllipticalWeight ( bool b ) {
  point_based_render->setEllipticalWeight(b);
}
