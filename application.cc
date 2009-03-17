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

  canvas_width = canvas_height = 512;
  windows_width = windows_height = canvas_width + 2*(canvas_width/32);

  trackball.center = vcg::Point3f(0, 0, 0);
  trackball.radius= 1;

  trackball_light.center = vcg::Point3f(0, 0, 0);
  trackball_light.radius= 1;

  clipRatioNear = 1.0;
  clipRatioFar = 10.0;
  fov = 45.0;

  render_mode = default_mode;

  point_based_render = NULL;

  fps_loop = 0;

  rotating = 0;
  show_points = false;
  selected = 0;

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

  Point3f ambient_light = Point3f( 0.0, 0.0, 0.0 );
  Point3f diffuse_light = Point3f( 1.0, 1.0, 1.0 );
  Point3f specular_light = Point3f( 1.0, 1.0, 1.0 );

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
  Point3f p = s->Center();
  glVertex3f(p[0], p[1], p[2]);
}

/// Renders a surfel as a opengl point primitive
/// @param s Pointer to surfel
void Application::glVertex(const surfelVectorIter it) const {
  Point3f p = it->Center();
  glVertex3f(p[0], p[1], p[2]);
}

/// Renders a vertex
/// @param p Given vertex position
void Application::glVertex(const Point3f p) const {
  glVertex3f(p[0], p[1], p[2]);
}

/// Render all points with OpenGL
void Application::drawPoints(void) {
  glPointSize(1.0);
  glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

  glBegin(GL_POINTS);
  
  for (surfelVectorIter it = primitives[0].getSurfels()->begin(); it != primitives[0].getSurfels()->end(); ++it) {
	Color4b c = it->Color();
	glColor4f(c[0], c[1], c[2], 1.0f);  
	glVertex(it);
  }
  glEnd();
}

void Application::setView( void )
{

  glViewport(0, 0, windows_width, windows_height);
  GLfloat fAspect = (GLfloat)windows_width/ (GLfloat)windows_height;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // Si deve mettere la camera ad una distanza che inquadri la sfera unitaria bene.

  float ratio = 1.75f;
  float objDist = ratio / tanf(vcg::math::ToRad(fov*.5f));
  
  float nearPlane = objDist - 2.f*clipRatioNear;
  float farPlane =  objDist + 10.f*clipRatioFar;

  if(nearPlane<=objDist*.1f) nearPlane=objDist*.1f;

  if(fov==5)
	glOrtho(-ratio*fAspect,ratio*fAspect,-ratio,ratio,objDist - 2.f*clipRatioNear, objDist+2.f*clipRatioFar);
  else    		
	gluPerspective(fov, fAspect, nearPlane, farPlane);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0, 0, objDist, 0, 0, 0, 0, 1, 0);

  // Compute factor for scaling projected sample radius size
  // the usual perspective foreshortening should take into account the
  // near plane, but since the viewport transformation already scales
  // accordingly, there is no need to multiple by N
  // Not 100% sure about this, but it is working better with scaling by N
  scale_factor = 1.0 / (tanf(vcg::math::ToRad(fov*.5f)) * 2.0);
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

  setView();
  // Reset camera position and direction
//   glMatrixMode(GL_PROJECTION);
//   glLoadIdentity();
//   glViewport(0, 0, canvas_width, canvas_height);
//   gluPerspective( 60, (GLfloat)canvas_width/(GLfloat)canvas_height, 0.001, 100.0 );
  
//   glMatrixMode(GL_MODELVIEW);
//   glLoadIdentity();

//   Point camera_offset (0, 0, 5);
//   gluLookAt(camera_offset[0], camera_offset[1], camera_offset[2],   0,0,0,   0,1,0);

  /** Set light direction **/
  glPushMatrix();
  trackball_light.GetView();
  trackball_light.Apply();
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  static float lightPosF[]={0.0, 0.0, 1.0, 0.0};
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosF);
  glPopMatrix();
  /** ******************** **/

  // Apply trackball transformation
  glPushMatrix();
  trackball.GetView();
  trackball.Apply();

  // Use bounding box to centralize and scale object
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
  vcg::Point3f vp = tr.camera.ViewPoint();
  glPopMatrix();

