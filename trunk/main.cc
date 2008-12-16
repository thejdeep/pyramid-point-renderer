/**
 * Point Based Renderer
 * 
 * Author : Ricardo Marroquim
 *
 * Date created : 12-01-2007
 *
 **/

#include "application.h"
#include "GL/glut.h"

static int g_Width = 816;                          // Initial window width
static int g_Height = 816;                         // Initial window height

bool depth_test;
bool back_face_culling;
int button_pressed;
bool active_shift;
bool active_ctrl;
bool active_alt;
int mask_size;
double reconstruction_filter_size;
double prefilter_size;
int material;
bool auto_rotate;
bool elliptical_weight;

Application *application;

void display( void )
{

//   static int frame = 0, time, timebase = 0, fps;

//   frame++;
//   time = glutGet(GLUT_ELAPSED_TIME);
	
//   if (time - timebase > 1000) {
// 	fps = frame*1000.0/(time-timebase);
// 	cout << fps << endl;
// 	timebase = time;
// 	frame = 0;
//   }
  
  application->draw();

  // Make sure changes appear onscreen
  glutSwapBuffers();
}

void idle( void ) {

  if (auto_rotate)
	display();
}


void reshape(GLint width, GLint height)
{
  g_Width = width;
  g_Height = height;
  
}

/// Keyboard keys function
/// @param key Pressed key
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void keyboard(unsigned char key_pressed, int x, int y) {
  switch (key_pressed) {
  case 27 :
  case 'q' :
  case 'Q' :
	delete application;
    exit(0);
    break;
  case '0' :
    material = 0;
    break;
  case '1' :
    material = 1;
    break;
  case '2' :
    material = 2;
    break;
  case '3' :
    material = 3;
    break;
  case '4' :
    material = 4;
    break;
  case '5' :
    material = 5;
    break;
  case '6' :
    material = 6;
    break;
  case 'r':
	auto_rotate = !auto_rotate;
	application->setAutoRotate( auto_rotate );
	break;
  case 'w':
	elliptical_weight = !elliptical_weight;
	application->setEllipticalWeight( elliptical_weight );
	cout << "Elliptical weight : " << elliptical_weight << endl;
	break;
  case 'd' :
	depth_test = !depth_test;
    application->setDepthTest ( depth_test );
    break;
  case 'b' :
	back_face_culling = !back_face_culling;
    application->setBackFaceCulling ( back_face_culling );
    break;
  case '.':
	application->increaseSelected ( );
	break;
  case ',':
	application->decreaseSelected ( );
	break;
  case '+' :
    mask_size ++;
    application->setGpuMask ( mask_size );
    break;
  case '-' :
    if (mask_size > 0)
      mask_size --;
    application->setGpuMask ( mask_size );
    break;
  case ']' :
    if (reconstruction_filter_size < 0.1)
      reconstruction_filter_size += 0.01;
    else
      reconstruction_filter_size += 0.1;
    application->setReconstructionFilter ( reconstruction_filter_size );
	cout << "filter size : " << reconstruction_filter_size << endl;
    break;
  case '[' :
    if (reconstruction_filter_size > 0.15)
      reconstruction_filter_size -= 0.1;
    else if (reconstruction_filter_size > 0.0)
      reconstruction_filter_size -= 0.01;
    application->setReconstructionFilter ( reconstruction_filter_size );
	cout << "filter size : " << reconstruction_filter_size << endl;
    break;
  }

  // material change
  switch (key_pressed) {
  case '0' :
  case '1' :
  case '2' :
  case '3' :
  case '4' :
  case '5' :
  case '6' :
    application->changeMaterial ( material );
  }

  glutPostRedisplay();
}

/// Keyboard keys function
/// @param key Pressed key
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void keyboardSpecial(int key_pressed, int x, int y) {
  switch (key_pressed) {
  case GLUT_KEY_F1 :
    application->changeRendererType ( 0 );
	cout << "PYRAMID POINTS" << endl;
    break;
  case GLUT_KEY_F2 :
    application->changeRendererType ( 1 );
	cout << "PYRAMID POINTS WITH COLOR" << endl;
    break;
  case GLUT_KEY_F3 :
    application->changeRendererType ( 2 );
	cout << "PYRAMID TEMPLATES WITH COLOR" << endl;
    break;
  }

  // reset values
  switch (key_pressed) {
  case GLUT_KEY_F1 :
  case GLUT_KEY_F2 :
  case GLUT_KEY_F3 :
    application->setGpuMask ( mask_size );
    application->changeMaterial ( material );
    application->setReconstructionFilter ( reconstruction_filter_size );
    application->setPrefilter ( prefilter_size );
	application->setDepthTest( depth_test );
	application->setBackFaceCulling( back_face_culling );
    break;
  }

  glutPostRedisplay();
}


