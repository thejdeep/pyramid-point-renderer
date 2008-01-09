/**
 * Point Based Renderer
 * 
 * Author : Ricardo Marroquim
 *
 * Date created : 12-01-2007
 *
 **/

#include <QtGui/QtGui>
#include "ui_interface.h"
#include "pprMainWindow.h"

Application *application;

/// Main Program
int main(int argc, char * argv []) {
 
  QApplication app(argc, argv);
  app.setQuitOnLastWindowClosed(true);
  
  pprMainWindow * form = new pprMainWindow; 

  form->widget->makeCurrent();

  application = new Application();

  form->application = application;
  form->widget->application = application;
  form->init();

  form->show();
  return app.exec();
}
