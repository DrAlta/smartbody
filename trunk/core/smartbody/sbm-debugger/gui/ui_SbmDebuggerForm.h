/********************************************************************************
** Form generated from reading UI file 'SbmDebuggerForm.ui'
**
** Created: Thu Jan 19 10:57:59 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SBMDEBUGGERFORM_H
#define UI_SBMDEBUGGERFORM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QToolBar>
#include <QtGui/QTreeWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionConnect;
    QAction *actionDebug;
    QAction *actionSettings;
    QAction *actionDisconnect;
    QAction *actionExit;
    QAction *actionResource_Viewer;
    QWidget *centralwidget;
    QWidget *RenderView;
    QTabWidget *sceneTab;
    QWidget *tab;
    QTreeWidget *sceneTree;
    QWidget *tab_2;
    QWidget *tab_3;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuSbm;
    QStatusBar *statusbar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1236, 817);
        actionConnect = new QAction(MainWindow);
        actionConnect->setObjectName(QString::fromUtf8("actionConnect"));
        actionDebug = new QAction(MainWindow);
        actionDebug->setObjectName(QString::fromUtf8("actionDebug"));
        QIcon icon;
        icon.addFile(QString::fromUtf8("C:/Users/reilly/.designer/images/monkey_on_128x128.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDebug->setIcon(icon);
        actionSettings = new QAction(MainWindow);
        actionSettings->setObjectName(QString::fromUtf8("actionSettings"));
        actionDisconnect = new QAction(MainWindow);
        actionDisconnect->setObjectName(QString::fromUtf8("actionDisconnect"));
        actionDisconnect->setEnabled(false);
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionResource_Viewer = new QAction(MainWindow);
        actionResource_Viewer->setObjectName(QString::fromUtf8("actionResource_Viewer"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        RenderView = new QWidget(centralwidget);
        RenderView->setObjectName(QString::fromUtf8("RenderView"));
        RenderView->setGeometry(QRect(10, 10, 1001, 761));
        sceneTab = new QTabWidget(centralwidget);
        sceneTab->setObjectName(QString::fromUtf8("sceneTab"));
        sceneTab->setGeometry(QRect(1020, 10, 181, 771));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        sceneTree = new QTreeWidget(tab);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        sceneTree->setHeaderItem(__qtreewidgetitem);
        sceneTree->setObjectName(QString::fromUtf8("sceneTree"));
        sceneTree->setGeometry(QRect(0, 0, 181, 741));
        sceneTree->setRootIsDecorated(true);
        sceneTree->setColumnCount(1);
        sceneTree->header()->setVisible(true);
        sceneTab->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        sceneTab->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        sceneTab->addTab(tab_3, QString());
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1236, 21));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuView = new QMenu(menubar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        menuSbm = new QMenu(menubar);
        menuSbm->setObjectName(QString::fromUtf8("menuSbm"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);
        toolBar = new QToolBar(MainWindow);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        MainWindow->addToolBar(Qt::LeftToolBarArea, toolBar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuView->menuAction());
        menubar->addAction(menuSbm->menuAction());
        menuFile->addAction(actionConnect);
        menuFile->addAction(actionDisconnect);
        menuFile->addAction(actionSettings);
        menuFile->addAction(actionExit);
        menuSbm->addAction(actionResource_Viewer);
        toolBar->addAction(actionDebug);

        retranslateUi(MainWindow);

        sceneTab->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        actionConnect->setText(QApplication::translate("MainWindow", "&Connect", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionConnect->setToolTip(QApplication::translate("MainWindow", "Connect to a Smartbody Process", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionConnect->setShortcut(QApplication::translate("MainWindow", "Ctrl+C", 0, QApplication::UnicodeUTF8));
        actionDebug->setText(QApplication::translate("MainWindow", "&Debug", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionDebug->setToolTip(QApplication::translate("MainWindow", "Simple Test", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionSettings->setText(QApplication::translate("MainWindow", "&Settings", 0, QApplication::UnicodeUTF8));
        actionSettings->setShortcut(QApplication::translate("MainWindow", "Ctrl+S", 0, QApplication::UnicodeUTF8));
        actionDisconnect->setText(QApplication::translate("MainWindow", "&Disconnect", 0, QApplication::UnicodeUTF8));
        actionDisconnect->setShortcut(QApplication::translate("MainWindow", "Ctrl+D", 0, QApplication::UnicodeUTF8));
        actionExit->setText(QApplication::translate("MainWindow", "E&xit", 0, QApplication::UnicodeUTF8));
        actionExit->setShortcut(QApplication::translate("MainWindow", "Ctrl+Q", 0, QApplication::UnicodeUTF8));
        actionResource_Viewer->setText(QApplication::translate("MainWindow", "&Resource Viewer", 0, QApplication::UnicodeUTF8));
        actionResource_Viewer->setShortcut(QApplication::translate("MainWindow", "Ctrl+R", 0, QApplication::UnicodeUTF8));
        sceneTab->setTabText(sceneTab->indexOf(tab), QApplication::translate("MainWindow", "Scene", 0, QApplication::UnicodeUTF8));
        sceneTab->setTabText(sceneTab->indexOf(tab_2), QApplication::translate("MainWindow", "Tab 2", 0, QApplication::UnicodeUTF8));
        sceneTab->setTabText(sceneTab->indexOf(tab_3), QApplication::translate("MainWindow", "Tab 3", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
        menuEdit->setTitle(QApplication::translate("MainWindow", "Edit", 0, QApplication::UnicodeUTF8));
        menuView->setTitle(QApplication::translate("MainWindow", "View", 0, QApplication::UnicodeUTF8));
        menuSbm->setTitle(QApplication::translate("MainWindow", "Sbm", 0, QApplication::UnicodeUTF8));
        toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SBMDEBUGGERFORM_H
