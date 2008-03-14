#ifndef _OPENGLWIDGET_
#define _OPENGLWIDGET_

#include <QtOpenGL/QGLWidget>
#include <QtGui/QFrame>
#include <QTime>
#include <QtGui/QLCDNumber>

#include "application.h"

#include <GL/glut.h>

// A wrapper over QGLWidget
class openGLWidget: public QGLWidget {
    
 private:

  QTime timer;
  double fps;
  QLCDNumber *fpsDisplay;

 protected:
  // idle function called every 1 millisecond
  void timerEvent(QTimerEvent *) {
    updateGL();
  }

 public:

  // These are all the MyOpenGLWidget constructors
  openGLWidget(QWidget* parent=0,
	       const QGLWidget* shareWidget = 0, Qt::WFlags f=0)
    : QGLWidget (parent, shareWidget, f) {
    startTimer(1);

    int argc = 0;
    char * argv[1];
    argv[0] = "";

    // GLUT initialization, just for writing text to screen
    glutInit(&argc, argv);

  }

    // A simple opengl drawing callback
    void paintGL() {
      static int elapsed_millisecs = 0;
      static int fps_loop = 0;      

      timer.restart(); 
      application->draw();
      elapsed_millisecs += timer.elapsed();

      fps_loop ++;
      if (fps_loop == 10) {
	fps = (elapsed_millisecs) / (double)fps_loop;
	fps =  1000.0 / fps;
	fpsDisplay->display(fps);

	elapsed_millisecs = 0;
	fps_loop = 0;	
      }



    }
    
    void setFpsDisplay ( QLCDNumber * d ) { fpsDisplay = d; }

    // A simple resize callback
    void resizeGL (int width, int height) {
      int side = width < height ? width : height;
      glViewport((width - side) / 2, (height - side) / 2, side, side);
    }

    void mouseReleaseEvent ( QMouseEvent * ) {

      application->mouseReleaseButton();
    }

    void mouseMoveEvent ( QMouseEvent * event ) {

      if (event->buttons() == Qt::LeftButton)
	application->mouseLeftMotion(event->x(), event->y());
      else if (event->buttons() == Qt::MidButton) {
	if (event->modifiers () == Qt::ShiftModifier)
	  application->mouseMiddleMotionShift(event->x(), event->y());
	else
	  application->mouseMiddleMotion(event->x(), event->y());
      }
      else if (event->buttons() == Qt::RightButton)
	application->mouseRightMotion(event->x(), event->y());    

      updateGL();
    }

    void mousePressEvent ( QMouseEvent * event ) {
    
      if (event->buttons() == Qt::LeftButton)     
	application->mouseLeftButton(event->x(), event->y());
      else if (event->buttons() == Qt::MidButton)
	application->mouseMiddleButton(event->x(), event->y());
      else if (event->buttons() == Qt::RightButton)
	application->mouseRightButton(event->x(), event->y());

    }

    Application *application;
};

#endif
