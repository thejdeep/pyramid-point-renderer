/**
 * Interface independent application class
 * 
 * Author : Ricardo Marroquim
 *
 * Date created : 20-12-2007
 *
 **/

#define GL_GLEXT_PROTOTYPES

#include "file_io.h"
#include <GL/glut.h>

// #define CANVAS_WIDTH  1024
// #define CANVAS_HEIGHT 1024
 


// #define CANVAS_WIDTH  512
// #define CANVAS_HEIGHT 512

/// Initialize global variables and opengl states
Application::Application( void ) {

  // Initialize camera with window canvas size
  camera = new Camera(CANVAS_WIDTH, CANVAS_HEIGHT);

  render_mode = GL_RENDER;

  point_based_render = NULL;

  number_surfels = 0;
  fps_loop = 0;
  active_shift = 0;
  analysis_filter_size = 0;

  lods_perc = true;

  color_model = true;
  elliptical_weight = true;
  depth_culling = true;
  rotating = 0;

  show_screen_info = true;
  show_points = false;
  show_splats = 1;

  timing_profile = 0;
  material_id = 0;
  selected_objs.clear();

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

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  camera->initLight();

  changeMaterial();

  glEnable(GL_NORMALIZE);
  CHECK_FOR_OGL_ERROR()	;
}

/// Renders a surfel as a opengl point primitive
/// @param s Pointer to surfel
void Application::glVertex(const Surfel * s) {
  Point p = s->position();
  glVertex3f(p.x(), p.y(), p.z());
}

/// Renders a surfel as a opengl point primitive
/// @param s Pointer to surfel
void Application::glVertex(surfelVectorIter it) {
  Point p = it->position();
  glVertex3f(p.x(), p.y(), p.z());
}

/// Renders a vertex
/// @param p Given vertex position
void Application::glVertex(Point p) {
  glVertex3f(p.x(), p.y(), p.z());
}

/// Render all points 
void Application::drawPoints(void)
{
  glPointSize(1.0);
  glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

  glBegin(GL_POINTS);
  
  for (int i = 0; i < num_objects; ++i) {
    vector< int > *prims = objects[i].getPrimitivesList();
    for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it)
      for (surfelVectorIter it = primitives[*prim_it].getSurfels()->begin(); it != primitives[*prim_it].getSurfels()->end(); ++it)
	glVertex(it);
  }
  glEnd();
}

/// Display func
void Application::draw(void) {

  if (primitives.size() == 0)
    return;

  for (int i = 0; i < 5; ++i)
    surfs_per_level[i] = 0;

  // Clear all buffers including pyramid algorithm buffers
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  point_based_render->clearBuffers();

  // Render objects primitives with pyramid algorithm
  for (unsigned int i = 0; i < objects.size(); ++i) {
//   for (vector<int>::iterator it = selected_objs.begin(); it != selected_objs.end(); ++it) {
//     int i = *it;
    
    // Reset camera position and direction
    camera->setView();

    // Compute rotated eye position for this object for back face culling   
    double eye[3];
    eye[0] = objects[i].getCenter()[0];
    eye[1] = objects[i].getCenter()[1];
    eye[2] = objects[i].getCenter()[2];

    // Compute the rotated eye (opposite direction) of the camera + object center position
    camera->computeEyePosition(*(objects[i].getRotationQuat()), eye);

//     double eye[3];
//     eye[0] = camera->positionVector()[0];
//     eye[1] = camera->positionVector()[1];
//     eye[2] = camera->positionVector()[2];
    
    point_based_render->setEye(eye);

    // Translate and rotate object
    objects[i].render();

    // Projects to image plane surfels of all primitives for this object
    vector< int >* prims = objects[i].getPrimitivesList();
    for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it) {
      Primitives * prim = &(primitives[*prim_it]);
      int type = prim->getRendererType();
      if ((type != TRIANGLES) && (type != LINES) && (type != NONE)) {
	if (type == PYRAMID_POINTS_LOD)
	  point_based_render->useLOD( true );
	else
	  point_based_render->useLOD( false );


	prim->eye = Point(eye[0], eye[1], eye[2]);
	prim->countNumVertsLOD(&surfs_per_level[0]);
	point_based_render->projectSamples( prim );	
      }
    }
  }

  //  cout << surfs_per_level[0] << " " << surfs_per_level[1] << " " << surfs_per_level[2] << " " << surfs_per_level[3] << endl;


  // Interpolates projected surfels using pyramid algorithm
  point_based_render->interpolate();
  // Computes per pixel color with deferred shading
  point_based_render->draw();

  // Only render objects without algorithm pyramid, i.e. opengl triangles and lines
  for (unsigned int i = 0; i < objects.size(); ++i) {
//   for (vector<int>::iterator it = selected_objs.begin(); it != selected_objs.end(); ++it) {
//     int i = *it;
    // Reset camera position and direction
    camera->setView();
    
    // Translate and rotate object
    objects[i].render();

    // Render primitives using opengl triangles or lines
    vector< int >* prims = objects[i].getPrimitivesList();
    for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it) {
      Primitives * prim = &(primitives[*prim_it]);
      int type = prim->getRendererType();
      if ((type == TRIANGLES) || (type == LINES))
	prim->render();
    }
  }
  
  glDisable (GL_LIGHTING);
  glDisable (GL_LIGHT0);
  glDisable (GL_COLOR_MATERIAL);

  if (show_points)
    drawPoints();

  screenText();

  if (rotating)
    camera->rotate();

}

