/********************************************************************************
** Form generated from reading UI file 'SbmDebuggerForm.ui'
**
** Created: Tue Apr 17 17:23:24 2012
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
#include <QtGui/QDockWidget>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
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
    QAction *actionToggleFreeLookCamera;
    QAction *actionSettings;
    QAction *actionDisconnect;
    QAction *actionExit;
    QAction *actionResource_Viewer;
    QAction *actionCommand_Window;
    QAction *actionData_Viewer;
    QAction *actionUtils_Window;
    QAction *actionFace_Viewer;
    QAction *actionBML_Creator;
    QAction *actionEnity_Creator;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_3;
    QWidget *RenderView;
    QHBoxLayout *horizontalLayout_2;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuSbm;
    QStatusBar *statusbar;
    QToolBar *toolBar;
    QDockWidget *sceneDockWidget;
    QWidget *dockWidgetContents;
    QHBoxLayout *horizontalLayout;
    QTabWidget *sceneTab;
    QWidget *tab;
    QHBoxLayout *horizontalLayout_4;
    QTreeWidget *sceneTree;
    QWidget *statisticsTab;
    QLabel *cameraPositionLabel;
    QLabel *networkFpsLabel;
    QLabel *rendererFpsLabel;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1274, 830);
        MainWindow->setBaseSize(QSize(1274, 830));
        actionConnect = new QAction(MainWindow);
        actionConnect->setObjectName(QString::fromUtf8("actionConnect"));
        actionToggleFreeLookCamera = new QAction(MainWindow);
        actionToggleFreeLookCamera->setObjectName(QString::fromUtf8("actionToggleFreeLookCamera"));
        QIcon icon;
        icon.addFile(QString::fromUtf8("images/monkey_on_128x128.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionToggleFreeLookCamera->setIcon(icon);
        actionSettings = new QAction(MainWindow);
        actionSettings->setObjectName(QString::fromUtf8("actionSettings"));
        actionDisconnect = new QAction(MainWindow);
        actionDisconnect->setObjectName(QString::fromUtf8("actionDisconnect"));
        actionDisconnect->setEnabled(false);
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionResource_Viewer = new QAction(MainWindow);
        actionResource_Viewer->setObjectName(QString::fromUtf8("actionResource_Viewer"));
        actionCommand_Window = new QAction(MainWindow);
        actionCommand_Window->setObjectName(QString::fromUtf8("actionCommand_Window"));
        actionData_Viewer = new QAction(MainWindow);
        actionData_Viewer->setObjectName(QString::fromUtf8("actionData_Viewer"));
        actionUtils_Window = new QAction(MainWindow);
        actionUtils_Window->setObjectName(QString::fromUtf8("actionUtils_Window"));
        actionFace_Viewer = new QAction(MainWindow);
        actionFace_Viewer->setObjectName(QString::fromUtf8("actionFace_Viewer"));
        actionBML_Creator = new QAction(MainWindow);
        actionBML_Creator->setObjectName(QString::fromUtf8("actionBML_Creator"));
        actionEnity_Creator = new QAction(MainWindow);
        actionEnity_Creator->setObjectName(QString::fromUtf8("actionEnity_Creator"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout_3 = new QHBoxLayout(centralwidget);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        RenderView = new QWidget(centralwidget);
        RenderView->setObjectName(QString::fromUtf8("RenderView"));
        RenderView->setMinimumSize(QSize(500, 500));
        RenderView->setMaximumSize(QSize(16777215, 16777215));
        horizontalLayout_2 = new QHBoxLayout(RenderView);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));

        horizontalLayout_3->addWidget(RenderView);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1274, 21));
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
        sceneDockWidget = new QDockWidget(MainWindow);
        sceneDockWidget->setObjectName(QString::fromUtf8("sceneDockWidget"));
        sceneDockWidget->setMaximumSize(QSize(524287, 524287));
        sceneDockWidget->setLayoutDirection(Qt::LeftToRight);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        horizontalLayout = new QHBoxLayout(dockWidgetContents);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        sceneTab = new QTabWidget(dockWidgetContents);
        sceneTab->setObjectName(QString::fromUtf8("sceneTab"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(sceneTab->sizePolicy().hasHeightForWidth());
        sceneTab->setSizePolicy(sizePolicy);
        sceneTab->setMinimumSize(QSize(0, 0));
        sceneTab->setMaximumSize(QSize(16777215, 16777215));
        sceneTab->setLayoutDirection(Qt::LeftToRight);
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        horizontalLayout_4 = new QHBoxLayout(tab);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        sceneTree = new QTreeWidget(tab);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(2, QString::fromUtf8("3"));
        __qtreewidgetitem->setText(1, QString::fromUtf8("2"));
        __qtreewidgetitem->setText(0, QString::fromUtf8("1"));
        sceneTree->setHeaderItem(__qtreewidgetitem);
        sceneTree->setObjectName(QString::fromUtf8("sceneTree"));
        sizePolicy.setHeightForWidth(sceneTree->sizePolicy().hasHeightForWidth());
        sceneTree->setSizePolicy(sizePolicy);
        sceneTree->setMinimumSize(QSize(0, 0));
        sceneTree->setMaximumSize(QSize(16777215, 16777215));
        sceneTree->setLayoutDirection(Qt::LeftToRight);
        sceneTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        sceneTree->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::EditKeyPressed);
        sceneTree->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        sceneTree->setRootIsDecorated(true);
        sceneTree->setColumnCount(3);
        sceneTree->header()->setVisible(true);
        sceneTree->header()->setCascadingSectionResizes(false);
        sceneTree->header()->setDefaultSectionSize(300);

        horizontalLayout_4->addWidget(sceneTree);

        sceneTab->addTab(tab, QString());
        statisticsTab = new QWidget();
        statisticsTab->setObjectName(QString::fromUtf8("statisticsTab"));
        cameraPositionLabel = new QLabel(statisticsTab);
        cameraPositionLabel->setObjectName(QString::fromUtf8("cameraPositionLabel"));
        cameraPositionLabel->setGeometry(QRect(10, 50, 211, 31));
        networkFpsLabel = new QLabel(statisticsTab);
        networkFpsLabel->setObjectName(QString::fromUtf8("networkFpsLabel"));
        networkFpsLabel->setGeometry(QRect(10, 30, 121, 16));
        rendererFpsLabel = new QLabel(statisticsTab);
        rendererFpsLabel->setObjectName(QString::fromUtf8("rendererFpsLabel"));
        rendererFpsLabel->setEnabled(true);
        rendererFpsLabel->setGeometry(QRect(10, 10, 121, 16));
        sceneTab->addTab(statisticsTab, QString());

        horizontalLayout->addWidget(sceneTab);

        sceneDockWidget->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), sceneDockWidget);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuView->menuAction());
        menubar->addAction(menuSbm->menuAction());
        menuFile->addAction(actionConnect);
        menuFile->addAction(actionDisconnect);
        menuFile->addAction(actionSettings);
        menuFile->addAction(actionExit);
        menuSbm->addAction(actionData_Viewer);
        menuSbm->addAction(actionResource_Viewer);
        menuSbm->addAction(actionCommand_Window);
        menuSbm->addAction(actionUtils_Window);
        menuSbm->addAction(actionFace_Viewer);
        menuSbm->addAction(actionBML_Creator);
        menuSbm->addAction(actionEnity_Creator);
        toolBar->addAction(actionToggleFreeLookCamera);

        retranslateUi(MainWindow);

        sceneTab->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Smartbody Debugger", 0, QApplication::UnicodeUTF8));
        actionConnect->setText(QApplication::translate("MainWindow", "&Connect", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionConnect->setToolTip(QApplication::translate("MainWindow", "Connect to a Smartbody Process", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionConnect->setShortcut(QApplication::translate("MainWindow", "Ctrl+C", 0, QApplication::UnicodeUTF8));
        actionToggleFreeLookCamera->setText(QApplication::translate("MainWindow", "Toggle Free Look", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionToggleFreeLookCamera->setToolTip(QApplication::translate("MainWindow", "Toggle Free Look", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionSettings->setText(QApplication::translate("MainWindow", "&Settings", 0, QApplication::UnicodeUTF8));
        actionSettings->setShortcut(QApplication::translate("MainWindow", "Ctrl+S", 0, QApplication::UnicodeUTF8));
        actionDisconnect->setText(QApplication::translate("MainWindow", "&Disconnect", 0, QApplication::UnicodeUTF8));
        actionDisconnect->setShortcut(QApplication::translate("MainWindow", "Ctrl+D", 0, QApplication::UnicodeUTF8));
        actionExit->setText(QApplication::translate("MainWindow", "E&xit", 0, QApplication::UnicodeUTF8));
        actionExit->setShortcut(QApplication::translate("MainWindow", "Ctrl+Q", 0, QApplication::UnicodeUTF8));
        actionResource_Viewer->setText(QApplication::translate("MainWindow", "&Resource Viewer", 0, QApplication::UnicodeUTF8));
        actionResource_Viewer->setShortcut(QApplication::translate("MainWindow", "Ctrl+R", 0, QApplication::UnicodeUTF8));
        actionCommand_Window->setText(QApplication::translate("MainWindow", "Command Window", 0, QApplication::UnicodeUTF8));
        actionCommand_Window->setShortcut(QApplication::translate("MainWindow", "Ctrl+M", 0, QApplication::UnicodeUTF8));
        actionData_Viewer->setText(QApplication::translate("MainWindow", "D&ata Viewer", 0, QApplication::UnicodeUTF8));
        actionData_Viewer->setShortcut(QApplication::translate("MainWindow", "Ctrl+A", 0, QApplication::UnicodeUTF8));
        actionUtils_Window->setText(QApplication::translate("MainWindow", "U&tils Window", 0, QApplication::UnicodeUTF8));
        actionUtils_Window->setShortcut(QApplication::translate("MainWindow", "Ctrl+T", 0, QApplication::UnicodeUTF8));
        actionFace_Viewer->setText(QApplication::translate("MainWindow", "Face Viewer", 0, QApplication::UnicodeUTF8));
        actionFace_Viewer->setShortcut(QApplication::translate("MainWindow", "Ctrl+F", 0, QApplication::UnicodeUTF8));
        actionBML_Creator->setText(QApplication::translate("MainWindow", "&BML Creator", 0, QApplication::UnicodeUTF8));
        actionBML_Creator->setShortcut(QApplication::translate("MainWindow", "Ctrl+B", 0, QApplication::UnicodeUTF8));
        actionEnity_Creator->setText(QApplication::translate("MainWindow", "Entity Creator", 0, QApplication::UnicodeUTF8));
        actionEnity_Creator->setShortcut(QApplication::translate("MainWindow", "Ctrl+E", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
        menuEdit->setTitle(QApplication::translate("MainWindow", "Edit", 0, QApplication::UnicodeUTF8));
        menuView->setTitle(QApplication::translate("MainWindow", "View", 0, QApplication::UnicodeUTF8));
        menuSbm->setTitle(QApplication::translate("MainWindow", "Tools", 0, QApplication::UnicodeUTF8));
        toolBar->setWindowTitle(QApplication::translate("MainWindow", "toolBar", 0, QApplication::UnicodeUTF8));
        sceneTab->setTabText(sceneTab->indexOf(tab), QApplication::translate("MainWindow", "Scene", 0, QApplication::UnicodeUTF8));
        cameraPositionLabel->setText(QApplication::translate("MainWindow", "Camera Pos:", 0, QApplication::UnicodeUTF8));
        networkFpsLabel->setText(QApplication::translate("MainWindow", "Network Fps:", 0, QApplication::UnicodeUTF8));
        rendererFpsLabel->setText(QApplication::translate("MainWindow", "Renderer Fps:", 0, QApplication::UnicodeUTF8));
        sceneTab->setTabText(sceneTab->indexOf(statisticsTab), QApplication::translate("MainWindow", "Statistics", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SBMDEBUGGERFORM_H
