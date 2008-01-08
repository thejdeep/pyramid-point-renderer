#ifndef _PPRMAINWINDOW_
#define _PPRMAINWINDOW_

#include <QtGui/QtGui>
#include "ui_interface.h"


class pprMainWindow : public QMainWindow, public Ui::MainWindow 
{
  Q_OBJECT
    
 public:
  pprMainWindow (QMainWindow *parent = 0);
  
  Application * application;

 protected slots:
  
  virtual void fileOpen( void );

  void on_modelsTreeWidget_itemClicked ( QTreeWidgetItem * item, int column );

};

#endif
