/**
 * Interface independent application class
 * 
 * Author : Ricardo Marroquim
 *
 * Date created : 20-12-2007
 *
 **/

#include "application.h"

/**
 * Initialize opengl and application state variables.
 **/
Application::Application( GLint default_mode ) {

  canvas_width = canvas_height = 768;

  trackball.center=vcg::Point3f(0, 0, 0);
  trackball.radius= 1;

  trackball_light.center=vcg::Point3f(0, 0, 0);
  trackball_light.radius= 1;


  render_mode = default_mode;

  point_based_render = NULL;

  fps_loop = 0;

  rotating = 0;
  show_points = false;
  selected = 0;

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


  /* Old camera init light routine*/
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glDisable (GL_COLOR_MATERIAL);

  Vector ambient_light = Vector( 0.0, 0.0, 0.0 );
  Vector diffuse_light = Vector( 1.0, 1.0, 1.0 );
  Vector specular_light = Vector( 1.0, 1.0, 1.0 );

  GLfloat al[] = {ambient_light[0], ambient_light[1],
		  ambient_light[2], 1.0};

  GLfloat dl[] = {diffuse_light[0], diffuse_light[1],
		  diffuse_light[2], 1.0};

  GLfloat sl[] = {specular_light[0], specular_light[1],
		  specular_light[2], 1.0};

  glLightfv(GL_LIGHT0, GL_AMBIENT, al);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, dl);
  glLightfv(GL_LIGHT0, GL_SPECULAR, sl);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

  check_for_ogl_error("Init");
}

Application::~Application( void ) {
  objects.clear();
  primitives.clear();
  delete point_based_render;
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
	LAL::Color c = it->color();
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
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(0, 0, canvas_width, canvas_height);
  gluPerspective( 45, (GLfloat)canvas_width/(GLfloat)canvas_height, 0.01, 100.0 );
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  Point camera_offset (0, 0, 5);
  gluLookAt(camera_offset[0], camera_offset[1], camera_offset[2],   0,0,0,   0,1,0);

  /** Set light direction **/
  glPushMatrix();
  trackball_light.GetView();
  trackball_light.Apply();
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  static float lightPosF[]={0.0, 0.0, 1.0, 0.0};
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosF);
  glPopMatrix();
  
  glPushMatrix();
  trackball.GetView();  
  trackball.Apply();

  float diag = 2.0f/FullBBox.Diag();
  glScalef(diag, diag, diag);
  glTranslatef(-FullBBox.Center()[0], -FullBBox.Center()[1], -FullBBox.Center()[2]);

  // Get eye position rotated in inverse direction
  // for backface culling
  glPushMatrix();
  Trackball tr;
  tr.track = trackball.track;
  tr.radius = trackball.radius;
  tr.center = trackball.center;
  tr.GetView();
//   vcg::Matrix44f rotM;
//   tr.track.rot.ToMatrix(rotM); 
//   vcg::Invert(rotM);
//   vcg::Point3<float> vp = rotM*vcg::Point3f(0, 0, camera_offset[2]);
  vcg::Point3<float> vp = tr.camera.ViewPoint();
  glPopMatrix();

  // Set eye for back face culling in vertex shader
  point_based_render->setEye( Point(vp[0], vp[1], vp[2]) );
  point_based_render->setScaleFactor( trackball.track.sca );

  if (selected == 0)
	for (unsigned int i = 0; i < primitives.size(); ++i)
	  // Project samples to screen space
	  point_based_render->projectSamples( &primitives[i] );
  else
	point_based_render->projectSamples( &primitives[selected-1] );

  // Interpolates projected surfels using pyramid algorithm
  point_based_render->interpolate();

  // Computes per pixel color with deferred shading
  point_based_render->draw();
  
  glDisable (GL_LIGHTING);
  glDisable (GL_LIGHT0);
  glDisable (GL_COLOR_MATERIAL);

  if (show_points)
    drawPoints();

  glPopMatrix();

  // necessary to compute correct fps
  //  glFinish();
}

