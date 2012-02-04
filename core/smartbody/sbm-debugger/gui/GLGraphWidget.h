#ifndef GL_GRAPH_WIDGET_H_
#define GL_GRAPH_WIDGET_H_

#include "QtCrtDbgOff.h"
#include <QGLWidget>
#include <QtGui>
#include <QtOpenGL>
#include "QtCrtDbgOn.h"

#include <gl\GLU.h>

#include "SbmDebuggerCommon.h"

class GLGraphWidget : QGLWidget
{
public:
   GLGraphWidget(const QRect& renderSize, Scene* scene, QWidget* parent = 0);
   ~GLGraphWidget();

Q_SIGNALS:
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void Draw();
    

private:

   void timerEvent(QTimerEvent * event);
   QColor qtClearColor;
   QBasicTimer timer;
};

#endif