#include "UtilsDialog.h"
#include "vhmsg-tt.h"
#include "vhcl.h"

UtilsDialog::UtilsDialog(Scene* pScene, GLWidget* pRenderView, QWidget* parent) : QDialog(parent)
{
   ui.setupUi(this);
   m_pScene = pScene;
   m_pRenderView = pRenderView;

   connect(ui.showAxesBox, SIGNAL(toggled(bool)), pRenderView, SLOT(ToggleShowAxes(bool)));
   connect(ui.showEyeBeamsBox, SIGNAL(toggled(bool)), pRenderView, SLOT(ToggleShowEyeBeams(bool)));
   connect(ui.allowBoneUpdatesBox, SIGNAL(toggled(bool)), pRenderView, SLOT(ToggleAllowBoneUpdates(bool)));
   connect(ui.GazeAtButton, SIGNAL(pressed()), this, SLOT(GazeAtPressed()));
   connect(ui.runBmlButton, SIGNAL(pressed()), this, SLOT(RunBmlPressed()));
   connect(ui.PlayAnimButton, SIGNAL(pressed()), this, SLOT(PlayAnimPressed()));
   connect(ui.refreshButton, SIGNAL(pressed()), this, SLOT(Refresh()));

   Refresh();
}

UtilsDialog::~UtilsDialog()
{
   
}

void UtilsDialog::GazeAtPressed()
{
   //"sbm test bml character {0} gaze target {1}", gazer, gazeTarget
   std::string message = vhcl::Format("sbm test bml character %s gaze target %s", GetSelectedChar().c_str(),
      ui.gazeTargetBox->currentText().toStdString().c_str());
   vhmsg::ttu_notify1(message.c_str());
}

void UtilsDialog::RunBmlPressed()
{
   std::string message = vhcl::Format("sbm bml char %s file %s", GetSelectedChar().c_str(),
      ui.bmlFilesBox->currentText().toStdString().c_str());
   vhmsg::ttu_notify1(message.c_str());
}

void UtilsDialog::PlayAnimPressed()
{
   std::string message = vhcl::Format("sbm test bml char %s anim %s", GetSelectedChar().c_str(),
      ui.animationNamesBox->currentText().toStdString().c_str());
   vhmsg::ttu_notify1(message.c_str());
}

void UtilsDialog::Refresh()
{
   ui.gazeTargetBox->clear();
   ui.selectedCharacterBox->clear();

   for (unsigned int i = 0; i < m_pScene->m_characters.size(); i++)
   {
      ui.selectedCharacterBox->addItem(m_pScene->m_characters[i].m_name.c_str());
      ui.gazeTargetBox->addItem(m_pScene->m_characters[i].m_name.c_str());
   }

   for (unsigned int i = 0; i < m_pScene->m_pawns.size(); i++)
   {
      ui.gazeTargetBox->addItem(m_pScene->m_pawns[i].m_name.c_str());
   }
}

std::string UtilsDialog::GetSelectedChar()
{
   return ui.selectedCharacterBox->currentText().toStdString();
}