#include "DataViewerDialog.h"
#include "vhcl.h"

//callbacks
bool GetStringList(void* caller, NetRequest* req);

DataViewerDialog::DataViewerDialog(SbmDebuggerClient* client, QWidget* parent) : QDialog(parent)
{
   m_client = client;
   m_pScene = m_client->GetScene();
   ui.setupUi(this);
   m_pGraphWidget = new GLGraphWidget(ui.renderSize->geometry(), m_pScene, this);

   ui.showRotaionAsBox->addItem("Quaternion");
   ui.showRotaionAsBox->addItem("Euler Angle");
   ui.showRotaionAsBox->addItem("Axis-Angle");
   ui.showRotaionAsBox->addItem("Quaternion Velocity");
   ui.showRotaionAsBox->addItem("Euler Velocity");
   ui.showRotaionAsBox->addItem("Axis-Angle Velocity");

   connect(ui.addChannelButton, SIGNAL(pressed()), this, SLOT(AddSelectedChannels()));
   connect(ui.removeChannelButton, SIGNAL(pressed()), this, SLOT(RemoveSelectedChannels()));
   connect(ui.showRotaionAsBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(ChangedRotationDisplay(const QString&)));
   connect(ui.refreshButton, SIGNAL(pressed()), this, SLOT(Refresh()));
   connect(ui.motionBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(ChangedSelectedMotion(const QString&)));
   connect(ui.characterNameBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(ChangedSelectedCharacter(const QString&)));

   Refresh();

}

DataViewerDialog::~DataViewerDialog()
{
   delete m_pGraphWidget;
}

void DataViewerDialog::Refresh()
{
   ui.characterNameBox->clear();
   for (unsigned int i = 0; i < m_pScene->m_characters.size(); i++)
   {
      ui.characterNameBox->addItem(m_pScene->m_characters[i].m_name.c_str());
   }

   m_client->SendSBMCommand(NetRequest::Get_Motion_Names, "string-array", "scene.getMotionNames()", GetStringList, this);
}

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

void DataViewerDialog::ChangedSelectedCharacter(const QString& text)
{
   string command = vhcl::Format("scene.getCharacter(\'%s\').getControllerNames()", text.toStdString().c_str());
   m_client->SendSBMCommand(NetRequest::Get_Controller_Names, "string-array", command, GetStringList, this);
}

void DataViewerDialog::ChangedSelectedMotion(const QString& text)
{
   string command = vhcl::Format("scene.getMotion(\'%s\').getChannels()", text.toStdString().c_str());
   m_client->SendSBMCommand(NetRequest::Get_Channel_Names, "string-array", command, GetStringList, this);
}

bool GetStringList(void* caller, NetRequest* req)
{
   DataViewerDialog* dlg = req->getCaller<DataViewerDialog*>();

   vector<string> args = req->Args();
   QStringList names;
   for (unsigned int i = 0; i < args.size(); i++)
   {
      names.append(args[i].c_str());
   }

   switch (req->Rid())
   {
   case NetRequest::Get_Motion_Names:
      dlg->ui.motionBox->clear();
      dlg->ui.motionBox->addItems(names);
      break;

   case NetRequest::Get_Channel_Names:
      dlg->ui.channelListBox->clear();
      dlg->ui.channelListBox->addItems(names);
      break;

   case NetRequest::Get_Controller_Names:
      dlg->ui.controllerBox->clear();
      dlg->ui.controllerBox->addItems(names);
      break;
   }

   return true;
}