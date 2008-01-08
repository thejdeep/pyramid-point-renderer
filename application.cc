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

// #define CANVAS_WIDTH  1024
// #define CANVAS_HEIGHT 1024
 
#define CANVAS_WIDTH  768
#define CANVAS_HEIGHT 768

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

  color_model = false;
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
  prefilter_size = 0.75;

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

  if (fps_loop == 0) {
    start_time = timer();
  }

  // Clear all buffers including pyramid algorithm buffers
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  point_based_render->clearBuffers();
  
  // Render objects primitives with pyramid algorithm
  for (int i = 0; i < num_objects; ++i) {

    // Reset camera position and direction
    camera->setView();

    // Compute rotated eye position for this object for back face culling
    double eye[3];
    camera->computeEyePosition(*(objects[i].getRotationQuat()), eye);
    point_based_render->setEye(eye);

    // Translate and rotate object
    objects[i].render();

    // Projects to image plane surfels of all primitives for this object
    vector< int >* prims = objects[i].getPrimitivesList();
    for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it) {
      Primitives * prim = &(primitives[*prim_it]);
      point_render_type_enum type = prim->getRendererType();
      if ((type != TRIANGLES) && (type != LINES) && (type != NONE)) {
	point_based_render->projectSamples( prim );
      }
    }
  }

  // Interpolates projected surfels using pyramid algorithm
  point_based_render->interpolate();
  // Computes per pixel color with deferred shading
  point_based_render->draw();

  // Only render objects without algorithm pyramid, i.e. opengl triangles and lines
  for (int i = 0; i < num_objects; ++i) 
    {

      // Reset camera position and direction
      camera->setView();

      // Translate and rotate object
      objects[i].render();

      // Render primitives using opengl triangles or lines
      vector< int >* prims = objects[i].getPrimitivesList();
      for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it) {
	Primitives * prim = &(primitives[*prim_it]);
	point_render_type_enum type = prim->getRendererType();
	if ((type == TRIANGLES) || (type == LINES))
	  prim->render();
      }
    }

  glDisable (GL_LIGHTING);
  glDisable (GL_LIGHT0);
  glDisable (GL_COLOR_MATERIAL);

  if (show_points)
    drawPoints();

  // compute frames per second for every 30 loops
  // fps variable is rendered as screen text
  ++fps_loop;

  if (fps_loop == 30) {

    double end_time = timer();
    fps = (end_time - start_time) / (double)fps_loop;

    fps = 1000.0 / fps;
    sps = (fps * number_surfels) / 1000000;

    fps_loop = 0;
  }

  if (rotating)
    camera->rotate();

  // Render the screen text
  if (show_screen_info) {
    glPushMatrix();
    glLoadIdentity();
//     if (render_mode == GL_RENDER)
//       screenText(CANVAS_WIDTH, CANVAS_HEIGHT);
    glPopMatrix();
  }

  //  glutSwapBuffers();
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

void Application::changeRendererType( point_render_type_enum type ) {
  if (!selected_objs.empty()) {
    for (vector< int >::iterator it = selected_objs.begin(); it != selected_objs.end(); ++it) {
      vector< int >* prims = objects[*it].getPrimitivesList();
      for (vector< int >::iterator prim_it = prims->begin(); prim_it != prims->end(); ++prim_it)
	primitives[*prim_it].setRendererType(type);
    }
  }

  // Resets the color material
  changeMaterial();
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
  else {
    point_based_render = new PyramidPointRenderTrees(CANVAS_WIDTH, CANVAS_HEIGHT);
  }

  assert (point_based_render);

  point_based_render->setReconstructionFilterSize(reconstruction_filter_size);
  point_based_render->setPrefilterSize(prefilter_size);
}


int Application::readFile ( char * filename ) {
  // read the models passed as command line arguments
  //  int read = readFile(argc, argv, &primitives, &objects);

  cout << "reading " << filename << endl;

  // Create a new primitive from given file
  primitives.push_back( Primitives( primitives.size() ) );
  readPlyTriangles (filename, (primitives.back()).getSurfels(), (primitives.back()).getTriangles());

  int id = objects.size();
  //  for (vector<Primitives>::iterator it = primitives.begin(); it != primitives.end(); ++it, ++id) {

  // Create a new object and connect to new primitive
  objects.push_back( Object( id ) );
  objects.back().addPrimitives( primitives.back().getId() );
  primitives.back().setType( 1.0 );
  primitives.back().setRendererType( PYRAMID_POINTS );

  num_objects = objects.size();

  // Count total number of points being rendered
  number_surfels += primitives.back().getSurfels()->size();

  cout << "objects : " << num_objects << endl;
  cout << "primitives : " << primitives.size() << endl;
  cout << "number of surfels : " << number_surfels << endl;
  
  //  if (!point_based_render)
  createPointRender( 0 );

  return id;
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
      camera->startQuatRotation(x, y, objects[*it].getRotationQuat());     
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
      camera->rotateQuat(x, y, objects[*it].getRotationQuat());
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
//   for (vector< int >::iterator it = selected_objs.begin(); it != selected_objs.end(); ++it)
//     cout << *it << endl;
}


