#include "Camera.h"
#include <QtOpenGL>

Camera::Camera() :
   m_Position(0, 0.2f, 3),
   m_Rotation(0, 0, 0),
   m_Right(1, 0, 0),
   m_Up(0, 1, 0),
   m_Forward(0, 0, 1)
{
   m_RotMatrix.setToIdentity();
   m_RotMatrix.translate(0, 0.2f, 3);
   m_CameraType = Follow_Renderer;
   m_MovementSpeed = 0.05f;
   m_RotationSpeed = 0.01f;
}

Camera::~Camera()
{
  
}

void Camera::Update()
{
   glRotatef(-m_Rotation.x(), 1, 0, 0);
   glRotatef(-m_Rotation.y(), 0, 1, 0);
   QVector3D pos(m_RotMatrix.column(3));
   glTranslated(-pos.x(), -pos.y(), -pos.z());
}

void Camera::Offset(const QVector3D& offset)
{
   m_Position += offset;
}

void Camera::Rotate(const QVector3D& offset)
{
   m_Rotation += offset;

   if (offset.x() != 0)
   {
      m_RotMatrix.rotate(m_Rotation.x(), QVector3D(1, 0, 0));
   }
   
   if (offset.y() != 0)
   {
      m_RotMatrix.rotate(m_Rotation.y(), QVector3D(0, 1, 0));
   }
}

void Camera::MoveX(float offset)
{
   QVector3D right(m_RotMatrix.column(0));
   m_RotMatrix.translate(right * offset);
}

void Camera::MoveY(float offset)
{
   QVector3D up(m_RotMatrix.column(1));
   m_RotMatrix.translate(up * offset);
}

void Camera::MoveZ(float offset)
{
   QVector3D forward(m_RotMatrix.column(2));
   m_RotMatrix.translate(forward * offset);
}

void Camera::SetCameraType(const string& type)
{
   if (type == "Follow Renderer")
      m_CameraType = Follow_Renderer;
   else if (type == "Free Look")
      m_CameraType = Free_Look;
   else
      printf("Camera type %s doesn't exist", type.c_str());
}

void Camera::UpdateOrientation(const QVector3D& amountToRotate)
{
   //QVector3D temp;
   //if (amountToRotate.x != 0)
   //{
   //   temp = m_Right;
   //   temp = cos(amountToRotate.x) * m_Right.x - sin(amountToRotate.x) * m_Right.x
   //   m_Right = temp;
   //}
}