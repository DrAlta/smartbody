
#include "vhcl.h"

#include "SbmDebuggerForm.h"

#include "vhmsg-tt.h"
#include "SbmDebuggerClient.h"

#include "SettingsDialog.h"
#include "ResourceDialog.h"


using std::vector;
using std::string;


SbmDebuggerClient c;


SbmDebuggerForm::SbmDebuggerForm(QMainWindow* mainWindow, QWidget *parent)
  : QWidget(parent)
{
  m_pMainWindow = mainWindow;
  ui.setupUi(mainWindow);
  m_pMainWindow->show();

  m_pGLWidget = new GLWidget(c.GetScene(), this);

  // setup renderer size and positioning
  QPoint rendererPosition = ui.RenderView->pos();
  QSize rendererSize = ui.RenderView->size();
  m_pGLWidget->setGeometry(ui.RenderView->x() + 40, ui.RenderView->y() + 25,
     rendererSize.width(), rendererSize.height());
  m_pMainWindow->layout()->addWidget(m_pGLWidget);

  // set vhmsg
  vhmsg::ttu_set_client_callback(VHMsgCallback);
  vhmsg::ttu_open();
  vhmsg::ttu_register("sbmdebugger");

  InitSignalsSlots();

  setUpdatesEnabled(true);

  // setup scene tree
  ui.sceneTree->insertTopLevelItem(Characters, new QTreeWidgetItem(ui.sceneTree, QStringList(QString("Characters"))));
  ui.sceneTree->insertTopLevelItem(Pawns, new QTreeWidgetItem(ui.sceneTree, QStringList(QString("Pawns"))));
  ui.sceneTree->setHeaderLabel(QString("Entities"));
}

SbmDebuggerForm::~SbmDebuggerForm()
{

}

void SbmDebuggerForm::InitSignalsSlots()
{
   connect(ui.actionConnect, SIGNAL(triggered()), this, SLOT(ShowConnectDialog()));
   connect(ui.actionDisconnect, SIGNAL(triggered()), this, SLOT(Disconnect()));
   connect(ui.actionSettings, SIGNAL(triggered()), this, SLOT(ShowSettingsDialog()));
   connect(ui.actionExit, SIGNAL(triggered()), m_pMainWindow, SLOT(close()));
   connect(ui.actionResource_Viewer, SIGNAL(triggered()), this, SLOT(ShowResourceDialog()));
   timer.start(10, this);
}

void SbmDebuggerForm::ShowConnectDialog()
{
   ConnectDialog dlg(this); 
   if (dlg.exec() == QDialog::Accepted)
   {
      // they hit ok to connect to a renderer
      QListWidget * listWidget = dlg.GetListWidget();
      if (listWidget->currentItem())
      {
         string sbmId = listWidget->currentItem()->text().toStdString();
         c.Connect(sbmId);
         vhmsg::ttu_wait(2);
         vhmsg::ttu_wait(2);
         if (c.GetConnectResult())
         {
             printf("Connect succeeded to id: %s\n", sbmId.c_str());
         }

         c.Init();

         c.StartUpdates(0.10f);

         // since you're connected, enable the disconnect button
         ui.actionDisconnect->setEnabled(true);
      }
   }
   else 
   {
      // TODO: They hit the Esc key, cancel button, or Close button
   }
}

void SbmDebuggerForm::ShowSettingsDialog()
{
   SettingsDialog dlg(this);
   connect(&dlg, SIGNAL(DialogFinished(const SettingsDialog*, int)), m_pGLWidget, SLOT(OnCloseSettingsDialog(const SettingsDialog*, int)));
   if (dlg.exec() == QDialog::Accepted)
   {

   }
   else
   {

   }
}

void SbmDebuggerForm::ShowResourceDialog()
{
   ResourceDialog dlg(this);
   if (dlg.exec() == QDialog::Accepted)
   {

   }
   else
   {

   }
}

void SbmDebuggerForm::Disconnect()
{
   ui.actionDisconnect->setEnabled(false);

   // TODO: send a disconnect message to the renderer
}

void SbmDebuggerForm::Update()
{
   int err = vhmsg::ttu_poll();
   if (err == vhmsg::TTU_ERROR)
   {
      printf("ttu_poll ERR\n");
   }

   c.Update();

   UpdateSceneTree();
   UpdateLabels();
}

void SbmDebuggerForm::UpdateSceneTree()
{
   string entityName = "";

   // character tree
   for (unsigned int i = 0; i < c.GetScene()->m_characters.size(); i++)
   {
      entityName = c.GetScene()->m_characters[i].m_name;
      QTreeWidgetItem* subTree = ui.sceneTree->topLevelItem(Characters);
      bool alreadyExistsInTree = false;
      for (int j = 0; j < subTree->childCount(); j++)
      {
         if (subTree->child(j)->text(Characters).toStdString() == entityName)
         {
            alreadyExistsInTree = true;
            break;
         }       
      }

      if (!alreadyExistsInTree)
      {
         // the entity isn't represented in the tree view, add it
         subTree->addChild(new QTreeWidgetItem(subTree, QStringList(QString(entityName.c_str()))));
      } 
   }

   // pawn tree
   for (unsigned int i = 0; i < c.GetScene()->m_pawns.size(); i++)
   {
      entityName = c.GetScene()->m_pawns[i].m_name;
      QTreeWidgetItem* subTree = ui.sceneTree->topLevelItem(Pawns);
      bool alreadyExistsInTree = false;
      for (int j = 0; j < subTree->childCount(); j++)
      {
         if (subTree->child(j)->text(Pawns).toStdString() == entityName)
         {
            alreadyExistsInTree = true;
            break;
         }       
      }

      if (!alreadyExistsInTree)
      {
         // the entity isn't represented in the tree view, add it
         subTree->addChild(new QTreeWidgetItem(subTree, QStringList(QString(entityName.c_str()))));
       } 
   }
}

void SbmDebuggerForm::UpdateLabels()
{
   //ui.rendererFpsLabel->
   ui.cameraPositionLabel->setText(("Camera Pos: " + m_pGLWidget->GetCameraPositionAsString()).c_str());
}

void SbmDebuggerForm::timerEvent(QTimerEvent * event)
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


void SbmDebuggerForm::VHMsgCallback( const char * op, const char * args, void * userData )
{
   c.ProcessVHMsgs(op, args);
}
