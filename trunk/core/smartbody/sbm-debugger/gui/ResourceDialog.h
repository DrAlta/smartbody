#ifndef RESOURCE_DIALOG_H_
#define RESOURCE_DIALOG_H_

#include "ui_ResourceDialog.h"

class ResourceDialog : public QDialog
{
   Q_OBJECT

public:
   ResourceDialog(QWidget *parent = 0);
   ~ResourceDialog();

   Ui::ResourceDialog ui;
};

#endif