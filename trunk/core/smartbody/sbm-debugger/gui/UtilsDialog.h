#ifndef UTILS_DIALOG_H_
#define UTILS_DIALOG_H_

#include "ui_UtilsDialog.h"
#include "GLWidget.h"

class UtilsDialog : public QDialog
{
   Q_OBJECT

public:
   UtilsDialog(Scene* pScene, GLWidget* pRenderView, QWidget* parent = 0);
   ~UtilsDialog();

private:
   Ui::UtilsDialog ui;
   Scene* m_pScene;
   GLWidget* m_pRenderView;
};

#endif