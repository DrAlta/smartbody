#ifndef RESOURCE_DIALOG_H_
#define RESOURCE_DIALOG_H_

#include "ui_ResourceDialog.h"
#include "SbmDebuggerCommon.h"
#include "SbmDebuggerClient.h"

using std::string;

class ResourceDialog : public QDialog
{
   Q_OBJECT

public:
   ResourceDialog(SbmDebuggerClient* client, QWidget *parent = 0);
   ~ResourceDialog();

   Ui::ResourceDialog ui;
   void AddEntry(const QString& pathType, QStringList& paths);

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

   SbmDebuggerClient* m_client;
   Scene* m_pScene;

   void SendGetAssetPathCommand(string assetType, NetRequest::RequestId rid);
};

#endif