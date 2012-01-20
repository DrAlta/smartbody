#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "QtCrtDbgOff.h"
#include <QGLWidget>
#include <QtGui>
#include <QtOpenGL>
#include "QtCrtDbgOn.h"

#include <gl\GLU.h>

#include "Camera.h"
#include "SbmDebuggerCommon.h"
#include "SettingsDialog.h"
#include "vhcl.h"

using std::string;


class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(Scene* scene, QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;
    virtual void Update();

    void SetScene(Scene* scene) { m_pScene = scene; }
    double GetFps();
    string GetCameraPositionAsString();
    string GetFpsAsString();

public slots:
    void OnCloseSettingsDialog(const SettingsDialog* dlg, int result);

Q_SIGNALS:
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *key);
    
private:

    QPoint lastPos;
    QColor qtGreen;
    QColor qtPurple;
    Camera m_Camera;
    float m_fPawnSize;
    float m_fJointRadius;
    Scene* m_pScene;
    GLUquadric* m_quadric;

    double m_msSinceLastFrame;
    double m_msSinceLastFramePrev;

    QBasicTimer timer;
    vhcl::Timer m_StopWatch;

    virtual void timerEvent(QTimerEvent * event);

    void DrawFloor();
    void DrawScene();
    void DrawCharacter(const Character* character);
    void DrawJoint(Joint* joint);
    void DrawPawn(const Pawn* pawn);
    void DrawCylinder(const float baseRadius, const float topRadius, const float height, const int slices, const int stacks);
    void DrawSphere(double radius, int slices = 10, int stacks = 10);
};


#endif