/// Screen text with commands info
void Application::screenText( void ) {

  int w = 400;
  int h = 200;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(24, 24, w+24, h+24);

  gluOrtho2D(0.0, 1.5, 0.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  /****/

  int total;
  if (lods_perc)
    total = surfs_per_level[0] + surfs_per_level[1] + surfs_per_level[2] + surfs_per_level[3];
  else
    total = surfs_per_level[4] / 10.0;
  double x_max = 0.0;

  // BACKGROUND
//   glColor4f(0.0, 0.0, 0.0, 1.0);
//   glBegin (GL_POLYGON);
//   glVertex2f (0.0, 0.0);
//   glVertex2f (1.5, 0.0);
//   glVertex2f (1.5, 1.0);
//   glVertex2f (0.0, 1.0);
//   glEnd();

  // SILVER
  x_max = (double)surfs_per_level[3] / (double)total;
  glColor4f(0.507540+0.192250, 0.507540+0.192250, 0.507540+0.192250, 1.000000);
  glBegin (GL_POLYGON);
  glVertex2f (0.0, 0.0);
  glVertex2f (x_max, 0.0);
  glVertex2f (x_max, 0.1);
  glVertex2f (0.0, 0.1);
  glEnd();

  // GOLD
  x_max = (double)surfs_per_level[2] / (double)total;
  glColor4f(0.751640+0.247250, 0.606480+0.199500, 0.226480+0.074500, 1.000000);
  glBegin (GL_POLYGON);
  glVertex2f (0.0, 0.15);
  glVertex2f (x_max, 0.15);
  glVertex2f (x_max, 0.25);
  glVertex2f (0.0, 0.25);
  glEnd();

  // TURQUOISE
  x_max = (double)surfs_per_level[1] / (double)total;
  glColor4f(0.396000+0.100000, 0.741510+0.187250, 0.691020+0.174500, 1.000000);
  glBegin (GL_POLYGON);
  glVertex2f (0.0, 0.3);
  glVertex2f (x_max, 0.3);
  glVertex2f (x_max, 0.4);
  glVertex2f (0.0, 0.4);
  glEnd();

  // RUBY
  x_max = (double)surfs_per_level[0] / (double)total;
  glColor4f(0.614240+0.174500, 0.041360+0.011750, 0.041360+0.011750, 1.000000);
  glBegin (GL_POLYGON);
  glVertex2f (0.0, 0.45);
  glVertex2f (x_max, 0.45);
  glVertex2f (x_max, 0.55);
  glVertex2f (0.0, 0.55);
  glEnd();
  /****/
  //  glColor3f(1.0, 1.0, 1.0);
  glColor3f(0.0, 0.0, 0.0);


  char text[5][20];
  if (lods_perc) {
    sprintf(text[0], "%.1f\%", (surfs_per_level[0] / (double)total) * 100.0);
    strcat(text[0], "%");
    sprintf(text[1], "%.1f", (surfs_per_level[1] / (double)total) * 100.0);
    strcat(text[1], "%");
    sprintf(text[2], "%.1f", (surfs_per_level[2] / (double)total) * 100.0);
    strcat(text[2], "%");
    sprintf(text[3], "%.1f", (surfs_per_level[3] / (double)total) * 100.0);
    strcat(text[3], "%");
    sprintf(text[4], "Total points rendered : %d", total);
  }
  else {
    sprintf(text[0], "%d", surfs_per_level[0]);
    sprintf(text[1], "%d", surfs_per_level[1]);
    sprintf(text[2], "%d", surfs_per_level[2]);
    sprintf(text[3], "%d", surfs_per_level[3]);
    sprintf(text[4], "Total points in all levels : %d", surfs_per_level[4]);
  }
  
  void* font = GLUT_BITMAP_HELVETICA_18;


  glRasterPos2d(((double)surfs_per_level[3] / (double)total) + 0.01, 0.0);
  for (char *s = &text[3][0]; *s; ++s)
    glutBitmapCharacter(font, *s);

  glRasterPos2d(((double)surfs_per_level[2] / (double)total) + 0.01, 0.15);
  for (char *s = &text[2][0]; *s; ++s)
    glutBitmapCharacter(font, *s);

  glRasterPos2d(((double)surfs_per_level[1] / (double)total) + 0.01, 0.30);
  for (char *s = &text[1][0]; *s; ++s)
    glutBitmapCharacter(font, *s);

  glRasterPos2d(((double)surfs_per_level[0] / (double)total) + 0.01, 0.45);
  for (char *s = &text[0][0]; *s; ++s)
    glutBitmapCharacter(font, *s);

  glRasterPos2d(0.0, 0.6);
  for (char *s = &text[4][0]; *s; ++s)
    glutBitmapCharacter(font, *s);


  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  camera->setView ();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

/// Reshape func
/// @param w New window width
/// @param h New window height
void Application::reshape(int w, int h) {

  //  glutReshapeWindow (w, h);

  camera->reshape(w, h);

  //  glutPostRedisplay();
}

/// Unproject a point from screen coordinates to world coordinates
/// @param p Point to be unprojected
/// @return Point in world coordinates
Point Application::unproject (const Point& p) {

  GLint viewport[4];
  GLdouble modelview[16];
  GLdouble projection[16];
  GLfloat winX, winY, winZ;
  GLdouble posX, posY, posZ;
      
  glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
  glGetDoublev( GL_PROJECTION_MATRIX, projection );
  glGetIntegerv( GL_VIEWPORT, viewport );

  winX = (float)p.x();
  winY = (float)viewport[3] - (float)p.y();      
  winZ = (float)p.z();
  gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
    
  return Point (posX, posY, posZ);
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

void Application::changeRendererType( point_render_type_enum type, int object_id ) {
  for (vector<int>::iterator it = selected_objs.begin(); it != selected_objs.end(); ++it) {
    //  for (unsigned int i = 0; i < objects.size(); ++i){
    vector< int >* prims = objects[*it].getPrimitivesList();
    for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it)
      primitives[*prim_it].setRendererType(type);
  }
  // Resets the color material
  changeMaterial();
}

void Application::changeRendererType( int type, int object_id ) {
  changeRendererType ( (point_render_type_enum)type, object_id );
}

void Application::createPointRender( int type ) {

  if (point_based_render)
    delete point_based_render;

  if (type == 0) {
    if (color_model)
      point_based_render = new PyramidPointRenderColor(CANVAS_WIDTH, CANVAS_HEIGHT);
    else
      point_based_render = new PyramidPointRender(CANVAS_WIDTH, CANVAS_HEIGHT);
  }
  else if (type == 1){
    point_based_render = new PyramidPointRenderTrees(CANVAS_WIDTH, CANVAS_HEIGHT);
  }
//   else if (type == 2) {
//     point_based_render = new PyramidPointRenderColor(CANVAS_WIDTH, CANVAS_HEIGHT);
//     point_based_render->useLOD(true);
//   }

  assert (point_based_render);

  point_based_render->setReconstructionFilterSize(reconstruction_filter_size);
  point_based_render->setPrefilterSize(prefilter_size);
  point_based_render->setDepthTest(depth_culling);
}

int Application::readSceneFile (const char * filename, vector<int> *objs_ids) {
  // Create a new primitive from given file

  int num_objs = readObjsFiles (filename, &primitives, &objects, objs_ids, camera);

  if ( num_objs > 0  ) {

    num_objects = objects.size();

    // Count total number of points being rendered
    for (vector<int>::iterator it = objs_ids->begin(); it < objs_ids->end(); ++it) {
      vector< int >* prims = objects[*it].getPrimitivesList();
      for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it) {
	number_surfels += primitives[*prim_it].getSurfels()->size();
      }
    }

    createPointRender( 0 );

    return num_objs;
  }

  return -1;
}

int Application::readPolFile (const char * filename, vector<int> *objs_ids) {
  // Create a new primitive from given file

  int num_objs = readTreeFiles (filename, &primitives, &objects, objs_ids);

  if ( num_objs > 0  ) {

    num_objects = objects.size();

    // Count total number of points being rendered
    for (vector<int>::iterator it = objs_ids->begin(); it < objs_ids->end(); ++it) {
      vector< int >* prims = objects[*it].getPrimitivesList();
      for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it) {
	number_surfels += primitives[*prim_it].getSurfels()->size();
      }
    }

    //  if (!point_based_render)
    createPointRender( 1 );

    return num_objs;
  }

  return -1;
}

