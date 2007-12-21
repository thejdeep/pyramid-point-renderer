#ifndef _OPENGLWIDGET_
#define _OPENGLWIDGET_

#include <QtOpenGL/QGLWidget>
#include <QtGui/QFrame>

#include "application.h"

// A wrapper over QGLWidget
class openGLWidget: public QGLWidget {
    
 public:

  // These are all the MyOpenGLWidget constructors
  openGLWidget(QWidget* parent=0,
	       const QGLWidget* shareWidget = 0, Qt::WFlags f=0)
    : QGLWidget (parent, shareWidget, f) {};

    // A simple opengl drawing callback
    void paintGL() {
      application->draw();
    }
    
    // A simple resize callback
    void resizeGL (int width, int height) {
      int side = width < height ? width : height;
      glViewport((width - side) / 2, (height - side) / 2, side, side);
    }

    void mouseReleaseEvent ( QMouseEvent * event ) {    
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
