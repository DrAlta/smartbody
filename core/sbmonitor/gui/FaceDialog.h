#ifndef FACE_DIALOG_H_
#define FACE_DIALOG_H_

#include "ui_FaceDialog.h"
#include "SbmDebuggerCommon.h"
#include "SbmDebuggerClient.h"

#include <QtGui/QDoubleSpinBox>
#include <QtGui/QSlider>
#include <QtGui/QHBoxLayout>
#include <QtGui/QScrollArea>

using std::string;
using std::vector;

class FaceDialog : public QDialog
{
   Q_OBJECT

public:
   FaceDialog(SbmDebuggerClient* client, QWidget* parent = 0);
   ~FaceDialog();
   void AddFacialExpression(const string& name, double weight);
   Ui::FaceDialog ui;
   SbmDebuggerClient* m_client;

   // used for storage until the sides ("left", "right", "both") of the au's are obtained
   vector<string> m_AUNums;

private slots:
   void Reset();
   void SliderValueChanged(int val);
   void CharacterSelectionChanged(const QString& selection);

private:
   
   Scene* m_pScene;
   vector<QSlider*> m_Sliders;
   vector<QObject*> m_scrollListChildren;
};

#endif