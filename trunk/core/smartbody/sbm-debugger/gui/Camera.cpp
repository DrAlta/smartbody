#include "Camera.h"
#include <QtOpenGL>

QVector3D WorldRight(1, 0, 0);
QVector3D WorldUp(0, 1, 0);
QVector3D WorldForward(0, 0, 1);

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
   m_RotationSpeed = 0.05f;
}

Camera::~Camera()
{
  
}

void Camera::Update()
{
   //glRotatef(-m_Rotation.x(), 1, 0, 0);
   //glRotatef(-m_Rotation.y(), 0, 1, 0);
   QMatrix4x4 mat = m_RotMatrix.inverted();
   glMultMatrixd(mat.data());
   //QVector3D pos(m_RotMatrix.column(3));
   //glTranslated(-pos.x(), -pos.y(), -pos.z());
}

void Camera::Offset(const QVector3D& offset)
{
   m_Position += offset;
}

void Camera::Rotate(const QVector3D& offset)
{
   if (offset.x() != 0)
   {
      m_RotMatrix.rotate(offset.x(), QVector3D(1, 0, 0)/*QVector3D(m_RotMatrix.column(0))*/);
   }
   
   if (offset.y() != 0)
   {
      m_RotMatrix.rotate(offset.y(), QVector3D(0, 1, 0)/*QVector3D(m_RotMatrix.column(1))*/);
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

void Camera::SetRotation(const QQuaternion& rot)
{
   QMatrix4x4 newMat;
   newMat.setToIdentity();
   newMat.translate(GetPosition());
   newMat.rotate(rot);
   m_RotMatrix = newMat;
}

void Camera::LookAt(const QVector3D& pos)
{
   // set the forward vector
   QVector3D forward = (pos - GetPosition()).normalized();
   m_RotMatrix.setColumn(2, forward);

   // set right vector
   QVector3D right = QVector3D::crossProduct(forward, QVector3D(0, 1, 0));
   m_RotMatrix.setColumn(0, right);

   QVector3D up = QVector3D::crossProduct(right, forward);
   m_RotMatrix.setColumn(1, up);
}