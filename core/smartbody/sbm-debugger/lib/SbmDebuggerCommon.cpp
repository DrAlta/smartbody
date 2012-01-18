
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


Vector3 Joint::GetWorldPosition()
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

Vector4 Joint::GetWorldRotation()
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

Joint * Character::FindJoint(const string & name, const vector<Joint *> & joints)
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
