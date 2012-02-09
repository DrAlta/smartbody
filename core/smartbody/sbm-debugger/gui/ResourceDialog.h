#ifndef RESOURCE_DIALOG_H_
#define RESOURCE_DIALOG_H_

#include "ui_ResourceDialog.h"
#include "SbmDebuggerCommon.h"

class ResourceDialog : public QDialog
{
   Q_OBJECT

public:
   ResourceDialog(Scene* pScene, QWidget *parent = 0);
   ~ResourceDialog();

   Ui::ResourceDialog ui;

public slots:
   void Refresh();

private:
   enum ResourceHeaders
   {
      Paths,
      Seq_Files,
      Skeletons,
      Bone_Map,
      Motions,
      Face_Definition,
      Event_Handlers,
      Pawns,
      Characters,
      _Scene,
      Services
   };

   Scene* m_pScene;
};

#endif