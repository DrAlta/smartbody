#ifndef GL_GRAPH_WIDGET_H_
#define GL_GRAPH_WIDGET_H_

#include "QtCrtDbgOff.h"
#include <QGLWidget>
#include <QtGui>
#include <QtOpenGL>
#include "QtCrtDbgOn.h"

#include <gl\GLU.h>

#include "SbmDebuggerCommon.h"

#include "vhcl_public.h"

#include <list>
using std::list;
using vhcl::Vector3;

class GLGraphWidget : QGLWidget
{
public:
   GLGraphWidget(const QRect& renderSize, Scene* scene, QWidget* parent = 0);
   ~GLGraphWidget();

   void AddLineGraphPoint(Vector3& position, Vector3& color);
   void AddLineGraphPoint(Vector3& position, Vector3& color, unsigned int maxSize);

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

   struct LineGraphPoint
   {
      Vector3 position;
      Vector3 color;
      LineGraphPoint(Vector3 _position, Vector3 _color)
      {
         position = _position;
         color = _color;
      }
   };
   list<LineGraphPoint*> m_LineGraphPoints;
};

#endif