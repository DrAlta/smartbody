#ifndef SBM_DEBUGGER_FORM_H_
#define SBM_DEBUGGER_FORM_H_

#include "QtCrtDbgOff.h"
#include <QtGui>
#include <QWidget>
#include "QtCrtDbgOn.h"

#include "ConnectDialog.h"
#include "SettingsDialog.h"
#include "GLWidget.h"
#include "ui_SbmDebuggerForm.h"
#include "ui_ConnectDialog.h"

class SbmDebuggerForm : public QWidget
 {
     Q_OBJECT

 public:
     SbmDebuggerForm(QMainWindow* mainWindow, QWidget *parent = 0);
     ~SbmDebuggerForm();

 private slots:
       void ShowConnectDialog();
       void ShowSettingsDialog();
       void Disconnect();

 private:
     Ui::MainWindow ui;
     QMainWindow* m_pMainWindow;
     GLWidget* m_pGLWidget;

 protected:
    void InitSignalsSlots();
    void Update();

    QBasicTimer timer;
    virtual void timerEvent(QTimerEvent * event);

    static void VHMsgCallback( const char * op, const char * args, void * userData );
 };

#endif
