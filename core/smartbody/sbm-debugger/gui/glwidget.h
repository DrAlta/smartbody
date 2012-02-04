#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "QtCrtDbgOff.h"
#include <QGLWidget>
#include <QtGui>
#include <QtOpenGL>
#include "QtCrtDbgOn.h"

#include <gl\GLU.h>

#include "vhcl_timer.h"
#include "Camera.h"
#include "SbmDebuggerCommon.h"
#include "SettingsDialog.h"


using std::string;
using std::vector;

#define SELECT_BUFF_SIZE 1024
#define PICKING_OFFSET 1000

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
    void SetMinimumSize(int w, int h) { m_MinSize.setWidth(w); m_MinSize.setHeight(h); }
    double GetFps();
    string GetCameraPositionAsString();
    string GetFpsAsString();

public slots:
    void OnCloseSettingsDialog(const SettingsDialog* dlg, int result);
    void ToggleFreeLook();
    void sceneTreeCurrentItemChanged ( QTreeWidgetItem * current, QTreeWidgetItem * previous );
    void sceneTreeItemDoubleClicked(QTreeWidgetItem * item, int column);
    void ToggleShowAxes(bool enabled);
    void ToggleShowEyeBeams(bool enabled);
    void ToggleAllowBoneUpdates(bool enabled);

signals:
    void JointPicked(const Pawn* jointOwner, const Joint* jointSelected);

Q_SIGNALS:
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    //void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *key);
    
private:

    enum GLMode
    {
      RENDER,
      SELECT,
    };

    struct SelectionData
    {
         Pawn* m_pObj;
         Joint* m_pJoint;
         SelectionData() { m_pObj = NULL; m_pJoint = NULL; }
    };

    QPoint lastPos;
    QColor qtPurple;
    Camera m_Camera;
    float m_fPawnSize;
    float m_fJointRadius;
    Scene* m_pScene;
    GLUquadric* m_quadric;
    QSize m_MinSize;

    double m_msSinceLastFrame;
    double m_msSinceLastFramePrev;

    QBasicTimer timer;
    vhcl::Timer m_StopWatch;

    GLMode m_GLMode;
    SelectionData m_SelData;
    int m_nPickingOffset;
    GLuint selectBuf[SELECT_BUFF_SIZE];

    virtual void timerEvent(QTimerEvent * event);

    // picking Functions
    void StartPicking();
    Joint* FindPickedJoint(int pickIndex);
    Joint* FindPickedJointRecursive(const Joint* joint);
    void ProcessHits(GLint hits, GLuint buffer[]);
    void StopPicking();
    void SetSelectedObject(Pawn* obj, Joint* joint);

    // Drawing Functions
    void DrawFloor();
    void DrawScene();
    void DrawCharacter(const Character* character);
    void DrawJoint(Joint* joint);
    void DrawPawn(const Pawn* pawn);
    void DrawCylinder(const float baseRadius, const float topRadius, const float height, const int slices, const int stacks);
    void DrawSphere(double radius, int slices = 10, int stacks = 10);
};


#endif
