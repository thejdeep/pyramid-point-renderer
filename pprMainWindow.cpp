#include "pprMainWindow.h"

pprMainWindow::pprMainWindow (QMainWindow *parent): QMainWindow(parent) 
{
  setupUi (this);

  connect( actionOpen, SIGNAL( triggered() ), this, SLOT( fileOpen() ) );
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
      //      statusBar()->message( sfile, 0 );
      object_id = application->readFile( filename );
      //      edtOpen->setText(sfile);

    }
  else
    {
//       statusBar()->message( "Loading cancelled.Could not read image file.", 2000 );
//       QImage dummy( 128, 128, 32 );
//       dummy.fill( Qt::green.rgb() );
//       buf = dummy;
    }

  QStringList name_split = sfile.split("/");

  modelsTreeWidget->insert( name_split.back(), object_id );
}


