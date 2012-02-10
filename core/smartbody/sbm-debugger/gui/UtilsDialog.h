#ifndef UTILS_DIALOG_H_
#define UTILS_DIALOG_H_

#include "ui_UtilsDialog.h"
#include "GLWidget.h"

using std::string;
using std::vector;

class UtilsDialog : public QDialog
{
   Q_OBJECT

public:
   UtilsDialog(Scene* pScene, GLWidget* pRenderView, QWidget* parent = 0);
   ~UtilsDialog();

private:
   std::string GetSelectedChar();

   Ui::UtilsDialog ui;
   Scene* m_pScene;
   GLWidget* m_pRenderView;

private slots:
   void GazeAtPressed();
   void RunBmlPressed();
   void PlayAnimPressed();
   void SpeakButtonPressed();
   void QueryAnimsPressed();
   void FilterAnims();
   void Refresh();
};

#endif