/// Mouse click function
/// @param button Clicked button
/// @param state Button state (clicked or released)
/// @param x X coordinate of mouse click
/// @param y Y coordinate of mouse click
void mouse(int button, int state, int x, int y) {
  
  button_pressed = button;

  if (state == GLUT_DOWN) {
	active_shift = active_ctrl = active_alt = 0;	
	
	switch (glutGetModifiers()) {
	case GLUT_ACTIVE_SHIFT : active_shift = 1; break;
	case GLUT_ACTIVE_CTRL : active_ctrl = 1; break;
	case GLUT_ACTIVE_ALT : active_alt = 1; break;
	case GLUT_ACTIVE_SHIFT | GLUT_ACTIVE_CTRL : active_shift = 1; active_ctrl = 1; break;
	case GLUT_ACTIVE_SHIFT | GLUT_ACTIVE_ALT : active_shift = 1; active_alt = 1; break;
	case GLUT_ACTIVE_CTRL | GLUT_ACTIVE_ALT : active_ctrl = 1; active_alt = 1; break;
	case GLUT_ACTIVE_SHIFT | GLUT_ACTIVE_CTRL | GLUT_ACTIVE_ALT : active_shift = 1; active_ctrl = 1; active_alt = 1; break;
	}

    if (button == GLUT_LEFT_BUTTON) {
      application->mouseLeftButton(x, y, active_shift, active_ctrl, active_alt );
    }
    else if (button == GLUT_MIDDLE_BUTTON) {
	  application->mouseMiddleButton(x, y, active_shift, active_ctrl, active_alt );
    }
    else if (button == GLUT_RIGHT_BUTTON) {
      application->mouseRightButton(x, y, active_shift, active_ctrl, active_alt );
    }
      
  }
  else if (state == GLUT_UP) {
	if (button == GLUT_LEFT_BUTTON) {
      application->mouseReleaseLeftButton(x, y, active_shift, active_ctrl, active_alt );
    }
    else if (button == GLUT_MIDDLE_BUTTON) {
	  application->mouseReleaseMiddleButton(x, y, active_shift, active_ctrl, active_alt );
    }
    else if (button == GLUT_RIGHT_BUTTON) {
      application->mouseReleaseRightButton(x, y, active_shift, active_ctrl, active_alt );
    }
    else if (button == 3) {
      application->mouseWheel(+1, active_shift, active_ctrl, active_alt );
    }
    else if (button == 4) {
      application->mouseWheel(-1, active_shift, active_ctrl, active_alt );
    }


    button_pressed = -1;
	active_shift = 0;
	active_ctrl = 0;
  }

  glutPostRedisplay();
}

// void mouseWheel(int button, int dir, int x, int y) {
//   if (dir > 0)
// 	application->mouseWheel(1);
//   else
// 	application->mouseWheel(-1);
// }


/// Mouse movement func
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void mouseMotion(int x, int y) {

  //  Point click = unproject (Point (x, y, 0.0));

  if (button_pressed == GLUT_LEFT_BUTTON)
    application->mouseLeftMotion(x, y, active_shift, active_ctrl, active_alt );
  else if (button_pressed == GLUT_MIDDLE_BUTTON)
	application->mouseMiddleMotion(x, y, active_shift, active_ctrl, active_alt );
  else if (button_pressed == GLUT_RIGHT_BUTTON)
	application->mouseRightMotion(x, y, active_shift, active_ctrl, active_alt );

  glutPostRedisplay();  
}


/// Main Program
int main(int argc, char * argv []) {

  // GLUT Window Initialization:
  glutInit (&argc, argv);
  glutInitWindowSize (g_Width, g_Height);
  glutInitDisplayMode ( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowPosition(150, 0);
  glutCreateWindow ("Point Based Rendering");

  mask_size = 1;
  reconstruction_filter_size = 1.0;
  prefilter_size = 1.0;
  material = 0;
  auto_rotate = false;
  elliptical_weight = false;
  depth_test = true;
  back_face_culling = true;

  application = new Application(PYRAMID_POINTS_COLOR);  

  if (argc < 2) {
    cerr << "    Usage :" << endl << " pyramid-point-renderer <ply_file>" << endl;
    exit(0);
  }

  if (strcmp (argv[1], "catacomba") == 0) {
	application->startFileReading();
	application->appendFile( "../plys/vcg/catacomba/dom_mal19_w4-O.ply" );
 	application->appendFile( "../plys/vcg/catacomba/dom_mal19_w4-W.ply" );
	application->appendFile( "../plys/vcg/catacomba/dom_mal19_w2_arc-D-1.ply" );
	application->appendFile( "../plys/vcg/catacomba/dom_mal19_w2_arc-D-2.ply" );
	application->appendFile( "../plys/vcg/catacomba/dom_mal19_w2_B-1.ply" );
	application->appendFile( "../plys/vcg/catacomba/dom_mal19_w2_B-2.ply" );
	application->finishFileReading();
	application->changeMaterial(5);
	back_face_culling = false;
	application->setBackFaceCulling ( back_face_culling );
  }
  else if (strcmp (argv[1], "scan") == 0) {
	application->startFileReading();
 	application->appendFile( "../plys/vcg/new/dom_mal19_w2_arc-S.ply" );
 	application->appendFile( "../plys/vcg/new/dom_mal19_w2_arc-N.ply" );
	application->appendFile( "../plys/vcg/new/dom_mal19_w2-N.ply" );
	application->appendFile( "../plys/vcg/new/dom_mal19_w2-M.ply" );
 	application->appendFile( "../plys/vcg/new/dom_mal19_w2-S.ply" );
  	application->appendFile( "../plys/vcg/new/dom_mal19_w4-M.ply" );
	application->finishFileReading();
	application->changeMaterial(5);
	back_face_culling = false;
	application->setBackFaceCulling ( back_face_culling );
  }
  else if (strcmp (argv[1], "piazza") == 0) {
	application->startFileReading();
 	application->appendFile( "../plys/TOF/piazza.ply" );
	application->finishFileReading();
	application->changeMaterial(5);
	back_face_culling = false;
	application->setBackFaceCulling ( back_face_culling );
  }
  else
	application->readFile( argv[1] );

  cout << "Total points : " << application->getNumberPoints() << endl;

  //GLUT callback functions
  glutDisplayFunc(display);
  glutIdleFunc(idle);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  //  glutMouseWheelFunc(mouseWheel);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(keyboardSpecial);

  glutMainLoop();

  return 0;
}
