#ifndef _MODELS_TREE_WIDGET_
#define _MODELS_TREE_WIDGET_

#include <QTreeWidget>
#include <QFile>

#include "application.h"

// A wrapper over QGLWidget
class ModelsTreeWidget : public QTreeWidget {

 public:

  // These are all the MyOpenGLWidget constructors
  ModelsTreeWidget(QWidget* parent = 0);
  
  Application *application;
  
  void insert ( QString name, int id );

  public slots:

    virtual void itemChanged( QTreeWidgetItem * item );

};


#endif