//   vcg::Matrix44f rotM;
//   trackball.track.rot.ToMatrix(rotM); 
//   vcg::Invert(rotM);
//   vcg::Point3<float> vp = rotM*vcg::Point3f(0, 0, camera_offset[2]);
  
  // Set eye for back face culling in vertex shader
  point_based_render->setEye( Point3f(vp[0], vp[1], vp[2]) );

  // Set factor for scaling projected radii of samples
  point_based_render->setScaleFactor( scale_factor ); 

  // project all primitives
  if (selected == 0)
	for (unsigned int i = 0; i < primitives.size(); ++i)
	  point_based_render->projectSamples( &primitives[i] );
  // project only selected part
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
int Application::readFile ( const char * filename, vector<Surfeld> *surfels ) {
  /** read using vcg plylib **/
  CMesh mesh;

  int mask = 0;

  tri::io::Importer<CMesh>::LoadMask(filename, mask);
  tri::io::Importer<CMesh>::Open(mesh, filename);

  bool color_per_vertex = false;
  if (mask & vcg::tri::io::Mask::IOM_VERTCOLOR)
	color_per_vertex = true;

  bool quality_per_vertex = false;
  if (mask & vcg::tri::io::Mask::IOM_VERTQUALITY)
	quality_per_vertex = true;

//  cout << "has normal per vertex " << quality_per_vertex << endl;
//  cout << "has color per vertex " << color_per_vertex << endl;
//  cout << "has radius per vertex " << mesh.HasPerVertexQuality() << endl;

  /// Compute BBox
  vcg::tri::UpdateBounding<CMesh>::Box(mesh);
  FullBBox.Add(mesh.bbox);

  // vcg::tri::UpdateNormals<CMesh>::PerVertex(mesh);

  // Load vertex arrays in primitive class
  unsigned int pos = 0;
  for (CMesh::VertexIterator vit = mesh.vert.begin(); vit != mesh.vert.end(); ++vit) {
	
	vcg::Point3f p = (*vit).P();
	vcg::Point3f n = (*vit).N();

 	double quality = 1.0;
	if (quality_per_vertex)
	  quality = (double)((*vit).Q());

	Color4b c (0.2, 0.2, 0.2, 1.0);
	if (color_per_vertex) {
	  c = Color4b ((GLubyte)(*vit).C()[0], (GLubyte)(*vit).C()[1], (GLubyte)(*vit).C()[2], 1.0);
	}

    double radius = (double)((*vit).Q());
	//double radius = 0.25;
	
	surfels->push_back ( Surfeld (p, n, c, quality, radius, pos) );
	++pos;
  }

  return mesh.vn;
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

  //cout << primitives.size() << " : " << filename << endl;

  primitives.push_back( Primitives( primitives.size() ) );
  int id = primitives.back().getId();

  objects[0].addPrimitives( id );

  int pts = readFile ( filename, (primitives.back()).getSurfels() );

//   if (primitives.size() == 1)
// 	computeNormFactors(primitives[id].getSurfels());
//   normalize(primitives[id].getSurfels());
//  primitives[id].setRendererType( render_mode );
//   primitives[id].clearSurfels();

  return pts;
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
/// @param shift Flag for shift key state down/up
/// @param ctrl Flag for control key state down/up
/// @param alt Flag for alt key state down/up
void Application::mouseLeftButton(int x, int y, bool shift, bool ctrl, bool alt) {
  int button = Trackball::BUTTON_LEFT;
  if (shift) button = button | Trackball::KEY_SHIFT;
  if (ctrl) button = button | Trackball::KEY_CTRL;
  if (alt) button = button | Trackball::KEY_ALT;
  trackball.MouseDown(x, canvas_height-y, button );
}

/// Mouse Middle Button Function, zoom
/// @param x X coordinate of mouse click
/// @param y Y coordinate of mouse click
/// @param shift Flag for shift key state down/up
/// @param ctrl Flag for control key state down/up
/// @param alt Flag for alt key state down/up
void Application::mouseMiddleButton(int x, int y, bool shift, bool ctrl, bool alt) {
  int button = Trackball::BUTTON_MIDDLE;
  if (shift) button = button | Trackball::KEY_SHIFT;
  if (ctrl) button = button | Trackball::KEY_CTRL;
  if (alt) button = button | Trackball::KEY_ALT;
  trackball.MouseDown(x, canvas_height-y, button );
}

/// Mouse Right Button Function, light translation
/// @param x X coordinate of mouse click
/// @param y Y coordinate of mouse click
/// @param shift Flag for shift key state down/up
/// @param ctrl Flag for control key state down/up
/// @param alt Flag for alt key state down/up
void Application::mouseRightButton(int x, int y, bool shift, bool ctrl, bool alt) {
  trackball_light.MouseDown(x, canvas_height-y, Trackball::BUTTON_LEFT);  
}

/// Mouse Release Function
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
/// @param shift Flag for shift key state down/up
/// @param ctrl Flag for control key state down/up
/// @param alt Flag for alt key state down/up
void Application::mouseReleaseLeftButton(int x, int y, bool shift, bool ctrl, bool alt ) {
  int button = Trackball::BUTTON_LEFT;
  if (shift) button = button | Trackball::KEY_SHIFT;
  if (ctrl) button = button | Trackball::KEY_CTRL;
  if (alt) button = button | Trackball::KEY_ALT;
  trackball.MouseUp(x, canvas_height-y, button );
}

/// Mouse Release Function
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
/// @param shift Flag for shift key state down/up
/// @param ctrl Flag for control key state down/up
/// @param alt Flag for alt key state down/up
void Application::mouseReleaseMiddleButton(int x, int y, bool shift, bool ctrl, bool alt ) {
  int button = Trackball::BUTTON_MIDDLE;
  if (shift) button = button | Trackball::KEY_SHIFT;
  if (ctrl) button = button | Trackball::KEY_CTRL;
  if (alt) button = button | Trackball::KEY_ALT;
  trackball.MouseUp(x, canvas_height-y, button );
}

/// Mouse Release Function
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
/// @param shift Flag for shift key state down/up
/// @param ctrl Flag for control key state down/up
/// @param alt Flag for alt key state down/up
void Application::mouseReleaseRightButton(int x, int y, bool shift, bool ctrl, bool alt ) {
  trackball_light.MouseUp(x, canvas_height-y, Trackball::BUTTON_LEFT);
}

/// Mouse left movement func, rotates the camera or selected object
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
/// @param shift Flag for shift key state down/up
/// @param ctrl Flag for control key state down/up
/// @param alt Flag for alt key state down/up
void Application::mouseLeftMotion(int x, int y, bool shift, bool ctrl, bool alt ) {
  trackball.MouseMove(x, canvas_height-y);
}

/// Mouse middle movement func, zooms the camera or selected object
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
/// @param shift Flag for shift key state down/up
/// @param ctrl Flag for control key state down/up
/// @param alt Flag for alt key state down/up
void Application::mouseMiddleMotion(int x, int y, bool shift, bool ctrl, bool alt ) {
  trackball.MouseMove(x, canvas_height-y);
}

/// Mouse right movement func, light translation
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
/// @param shift Flag for shift key state down/up
/// @param ctrl Flag for control key state down/up
/// @param alt Flag for alt key state down/up
void Application::mouseRightMotion(int x, int y, bool shift, bool ctrl, bool alt) {
  trackball_light.MouseMove(x, canvas_height-y);
}

/// Mouse wheel control
/// @param step Wheel direction +1 (forward) or -1(back)
/// @param shift Flag for shift key state down/up
/// @param ctrl Flag for control key state down/up
/// @param alt Flag for alt key state down/up
void Application::mouseWheel( int step, bool shift, bool ctrl, bool alt ) {
  float notch = 0.3 * step;

  if (shift && ctrl) 
	clipRatioFar *= powf(1.2f, notch);
  else if (shift)
	fov = math::Clamp(fov*powf(1.2f,notch),5.0f,90.0f);
  else if (ctrl)
	clipRatioNear *= powf(1.2f, notch);  
  else if (ctrl && alt)
	clipRatioNear *= powf(1.2f, notch);  
  else
	trackball.MouseWheel( notch );
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
 * Sets the quality threshold for interpolating samples.
 * @param q Quality threshold.
 **/
void Application::setQualityThreshold ( double q ) { 
  if (point_based_render)
    point_based_render->setQualityThreshold(q);
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

/**
 * Turns quality per vertex on/off.
 * @param c Quality per vertex state.
 **/
void Application::setQualityPerVertex ( bool c ) {
  point_based_render->setQualityPerVertex(c);
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