/**
 * Reads a ply file, creates an object and
 * loads the vertices and triangles in the associated primitive.
 * @param filename Given file name.
 * @return Id number of created object.
 **/
int Application::readFile ( const char * filename ) {
  // read the models passed as command line arguments
  //  int read = readFile(argc, argv, &primitives, &objects);

  // Create a new primitive from given file
  primitives.push_back( Primitives( primitives.size() ) );

  primitives.back().setPerVertexColor(0);

  // Create a new object
  int id = objects.size();
  objects.push_back( Object( id ) );
  objects.back().setFilename( filename );

  readPlyTrianglesColor (filename, (primitives.back()).getSurfels(), (primitives.back()).getTriangles());

  // connect new object to new primitive
  objects.back().addPrimitives( primitives.back().getId() );
  primitives.back().setType( 1.0 );
  primitives.back().setRendererType( PYRAMID_POINTS );

  num_objects = objects.size();

  // Count total number of points being rendered
  number_surfels += primitives.back().getSurfels()->size();
  
  //  if (!point_based_render)
  createPointRender( 0 );

  return id;
}

int Application::readLodFile ( const char * filename ) {

  char lodFilename[200];
  strcpy(lodFilename, filename);
  char* ptr = strstr(filename, ".lod");
  strncpy (ptr, "", 4);

  int id = objects.size();
  //  for (vector<Primitives>::iterator it = primitives.begin(); it != primitives.end(); ++it, ++id) {

  // Create a new object and connect to new primitive
  objects.push_back( Object( id ) );
  objects.back().setFilename( lodFilename );

  primitives.push_back( Primitives( primitives.size() ) );
  
  readPlyTrianglesColor (filename, (primitives.back()).getSurfels(), (primitives.back()).getTriangles());

  primitives.back().readFileLOD(lodFilename);
  
//   } else {

//     cout << "reading " << filename << " ..." << flush;

//     // Create a new primitive from given file
//     readPlyTriangles (filename, (primitives.back()).getSurfels(), (primitives.back()).getTriangles());
//     //    readPlyTriangles (filename, (primitives.back()).getSurfels());

//     cout << " readed! " << endl;

//     primitives.back().createLOD();

//     cout << "writting in file " << str << flush;

//     primitives.back().writeFileLOD(str.c_str());

//     cout << " written! " << endl;

    primitives.back().setType( 1.0 );
    primitives.back().setRendererType( PYRAMID_POINTS_LOD );

//   }

  cout << endl
       << "### Number of PATCHES in ###" << endl
       << "Lowest Resolution : " << primitives.back().numPrimitivesIn()  << endl
       << "All Resolutions   : " << primitives.back().numPrimitivesLOD() << endl
       << "Level 0   : " << primitives.back().numPrimitivesInLevel(0) << endl
       << endl;

  objects.back().addPrimitives( primitives.back().getId() );

  num_objects = objects.size();

  // Count total number of points being rendered
  number_surfels += primitives.back().getSurfels()->size();

  cout << "objects : " << num_objects << endl;
  cout << "primitives : " << primitives.size() << endl;
  cout << "number of surfels : " << number_surfels << endl;
  
  //  if (!point_based_render)
  createPointRender( 0 );

  cout << "created point renderer" << endl;

  return id;
}

