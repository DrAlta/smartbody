#include <QApplication>
#include <QDialog>
#include "SbmDebuggerForm.h"
//#include "ui_SbmDebuggerForm.h"
//#include "ui_ConnectDialog.h"

int main(int argc, char *argv[])
{
   QApplication app(argc, argv);
   QMainWindow *widget = new QMainWindow;
   SbmDebuggerForm sbmDebugger(widget, widget);

   //SbmDebuggerForm sbmDebugger;
   return app.exec();
}