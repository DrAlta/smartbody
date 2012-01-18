
#ifndef SBMDEBUGGERCOMMON_H
#define SBMDEBUGGERCOMMON_H

#include <cmath>
#include <string>
#include <vector>


#define LERP(a, b, t) (a + (b - a) * t)
#define PIOVER180 3.14159265358979323846 / 180.0f
#define RAD_TO_DEG 180.0f / 3.14159265358979323846f

class Vector3;
class Vector4
{
public:
   double x;
   double y;
   double z;
   double w;

public:
   Vector4() {}
   Vector4(double _x, double _y, double _z, double _w);
   virtual ~Vector4() {}

   Vector4 operator*(const Vector4& other)
   {
      Vector4 retval;
      retval.x = x * other.x;
      retval.y = y * other.y;
      retval.z = z * other.z;
      retval.w = z * other.w;
      return retval;
   }

   Vector4 operator*=(const Vector4& other)
   {
      this->x *= other.x;
      this->y *= other.y;
      this->z *= other.z;
      this->w *= other.w;
      return (*this);
   }

   void ToAxisAngle(Vector3& axis, float& angle);
};


class Vector3
{
public:
   double x;
   double y;
   double z;

public:
   Vector3() {}
   virtual ~Vector3() {}

   Vector3 operator+(const Vector3& other)
   {
      Vector3 retval;
      retval.x = x + other.x;
      retval.y = y + other.y;
      retval.z = z + other.z;
      return retval;
   }

   Vector3& operator+=(const Vector3& other)
   {
      this->x += other.x;
      this->y += other.y;
      this->z += other.z;
      return (*this);
   }

   Vector3 operator-(const Vector3& other)
   {
      Vector3 retval;
      retval.x = x - other.x;
      retval.y = y - other.y;
      retval.z = z - other.z;
      return retval;
   }

   Vector3 operator*(float multiplier)
   {
      Vector3 retval;
      retval.x = x * multiplier;
      retval.y = y * multiplier;
      retval.z = z * multiplier;
      return retval;
   }

   double Magnitude()
   {
      return (x*x) + (y*y) + (z*z);
   }

   static Vector3 ConvertFromQuat(double x, double y, double z, double w)
   {
      Vector4 quat;
      quat.x = x; quat.y = y; quat.z = z; quat.w = w;
      return ConvertFromQuat(quat);
   }

   static Vector3 ConvertFromQuat(const Vector4& q1)
   {
      double test = q1.x*q1.y + q1.z*q1.w;

      double sqx = q1.x*q1.x;
      double sqy = q1.y*q1.y;
      double sqz = q1.z*q1.z;

      Vector3 eulerAngles;
      eulerAngles.y = (atan2(2*q1.y*q1.w-2*q1.x*q1.z , 1 - 2*sqy - 2*sqz)) * RAD_TO_DEG;
      eulerAngles.z = (asin(2*test)) * RAD_TO_DEG;
      eulerAngles.x = (atan2(2*q1.x*q1.w-2*q1.y*q1.z , 1 - 2*sqx - 2*sqz)) * RAD_TO_DEG;
      return eulerAngles;
   }
};


class Joint
{
//private:
public:
   std::string m_name;

   Vector3 posOrig;
   Vector4 rotOrig;

   Vector3 pos;
   Vector4 rot;

   std::vector<Joint *> m_joints;
   Joint * m_parent;

public:
   Joint();
   virtual ~Joint();

   Vector3 GetWorldPosition();
   Vector4 GetWorldRotation();
};


class Pawn
{
//private:
public:
   std::string m_name;
   Vector3 pos;
   Vector4 rot;

public:
   Pawn()
   {
      pos.x = 0;
      pos.y = 0;
      pos.z = 0;
      rot.x = 0;
      rot.y = 0;
      rot.z = 0;
      rot.w = 0;
   }

   virtual ~Pawn()
   {
   }
};


class Character : public Pawn
{
public:
   std::vector<Joint *> m_joints;

public:
   Character() : Pawn()
   {
   }

   virtual ~Character()
   {
   }

   Joint * FindJoint(const std::string & name) { return FindJoint(name, m_joints); }

   static Joint * FindJoint(const std::string & name, const std::vector<Joint *> & joints);
};


class DebuggerCamera
{
//private:
public:
   Vector3 pos;
   Vector4 rot;
   double fovY;
   double aspect;
   double zNear;
   double zFar;

public:
   DebuggerCamera()
   {
      pos.x = 0;
      pos.y = 0;
      pos.z = 0;
      rot.x = 0;
      rot.y = 0;
      rot.z = 0;
      rot.w = 1;
      fovY = 45;
      aspect = 1.5;
      zNear = 0.1;
      zFar = 100;
   }

   virtual ~DebuggerCamera() {}
};


class Scene
{
//private:
public:
   std::vector<Pawn> m_pawns;
   std::vector<Character> m_characters;
   DebuggerCamera m_camera;

public:
   Scene()
   {
   }

   virtual ~Scene()
   {
   }
};


#endif  // SBMDEBUGGERCOMMON_H
