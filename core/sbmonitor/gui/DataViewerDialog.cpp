#include "vhcl.h"

#include "DataViewerDialog.h"

//callbacks
bool GetStringList(void* caller, NetRequest* req);

DataViewerDialog::DataViewerDialog(SbmDebuggerClient* client, QWidget* parent) : QDialog(parent)
{
   m_client = client;
   m_pScene = m_client->GetScene();
   ui.setupUi(this);
   m_pGraphWidget = new GLGraphWidget(ui.renderSize->geometry(), m_pScene, this);
   //m_pGraphWidget->AddLineGraphPoint(Vector3(0, 0, -10.0f), Vector3(1, 0, 1), ui.numFramesBox->value());
   timer.start(10, this);

   ui.showRotaionAsBox->addItem("Quaternion");
   ui.showRotaionAsBox->addItem("Euler Angle");
   //ui.showRotaionAsBox->addItem("Axis-Angle");
   //ui.showRotaionAsBox->addItem("Quaternion Velocity");
   //ui.showRotaionAsBox->addItem("Euler Velocity");
   //ui.showRotaionAsBox->addItem("Axis-Angle Velocity");

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

void DataViewerDialog::Update()
{
   if (ui.monitoredListBox->count() == 0 || ui.characterNameBox->count() == 0)
      return; 

   // find the character that is selected
   Character* pSelectedCharacter = m_pScene->FindCharacter(ui.characterNameBox->currentText().toStdString());
   if (!pSelectedCharacter)
      return;
   
   static float somefloat = 0.1f;
   
   // go through the monitored joints and update the graph with the new data
   for (int i = 0; i < ui.monitoredListBox->count(); i++)
   {
      QListWidgetItem* item = ui.monitoredListBox->item(i);
      QString channelName = item->text();
      QString suffix = "";
      double value = 0;
      int suffixIndex = channelName.lastIndexOf("_");
      
      if (suffixIndex != -1)
      {
         suffix = channelName.right(suffixIndex);
         channelName = channelName.remove(suffixIndex, channelName.length() - suffixIndex);
      }

      static float someFloat = 0;
      
      Joint* joint = pSelectedCharacter->FindJoint(channelName.toStdString());

      if (joint)
      {
         if (suffix.contains("x", Qt::CaseInsensitive))
            value = joint->pos.x;
         else if (suffix.contains("y", Qt::CaseInsensitive))
            value = joint->pos.y;
         else if (suffix.contains("z", Qt::CaseInsensitive))
            value = joint->pos.z;
         //else if (suffix.contains("quat", Qt::CaseInsensitive))
         //   value = joint->rot;

         m_pGraphWidget->AddLineGraphPoint(Vector3(someFloat, value, -10.0f), Vector3(1, 0, 1), ui.numFramesBox->value());
         someFloat += 0.01f;
      }
   }
}

void DataViewerDialog::timerEvent(QTimerEvent * event)
{
   if (event->timerId() == timer.timerId())
   {
      Update();
   }
   else
   {
      QWidget::timerEvent(event);
   }
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
      //for (int i = 0; i < names.length(); i++)
      //{
      //   if (names[i].contains("Quat"))
      //   {
      //      names[i].append(" (4)");
      //   }
      //   else
      //   {
      //      names[i].append(" (1)");
      //   }
      //}
      dlg->ui.channelListBox->addItems(names);
      break;

   case NetRequest::Get_Controller_Names:
      dlg->ui.controllerBox->clear();
      dlg->ui.controllerBox->addItems(names);
      break;
   }

   return true;
}
