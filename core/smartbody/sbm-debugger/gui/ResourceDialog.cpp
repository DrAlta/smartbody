#include "ResourceDialog.h"

#include "vhmsg-tt.h"
#include "vhcl.h"

// callbacks
bool GetPathsCB(void* caller, NetRequest* req);

ResourceDialog::ResourceDialog(SbmDebuggerClient* client, QWidget *parent) 
: QDialog(parent)
{
   m_client = client;
   m_pScene = client->GetScene();
   ui.setupUi(this);

   connect(ui.refreshButton, SIGNAL(pressed()), this, SLOT(Refresh()));

   ui.resourceTree->setHeaderLabel("Resources");

   QTreeWidgetItem* paths = new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Paths")));
   ui.resourceTree->insertTopLevelItem(Paths, paths);
   paths->addChild(new QTreeWidgetItem(paths, QStringList(QString("Sequence Paths"))));
   paths->addChild(new QTreeWidgetItem(paths, QStringList(QString("ME Paths"))));
   paths->addChild(new QTreeWidgetItem(paths, QStringList(QString("Audio Paths"))));
   paths->addChild(new QTreeWidgetItem(paths, QStringList(QString("Mesh Paths"))));

   ui.resourceTree->insertTopLevelItem(Seq_Files, new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Seq Files"))));
   ui.resourceTree->insertTopLevelItem(Skeletons, new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Skeletons"))));
   ui.resourceTree->insertTopLevelItem(Bone_Map, new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Bone Map"))));
   ui.resourceTree->insertTopLevelItem(Motions, new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Motions"))));
   ui.resourceTree->insertTopLevelItem(Face_Definition, new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Face Definition"))));
   ui.resourceTree->insertTopLevelItem(Event_Handlers, new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Event Handlers"))));
   ui.resourceTree->insertTopLevelItem(Pawns, new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Pawns"))));
   ui.resourceTree->insertTopLevelItem(Characters, new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Characters"))));
   ui.resourceTree->insertTopLevelItem(_Scene, new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Scene"))));
   ui.resourceTree->insertTopLevelItem(Services, new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Services"))));

   Refresh();
}

ResourceDialog::~ResourceDialog()
{
   
}

void ResourceDialog::SendGetAssetPathCommand(string assetType, NetRequest::RequestId rid)
{
   string command = vhcl::Format("scene.getAssetPaths(\'%s\')", assetType.c_str());
   m_client->SendSBMCommand(rid, "string-array", command, GetPathsCB, this);
}

void ResourceDialog::Refresh()
{
   SendGetAssetPathCommand("seq", NetRequest::Get_Seq_Asset_Paths);
   SendGetAssetPathCommand("me", NetRequest::Get_ME_Asset_Paths);
   SendGetAssetPathCommand("audio", NetRequest::Get_Audio_Asset_Paths);
   SendGetAssetPathCommand("mesh", NetRequest::Get_Mesh_Asset_Paths);
}

void ResourceDialog::AddEntry(const QString& pathType, QStringList& paths)
{
   QTreeWidgetItem* widget = ui.resourceTree->topLevelItem(Paths);

   // find the specified path
   for (int i = 0; i < widget->childCount(); i++)
   {
      QTreeWidgetItem* child = widget->child(i);
      if (child->text(0) == pathType)
      {
         // remove all old children first
         while (child->childCount() > 0)
         {
            child->removeChild(child->child(0));
         }

         // add new children
         for (int j = 0; j < paths.size(); j++)
         {
            child->addChild(new QTreeWidgetItem(QStringList(paths[j])));
         }
         return;
      }
   }
}

bool GetPathsCB(void* caller, NetRequest* req)
{
   ResourceDialog* dlg = req->getCaller<ResourceDialog*>();
   vector<string> args = req->Args();
   QStringList list;

   for (unsigned int i = 0; i < args.size(); i++)
   {
      list.append(args[i].c_str());
   }

   switch (req->Rid())
   {
   case NetRequest::Get_Seq_Asset_Paths:
      dlg->AddEntry("Sequence Paths", list);
      break;

   case NetRequest::Get_ME_Asset_Paths:
      dlg->AddEntry("ME Paths", list);
      break;

   case NetRequest::Get_Audio_Asset_Paths:
      dlg->AddEntry("Audio Paths", list);
      break;

   case NetRequest::Get_Mesh_Asset_Paths:
      dlg->AddEntry("Mesh Paths", list);
      break;
   }
   
   return true;
}