/********************************************************************************
** Form generated from reading UI file 'ResourceDialog.ui'
**
** Created: Wed Jan 18 13:38:10 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RESOURCEDIALOG_H
#define UI_RESOURCEDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHeaderView>
#include <QtGui/QPlainTextEdit>

QT_BEGIN_NAMESPACE

class Ui_ResourceDialog
{
public:
    QDialogButtonBox *buttonBox;
    QPlainTextEdit *plainTextEdit;

    void setupUi(QDialog *ResourceDialog)
    {
        if (ResourceDialog->objectName().isEmpty())
            ResourceDialog->setObjectName(QString::fromUtf8("ResourceDialog"));
        ResourceDialog->resize(400, 300);
        buttonBox = new QDialogButtonBox(ResourceDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(290, 20, 81, 241));
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Ok);
        plainTextEdit = new QPlainTextEdit(ResourceDialog);
        plainTextEdit->setObjectName(QString::fromUtf8("plainTextEdit"));
        plainTextEdit->setGeometry(QRect(10, 20, 271, 271));
        plainTextEdit->setReadOnly(true);

        retranslateUi(ResourceDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ResourceDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ResourceDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(ResourceDialog);
    } // setupUi

    void retranslateUi(QDialog *ResourceDialog)
    {
        ResourceDialog->setWindowTitle(QApplication::translate("ResourceDialog", "Resource", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ResourceDialog: public Ui_ResourceDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RESOURCEDIALOG_H
