
#include "vhcl.h"

#include "ConnectDialog.h"

#include "vhmsg-tt.h"
#include "SBMDebuggerClient.h"


using std::vector;
using std::string;


extern SbmDebuggerClient c;


ConnectDialog::ConnectDialog(QWidget *parent)
  : QDialog(parent)
{
   ui.setupUi(this);

   //Query processes that are using sbm here
   c.QuerySbmProcessIds();
   vhmsg::ttu_wait(2);
   vhmsg::ttu_wait(2);
   vector<string> ids = c.GetSbmProcessIds();

   for (size_t i = 0; i < ids.size(); i++)
   {
      ui.listWidget->addItem(ids[i].c_str());
   }

   //connect(this, SIGNAL(mouseDoubleClickEvent(QMouseEvent*)), ui.listWidget, SLOT(mouseDoubleClickEvent(QMouseEvent*)));
}

void ConnectDialog::accept()
{
   done(Accepted);
}

void ConnectDialog::reject()
{
   done(Rejected);
}

void ConnectDialog::keyPressEvent(QKeyEvent *key)
{
   QDialog::keyPressEvent(key);
   if ((key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter)
      && ui.listWidget->currentIndex().row() > -1) 
   {
      done(Accepted);
   }
}

void ConnectDialog::mouseDoubleClickEvent(QMouseEvent *event)
{
   
}