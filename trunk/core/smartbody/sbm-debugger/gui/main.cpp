
#include "vhcl.h"

#include "QtCrtDbgOff.h"
#include <QApplication>
#include <QDialog>
#include "QtCrtDbgOn.h"

#include "SbmDebuggerForm.h"

#include "vhmsg-tt.h"


int main(int argc, char *argv[])
{
   vhcl::Crash::EnableExceptionHandling(true);
   //vhcl::Memory::EnableDebugFlags();   // not working due to ttu_open(), VH-345

   QApplication app(argc, argv);
   QMainWindow *widget = new QMainWindow;
   SbmDebuggerForm sbmDebugger(widget, widget);

   return app.exec();
}
