#include "pprMainWindow.h"

/**
 * Main Window Constructor
 **/
pprMainWindow::pprMainWindow (QMainWindow *parent): QMainWindow(parent) 
{
  setupUi (this);

  connect( actionOpen, SIGNAL( triggered() ), this, SLOT( fileOpen() ) );
  connect( actionWrite_LOD, SIGNAL( triggered() ), this, SLOT( writeLod() ) );
  connect( actionWrite_Scene, SIGNAL( triggered() ), this, SLOT( writeScene() ) );

  modelsTreeWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );
  widget->setFpsDisplay ( lcdNumberFps ); 
}

/**
 * Initialize boxes values.
 **/
void pprMainWindow::init( void ) {
 doubleSpinBoxReconstructionFilter->setValue( application->getReconstructionFilter() );
 doubleSpinBoxPrefilter->setValue( application->getPrefilter() );
 doubleSpinBoxReconstructionFilter->setDecimals(5);
 doubleSpinBoxPrefilter->setDecimals(5);
 doubleSpinBoxReconstructionFilter->setSingleStep(0.1);
 doubleSpinBoxPrefilter->setSingleStep(0.1);

 comboColors->setCurrentIndex( application->getMaterial() );
 checkBoxDepthTest->setCheckState(Qt::Checked);

}

void pprMainWindow::writeLod( void ) {
  application->writeLodFile();
}

void pprMainWindow::writeScene( void ) {
  application->writeSceneFile();
}

/**
 * Opens a file with the dialog box.
 **/
void pprMainWindow::fileOpen( void )
{
  QString sfile;
		
  sfile = QFileDialog::getOpenFileName(this, tr("Open Model"), "../plys/", tr("Files (*.ply *.pol *.lod *.scn)"));
  
  vector<int> objs_ids;

  QStringList name_split = sfile.split("/");
  QStringList name_split2 = name_split.back().split("."); 
  QString filetype = name_split2.back();
  QString objectName = name_split2.front();

  const char* filename = sfile.toLatin1().data();

  if ( !sfile.isEmpty() ) {  
    if (filetype.compare("ply") == 0)
      objs_ids.push_back( application->readFile( filename ) );
    else if (filetype.compare("pol") == 0)
      application->readPolFile( filename, &objs_ids );
    else if (filetype.compare("lod") == 0)
      objs_ids.push_back( application->readLodFile( filename ) );
    else if (filetype.compare("scn") == 0)
      application->readSceneFile( filename, &objs_ids );
    else
      cout << "File extension not supported" << endl;
  }
  else {
    // statusBar()->message( "Loading cancelled. Could not read image file.", 2000 );
  }

  modelsTreeWidget->clearSelection();

  // Inlcude -LOD at the end of the name on the list for better identification
  if (filetype.compare("lod") == 0)
    objectName.append ( "-LOD" );

  for (unsigned int i = 0; i < objs_ids.size(); ++i) {
    modelsTreeWidget->insert( objectName, objs_ids[i] );
    application->setSelectedObject( objs_ids[i] );
  }

  selectCurrObject();

  if (filetype.compare("lod") == 0)
    checkBoxLOD->setCheckState( (Qt::CheckState) 2 );
}

/**
 * Displays the info and sets the boxes according to the
 * current selected item from list.
 **/
void pprMainWindow::selectCurrObject ( void ) {

  QTreeWidgetItem *curr = modelsTreeWidget->currentItem();

  if (!curr)
    return;

  int id = (curr->text(0)).toInt();

  application->clearSelectedObjects ( );

  if (id == -1)
    return;

  application->setSelectedObject( id );

  int rtype = application->getRendererType( id );

  comboRendererType->setCurrentIndex( rtype );

  QString info = modelsTreeWidget->currentItem()->text(1).toUpper();
  info.append("<br>");
  info.append("Points : ");
  info.append( QString::number(application->getNumberPoints( id )));
  info.append("<br>");
  info.append("Triangles : ");
  info.append( QString::number(application->getNumberTriangles( id )));

  textObjectInfo->setText ( info );
}

/**
 * Changes item selection.
 **/
