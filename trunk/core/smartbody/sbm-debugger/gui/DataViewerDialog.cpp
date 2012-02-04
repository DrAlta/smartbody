#include "DataViewerDialog.h"

DataViewerDialog::DataViewerDialog(Scene* scene, QWidget* parent) : QDialog(parent)
{
   ui.setupUi(this);
   m_pGraphWidget = new GLGraphWidget(ui.renderSize->geometry(), scene, this);
   m_pScene = scene;

   for (unsigned int i = 0; i < m_pScene->m_characters.size(); i++)
   {
      ui.characterNameBox->addItem(m_pScene->m_characters[i].m_name.c_str());
   }
   
   //if (m_pScene->m_characters.size() > 0)
   //{
   //   AddAllJointsToList(ui.channelListBox, m_pScene->m_characters[0].m_joints);
   //}

   ui.showRotaionAsBox->addItem("Quaternion");
   ui.showRotaionAsBox->addItem("Euler Angle");
   ui.showRotaionAsBox->addItem("Axis-Angle");
   ui.showRotaionAsBox->addItem("Quaternion Velocity");
   ui.showRotaionAsBox->addItem("Euler Velocity");
   ui.showRotaionAsBox->addItem("Axis-Angle Velocity");

   connect(ui.addChannelButton, SIGNAL(pressed()), this, SLOT(AddSelectedChannels()));
   connect(ui.removeChannelButton, SIGNAL(pressed()), this, SLOT(RemoveSelectedChannels()));
   connect(ui.showRotaionAsBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(ChangedRotationDisplay(const QString&)));
}

DataViewerDialog::~DataViewerDialog()
{
   delete m_pGraphWidget;
}

//void DataViewerDialog::AddAllJointsToList(QListWidget* list, std::vector<Joint*>& joints)
//{
//   for (unsigned int i = 0; i < joints.size(); i++)
//   {
//      list->addItem(joints[i]->m_name.c_str());
//      AddAllJointsToList(list, joints[i]->m_joints);
//   }
//}

void DataViewerDialog::AddSelectedChannels()
{
   QList<QListWidgetItem*> selectedItems = ui.channelListBox->selectedItems();
   for (int i = 0; i < selectedItems.length(); i++)
   {
      ui.monitoredListBox->addItem(selectedItems[i]->text());
      delete ui.channelListBox->takeItem(ui.channelListBox->row(selectedItems[i]));
   }
}

void DataViewerDialog::RemoveSelectedChannels()
{
   QList<QListWidgetItem*> selectedItems = ui.monitoredListBox->selectedItems();
   for (int i = 0; i < selectedItems.length(); i++)
   {
      ui.channelListBox->addItem(selectedItems[i]->text());
      delete ui.monitoredListBox->takeItem(ui.monitoredListBox->row(selectedItems[i]));
   }
}

void DataViewerDialog::ChangedRotationDisplay(const QString& text)
{
   ui.wCheckBox->setEnabled(text == "Quaternion");
}