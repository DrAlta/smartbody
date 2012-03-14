#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
   ui.setupUi(this);

   ui.cameraControlBox->addItem("Free Look");
   ui.cameraControlBox->addItem("Follow Renderer");
   ui.unitsBox->addItem("0.01");
   ui.unitsBox->addItem("1.0");
   ui.unitsBox->setCurrentIndex(1);
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

