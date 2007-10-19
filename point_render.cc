/**
 * Point Based Renderer
 * 
 * Author : Ricardo Marroquim
 *
 * Date created : 12-01-2007
 *
 **/

#include "file_io.h"
#include "camera.h"

#include "materials.h"

#define CANVAS_WIDTH  1024
#define CANVAS_HEIGHT 1024
 
// #define CANVAS_WIDTH  768
// #define CANVAS_HEIGHT 768

// #define CANVAS_WIDTH  512
// #define CANVAS_HEIGHT 512


Camera * camera;
TriangleRenderer * triangle_renderer;

int material_id;
int selected_obj;
vector<Object> objects;
int num_objects;

// Vector of point samples
std::vector<Surfel> surfels;
// Vector of triangles with indices to surfels
std::vector<Triangle> triangles;

//****** Globals for faster matrix operations *******/
double inv3 = 1.0/3.0;
double root3 = sqrt(3.0);

/***************************************************/

// Function declarations
void screenButtons (int, int );
Point unproject (const Point&);
void keyboard(unsigned char key, int x, int y);
void glVertex(surfelVectorIter it);
void createPointRender(int type);
void changeMaterial(void);
void changeRendererType( int type );

/// Interface function and variables
#include "interface.cc"

/// Renders a surfel as a opengl point primitive
/// @param s Pointer to surfel
void glVertex(const Surfel * s) {
  Point p = s->position();
  glVertex3f(p.x(), p.y(), p.z());
}

/// Renders a surfel as a opengl point primitive
/// @param s Pointer to surfel
void glVertex(surfelVectorIter it) {
  Point p = it->position();
  glVertex3f(p.x(), p.y(), p.z());
}

/// Renders a vertex
/// @param p Given vertex position
void glVertex(Point p) {
  glVertex3f(p.x(), p.y(), p.z());
}

/// Render all points 
void drawPoints(void)
{
  glPointSize(5.0);
  glColor4f(1.0f, 0.0f, 0.0f, 1.0f);

  glBegin(GL_POINTS);
  
  for (int i = 0; i < num_objects; ++i) {
    for (surfelVectorIter it = objects[i].getSurfels()->begin(); 
	 it != objects[i].getSurfels()->end(); ++it)  
      glVertex(it);
  }

  glEnd();
}

/// Display func
void draw(void) {

#ifdef TIMING
  static double timings[4] = {0, 0, 0, 0};
#endif

  if (fps_loop == 0) {
    start_time = timer();
  }

  // Set camera position and direction
  camera->setView();

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Compute eye coordinate
  // Rotate eye on opposite direction
  double c[4];
  camera->eyeVec(c);
  c[3] = 1.0;
  Quat q = camera->rotationQuat();
  // Invert quaternion -- simulates inverse of modelview
  q.x *= -1; q.y *= -1; q.z *= -1;
  q.rotate(c);
  double eye[3] = {c[0], c[1], c[2]};
  point_based_render->setEye(eye);
    
    // set light direction and zoom factor
//     double light_dir[3];
//     camera->lightVec(light_dir);
//     point_based_render->setLight(light_dir);
//     point_based_render->setZoomFactor(camera->zoom());

  // pyramid interpolation algorithm
#ifdef TIMING

  point_based_render->draw(timing_profile);

#else

  point_based_render->clearBuffers();

  for (int i = 0; i < num_objects; ++i) {
    if (objects[i].getRendererType() != TRIANGLES) {
      point_based_render->projectSamples( &objects[i] );
      camera->setView();

    }
  }    

  point_based_render->interpolate();

  camera->setView();

  for (int i = 0; i < num_objects; ++i)
    if (objects[i].getRendererType() == TRIANGLES)
      objects[i].render();

  glDisable (GL_LIGHTING);
  glDisable (GL_LIGHT0);
  glDisable (GL_COLOR_MATERIAL);


  if (show_points)
    drawPoints();

#endif

  // compute frames per second
  // fps variable is rendered on screen text
  ++fps_loop;
  if (fps_loop == 30) {
    double end_time = timer();
    fps = (end_time - start_time) / (double)fps_loop;
    #ifndef TIMING
    fps = 1000.0 / fps;
    #endif
    fps_loop = 0;
  }

  if (rotating)
    camera->rotate();

  glFinish();

#ifdef TIMING

  if (fps_loop == 0) {
      timings[timing_profile] = fps;
      timing_profile ++;
    }

  if (timing_profile == 4) {
    cout << "PREPARE    : " << timings[0] << endl;
    cout << "PROJECT    : " << timings[1] - timings[0] << endl;
    cout << "PYRAMID_POINTS    : " << timings[2] - timings[1] << endl;
    cout << "SHADE      : " << timings[3] - timings[2]<< endl;
    cout << "TOTAL      : " << timings[3] << endl;
    cout << "FPS        : " << 1000.0 / timings[3] << endl;
    exit(0);
  }

#endif

#ifndef TIMING

  // Render the screen text
  if (show_screen_info) {
    glPushMatrix();
    glLoadIdentity();
    if (render_mode == GL_RENDER)
      screenText(CANVAS_WIDTH, CANVAS_HEIGHT);
    glPopMatrix();
  }

#endif

  glutSwapBuffers();
}

