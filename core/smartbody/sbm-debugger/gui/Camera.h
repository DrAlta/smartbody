#ifndef CAMERA_H_
#define CAMERA_H_

#include <QVector3D>
#include <QMatrix4x4>

using std::string;

class Camera
{
public:
   enum CameraControlType
   {
      Follow_Renderer,
      Free_Look,
      Follow_Character
   };

   Camera();
   ~Camera();

   void Update();
   void Offset(const QVector3D& offset);
   void Rotate(const QVector3D& offset);

   // used for moving the camera along it's orientation vectors
   void MoveX(float offset);
   void MoveY(float offset);
   void MoveZ(float offset);
   void SetCameraType(const string& type);
   QVector3D GetPosition() { return QVector3D(m_RotMatrix.column(3)); }

   void SetPosition(const QVector3D& pos) { m_RotMatrix.setColumn(3, QVector4D(pos, 1.0f)); }
   void SetRotation(const QQuaternion& rot);  
   CameraControlType GetCameraType() { return m_CameraType; }
   bool FollowRenderer() { return m_CameraType == Follow_Renderer; }

   void SetMovementSpeed(double movementSpeed) { m_MovementSpeed = movementSpeed; }
   void SetRotationSpeed(double rotationSpeed) { m_RotationSpeed = rotationSpeed; } 
   double GetMovementSpeed() { return m_MovementSpeed; }
   double GetRotationSpeed() { return m_RotationSpeed; }

   void LookAt(const QVector3D& pos);

private:
   QVector3D m_Position;
   QVector3D m_Rotation;

   // orientation vectors
   QVector3D m_Right;
   QVector3D m_Up;
   QVector3D m_Forward;
   QMatrix4x4 m_RotMatrix;

   CameraControlType m_CameraType;
   double m_MovementSpeed;
   double m_RotationSpeed;

   void UpdateOrientation(const QVector3D& amountToRotate);
};

#endif