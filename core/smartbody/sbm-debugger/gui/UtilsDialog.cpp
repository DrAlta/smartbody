#include "UtilsDialog.h"
#include "vhmsg-tt.h"

UtilsDialog::UtilsDialog(Scene* pScene, GLWidget* pRenderView, QWidget* parent) : QDialog(parent)
{
   ui.setupUi(this);
   m_pScene = pScene;
   m_pRenderView = pRenderView;

   connect(ui.showAxesBox, SIGNAL(toggled(bool)), pRenderView, SLOT(ToggleShowAxes(bool)));
   connect(ui.showEyeBeamsBox, SIGNAL(toggled(bool)), pRenderView, SLOT(ToggleShowEyeBeams(bool)));
   connect(ui.allowBoneUpdatesBox, SIGNAL(toggled(bool)), pRenderView, SLOT(ToggleAllowBoneUpdates(bool)));
}

UtilsDialog::~UtilsDialog()
{
   
}