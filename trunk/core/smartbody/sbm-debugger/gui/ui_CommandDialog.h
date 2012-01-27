/********************************************************************************
** Form generated from reading UI file 'CommandDialog.ui'
**
** Created: Thu Jan 26 15:57:35 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMMANDDIALOG_H
#define UI_COMMANDDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CommandDialog
{
public:
    QVBoxLayout *verticalLayout;
    QPlainTextEdit *outputTextEdit;
    QFrame *frame;
    QHBoxLayout *horizontalLayout;
    QPushButton *runButton;
    QPushButton *clearTopButton;
    QTabWidget *tabWidget;
    QWidget *sbmTab;
    QPlainTextEdit *sbmTextEdit;
    QWidget *pythonTab;
    QPlainTextEdit *pythonTextEdit;

    void setupUi(QDialog *CommandDialog)
    {
        if (CommandDialog->objectName().isEmpty())
            CommandDialog->setObjectName(QString::fromUtf8("CommandDialog"));
        CommandDialog->resize(747, 539);
        CommandDialog->setLayoutDirection(Qt::LeftToRight);
        verticalLayout = new QVBoxLayout(CommandDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        outputTextEdit = new QPlainTextEdit(CommandDialog);
        outputTextEdit->setObjectName(QString::fromUtf8("outputTextEdit"));
        outputTextEdit->setReadOnly(true);

        verticalLayout->addWidget(outputTextEdit);

        frame = new QFrame(CommandDialog);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setLayoutDirection(Qt::LeftToRight);
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(frame);
        horizontalLayout->setSpacing(9);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(9, -1, 600, -1);
        runButton = new QPushButton(frame);
        runButton->setObjectName(QString::fromUtf8("runButton"));
        runButton->setMaximumSize(QSize(75, 16777215));

        horizontalLayout->addWidget(runButton);

        clearTopButton = new QPushButton(frame);
        clearTopButton->setObjectName(QString::fromUtf8("clearTopButton"));
        clearTopButton->setMaximumSize(QSize(75, 16777215));

        horizontalLayout->addWidget(clearTopButton);


        verticalLayout->addWidget(frame);

        tabWidget = new QTabWidget(CommandDialog);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        sbmTab = new QWidget();
        sbmTab->setObjectName(QString::fromUtf8("sbmTab"));
        sbmTextEdit = new QPlainTextEdit(sbmTab);
        sbmTextEdit->setObjectName(QString::fromUtf8("sbmTextEdit"));
        sbmTextEdit->setGeometry(QRect(0, 0, 731, 241));
        tabWidget->addTab(sbmTab, QString());
        pythonTab = new QWidget();
        pythonTab->setObjectName(QString::fromUtf8("pythonTab"));
        pythonTextEdit = new QPlainTextEdit(pythonTab);
        pythonTextEdit->setObjectName(QString::fromUtf8("pythonTextEdit"));
        pythonTextEdit->setGeometry(QRect(0, 0, 731, 211));
        tabWidget->addTab(pythonTab, QString());

        verticalLayout->addWidget(tabWidget);


        retranslateUi(CommandDialog);

        tabWidget->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(CommandDialog);
    } // setupUi

    void retranslateUi(QDialog *CommandDialog)
    {
        CommandDialog->setWindowTitle(QApplication::translate("CommandDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        runButton->setText(QApplication::translate("CommandDialog", "Run", 0, QApplication::UnicodeUTF8));
        clearTopButton->setText(QApplication::translate("CommandDialog", "Clear", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(sbmTab), QApplication::translate("CommandDialog", "Sbm", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(pythonTab), QApplication::translate("CommandDialog", "Python", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class CommandDialog: public Ui_CommandDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMMANDDIALOG_H
