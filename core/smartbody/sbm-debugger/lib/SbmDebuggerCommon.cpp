
#include "vhcl.h"

#include "SbmDebuggerCommon.h"

#include <string>
#include <vector>

#include <stdio.h>
#include <conio.h>

#include "vhmsg-tt.h"

using std::string;
using std::vector;

#define LERP(a, b, t) (a + (b - a) * t)
//#define TOLERANCE 0.00001f

//3.14159265358979323846


Joint::Joint()
{
   posOrig.x = 0;
   posOrig.y = 0;
   posOrig.z = 0;
   rotOrig.x = 0;
   rotOrig.y = 0;
   rotOrig.z = 0;
   rotOrig.w = 0;

   pos.x = 0;
   pos.y = 0;
   pos.z = 0;
   rot.x = 0;
   rot.y = 0;
   rot.z = 0;
   rot.w = 0;

   m_parent = NULL;
}


Joint::~Joint()
{
}

Vector4::Vector4(double _x, double _y, double _z, double _w)
{
   x = _x;
   y = _y;
   z = _z;
   w = _w;
}

void Vector4::ToAxisAngle(Vector3& axis, float& angle)
{
   float scale = (float)sqrt(x * x + y * y + z * z);
   if (scale != 0)
   {
      axis.x = x / scale;
	   axis.y = y / scale;
	   axis.z = z / scale;
   }
	
	angle = ((float)acos(w) * 2.0f) * RAD_TO_DEG;
}


Vector3 Joint::GetWorldPosition() const
{
   Vector3 worldPosition = posOrig + pos;
   Joint* parent = m_parent;
   while (parent)
   {
      worldPosition += (parent->posOrig + parent->pos);
      parent = parent->m_parent;
   }

   return worldPosition;
}

Vector3 Joint::GetLocalPosition()  const
{
   return posOrig + pos;
}

Vector4 Joint::GetWorldRotation()  const
{
   Vector4 worldRotation = rotOrig * rot;
   Joint* parent = m_parent;
   while (parent)
   {
      worldRotation *= (parent->rotOrig * parent->rot);
      parent = parent->m_parent;
   }

   return worldRotation;
}

Vector4 Joint::GetLocalRotation()  const
{
   return rotOrig * rot;
}

std::string Joint::GetPositionAsString(bool worldPos)  const
{
   Vector3 pos = worldPos ? GetWorldPosition() : GetLocalPosition();
   return vhcl::Format("x: %.2f y: %.2f z: %.2f", pos.x, pos.y, pos.z);
}

std::string Joint::GetRotationAsString(bool worldRot)  const
{
   Vector4 rot = worldRot ? GetWorldRotation() : GetLocalRotation();
   return vhcl::Format("w: %.2f x: %.2f y: %.2f z: %.2f", rot.w, rot.x, rot.y, rot.z);
}

Vector3 Pawn::GetWorldPosition() const
{
   Joint* joint = FindJoint("world_offset");
   return joint ? joint->pos : Vector3();
}

Joint* Pawn::GetWorldOffset() const
{
   return FindJoint("world_offset");
}

Joint * Pawn::FindJoint(const string & name, const vector<Joint *> & joints)
{
   for (size_t i = 0; i < joints.size(); i++)
   {
      Joint * j = joints[i];
      if (j->m_name == name)
      {
         return j;
      }
   }

   for (size_t i = 0; i < joints.size(); i++)
   {
      Joint * j = joints[i];
      Joint * ret = FindJoint(name, j->m_joints);
      if (ret)
      {
         return ret;
      }
   }

   return NULL;
}


Character* Scene::FindCharacter(const std::string & name)
{
   for (unsigned int i = 0; i < m_characters.size(); i++)
   {
      if (name == m_characters[i].m_name)
      {
         return &m_characters[i];
      }
   }

   return NULL;
}

Pawn* Scene::FindPawn(const std::string & name)
{
   for (unsigned int i = 0; i < m_pawns.size(); i++)
   {
      if (name == m_pawns[i].m_name)
      {
         return &m_pawns[i];
      }
   }

   return NULL;
}

Pawn* Scene::FindSbmObject(const std::string & name)
{
   Pawn* object = FindCharacter(name);
   return object ? object : FindPawn(name);
}