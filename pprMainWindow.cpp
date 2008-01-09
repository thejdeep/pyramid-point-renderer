#include "pprMainWindow.h"

pprMainWindow::pprMainWindow (QMainWindow *parent): QMainWindow(parent) 
{
  setupUi (this);

  connect( actionOpen, SIGNAL( triggered() ), this, SLOT( fileOpen() ) );

  modelsTreeWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );
}

void pprMainWindow::init( void ) {
 doubleSpinBoxReconstructionFilter->setValue( application->getReconstructionFilter() );
 doubleSpinBoxPrefilter->setValue( application->getPrefilter() );
 doubleSpinBoxReconstructionFilter->setDecimals(3);
 doubleSpinBoxPrefilter->setDecimals(3);

 comboColors->setCurrentIndex( application->getMaterial() );
}

void pprMainWindow::fileOpen( void )
{
  QImage textura,buf;
  QString sfile;
		
  sfile = QFileDialog::getOpenFileName(this,
				       tr("Open Model"), "../plys/", tr("Files (*.ply *.pol)"));

  char *filename = QFile::encodeName( sfile ).data();
  int object_id = 0;

  if ( !sfile.isEmpty() )
    {
      object_id = application->readFile( filename );
    }
  else
    {
      //      statusBar()->message( "Loading cancelled.Could not read image file.", 2000 );
    }

  QStringList name_split = sfile.split("/");
  QStringList name_split2 = name_split.back().split(".");

  modelsTreeWidget->clearSelection();

  modelsTreeWidget->insert( name_split2.front(), object_id );

  application->setSelectedObject( object_id );
  selectCurrObject();

}

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
  application->changeRendererType ( index, (modelsTreeWidget->currentItem()->text(0)).toInt()  );
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