int Application::writeSceneFile ( void ) {
  char * fn = "../plys/scene.scn";
  ofstream out(fn, ios::trunc);

  out << "#Scene file : type;renderType;material id -- id;x;y;z;q.x;q.y;q.z;q.a;n" << endl;
  out << "PRIMITIVES " << primitives.size() << endl;
  out << "OBJECTS " << objects.size() << endl;

  for (int i = 0; i < primitives.size(); ++i) {
    out << i << " " << objects[i].filename() << " " << 1.0 << " " << primitives[i].getRendererType() << " " <<
      primitives[i].getMaterial() << endl;
  }

  for (int i = 0; i < objects.size(); ++i) {
    out << i << " " << objects[i].getCenter()[0] << " " << objects[i].getCenter()[1] << " " << objects[i].getCenter()[2] << " " <<
      objects[i].getRotationQuat()->x << " " << objects[i].getRotationQuat()->y << " " << objects[i].getRotationQuat()->z << " " <<
      objects[i].getRotationQuat()->a  << " 1 " << i << endl;
  }

  out << camera->positionVector()[0] << " " << camera->positionVector()[1] << " " << camera->positionVector()[2] << " " <<
    camera->rotationQuat().x << " " << camera->rotationQuat().y << " " << camera->rotationQuat().z << " " << camera->rotationQuat().a << endl;
  
  out << camera->lightVector()[0] << " " << camera->lightVector()[1] << " " << camera->lightVector()[2] << endl;

  cout << "Wrote scene : scene.scn" << endl;

}