/// Keyboard special keys function
/// @param key Pressed key
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
//void specialKey(int key_pressed, int x, int y) {
//   switch (key_pressed) {
//   case GLUT_KEY_F1:
//     changeRendererType(PYRAMID_POINTS);
//     show_splats = 1;
//     break;
//   case GLUT_KEY_F2:
//     changeRendererType(PYRAMID_TRIANGLES);
//     show_splats = 2;
//     break;
//   case GLUT_KEY_F3:
//     changeRendererType(PYRAMID_HYBRID);
//     show_splats = 3;
//     break;
//   case GLUT_KEY_F4:
//     changeRendererType(PYRAMID_LINES);
//     show_splats = 4;
//     break;
//   case GLUT_KEY_F5:
//     changeRendererType(TRIANGLES);
//     show_splats = 5;
//     break;
//   case GLUT_KEY_F6:
//     changeRendererType(LINES);
//     show_splats = 6;
//     break;
//   case GLUT_KEY_F7:
//     changeRendererType(PYRAMID_HYBRID_TEST);
//     show_splats = 7;
//     break;
//   case GLUT_KEY_F10:
//     changeRendererType(NONE);
//     show_splats = 10;
//     break;
//   case GLUT_KEY_F12:
//     show_splats = 0;
//     show_points = true;
//     break;
//   };
//   glutPostRedisplay();
//}

/// Keyboard keys function
/// @param key Pressed key
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
//void keyboard(unsigned char key_pressed, int x, int y) {
 //  Quat q;
//   if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
//     switch (key_pressed) {     
//     case '+' :
//     case '=' :
//       prefilter_size += 0.25;
//       point_based_render->setPrefilterSize(prefilter_size);
//       break;
//     case '-' :
//     case '_' :
//       if (prefilter_size > 0.0)
// 	prefilter_size -= 0.25;
//       point_based_render->setPrefilterSize(prefilter_size);
//       break;
//     };
//   }
//   else {
//     switch (key_pressed) {
//     case 27 :
//     case 'q' :
//     case 'Q' :
//       exit(0);
//       break;
//     case 'h' : 
//     case 'H' :
//       show_screen_info = !show_screen_info;
//       break;
//     case 'v' :
//     case 'V' :
//       camera->switchViewMode();  
//       break;
//     case 'p' :
//     case 'P' :
//       show_points = !show_points;
//       break;
//     case 'r' :
//     case 'R' :
//       rotating = !rotating;
//       break;
//     case '1' :
//       selected_obj = 0;
//       cout << "selected : 1" << endl;
//       break;
//     case '2' :
//       if (num_objects > 1) {
// 	selected_obj = 1;
// 	cout << "selected : 2" << endl;
//       }
//       break;
//     case '3' :
//       if (num_objects > 2) {
// 	selected_obj = 2;
// 	cout << "selected : 3" << endl;
//       }
//       break;
//     case '4' :
//       if (num_objects > 3) {
// 	selected_obj = 3;
// 	cout << "selected : 4" << endl;
//       }
//       break;
//     case '5' :
//       if (num_objects > 4) {
// 	selected_obj = 4;
// 	cout << "selected : 5" << endl;
//       }
//     case '6' :
//       if (num_objects > 5) {
// 	selected_obj = 5;
// 	cout << "selected : 6" << endl;
//       }
//     case '7' :
//       if (num_objects > 6) {
// 	selected_obj = 6;
// 	cout << "selected : 7" << endl;
//       }
//     case '8' :
//       if (num_objects > 7) {
// 	selected_obj = 7;
// 	cout << "selected : 8" << endl;
//       }
//     case '9' :
//       if (num_objects > 8) {
// 	selected_obj = 8;
// 	cout << "selected : 9" << endl;
//       }
//       break;
//     case '0' :
//       selected_obj = -1;
//       cout << "no object selected" << endl;
//       q = camera->rotationQuat();
//       cout << "quat : " << q.a << " " << q.x << " " << q.y << " " << q.z << endl;
//       double c[4];
//       camera->eyeVec(c);
//       cout << "eye : " << c[0] << " " << c[1] << " " << c[2] << endl;      
//       break;
//     case 't':
//       fps_loop = 100;
//       break;
//     case 'z' :
//       depth_culling = !depth_culling;
//       point_based_render->setDepthTest(depth_culling);
//       cout << "depth test : " << depth_culling << endl;
//       break;
//     case 'e' :
//       elliptical_weight = !elliptical_weight;
//       point_based_render->setEllipticalWeight(elliptical_weight);
//       cout << "elliptical_weight : " << elliptical_weight << endl;
//       break;
//     case 'c' :
//       color_model = !color_model;
//       createPointRender ( PYRAMID_POINTS );
//       cout << "color_model : " << color_model << endl;
//       break;
//     case ']' :
//       ++material_id;
//       if (material_id == NUM_MATERIALS)
// 	material_id = 0;
//       changeMaterial();
//       cout << "material : " << material_id << endl;
//       break;
//     case '[' :
//       --material_id;
//       if (material_id < 0)
// 	material_id = NUM_MATERIALS - 1;
//       changeMaterial();
//       cout << "material : " << material_id << endl;
//       break;
//     case '.' :
//       analysis_filter_size += 1;
//       if (analysis_filter_size > 10)
// 	analysis_filter_size = 0;
//       cout << "analysis_filter_size : " << analysis_filter_size << endl;
//       break;
//     case ',' :
//       analysis_filter_size -= 1;
//       if (analysis_filter_size < 0)
// 	analysis_filter_size = 0;
//       cout << "analysis_filter_size : " << analysis_filter_size << endl;
//       break;
//     case '+' :
//     case '=' :
//       if (reconstruction_filter_size > 0.1)
// 	reconstruction_filter_size += 0.1;
//       else
// 	reconstruction_filter_size += 0.01;
//       point_based_render->setReconstructionFilterSize(reconstruction_filter_size);
//       break;
//     case '-' :
//     case '_' :
//       if (reconstruction_filter_size > 0.0) {
// 	if (reconstruction_filter_size > 0.2)
// 	  reconstruction_filter_size -= 0.1;
// 	else	
// 	  reconstruction_filter_size -= 0.01;
//       }

//       point_based_render->setReconstructionFilterSize(reconstruction_filter_size);
//       break;
//     };
//   }
//   glutPostRedisplay(); 
//}

