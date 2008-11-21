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

int button_pressed;
bool active_shift;
int mask_size;
double reconstruction_filter_size;

Application *application;

void display(void)
{
   application->draw();

   // Make sure changes appear onscreen
   glutSwapBuffers();
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
    exit(0);
    break;
  case '0' :
    application->changeMaterial ( 0 );
    break;
  case '1' :
    application->changeMaterial ( 1 );
    break;
  case '2' :
    application->changeMaterial ( 2 );
    break;
  case '3' :
    application->changeMaterial ( 3 );
    break;
  case '4' :
    application->changeMaterial ( 4 );
    break;
  case '5' :
    application->changeMaterial ( 5 );
    break;
  case 'd' :
    application->toggleDepthTest ( );
    break;
  case 'b' :
    application->toggleBackFaceCulling ( );
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
    break;
  case '[' :
    if (reconstruction_filter_size > 0.1)
      reconstruction_filter_size -= 0.1;
    else if (reconstruction_filter_size > 0.0)
      reconstruction_filter_size -= 0.01;
    application->setReconstructionFilter ( reconstruction_filter_size );
    break;
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
    break;
  case GLUT_KEY_F2 :
    application->changeRendererType ( 1 );
    break;
  case GLUT_KEY_F3 :
    application->changeRendererType ( 2 );
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
  
  if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
    active_shift = 1;
  else
    active_shift = 0;

  button_pressed = button;
  //Point click = unproject(Point (x, y, 0.0));

  if (state == GLUT_DOWN) {
    if (button == GLUT_LEFT_BUTTON) {
      application->mouseLeftButton(x, y);
    }
    else if (button == GLUT_MIDDLE_BUTTON) {
      application->mouseMiddleButton(x, y);
    }
    else if (button == GLUT_RIGHT_BUTTON) {
      application->mouseRightButton(x, y);
    }
      
  }
  else if (state == GLUT_UP) {
    button_pressed = -1;
    //    camera->endRotation();
  }

  glutPostRedisplay();
}

/// Mouse movement func
/// @param x X coordinate of mouse pointer
/// @param y Y coordinate of mouse pointer
void mouseMotion(int x, int y) {

  //  Point click = unproject (Point (x, y, 0.0));

  if (button_pressed == GLUT_LEFT_BUTTON) {
    application->mouseLeftMotion(x, y);
  }
  else if (button_pressed == GLUT_MIDDLE_BUTTON) {
    if (active_shift)
      application->mouseMiddleMotionShift(x, y);
    else
      application->mouseMiddleMotion(x, y);
  }
   else if (button_pressed == GLUT_RIGHT_BUTTON) {
     application->mouseRightMotion(x, y);

   }

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

  application = new Application(PYRAMID_POINTS);
  //  application->readFile( "../plys/dragon.ply" );

  if (argc < 2) {
    cerr << "    Usage :" << endl << " pyramid-point-renderer <ply_file>" << endl;
    exit(0);
  }
 application->readFile( argv[1] );

  //GLUT callback functions
  glutDisplayFunc(display);
  //  glutIdleFunc(draw);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(keyboardSpecial);

  glutMainLoop();

  return 0;
}