/// Reshape func
/// @param w New window width
/// @param h New window height
void Application::reshape(int w, int h) {
  //  camera->reshape(w, h);
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
 * Reads a ply file using the VCG library
 * @param filename Given file name.
 * @param surfels Pointer to surfel vector to be filled with mesh data.
 **/
void Application::readFile ( const char * filename, vector<Surfeld> *surfels ) {
  /** read using vcg plylib **/
  CMesh mesh;

  tri::io::Importer<CMesh>::Open(mesh, filename);

  cout << "vertices : " << mesh.vn << endl;
  cout << "faces : " << mesh.fn << endl;

//   cout << "has normal per vertex " << mesh.HasPerVertexNormal() << endl;
//   cout << "has color per vertex " << mesh.HasPerVertexColor() << endl;
//   cout << "has radius per vertex " << mesh.HasPerVertexQuality() << endl;

  /// Compute BBox
  vcg::tri::UpdateBounding<CMesh>::Box(mesh);
  FullBBox.Add(mesh.bbox);

  float diag = 2.0f/FullBBox.Diag();
  cout << "diag : " << diag << endl;
  cout << "center : " << FullBBox.Center()[0] << " " << FullBBox.Center()[1] << " " << FullBBox.Center()[2] << endl;

  //vcg::tri::UpdateNormals<CMesh>::PerVertex(mesh);

  CMesh::VertexIterator vit;

  unsigned int pos = 0;
  for (vit = mesh.vert.begin(); vit != mesh.vert.end(); ++vit) {
	
	vcg::Point3f p = (*vit).P();
	Point p_lal (p[0], p[1], p[2]);
	vcg::Point3f n = (*vit).N();
	Vector n_lal (n[0], n[1], n[2]);
 	vcg::Color4b c = (*vit).C();
 	LAL::Color c_lal (c[0]/255.0, c[1]/255.0, c[2]/255.0);

	double radius = (double)((*vit).Q());
	if (radius <= 0.0)
	  radius = 0.001;

	surfels->push_back ( Surfeld (p_lal, n_lal, c_lal, radius, pos) );
	++pos;
  }
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

  readFile ( filename, (primitives.back()).getSurfels() );

  // connect new object to new primitive
  objects[0].addPrimitives( primitives.back().getId() );
  // Sets the default rendering algorithm
  primitives[0].setRendererType( render_mode );

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

  cout << primitives.size() << " : " << filename << endl;;

  primitives.push_back( Primitives( primitives.size() ) );
  int id = primitives.back().getId();

  objects[0].addPrimitives( id );

  readFile ( filename, (primitives.back()).getSurfels() );

//   if (primitives.size() == 1)
// 	computeNormFactors(primitives[id].getSurfels());

//   normalize(primitives[id].getSurfels());
  
//   primitives[id].setRendererType( render_mode );

//   primitives[id].clearSurfels();

  return 0;
}

int Application::finishFileReading ( void ) { 

  for (unsigned int i = 0; i < primitives.size(); ++i) {
	primitives[i].setRendererType( render_mode );
	//primitives[i].clearSurfels();
  }

  createPointRenderer();

  return 0;
}

/// Mouse Left Button Function, starts rotation
/// @param x X coordinate of mouse click
/// @param y Y coordinate of mouse click
void Application::mouseLeftButton(int x, int y) { 
  trackball.MouseDown(x,  canvas_height-y, Trackball::BUTTON_LEFT);
}

/// Mouse Middle Button Function, zoom
/// @param x X coordinate of mouse click
/// @param y Y coordinate of mouse click
void Application::mouseMiddleButton(int x, int y) {
  trackball.MouseDown(x,  canvas_height-y, Trackball::BUTTON_MIDDLE);
  last_y = y;
}

/// Mouse Right Button Function, light translation
/// @param x X coordinate of mouse click
/// @param y Y coordinate of mouse click
void Application::mouseRightButton(int x, int y) {
  trackball_light.MouseDown(x,  canvas_height-y, Trackball::BUTTON_LEFT);  
}

/// Mouse Release Function
void Application::mouseReleaseLeftButton( void ) {
  trackball.ButtonUp(Trackball::BUTTON_LEFT);
}

/// Mouse Release Function
void Application::mouseReleaseMiddleButton( void ) {
  trackball.ButtonUp(Trackball::BUTTON_MIDDLE);
}

/// Mouse Release Function
void Application::mouseReleaseRightButton( void ) {
  trackball_light.ButtonUp(Trackball::BUTTON_LEFT);
}

/// Mouse left movement func, rotates the camera or selected object
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void Application::mouseLeftMotion(int x, int y) {
  trackball.MouseMove(x,  canvas_height-y);
}

/// Mouse middle movement func, zooms the camera or selected object
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void Application::mouseMiddleMotion(int x, int y) {
  if (y - last_y < 0)
	trackball.MouseWheel(0.25);
  else
	trackball.MouseWheel(-0.25);

  last_y = y;
}

/// Mouse middle movement func, zooms the camera or selected object
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void Application::mouseMiddleMotionShift(int x, int y) {
  trackball.MouseMove(x,  canvas_height-y);
}

/// Mouse right movement func, light translation
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void Application::mouseRightMotion(int x, int y) {
  trackball_light.MouseMove(x,  canvas_height-y);
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

void Application::increaseSelected ( void ) {
  selected++;
  if (selected > (int)primitives.size())
	selected = 0;
  cout << "selected : " << selected << endl;

}

void Application::decreaseSelected ( void ) {
  selected--;
  if (selected < 0)
	selected = primitives.size();
  cout << "selected : " << selected << endl;
}
