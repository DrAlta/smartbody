#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
   ui.setupUi(this);

   ui.cameraControlBox->addItem("Free Look");
   ui.cameraControlBox->addItem("Follow Renderer");
}

SettingsDialog::~SettingsDialog()
{
   
}

void SettingsDialog::accept()
{
   done(Accepted);
   emit DialogFinished(this, Accepted);
}

void SettingsDialog::reject()
{
   done(Rejected);
   emit DialogFinished(this, Rejected);
}

