/**
 * Interface independent application class
 * 
 * Author : Ricardo Marroquim
 *
 * Date created : 20-12-2007
 *
 **/

#include "application.h"

#include "GL/glut.h"

/**
 * Initialize opengl and application state variables.
 * @param default_mode Defines the initial rendering mode.
 **/
Application::Application( GLint default_mode, int w, int h ) {

  canvas_width = w;
  canvas_height = h;
  windows_width = w;
  windows_height = h;

  trackball.center = vcg::Point3f(0, 0, 0);
  trackball.radius= 1;

  trackball_light.center = vcg::Point3f(0, 0, 0);
  trackball_light.radius = 1;

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

  check_for_ogl_error("Init");
}

Application::~Application( void ) {
  objects.clear();
  delete point_based_render;
}

/// Render all points with OpenGL
void Application::drawPoints(void) {
  glPointSize(1.0);
  glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

  glBegin(GL_POINTS);
  
  for (surfelVectorIter it = objects[0].getSurfels()->begin(); it != objects[0].getSurfels()->end(); ++it) {
    Color4b c = it->Color();
    glColor4f(c[0], c[1], c[2], 1.0f);
    Point3f p = it->Center();
    glVertex3f(p[0], p[1], p[2]);
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

    // static int time;
    // static int frame = 0;
    // if (frame == 0)
    // 	time = glutGet(GLUT_ELAPSED_TIME);
    // frame ++;

  if (objects.size() == 0)
    return;  

  // Clear all buffers including pyramid algorithm buffers
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  point_based_render->clearBuffers();

  // initializes matrices, perspective and look at
  setView();

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

  // Loads current OpenGl state matrices into trackball matrices
  trackball.GetView();  

  // aplly transformations : translate to origin, multiply by trackball matrix, translate back
  trackball.Apply();

  // Use bounding box to centralize and scale object
  float diag = 2.0f/FullBBox.Diag();
  glScalef(diag, diag, diag);
  glTranslatef(-FullBBox.Center()[0], -FullBBox.Center()[1], -FullBBox.Center()[2]);

  /// Get eye position rotated in inverse direction for backface culling
  Matrix44f model;
  glGetv(GL_MODELVIEW_MATRIX,model);
  Invert(model);
  vcg::Point3f vp = model* Point3f(0., 0., 0.);
 
  // Set eye for back face culling in vertex shader of projection phase
  point_based_render->setEye( Point3f(vp[0], vp[1], vp[2]) );

  // Set factor for scaling projected radii of samples in projection phase
  point_based_render->setScaleFactor( scale_factor );

  // project all objects
  if (selected == 0)
    for (unsigned int i = 0; i < objects.size(); ++i)
      point_based_render->projectSamples( &objects[i] );
  // project only selected part
  else
    point_based_render->projectSamples( &objects[selected-1] );

  // Interpolates projected surfels using pyramid algorithm (pull-push)
  point_based_render->interpolate();

  // Computes per pixel color with deferred shading
  point_based_render->draw();

  glDisable (GL_LIGHTING);
  glDisable (GL_LIGHT0);
  glDisable (GL_COLOR_MATERIAL);

  if (show_points)
    drawPoints();

  glPopMatrix();

  // if (frame == 10) {
  //   frame = 0;
  //   int endtime = glutGet(GLUT_ELAPSED_TIME);
  //   cout << "FPS : " << 10*1000.0/(endtime-time) << endl; 
  // }

  /// uncomment this to flush frames every time, so you can better compute the true time to compute one frame,
  /// but of course, this will slow down a little the rendering since the graphics board must wait for everything
  /// to finish before continuing with the next frame
  //glFinish();
}

/// Reshape func
/// @param w New window width
/// @param h New window height
void Application::reshape(int w, int h) {
  canvas_width = w;
  canvas_height = h;
  windows_width = w;
  windows_height = h;

  createPointRenderer();

  cout << "reshaping " << w << " " << h <<  endl;
}

/**
 * Changes the rendering algorithm.
 * @param type Rendering mode.
 **/
void Application::changeRendererType( int type ) {
  for (unsigned int i = 0; i < objects.size(); ++i)
    objects[i].setRendererType((point_render_type_enum) type);
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
  // else if (render_mode == PYRAMID_ELLIPSES)
  // 	point_based_render = new PyramidPointRendererElipse(canvas_width, canvas_height);
  // else if (render_mode == PYRAMID_TEMPLATES)
  //   point_based_render = new PyramidPointRendererER(canvas_width, canvas_height);

  assert (point_based_render);

  ((PyramidPointRendererBase*)point_based_render)->createShaders();  

}

/**
 * Reads a ply file using the VCG library
 * @param filename Given file name.
 * @param surfels Pointer to surfel vector to be filled with mesh data.
 **/
int Application::readSurfelFile ( const char * filename, vector<Surfeld>& surfels, bool eliptical ) {
  /** read using vcg plylib **/
  CMesh mesh;

  int mask = 0;

  tri::io::Importer<CMesh>::LoadMask(filename, mask);
  tri::io::Importer<CMesh>::Open(mesh, filename);

  bool normal_per_vertex = false;
  if (mask & vcg::tri::io::Mask::IOM_VERTNORMAL)
    normal_per_vertex = true;

  bool color_per_vertex = false;
  if (mask & vcg::tri::io::Mask::IOM_VERTCOLOR)
    color_per_vertex = true;

  bool radius_per_vertex = false;
  if (mask & vcg::tri::io::Mask::IOM_VERTRADIUS)
    radius_per_vertex = true;

  cout << "has normal per vertex : " << normal_per_vertex << endl;
  cout << "has color per vertex : " << color_per_vertex << endl;
  cout << "has radius per vertex : " << radius_per_vertex << endl;

  /// Compute BBox
  vcg::tri::UpdateBounding<CMesh>::Box(mesh);
  FullBBox.Add(mesh.bbox);

  // vcg::tri::UpdateNormals<CMesh>::PerVertex(mesh);

  // Load vertex arrays in primitive class
  unsigned int pos = 0;
  for (CMesh::VertexIterator vit = mesh.vert.begin(); vit != mesh.vert.end(); ++vit) {
	
    vcg::Point3f p = (*vit).P();
    vcg::Point3f n = (*vit).N();

    Color4b c (0.2, 0.2, 0.2, 1.0);
    if (color_per_vertex) {
      c = Color4b ((GLubyte)(*vit).C()[0], (GLubyte)(*vit).C()[1], (GLubyte)(*vit).C()[2], 1.0);
    }

    double radius = 0.25;
    if (radius_per_vertex)
      radius = (double)((*vit).R());
	
    surfels.push_back ( Surfeld (p, n, c, radius, pos) );
    ++pos;
  }

  return mesh.vn;
}

/**
 * Reads a ply file, and loads the vertices and triangles in the associated primitive.
 * @param filename Given file name.
 **/
void Application::readFile ( const char * filename, bool eliptical ) {

  // Create a new primitive from given file
  objects.push_back( Object( objects.size() ) );

  if(eliptical)
    {
      IOSurfels<double>::LoadSurfels(filename, *(objects.back()).getSurfels());

    }else
    {
      IOSurfels<double>::LoadMesh(filename, *(objects.back()).getSurfels());
    }

  //readSurfelFile ( filename, (objects.back()).getSurfels() );

  // Sets the default rendering algorithm
  objects[0].setRendererType( render_mode );

  createPointRenderer( );
}




/// Reads a single file from a list
/// This is used when reading a directory with multiples files composing a model.
/// @param filename The ply file.
/// @return Number of points read from ply file.
int Application::appendFile ( const char * filename ) { 
  // Create a new primitive from given file
  objects.push_back( Object( objects.size() ) );
  int pts = readSurfelFile ( filename, *(objects.back()).getSurfels() );
  return pts;
}

/// Finalizes the multiple files reading routine.
/// Creates all objects arrays.
int Application::finishFileReading ( void ) {

  for (unsigned int i = 0; i < objects.size(); ++i)
    objects[i].setRendererType( render_mode );
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
  for (unsigned int i = 0; i < objects.size(); ++i)
    num_pts += objects[i].numberPoints();

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
  for (unsigned int i = 0; i < objects.size(); ++i) {
    // Reset renderer type to load per vertex color or default color in vertex array
    objects[i].setRendererType( objects[i].getRendererType() );
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

/// Cycles through objects list for displaying individual parts of the model.
/// When selected = 0 displays all files
void Application::increaseSelected ( void ) {
  selected++;
  if (selected > (int)objects.size())
    selected = 0;
  cout << "selected : " << selected << endl;

}

/// Cycles through objects list for displaying individual parts of the model.
/// When selected = 0 displays all files
void Application::decreaseSelected ( void ) {
  selected--;
  if (selected < 0)
    selected = objects.size();
  cout << "selected : " << selected << endl;
}
