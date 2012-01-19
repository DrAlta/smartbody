/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <math.h>

#include "glwidget.h"
#include "vhcl.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif


using std::string;
using std::vector;


//! [0]
GLWidget::GLWidget(Scene* scene, QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    m_quadric = gluNewQuadric();
    SetScene(scene);
    xRot = 0;
    yRot = 0;
    zRot = 0;
    
    m_fPawnSize = 1.0f;
    m_fJointRadius = 1.25f;

    qtGreen = QColor::fromCmykF(0.40, 0.0, 1.0, 0.0);
    qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);

    setFocusPolicy(Qt::StrongFocus);

    timer.start(100, this);
}

GLWidget::~GLWidget()
{
}

QSize GLWidget::minimumSizeHint() const
{
   return size();
   //return QSize(400, 400);
}

QSize GLWidget::sizeHint() const
{
   QSize s = size();
   return QSize(s.width(), s.height());
}

static void qNormalizeAngle(float &angle)
{
    while (angle < 0)
        angle += 360 /** 16*/;
    while (angle > 360 * 16)
        angle -= 360 /** 16*/;
}

void GLWidget::setXRotation(float angle)
{
    qNormalizeAngle(angle);
    if (angle != xRot) 
    {
        m_Camera.Rotate(QVector3D(angle, 0, 0));
        emit xRotationChanged(angle);
        updateGL();
    }
}
//! [5]

