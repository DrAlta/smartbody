#ifndef COMMAND_DIALOG_H_
#define COMMAND_DIALOG_H_

#include "ui_CommandDialog.h"
#include "SbmDebuggerClient.h"

using std::string;
using std::vector;

// callbacks
bool SbmCommandReturned(void* caller, NetRequest* req);

class CommandDialog : public QDialog
{
   Q_OBJECT

public slots:
   void RunCode();
   void ClearOutputBox();

public:
   CommandDialog(SbmDebuggerClient* client, QWidget *parent = 0);
   ~CommandDialog();

   QPlainTextEdit* CurrentTextEditor() { return ui.tabWidget->currentIndex() == 0 ? ui.sbmTextEdit : ui.pythonTextEdit; }

   Ui::CommandDialog ui;

private:
   SbmDebuggerClient* m_client;
   vector<string> m_previousCommands;

   void SaveCommand(string& command);
};

#endif