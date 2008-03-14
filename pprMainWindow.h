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

  void init( void );

 protected slots:
  
  virtual void fileOpen( void );
  virtual void writeLod( void );
  virtual void writeScene( void );

  void on_modelsTreeWidget_itemClicked ( QTreeWidgetItem * item, int column );
  void on_comboRendererType_currentIndexChanged( int index );
  void on_comboColors_currentIndexChanged( int index );

  void on_doubleSpinBoxReconstructionFilter_valueChanged( double d );
  void on_doubleSpinBoxPrefilter_valueChanged( double d );

  void on_checkBoxPerVertexColor_stateChanged( int state );
  void on_checkBoxAutoRotate_stateChanged( int state );
  void on_checkBoxLOD_stateChanged( int state );
  void on_checkBoxLODColors_stateChanged( int state );
  void on_checkBoxDepthTest_stateChanged( int state );
  void on_checkBoxColorBars_stateChanged( int state );

  void on_actionCreateKeyFrame_triggered( bool ckecked = false );
  void on_actionRunKeyFrames_triggered( bool ckecked = false );
  void on_actionLoadKeyFrames_triggered( bool ckecked = false );
  void on_actionWriteKeyFrames_triggered( bool ckecked = false );
  void on_actionClearKeyFrames_triggered( bool ckecked = false );

 private:

  void selectCurrObject ( void );

  void keyPressEvent( QKeyEvent* event);

};

#endif