void pprMainWindow::on_modelsTreeWidget_itemClicked ( QTreeWidgetItem * item, int column )  {
  application->clearSelectedObjects ();

  if (item == 0)
    application->setSelectedObject( -1 );
  else {
    QList<QTreeWidgetItem*> selected = modelsTreeWidget->selectedItems();
    for (QList<QTreeWidgetItem*>::iterator it = selected.begin(); it != selected.end(); ++it) {
      application->setSelectedObject( ((*it)->text(0)).toInt() );
    }
  }

  selectCurrObject ();
  widget->updateGL();
}

void pprMainWindow::on_checkBoxDepthTest_stateChanged( int state ) {
  if (state == Qt::Checked)
    application->setDepthTest(true);
  else
    application->setDepthTest(false);
  widget->updateGL();
}

void pprMainWindow::on_checkBoxColorBars_stateChanged( int state ) {
  if (state == Qt::Checked)
    application->setColorBars(true);
  else
    application->setColorBars(false);
  widget->updateGL();
}

void pprMainWindow::on_checkBoxPerVertexColor_stateChanged( int state ) {
  if (state == Qt::Checked)
    application->setPerVertexColor(true, (modelsTreeWidget->currentItem()->text(0)).toInt());
  else
    application->setPerVertexColor(false, (modelsTreeWidget->currentItem()->text(0)).toInt());
  widget->updateGL();
}

void pprMainWindow::on_checkBoxAutoRotate_stateChanged( int state ) {
  if (state == Qt::Checked)
    application->setAutoRotate(true);
  else
    application->setAutoRotate(false);
  widget->updateGL();
}

void pprMainWindow::on_checkBoxLOD_stateChanged( int state ) {
  selectCurrObject();

  if (state == Qt::Checked)
    application->useLOD(true, (modelsTreeWidget->currentItem()->text(0)).toInt());
  else
    application->useLOD(false, (modelsTreeWidget->currentItem()->text(0)).toInt());

  widget->updateGL();
}

void pprMainWindow::on_checkBoxLODColors_stateChanged( int state ) {
  if (state == Qt::Checked)
    application->setLodColors(true);
  else
    application->setLodColors(false);
  widget->updateGL();
}

void pprMainWindow::on_comboRendererType_currentIndexChanged( int index ) {
  application->changeRendererType ( index, (modelsTreeWidget->currentItem()->text(0)).toInt()  );
  widget->updateGL();
}

void pprMainWindow::on_comboColors_currentIndexChanged( int index ) {
  //  application->changeMaterial ( index  );
  application->changeMaterial ( index, (modelsTreeWidget->currentItem()->text(0)).toInt() );
  widget->updateGL();
}

void pprMainWindow::on_doubleSpinBoxReconstructionFilter_valueChanged( double d ) {
  application->setReconstructionFilter ( d );
  widget->updateGL();
}

void pprMainWindow::on_doubleSpinBoxPrefilter_valueChanged( double d ) {
  application->setPrefilter ( d );
  widget->updateGL();
}

void pprMainWindow::on_actionCreateKeyFrame_triggered( bool ckecked ) {
  application->createKeyFrame();
}

void pprMainWindow::on_actionRunKeyFrames_triggered( bool ckecked ) {
  application->runFrames();
}

void pprMainWindow::on_actionLoadKeyFrames_triggered( bool ckecked ) {
  QString sfile;
		
  sfile = QFileDialog::getOpenFileName(this, tr("Open Key Frames"), "./", tr("Files (*.frames)"));

  const char* filename = sfile.toLatin1();

  vector<int> objs_ids;

  QStringList name_split = sfile.split("/");
  QStringList name_split2 = name_split.back().split("."); 

  QString filetype = name_split2.back();
  if ( !sfile.isEmpty() ) {  
    if (filetype.compare("frames") == 0)
      application->loadKeyFrames( filename );
    else
      cout << "File extension not supported" << endl;
  }
  else {
    // statusBar()->message( "Loading cancelled.Could not read image file.", 2000 );
  }
}

void pprMainWindow::on_actionWriteKeyFrames_triggered( bool ckecked ) {
  application->writeKeyFrames();
}

void pprMainWindow::on_actionClearKeyFrames_triggered( bool ckecked ) {
  application->clearFrames();
}

void pprMainWindow::keyPressEvent( QKeyEvent* event) {
  if (event->key() == Qt::Key_Q)
    close();
  else if (event->key() == Qt::Key_K)
    application->createKeyFrame();
  else if (event->key() == Qt::Key_R)
    application->runFrames();
  else if (event->key() == Qt::Key_P)
    application->switchLodsPerc();
}
