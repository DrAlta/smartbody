#ifndef COMMAND_DIALOG_H_
#define COMMAND_DIALOG_H_

#include "ui_CommandDialog.h"

class CommandDialog : public QDialog
{
   Q_OBJECT

public slots:
   void RunCode();
   void ClearOutputBox();

public:
   CommandDialog(QWidget *parent = 0);
   ~CommandDialog();

   Ui::CommandDialog ui;
};

#endif