/// Reshape func
/// @param w New window width
/// @param h New window height
void reshape(int w, int h) {

  glutReshapeWindow (w, h);

  camera->reshape(w, h);

  glutPostRedisplay();
}

/// Unproject a point from screen coordinates to world coordinates
/// @param p Point to be unprojected
/// @return Point in world coordinates
Point unproject (const Point& p) {

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

/// Mouse click function
/// @param button Clicked button
/// @param state Button state (clicked or released)
/// @param x X coordinate of mouse click
/// @param y Y coordinate of mouse click
void mouse(int button, int state, int x, int y) {
  
  if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
    active_shift = 1;
  else
    active_shift = 0;

  button_pressed = button;
  Point click = unproject(Point (x, y, 0.0));

  if (state == GLUT_DOWN) {
    if (button == GLUT_LEFT_BUTTON) {

      camera->startRotation(x, y);

      // Check if clicked in one of interface buttons ( flags )
      screenButtons (x, y);
      
    }
    else if (button == GLUT_MIDDLE_BUTTON) {
      camera->mouseSetClick(x, y);
    }
    else if (button == GLUT_RIGHT_BUTTON) {
      camera->mouseSetClick(x, y);
    }
      
  }
  else if (state == GLUT_UP) {
    button_pressed = -1;
    camera->endRotation();
  }

  glutPostRedisplay();
}

/// Mouse movement func
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void mouseMotion(int x, int y) {

  Point click = unproject (Point (x, y, 0.0));

  if (button_pressed == GLUT_LEFT_BUTTON) {
    camera->rotate(x, y);
  }
  else if (button_pressed == GLUT_MIDDLE_BUTTON) {
    if (active_shift)
      camera->translate(x, y);
    else
      camera->zooming (x, y);
  }
  else if (button_pressed == GLUT_RIGHT_BUTTON) {
    camera->lightTranslate(x, y);
  }

  glutPostRedisplay();
}

/**
 * Change material properties.
 **/
void changeMaterial(void) {

  GLfloat material_ambient[4] = {Mats[material_id][0], Mats[material_id][1], Mats[material_id][2], Mats[material_id][3]};  
  GLfloat material_diffuse[4] = {Mats[material_id][4], Mats[material_id][5], Mats[material_id][6], Mats[material_id][7]};  
  GLfloat material_specular[4] = {Mats[material_id][8], Mats[material_id][9], Mats[material_id][10], Mats[material_id][11]};
  GLfloat material_shininess = Mats[material_id][12];

  glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
  glMaterialf (GL_FRONT, GL_SHININESS, material_shininess);
}

/// Keyboard special keys function
/// @param key Pressed key
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void specialKey(int key_pressed, int x, int y) {
  switch (key_pressed) {
  case GLUT_KEY_F1:
    changeRendererType(PYRAMID_POINTS);
    show_splats = 1;
    break;
  case GLUT_KEY_F2:
    changeRendererType(PYRAMID_TRIANGLES);
    show_splats = 2;
    break;
  case GLUT_KEY_F3:
    changeRendererType(PYRAMID_HYBRID);
    show_splats = 3;
    break;
  case GLUT_KEY_F4:
    changeRendererType(EWA_SPLATTING_INTERPOLATE_NORMALS);
    show_splats = 4;
    break;
  case GLUT_KEY_F5:
    changeRendererType(TRIANGLES);
    show_splats = 5;
    break;
  case GLUT_KEY_F6:
    show_splats = 6;
    break;
  case GLUT_KEY_F12:
    show_splats = 0;
    show_points = true;
    break;
  };
  glutPostRedisplay();
}


/// Keyboard keys function
/// @param key Pressed key
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void keyboard(unsigned char key_pressed, int x, int y) {

  if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
    switch (key_pressed) {
    case '+' :
      prefilter_size += 0.25;
      point_based_render->setPrefilterSize(prefilter_size);
      break;
    case '-' :
      if (prefilter_size > 0.0)
	prefilter_size -= 0.25;
      point_based_render->setPrefilterSize(prefilter_size);
      break;
    };
  }
  else {
    switch (key_pressed) {
    case 27 :
    case 'q' :
    case 'Q' :
      exit(0);
      break;
    case 'h' : 
    case 'H' :
      show_screen_info = !show_screen_info;
      break;
    case 'v' :
    case 'V' :
      camera->switchViewMode();  
      break;
    case 'p' :
    case 'P' :
      show_points = !show_points;
      break;
    case 'r' :
    case 'R' :
      rotating = !rotating;
      break;
    case '1' :
      selected_obj = 0;
      cout << "selected : 1" << endl;
      break;
    case '2' :
      if (num_objects > 1) {
	selected_obj = 1;
	cout << "selected : 2" << endl;
      }
      break;
    case '3' :
      if (num_objects > 2) {
	selected_obj = 2;
	cout << "selected : 3" << endl;
      }
      break;
    case '4' :
      if (num_objects > 3) {
	selected_obj = 3;
	cout << "selected : 4" << endl;
      }
      break;
    case 't':
      fps_loop = 100;
      break;
    case 'z' :
      depth_culling = !depth_culling;
      point_based_render->setDepthTest(depth_culling);
      break;
    case ']' :
      ++material_id;
      if (material_id == NUM_MATERIALS)
	material_id = 0;
      changeMaterial();
      break;
    case '[' :
      --material_id;
      if (material_id < 0)
	material_id = NUM_MATERIALS - 1;
      changeMaterial();
      break;
    case '.' :
      analysis_filter_size += 1;
      if (analysis_filter_size > 10)
	analysis_filter_size = 0;
      cout << "analysis_filter_size : " << analysis_filter_size << endl;
      break;
    case ',' :
      analysis_filter_size -= 1;
      if (analysis_filter_size < 0)
	analysis_filter_size = 0;
      cout << "analysis_filter_size : " << analysis_filter_size << endl;
      break;
    case '+' :
      if (reconstruction_filter_size > 0.2)
	reconstruction_filter_size += 0.1;
      else
	reconstruction_filter_size += 0.01;
      point_based_render->setReconstructionFilterSize(reconstruction_filter_size);
      break;
    case '-' :
      if (reconstruction_filter_size > 0.0) {
	if (reconstruction_filter_size > 0.2)
	  reconstruction_filter_size -= 0.1;
	else	
	  reconstruction_filter_size -= 0.01;
      }

      point_based_render->setReconstructionFilterSize(reconstruction_filter_size);
      break;
    };
  }
  glutPostRedisplay(); 
}

