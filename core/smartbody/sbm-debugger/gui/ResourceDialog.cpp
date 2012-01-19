#include "ResourceDialog.h"

#include "vhmsg-tt.h"

ResourceDialog::ResourceDialog(QWidget *parent) 
: QDialog(parent)
{
   ui.setupUi(this);

   // TODO: Send a vhmsg to populate 
   //vhmsg::ttu_notify1("resource");
   //ui.plainTextEdit->insertPlainText("Add text to the box like this");
}

ResourceDialog::~ResourceDialog()
{
   
}