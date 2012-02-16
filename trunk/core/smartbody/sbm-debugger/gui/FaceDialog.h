#ifndef FACE_DIALOG_H_
#define FACE_DIALOG_H_

#include "ui_FaceDialog.h"
#include "SbmDebuggerCommon.h"

#include <QtGui/QDoubleSpinBox>
#include <QtGui/QSlider>
#include <QtGui/QHBoxLayout>

using std::string;
using std::vector;

class FaceDialog : public QDialog
{
   Q_OBJECT

public:
   FaceDialog(Scene* pScene, QWidget* parent = 0);
   ~FaceDialog();

private slots:
   void Reset();
   void SliderValueChanged(int val);

private:
   void AddFacialExpression(const string& name, double weight);
  
   Ui::FaceDialog ui;
   Scene* m_pScene;
   vector<QSlider*> m_Sliders;
};

#endif