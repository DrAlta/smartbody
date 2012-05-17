#ifndef UTILS_DIALOG_H_
#define UTILS_DIALOG_H_

#include "QtCrtDbgOff.h"
#include <QtGui>
#include "QtCrtDbgOn.h"

#include "ui_UtilsDialog.h"
#include "SbmDebuggerClient.h"

class Scene;
class GLWidget;

using std::vector;
using std::string;

class UtilsDialog : public QDialog
{
   Q_OBJECT

public:
   UtilsDialog(SbmDebuggerClient* client, GLWidget* pRenderView, QWidget* parent = 0);
   ~UtilsDialog();
   vector<string> m_animations;
   Ui::UtilsDialog ui;

private:
   std::string GetSelectedChar();
   SbmDebuggerClient* m_client;
   Scene* m_pScene;
   GLWidget* m_pRenderView;

private slots:
   void GazeAtPressed();
   void StopGazePressed();
   void RunBmlPressed();
   void PlayAnimPressed();
   void SetPosturePressed();
   void SpeakButtonPressed();
   void QueryAnimsPressed();
   void FilterAnims();
   void Refresh();
};

#endif
