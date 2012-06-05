#include "GLGraphwidget.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLGraphWidget::GLGraphWidget(const QRect& renderSize, Scene* scene, QWidget* parent) : QGLWidget(parent)
{
   setGeometry(renderSize);

   //qtClearColor = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);
   qtClearColor.setRgb(0, 0, 0);

   timer.start(100, this);
}

GLGraphWidget::~GLGraphWidget()
{
   while (m_LineGraphPoints.size() != 0)
   {
      LineGraphPoint* pPoint = m_LineGraphPoints.front();
      m_LineGraphPoints.pop_front();
      delete pPoint;
   }

   m_LineGraphPoints.clear();
}

void GLGraphWidget::initializeGL()
{
    qglClearColor(qtClearColor.dark());

    //float pos1[4] = {1500.0, 1500.0, 1500.0, 1.0};
	float pos0[4] = {-15000.0f, -12000.0f, 15000.0f, 1.0f};
	float ambient[4] = {0.2f, 0.2f, 0.2f, 1.0f};
	float diffuse[4] = {0.6f, 0.6f, 0.6f, 1.0f};
	float specular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	//float lmodel_ambient[4] = {0.4, 0.4, 0.4, 1.0};

	glLightfv(GL_LIGHT1, GL_POSITION, pos0);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);

	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);       
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT1);
	glEnable(GL_DEPTH_TEST);

   QRect renderSize = geometry();
	//glViewport ( 0, 0, renderSize.width(), renderSize.height() );
   glViewport(geometry().x() - 50, 0, renderSize.width() + 50, renderSize.height());

	/*      glCullFace ( GL_BACK );
	      glDepthFunc ( GL_LEQUAL );
	      glFrontFace ( GL_CCW );
	      glEnable ( GL_POLYGON_SMOOTH );
	      glEnable ( GL_POINT_SMOOTH );
	      glPointSize ( 1.0 );
	      glShadeModel ( GL_SMOOTH );
         glColorMaterial( GL_FRONT_AND_BACK, GL_DIFFUSE );
	      glEnable( GL_COLOR_MATERIAL );
	      glEnable( GL_NORMALIZE );*/

    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT1);
    glEnable(GL_MULTISAMPLE);
    glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable ( GL_COLOR_MATERIAL );
    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
    glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
    

   glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, +0.5, -10.5, +10.5, 0.1, 15.0);
    //glOrtho(renderSize.x(), renderSize.x() + renderSize.width(), -20, 20, 0.1f, 1000);

    glMatrixMode(GL_MODELVIEW);
}

void GLGraphWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glPushMatrix();
      Draw();
    glPopMatrix();
}

void GLGraphWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    QRect renderSize = geometry();
    //glViewport((width - side) / 2, (height - side) / 2, side, side);
    glViewport(geometry().x() - 50, 0, width + 50, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, +0.5, -10.5, +10.5, 0.1, 15.0);
    //glOrtho(renderSize.x(), renderSize.x() + renderSize.width(), -20, 20, 0.1, 1000);

    glMatrixMode(GL_MODELVIEW);
}

void GLGraphWidget::Draw()
{
   const int y_label_num = 10;
   const int x_label_num = 10;
   const float x_size = 800.0f;
	const float y_size = 1.0f;

   glTranslatef(-5.5f, 0, 0);

   //float y_length = 1.0f;//GetYScale()*y_size;
	//float x_length = 1.0f;//GetXScale();
   float y_length = 4;//GetYScale()*y_size;
	float x_length = 11.65f;//GetXScale();
	glColor3f(0.1f, 0.1f, 0.1f);	
	float x;
	glBegin(GL_LINES);
	   for(int i = y_label_num; i > 0; --i)
	   {
		   glVertex3f(0, i*y_length/y_label_num, -10.0f);
		   glVertex3f(x_length, i*y_length/y_label_num, -10.0f);
		   glVertex3f(0, -i*y_length/y_label_num, -10.0f);
		   glVertex3f(x_length, -i*y_length/y_label_num, -10.0f);

	   }
	   for(int i = x_label_num; i > 0; --i)
	   {
		   x = i*x_length/x_label_num;
		   x = x*((int)(i*x_size/x_label_num))/(i*x_size/x_label_num);
		   glVertex3f(x, y_length, -10.0f);
		   glVertex3f(x, -y_length, -10.0f);
	   }
	glEnd();

   glColor3f(0.5f, 0.5f, 0.5f);
	glBegin(GL_LINES);
		glVertex3f(0.0f, 0.0f, -10.0f);
		glVertex3f(x_length, 0.0f, -10.0f);

		glVertex3f(0.0f, y_length, -10.0f);
		glVertex3f(0.0f, -y_length, -10.0f);
	glEnd();

   // render line graph points
   glBegin(GL_LINE_STRIP);
      list<LineGraphPoint*>::iterator it;
      LineGraphPoint* pPoint = NULL;
      for (it = m_LineGraphPoints.begin(); it != m_LineGraphPoints.end(); ++it)
      {
         pPoint = (*it);
         glColor3d(pPoint->color.x, pPoint->color.y, pPoint->color.z);
         glVertex3d(pPoint->position.x, pPoint->position.y, pPoint->position.z);
      }
   glEnd();
}

void GLGraphWidget::timerEvent(QTimerEvent * event)
{
   if (event->timerId() == timer.timerId())
   {
      updateGL();
   }
   else
   {
      QWidget::timerEvent(event);
   }
}

void GLGraphWidget::AddLineGraphPoint(Vector3& position, Vector3& color)
{
   m_LineGraphPoints.push_back(new LineGraphPoint(position, color));
}

void GLGraphWidget::AddLineGraphPoint(Vector3& position, Vector3& color, unsigned int maxSize)
{
   AddLineGraphPoint(position, color);

   while (m_LineGraphPoints.size() > maxSize)
   {
      LineGraphPoint* pPoint = m_LineGraphPoints.front();
      m_LineGraphPoints.pop_front();
      delete pPoint;
   }
}