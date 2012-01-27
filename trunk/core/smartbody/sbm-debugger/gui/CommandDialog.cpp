#include "CommandDialog.h"

CommandDialog::CommandDialog(QWidget *parent) : QDialog(parent)
{
   ui.setupUi(this);

   connect(ui.runButton, SIGNAL(pressed()), this, SLOT(RunCode()));
   connect(ui.clearTopButton, SIGNAL(pressed()), this, SLOT(ClearOutputBox()));
}
   
CommandDialog::~CommandDialog()
{
   
}

void CommandDialog::RunCode()
{
   if (ui.tabWidget->currentIndex() == 0) // sbm code writer
   {
      ui.outputTextEdit->appendPlainText(ui.sbmTextEdit->toPlainText());
   }
   else
   {
      ui.outputTextEdit->appendPlainText(ui.pythonTextEdit->toPlainText());
   }
}

void CommandDialog::ClearOutputBox()
{
   ui.outputTextEdit->clear();
}