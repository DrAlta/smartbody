#include "vhcl.h"

#include "UtilsDialog.h"

#include <algorithm>

#include "vhmsg-tt.h"


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
   connect(ui.SpeakButton, SIGNAL(pressed()), this, SLOT(SpeakButtonPressed()));
   connect(ui.QueryAnimButton, SIGNAL(pressed()), this, SLOT(QueryAnimsPressed()));
   connect(ui.refreshButton, SIGNAL(pressed()), this, SLOT(Refresh()));
   connect(ui.animFilterBox, SIGNAL(textChanged()), this, SLOT(FilterAnims()));

   Refresh();
}

UtilsDialog::~UtilsDialog()
{
   
}

void UtilsDialog::GazeAtPressed()
{
   //"sbm test bml character {0} gaze target {1}", gazer, gazeTarget
   string message = vhcl::Format("sbm test bml character %s gaze target %s", GetSelectedChar().c_str(),
      ui.gazeTargetBox->currentText().toStdString().c_str());
   vhmsg::ttu_notify1(message.c_str());
}

void UtilsDialog::RunBmlPressed()
{
   string message = vhcl::Format("sbm bml char %s file %s", GetSelectedChar().c_str(),
      ui.bmlFilesBox->currentText().toStdString().c_str());
   vhmsg::ttu_notify1(message.c_str());
   
   if (ui.bmlFilesBox->findText(ui.bmlFilesBox->currentText()) == -1)
   {
      // it's not already in the list, so add it
      ui.bmlFilesBox->insertItem(0, ui.bmlFilesBox->currentText());
   }
}

void UtilsDialog::PlayAnimPressed()
{
   string message = vhcl::Format("sbm test bml char %s anim %s", GetSelectedChar().c_str(),
      ui.animationNamesBox->currentText().toStdString().c_str());
   vhmsg::ttu_notify1(message.c_str());
}

void UtilsDialog::SpeakButtonPressed()
{
   //sbm test bml character {0} recipient ALL speech \"{1}\"", speaker, text
   string message = vhcl::Format("sbm test bml character %s recipient ALL speech %s", GetSelectedChar().c_str(),
      ui.ttsBox->toPlainText().toStdString().c_str());
   vhmsg::ttu_notify1(message.c_str());
}

void UtilsDialog::QueryAnimsPressed()
{
   vhmsg::ttu_notify1("sbm vhmsglog on");
   vhmsg::ttu_notify1("sbm resource motion");
}

void UtilsDialog::FilterAnims()
{
   // only keep animations in the combo box that match the filter string
   ui.animationNamesBox->clear();
   string temp = "";
   string filterText = ui.animFilterBox->toPlainText().toStdString();
   transform(filterText.begin(), filterText.end(), filterText.begin(), ::tolower);

   for (unsigned int i = 0; i < m_pScene->m_animations.size(); i++)
   {
      temp = m_pScene->m_animations[i];
      transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
      if (temp.find(filterText) != string::npos)
      {
         ui.animationNamesBox->addItem(m_pScene->m_animations[i].c_str());
      }
   }
}

void UtilsDialog::Refresh()
{
   ui.gazeTargetBox->clear();
   ui.selectedCharacterBox->clear();
   ui.animationNamesBox->clear();

   for (unsigned int i = 0; i < m_pScene->m_characters.size(); i++)
   {
      ui.selectedCharacterBox->addItem(m_pScene->m_characters[i].m_name.c_str());
      ui.gazeTargetBox->addItem(m_pScene->m_characters[i].m_name.c_str());
   }

   for (unsigned int i = 0; i < m_pScene->m_pawns.size(); i++)
   {
      ui.gazeTargetBox->addItem(m_pScene->m_pawns[i].m_name.c_str());
   }

   FilterAnims();
}

std::string UtilsDialog::GetSelectedChar()
{
   return ui.selectedCharacterBox->currentText().toStdString();
}
