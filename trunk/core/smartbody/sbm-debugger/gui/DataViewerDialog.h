#ifndef DATA_VIEWER_DIALOG_H_
#define DATA_VIEWER_DIALOG_H_

#include "ui_DataViewerDialog.h"
#include "SbmDebuggerCommon.h"
#include "GLGraphWidget.h"

class DataViewerDialog : public QDialog
{
   Q_OBJECT

public:
   DataViewerDialog(Scene* scene, QWidget* parent = 0);
   ~DataViewerDialog();

private slots:
   void AddSelectedChannels();
   void RemoveSelectedChannels();
   void ChangedRotationDisplay(const QString&);

private:
   Ui::DataViewerDialog ui;
   Scene* m_pScene;
   GLGraphWidget* m_pGraphWidget;

   //void AddAllJointsToList(QListWidget* list, std::vector<Joint*>& joints);
};

#endif