void GLWidget::setYRotation(float angle)
{
    qNormalizeAngle(angle);
    if (angle != yRot) 
    {
        m_Camera.Rotate(QVector3D(0, angle, 0));
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(float angle)
{
    qNormalizeAngle(angle);
    if (angle != zRot)
    {
        m_Camera.Rotate(QVector3D(0, 0, angle));
        emit zRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::OnCloseSettingsDialog(const SettingsDialog* dlg, int result)
{
   if (result == QDialog::Accepted)
   {
      m_Camera.SetCameraType(dlg->ui.cameraControlBox->currentText().toStdString());
      m_Camera.SetMovementSpeed(dlg->ui.cameraMovementSpeedBox->value());
      m_Camera.SetRotationSpeed(dlg->ui.cameraRotationSpeedBox->value());
   }
}

void GLWidget::initializeGL()
{
    qglClearColor(qtPurple.dark());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_MULTISAMPLE);
    static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glPushMatrix();
       m_Camera.Update(); 
       DrawFloor();
       DrawScene();
    glPopMatrix();
}

void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    //glViewport((width - side) / 2, (height - side) / 2, side, side);
    glViewport(geometry().x() - 50, geometry().y() - 35, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (double)width / (double)height, 0.1f, 1000);

    glMatrixMode(GL_MODELVIEW);
}

void GLWidget::DrawCylinder(float baseRadius, float topRadius, float height, int slices, int stacks)
{
   gluCylinder(m_quadric, baseRadius, topRadius, height, slices, stacks);
}

void GLWidget::DrawSphere(double radius, int slices, int stacks)
{
   gluSphere(m_quadric, radius, slices, stacks);
}

void GLWidget::DrawFloor()
{
   glPushMatrix();
   glTranslated(0, 0, 0);
   glRotated(180, 1, 0, 0);
   glBegin(GL_QUADS);  
      glVertex3f(-1, 0, 1);
      glNormal3f(0, 1, 0);
      
	   glVertex3f(-1, 0, -1);
      glNormal3f(0, 1, 0);
      
	   glVertex3f(1, 0, -1);
      glNormal3f(0, 1, 0);
      
	   glVertex3f(1, 0, 1);
      glNormal3f(0, 1, 0);
   glEnd();
   glPopMatrix();
}

void GLWidget::DrawScene()
{
   // draw characters
   vector<Character> characters = m_pScene->m_characters;
   for (unsigned int i = 0; i < characters.size(); i++)
   {
      glPushMatrix();
      DrawCharacter(&characters[i]);
      glPopMatrix();
   }
   
   // draw pawns
   vector<Pawn> pawns = m_pScene->m_pawns;
   for (unsigned int i = 0; i < pawns.size(); i++)
   {
      glPushMatrix();
      DrawPawn(&pawns[i]);
      glPopMatrix();
   }
}

//Vector3 GetLocalRotation(Joint* joint)
QMatrix4x4 GetLocalRotation(Joint* joint)
{
   QQuaternion worldRotation = QQuaternion(joint->rotOrig.w, joint->rotOrig.x, joint->rotOrig.y, joint->rotOrig.z) 
      * QQuaternion(joint->rot.w, joint->rot.x, joint->rot.y, joint->rot.z);

   QMatrix4x4 mat;
   mat.setToIdentity();
   mat.rotate(worldRotation);
   //mat = mat.transposed();
   //mat.rotate(180, QVector3D(0, 1, 0));
   return mat;
   /*Vector4 convertedRotation(worldRotation.x(), worldRotation.y(), worldRotation.z(), worldRotation.scalar());
   return Vector3::ConvertFromQuat(convertedRotation);*/
}

void GLWidget::DrawCharacter(const Character* character)
{
   glTranslated(character->pos.x, character->pos.y, character->pos.z);

   for (unsigned int i = 0; i < character->m_joints.size(); i++)
   {
      DrawJoint(character->m_joints[i]);
   }
}

void GLWidget::DrawJoint(Joint* joint)
{
   glPushMatrix();
   Vector3 jointPos = (joint->posOrig + joint->pos);

   //if (joint->m_name == "l_shoulder")
   //{
   //   int x = 10;
   //}
  
   glTranslated(jointPos.x, jointPos.y, jointPos.z);
   QMatrix4x4 rotationMat = GetLocalRotation(joint);
   glMultMatrixd(rotationMat.data());  
   
   DrawSphere(m_fJointRadius); 

   // draw a connecting bone between the 2 joints
   if (joint->m_parent)
   {
      glPushMatrix();
      /*Vector3 parentJointPos = (joint->m_parent->posOrig + joint->m_parent->pos);
      double jointLength = (jointPos - parentJointPos).Magnitude();
      // draw the bone
      DrawCylinder(0.01f, 0.01f, jointLength, 10, 10);*/

      rotationMat = rotationMat.transposed();
      glMultMatrixd(rotationMat.data());
      glBegin(GL_LINES);
         glVertex3f(0, 0, 0);
         glVertex3f(-jointPos.x, -jointPos.y, -jointPos.z);  
      glEnd();
      glPopMatrix();
   }   

   for (unsigned int i = 0; i < joint->m_joints.size(); i++)
   {   
      DrawJoint(joint->m_joints[i]);
   }
   glPopMatrix();
}

void GLWidget::DrawPawn(const Pawn* pawn)
{
   glTranslated(pawn->pos.x, pawn->pos.y, pawn->pos.z);
   //glRotatef(); // TODO: Perform rotation, convert quat to euler angles
   DrawSphere(m_fPawnSize);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
   lastPos.setX(0); lastPos.setY(0);
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
   if (event->delta() > 0)
   {
      m_Camera.MoveZ(-m_Camera.GetMovementSpeed());
   }
   else
   { 
      m_Camera.MoveZ(m_Camera.GetMovementSpeed());
   }
   updateGL();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) 
    {
        //setXRotation(xRot + GetCameraRotationSpeed() * dy);
        setYRotation(yRot + m_Camera.GetRotationSpeed() * dx);
    } 
    //else if (event->buttons() & Qt::RightButton) 
    //{
    //    setXRotation(xRot /*+ 8*/ * dy);
    //    setZRotation(zRot /*+ 8*/ * dx);
    //}
    lastPos = event->pos();
}

void GLWidget::keyPressEvent(QKeyEvent *key)
{
   if (key->key() == Qt::Key_A // left
      || key->key() == Qt::Key_Left) 
   {
      m_Camera.MoveX(-m_Camera.GetMovementSpeed());
   }
   else if (key->key() == Qt::Key_D // right
      || key->key() == Qt::Key_Right) 
   {
      m_Camera.MoveX(m_Camera.GetMovementSpeed());
   }
   else if (key->key() == Qt::Key_W // forward
      || key->key() == Qt::Key_Up)  
   {
      m_Camera.MoveZ(-m_Camera.GetMovementSpeed());
   }
   else if (key->key() == Qt::Key_S // back
      || key->key() == Qt::Key_Down) 
   {
      m_Camera.MoveZ(m_Camera.GetMovementSpeed());
   }
   else if (key->key() == Qt::Key_Q) // up
   {
      m_Camera.MoveY(m_Camera.GetMovementSpeed());
   }
   else if (key->key() == Qt::Key_E) // down
   {
      m_Camera.MoveY(-m_Camera.GetMovementSpeed());
   }

   updateGL();
}

void GLWidget::timerEvent(QTimerEvent * event)
{
   if (event->timerId() == timer.timerId())
   {
      Update();
   }
   else
   {
      QWidget::timerEvent(event);
   }
}

void GLWidget::Update()
{
   if (m_Camera.FollowRenderer())
   {
      // TODO: this needs to be refactored to remove conversions between coordinate systems
      // and conversion from different types of vectors
      DebuggerCamera cam = m_pScene->m_camera;
      gluPerspective(cam.fovY, cam.aspect, cam.zNear, cam.zFar);
      m_Camera.SetPosition(QVector3D(cam.pos.x, cam.pos.y, cam.pos.z));
      Vector3 eulerAngles = Vector3::ConvertFromQuat(cam.rot.x, cam.rot.y, cam.rot.z, cam.rot.w);
      m_Camera.SetRotation(QVector3D(eulerAngles.x, eulerAngles.y, eulerAngles.z));
   }
   
   updateGL();
}

string GLWidget::GetCameraPositionAsString()
{
  QVector3D pos = m_Camera.GetPosition();
  return vhcl::Format("x: %.2f    y: %.2f    z: %.2f", pos.x(), pos.y(), pos.z());
}