void changeRendererType( int type ) {
  objects[selected_obj].setRendererType(type);
}

void createPointRender( int type ) {

  delete point_based_render;

  point_based_render = new PyramidPointRender(CANVAS_WIDTH, CANVAS_HEIGHT);

  assert (point_based_render);

  point_based_render->setReconstructionFilterSize(reconstruction_filter_size);
  point_based_render->setPrefilterSize(prefilter_size);

  glutPostRedisplay(); 
}

/// Initialize global variables and opengl states
void init(void) {
  render_mode = GL_RENDER;

  fps_loop = 0;

  button_pressed = -1;
  active_shift = 0;

  analysis_filter_size = 0;

  depth_culling = 1;
  output_type = 0;
  rotating = 0;

  show_screen_info = true;
  show_points = false;
  show_splats = 1;

  timing_profile = 0;

  material_id = 0;
  selected_obj = 0;

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

/// Main Program
int main(int argc, char * argv []) {

  // Initialize camera with window canvas size
  camera = new Camera(CANVAS_WIDTH, CANVAS_HEIGHT);

  //GLUT initialization
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(150, 0);
  glutInitWindowSize(camera->screenWidth(), camera->screenHeight());
  glutCreateWindow("Point Based Rendering");

  //GLUT callback functions
  glutDisplayFunc(draw);
  glutIdleFunc(draw);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(specialKey);

  // Initialize global variables
  init();

  // Read model file (.normals)
  //int read = readPoints(argc, argv, &surfels);
  //int read = readPointsAndTriangles(argc, argv, &surfels, &triangles);

  int read = readModels(argc, argv, &objects);
  num_objects = objects.size();

  cout << "objects : " << num_objects << endl;
  for (int i = 0; i < num_objects; ++i) {
    objects[i].setRendererType( PYRAMID_POINTS );
    //objects[i].setArrays();
    //    objects[i].setDisplayList();
    //objects[i].setHybridDisplayList();
    cout << "object " << i << endl <<
      "  num points    : " << objects[i].getSurfels()->size() <<
      "  num triangles : " << objects[i].getTriangles()->size() <<
      "  render type   : " << objects[i].getRendererType() << endl;
  }

  createPointRender( PYRAMID_POINTS );

  if (read == 0)
    exit(0);


#ifdef TIMING
  if (argc < 2)
    cout << "MODEL      : ../models/mannequin.sls.normals" << endl;
  else
    cout << "MODEL      : " << (char*)argv[1] << endl;
  cout << "NUM POINTS : " << number_surfels << endl;
  if (FBO_FORMAT == 34842)
    cout << "TEX TYPE   : 16F" << endl;
  else
    cout << "TEX TYPE   : 32F" << endl;
#endif

  glutMainLoop();

  return 0;
}