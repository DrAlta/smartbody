#include "Camera.h"
#include <QtOpenGL>

QVector3D WorldUp(0, 1, 0);

Camera::Camera() :
   m_Position(0, 0.2f, 3),
   m_Rotation(0, 0, 0),
   m_Scale(1, 1, 1),
   m_CameraType(Follow_Renderer),
   m_MovementSpeed(0.05f),
   m_RotationSpeed(0.05f)
{
   m_RotMatrix.setToIdentity();
   m_RotMatrix.translate(0, 0.2f, 3);
}

Camera::~Camera()
{
  
}

void Camera::Draw()
{
   glScaled(m_Scale.x(), m_Scale.y(), m_Scale.z());
   QMatrix4x4 mat = m_RotMatrix.inverted();
   glMultMatrixd(mat.data());
}

void Camera::Offset(const QVector3D& offset)
{
   m_Position += offset;
}

void Camera::Rotate(const QVector3D& offset)
{
   if (offset.x() != 0)
   {
      m_RotMatrix.rotate(offset.x(), QVector3D(1, 0, 0));
   }
   
   if (offset.y() != 0)
   {
      m_RotMatrix.rotate(offset.y(), QVector3D(0, 1, 0));
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
   m_RotMatrix.setColumn(2, forward * -m_Scale.z());

   // set right vector
   QVector3D right = QVector3D::crossProduct(forward, QVector3D(0, 1, 0));
   m_RotMatrix.setColumn(0, right);

   QVector3D up = QVector3D::crossProduct(right, forward);
   m_RotMatrix.setColumn(1, up);
}