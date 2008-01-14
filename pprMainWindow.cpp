#include "pprMainWindow.h"

/**
 * Main Window Constructor
 **/
pprMainWindow::pprMainWindow (QMainWindow *parent): QMainWindow(parent) 
{
  setupUi (this);

  connect( actionOpen, SIGNAL( triggered() ), this, SLOT( fileOpen() ) );

  modelsTreeWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );
}

/**
 * Initialize boxes values.
 **/
void pprMainWindow::init( void ) {
 doubleSpinBoxReconstructionFilter->setValue( application->getReconstructionFilter() );
 doubleSpinBoxPrefilter->setValue( application->getPrefilter() );
 doubleSpinBoxReconstructionFilter->setDecimals(3);
 doubleSpinBoxPrefilter->setDecimals(3);
 doubleSpinBoxReconstructionFilter->setSingleStep(0.01);
 doubleSpinBoxPrefilter->setSingleStep(0.01);

 comboColors->setCurrentIndex( application->getMaterial() );
}

/**
 * Opens a file with the dialog box.
 **/
void pprMainWindow::fileOpen( void )
{
  QImage textura,buf;
  QString sfile;
		
  sfile = QFileDialog::getOpenFileName(this, tr("Open Model"), "../plys/", tr("Files (*.ply *.pol)"));

  char *filename = QFile::encodeName( sfile ).data();
  vector<int> objs_ids;

  QStringList name_split = sfile.split("/");
  QStringList name_split2 = name_split.back().split(".");

  if ( !sfile.isEmpty() ) {
    QString filetype = name_split2.back();
    if (filetype.compare("ply") == 0)
      objs_ids.push_back( application->readFile( filename ) );
    else if (filetype.compare("pol") == 0)
      application->readPolFile( filename, &objs_ids );
    else
      cout << "File extension not supported" << endl;
  }
  else {
    // statusBar()->message( "Loading cancelled.Could not read image file.", 2000 );
  }

  modelsTreeWidget->clearSelection();

  for (unsigned int i = 0; i < objs_ids.size(); ++i) {
    modelsTreeWidget->insert( name_split2.front(), objs_ids[i] );
    application->setSelectedObject( objs_ids[i] );
  }

  selectCurrObject();
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

void pprMainWindow::on_comboRendererType_currentIndexChanged( int index ) {
  //application->changeRendererType ( index, (modelsTreeWidget->currentItem()->text(0)).toInt()  );
  widget->updateGL();
}

void pprMainWindow::on_comboColors_currentIndexChanged( int index ) {
  application->changeMaterial ( index  );
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

void pprMainWindow::keyPressEvent( QKeyEvent* event) {
  if (event->key() == Qt::Key_Q)
    close();
}
