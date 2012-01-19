/********************************************************************************
** Form generated from reading UI file 'SettingsDialog.ui'
**
** Created: Wed Jan 18 12:54:54 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGSDIALOG_H
#define UI_SETTINGSDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QTabWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SettingsDialog
{
public:
    QDialogButtonBox *buttonBox;
    QTabWidget *tabWidget;
    QWidget *general_tab;
    QDoubleSpinBox *positionScaleBox;
    QLabel *positionScaleLabel;
    QWidget *camera_tab;
    QComboBox *cameraControlBox;
    QLabel *cameraControlLabel;

    void setupUi(QDialog *SettingsDialog)
    {
        if (SettingsDialog->objectName().isEmpty())
            SettingsDialog->setObjectName(QString::fromUtf8("SettingsDialog"));
        SettingsDialog->resize(466, 260);
        buttonBox = new QDialogButtonBox(SettingsDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setGeometry(QRect(380, 30, 81, 51));
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        tabWidget = new QTabWidget(SettingsDialog);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(10, 10, 361, 231));
        general_tab = new QWidget();
        general_tab->setObjectName(QString::fromUtf8("general_tab"));
        positionScaleBox = new QDoubleSpinBox(general_tab);
        positionScaleBox->setObjectName(QString::fromUtf8("positionScaleBox"));
        positionScaleBox->setGeometry(QRect(10, 30, 62, 22));
        positionScaleBox->setSingleStep(0.1);
        positionScaleBox->setValue(0.01);
        positionScaleLabel = new QLabel(general_tab);
        positionScaleLabel->setObjectName(QString::fromUtf8("positionScaleLabel"));
        positionScaleLabel->setGeometry(QRect(10, 10, 71, 16));
        tabWidget->addTab(general_tab, QString());
        camera_tab = new QWidget();
        camera_tab->setObjectName(QString::fromUtf8("camera_tab"));
        cameraControlBox = new QComboBox(camera_tab);
        cameraControlBox->setObjectName(QString::fromUtf8("cameraControlBox"));
        cameraControlBox->setGeometry(QRect(10, 30, 101, 22));
        cameraControlLabel = new QLabel(camera_tab);
        cameraControlLabel->setObjectName(QString::fromUtf8("cameraControlLabel"));
        cameraControlLabel->setGeometry(QRect(10, 10, 81, 16));
        tabWidget->addTab(camera_tab, QString());

        retranslateUi(SettingsDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), SettingsDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), SettingsDialog, SLOT(reject()));

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(SettingsDialog);
    } // setupUi

    void retranslateUi(QDialog *SettingsDialog)
    {
        SettingsDialog->setWindowTitle(QApplication::translate("SettingsDialog", "Settings", 0, QApplication::UnicodeUTF8));
        positionScaleBox->setPrefix(QString());
        positionScaleLabel->setText(QApplication::translate("SettingsDialog", "Position Scale", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(general_tab), QApplication::translate("SettingsDialog", "General", 0, QApplication::UnicodeUTF8));
        cameraControlLabel->setText(QApplication::translate("SettingsDialog", "Camera Control", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(camera_tab), QApplication::translate("SettingsDialog", "Camera", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class SettingsDialog: public Ui_SettingsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGSDIALOG_H
