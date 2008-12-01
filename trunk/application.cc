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

  canvas_width = canvas_height = 768;

  // Initialize camera
  camera = new Camera(canvas_width + canvas_width/16,
		      canvas_height + canvas_height/16);

  render_mode = default_mode;

  point_based_render = NULL;

  fps_loop = 0;

  rotating = 0;
  show_points = false;

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

  glEnable(GL_NORMALIZE);
  check_for_ogl_error();

}

Application::~Application( void ) {
  objects.clear();
  primitives.clear();
  delete point_based_render;
  delete camera;
}

/// Renders a surfel as a opengl point primitive
/// @param s Pointer to surfel
void Application::glVertex(const Surfeld * s) const {
  Point p = s->Center();
  glVertex3f(p[0], p[1], p[2]);
}

/// Renders a surfel as a opengl point primitive
/// @param s Pointer to surfel
void Application::glVertex(const surfelVectorIter it) const {
  Point p = it->Center();
  glVertex3f(p[0], p[1], p[2]);
}

/// Renders a vertex
/// @param p Given vertex position
void Application::glVertex(const Point p) const {
  glVertex3f(p[0], p[1], p[2]);
}

/// Render all points with OpenGL
void Application::drawPoints(void) {
  glPointSize(1.0);
  glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

  glBegin(GL_POINTS);
  
  for (surfelVectorIter it = primitives[0].getSurfels()->begin(); it != primitives[0].getSurfels()->end(); ++it) {
	Color c = it->color();
	glColor4f(c[0], c[1], c[2], 1.0f);  
	glVertex(it);
  }
  glEnd();
}

/** 
 * Display method to render the models.
 **/
void Application::draw( void ) {

  if (primitives.size() == 0)
    return;

  // Clear all buffers including pyramid algorithm buffers
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  point_based_render->clearBuffers();

  // Reset camera position and direction
  camera->setView();

  // Set eye for back face culling in vertex shader
  point_based_render->setEye( camera->positionVector() );

  for (unsigned int i = 0; i < primitives.size(); ++i) {
    // Project samples to screen space
	point_based_render->projectSamples( &primitives[i] );
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
  for (unsigned int i = 0; i < primitives.size(); ++i)
	primitives[i].setRendererType(type);
}

/**
 * Changes the rendering algorithm.
 * @param type Rendering mode.
 **/
void Application::changeRendererType( int type ) {
  changePrimitivesRendererType ( (point_render_type_enum) type );
  render_mode = type;
  createPointRenderer( );
}

/**
 * Defines the rendering algorithm.
 * point_based_render is of generic type, depending on
 * the choice one of the inherited classes is instanced.
 **/
void Application::createPointRenderer( void ) {  
  
  if (point_based_render)
	delete point_based_render;

  if (render_mode == PYRAMID_POINTS)
	point_based_render = new PyramidPointRenderer(canvas_width, canvas_height);
  else if (render_mode == PYRAMID_POINTS_COLOR)
	point_based_render = new PyramidPointRendererColor(canvas_width, canvas_height);
  else if (render_mode == PYRAMID_TEMPLATES)
    point_based_render = new PyramidPointRendererER(canvas_width, canvas_height);

  assert (point_based_render);

  ((PyramidPointRendererBase*)point_based_render)->createShaders();
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

  // Create a new object with id 0
  objects.push_back( Object( 0 ) );
  objects.back().setFilename( filename );

  MYreadPlyTrianglesColor (filename, (primitives.back()).getSurfels(), (primitives.back()).getTriangles());

  computeNormFactors((primitives.back()).getSurfels());
  normalize((primitives.back()).getSurfels());

  // connect new object to new primitive
  objects[0].addPrimitives( primitives.back().getId() );
  // Sets the default rendering algorithm
  primitives[0].setRendererType( render_mode );

  //  if (!point_based_render)
  createPointRenderer( );

  return 0;
}

int Application::startFileReading ( void ) {
  // Create a new object with id 0
  objects.push_back( Object( 0 ) ); 

  return 0;
}

int Application::appendFile ( const char * filename ) { 

   // Create a new primitive from given file
  primitives.push_back( Primitives( primitives.size() ) );
  objects[0].addPrimitives( primitives.back().getId() );

  readPlyTrianglesColor (filename, (primitives.back()).getSurfels(), (primitives.back()).getTriangles());

  return 0;
}

int Application::finishFileReading ( void ) { 

  computeNormFactors((primitives.back()).getSurfels());
  
  for (unsigned int i = 0; i < primitives.size(); ++i) {
	normalize(primitives[i].getSurfels());
	// Sets the default rendering algorithm
	primitives[i].setRendererType( render_mode );
  }

  createPointRenderer();

  return 0;
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

/**
 * Returns the model's number of points.
 * @return Number of points.
 **/
int Application::getNumberPoints ( void ) {
  
  int num_pts = 0;
  for (unsigned int i = 0; i < primitives.size(); ++i)
	num_pts += primitives[i].numberPoints();

  return num_pts;
}

/**
 * Sets the reconstruction filter size.
 * Multiplicator of the radius size.
 * @param s Reconstruction filter size.
 **/
void Application::setReconstructionFilter ( double s ) { 
  if (point_based_render)
    point_based_render->setReconstructionFilterSize(s);
}

/**
 * Sets the prefilter size.
 * @param s Prefilter size.
 **/
void Application::setPrefilter ( double s ) { 
  if (point_based_render)
    point_based_render->setPrefilterSize(s);
}

/**
 * Sets the kernel size for the template rendering algorithm only.
 * @param m Kernel size mxm.
 **/
void Application::setGpuMask ( int m ) {
  point_based_render->setGpuMaskSize( m );
}

/**
 * Sets per-vertex color on/off if the model has
 * color information per point.
 * @param c Per-vertex color state.
 **/
void Application::setPerVertexColor ( bool c ) {
  for (unsigned int i = 0; i < primitives.size(); ++i) {
	// Reset renderer type to load per vertex color or default color in vertex array
	primitives[i].setRendererType( primitives[i].getRendererType() );
  }
}

/**
 * Turns auto-rotate on/off.
 * Auto-rotate turns the model continuously.
 * @param r Auto-rotate state.
 **/
void Application::setAutoRotate ( bool r ) {
  rotating = r;
  if (r == true)
	camera->startRotation(canvas_width/2,canvas_height/2);
}

/**
 * Turns depth test on/off.
 * @param d Depth test state.
 **/
void Application::setDepthTest ( bool d ) {
  if (point_based_render)
    point_based_render->setDepthTest(d);
}

/**
 * Change model material properties.
 * @param mat Id of material (see materials.h for list)
 **/
void Application::changeMaterial( int mat ) {  
  point_based_render->setMaterial( mat );
}

/**
 * Turns backface culling on/off.
 * @param b Backface culling state.
 **/
void Application::setBackFaceCulling ( bool c ) {
  if (point_based_render)
	point_based_render->setBackFaceCulling(c);
}

/**
 * Turns elliptical weights on/off.
 * @param b Elliptical weight state.
 **/
void Application::setEllipticalWeight ( bool b ) {
  point_based_render->setEllipticalWeight(b);
}
