#include "modelsTreeWidget.h"

ModelsTreeWidget::ModelsTreeWidget(QWidget* parent) : QTreeWidget (parent) {
  setColumnCount(2);
  setColumnWidth(0, 50);
}

void ModelsTreeWidget::itemChanged( QTreeWidgetItem * item ) {
  cout << QFile::encodeName( item->text(0) ).data() << endl;
}

void ModelsTreeWidget::insert ( QString name, int id ) {

  QStringList name_list = (QStringList() << QString::number(id) << name );
  QTreeWidgetItem * newItem = new QTreeWidgetItem ( name_list, 0 );
  
  addTopLevelItem ( newItem );    
  newItem->setSelected(1);
  setCurrentItem( newItem );
}
