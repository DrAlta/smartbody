#include "ResourceDialog.h"

#include "vhmsg-tt.h"

ResourceDialog::ResourceDialog(QWidget *parent) 
: QDialog(parent)
{
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
   ui.resourceTree->insertTopLevelItem(Scene, new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Scene"))));
   ui.resourceTree->insertTopLevelItem(Services, new QTreeWidgetItem(ui.resourceTree, QStringList(QString("Services"))));
}

ResourceDialog::~ResourceDialog()
{
   
}

void ResourceDialog::Refresh()
{
   
}