int Application::writeLodFile ( void ) {
  if (selected_objs.empty())
    return -1;

  Object *obj = &objects[selected_objs.front()];

  Primitives *prim = &primitives[obj->getPrimitivesList()->front()];

  prim->createLOD();

  char lodFilename[200];
  strcpy (lodFilename, obj->filename());
  strcat (lodFilename, ".lod");

  cout << "writting in file " << lodFilename << flush;

  prim->writeFileLOD( lodFilename );
  
  cout << " written! " << endl;
  return 1;
}

/// Mouse Left Button Function, starts rotation
/// @param x X coordinate of mouse click
/// @param y Y coordinate of mouse click
void Application::mouseLeftButton(int x, int y) {
  
//   if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
//     active_shift = 1;
//   else
//     active_shift = 0;

  //Point click = unproject(Point (x, y, 0.0));

  if (selected_objs.empty())
    camera->startRotation(x, y);
  else {
    for (vector< int >::iterator it = selected_objs.begin(); it != selected_objs.end(); ++it)
      camera->startQuatRotation(x, y, objects[*it].getRotationQuat(), objects[*it].getCenter());
  }
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

  if (selected_objs.empty())
    camera->rotate(x, y);
  else {
    for (vector< int >::iterator it = selected_objs.begin(); it != selected_objs.end(); ++it)
      camera->rotateQuat(x, y, objects[*it].getRotationQuat(), objects[*it].getCenter());
  }
}

