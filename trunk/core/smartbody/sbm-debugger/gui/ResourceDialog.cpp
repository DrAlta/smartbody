#include "ResourceDialog.h"

#include "vhmsg-tt.h"

ResourceDialog::ResourceDialog(Scene* pScene, QWidget *parent) 
: QDialog(parent)
{
   m_pScene = pScene;
   ui.setupUi(this);

   connect(ui.refreshButton, SIGNAL(pressed()), this, SLOT(Refresh()));

   ui.resourceTree->setHeaderLabel("Resources");

   QTreeWidgetItem* paths = new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Paths")));
   ui.resourceTree->insertTopLevelItem(Paths, paths);
   paths->addChild(new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Sequence Paths"))));
   paths->addChild(new QTreeWidgetItem(ui.resourceTree, QStringList(QString("ME Paths"))));
   paths->addChild(new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Audio Paths"))));
   paths->addChild(new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Mesh Paths"))));

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

void ResourceDialog::Refresh()
{
   //Path
   QTreeWidgetItem* widget = ui.resourceTree->topLevelItem(Paths);
   for (int i = 0; i < widget->childCount(); i++)
   {
      QTreeWidgetItem* child = widget->child(i);
      if (child->text(0) == "Sequence Paths")
      {
         for (unsigned int j = 0; j < m_pScene->m_sequencePaths.size(); j++)
         {
            child->addChild(new QTreeWidgetItem(QStringList(QString(m_pScene->m_sequencePaths[i].c_str()))));
         }
      }
      else if (child->text(0) == "ME Paths")
      {
         for (unsigned int j = 0; j < m_pScene->m_mePaths.size(); j++)
         {
            child->addChild(new QTreeWidgetItem(QStringList(QString(m_pScene->m_mePaths[i].c_str()))));
         }
      }
      else if (child->text(0) == "Audio Paths")
      {
         for (unsigned int j = 0; j < m_pScene->m_audioPaths.size(); j++)
         {
            child->addChild(new QTreeWidgetItem(QStringList(QString(m_pScene->m_audioPaths[i].c_str()))));
         }
      }
      else if (child->text(0) == "Mesh Paths")
      {
         for (unsigned int j = 0; j < m_pScene->m_meshPaths.size(); j++)
         {
            child->addChild(new QTreeWidgetItem(QStringList(QString(m_pScene->m_meshPaths[i].c_str()))));
         }
      }
   }
}