/// Mouse middle movement func, zooms the camera or selected object
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void Application::mouseMiddleMotion(int x, int y) {
  if (selected_objs.empty())
    camera->zooming (x, y);
  else {
    for (vector< int >::iterator it = selected_objs.begin(); it != selected_objs.end(); ++it)
      camera->zoomingVec(x, y, objects[*it].getCenter());
    camera->updateMouse();
  }
}

/// Mouse middle movement func, zooms the camera or selected object
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void Application::mouseMiddleMotionShift(int x, int y) {
  if (selected_objs.empty())
    camera->translate(x, y);
  else {
    for (vector< int >::iterator it = selected_objs.begin(); it != selected_objs.end(); ++it)
      camera->translateVec(x, y, objects[*it].getCenter());
    camera->updateMouse();
  }
}

/// Mouse right movement func, light translation
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void Application::mouseRightMotion(int x, int y) {
  camera->lightTranslate(x, y);
}

void Application::clearSelectedObjects ( void ) {
  selected_objs.clear();
}

void Application::setSelectedObject ( int id ) {    
  selected_objs.push_back( id );
}

int Application::getRendererType ( int object_id ) {

    // Projects to image plane surfels of all primitives for this object
  vector< int >* prims = objects[object_id].getPrimitivesList();
  Primitives * prim = &(primitives[prims->front()]);
  return (int)prim->getRendererType();
}

int Application::getNumberPoints ( int object_id ) {

  int num = 0;
  // Projects to image plane surfels of all primitives for this object
  vector< int >* prims = objects[object_id].getPrimitivesList();
  for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it) {
    Primitives * prim = &(primitives[*prim_it]);
    num += prim->numberPoints();
  }

  return num;
}

int Application::getNumberTriangles ( int object_id ) {

  int num = 0;
  // Projects to image plane surfels of all primitives for this object
  vector< int >* prims = objects[object_id].getPrimitivesList();
  for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it) {
    Primitives * prim = &(primitives[*prim_it]);
    num += prim->numberTriangles();
  }

  return num;
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

void Application::setPerVertexColor ( bool b, int object_id ) {
  vector< int >* prims = objects[object_id].getPrimitivesList();
  for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it) {
    primitives[*prim_it].setPerVertexColor(b);
    // Reset renderer type to load per vertex color or default color in vertex array
    primitives[*prim_it].setRendererType( primitives[*prim_it].getRendererType() );
  }
}

void Application::setAutoRotate ( bool r ) {
  rotating = r;
}

void Application::setDepthTest ( bool d ) {
  depth_culling = d;
  if (point_based_render)
    point_based_render->setDepthTest(depth_culling);
}

void Application::useLOD ( bool lod, int object_id ) {
  int type = PYRAMID_POINTS;
  if ( lod )
    type = PYRAMID_POINTS_LOD;

  //  for (unsigned int i = 0; i < objects.size(); ++i){
  for (vector<int>::iterator it = selected_objs.begin(); it != selected_objs.end(); ++it) {
    vector< int >* prims = objects[*it].getPrimitivesList();
    for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it)
      primitives[*prim_it].setRendererType( type );
  }
}

void Application::changeMaterial( int mat, int object_id ) {
  for (vector<int>::iterator it = selected_objs.begin(); it != selected_objs.end(); ++it) {
    vector< int >* prims = objects[*it].getPrimitivesList();
    for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it) {
      primitives[*prim_it].setMaterial( mat );
      primitives[*prim_it].setRendererType( primitives[*prim_it].getRendererType() );
    }
  }  
}

void Application::setLodColors ( bool c ) {
  point_based_render->useColorPerLOD( c );
}

void Application::switchLodsPerc ( void ) {
  lods_perc = !lods_perc;
}
