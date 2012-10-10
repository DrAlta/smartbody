/*
 *  sr_camera.cpp - part of Motion Engine and SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmarBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Marcelo Kallmann, USC (currently UC Merced)
 */
 
#include <vhcl.h>
# include <math.h>
# include <sr/sr_box.h>
# include <sr/sr_plane.h>
# include <sr/sr_camera.h>
#include <sstream>
#include <sbm/mcontrol_util.h>

//# define SR_USE_TRACE1  // ray
//# include <sr/sr_trace.h>

//=================================== SrCamera ===================================

SrCamera::SrCamera ()
 {
   init ();
 }

SrCamera::SrCamera ( const SrCamera* c )
         :eye(c->eye), center(c->center), up(c->up)
 {
   fovy  = c->fovy;
   znear = c->znear;
   zfar  = c->zfar;
   aspect = c->aspect;
   scale = c->scale;
 }

SrCamera::SrCamera ( const SrPnt& e, const SrPnt& c, const SrVec& u )
         : eye(e), center(c), up(u)
 {
   fovy  = SR_TORAD(60);
   znear = 1.0f; 
   zfar  = 10000.0f; 
   aspect = 1.0f;
 }


void SrCamera::setScale(float s)
{
	scale = s;
}

float SrCamera::getScale()
{
	return scale;
}

void SrCamera::setEye(float x, float y, float z)
{
	eye.x = x;
	eye.y = y;
	eye.z = z;
}

SrVec SrCamera::getEye()
{
	return eye;
}

void SrCamera::setCenter(float x, float y, float z)
{
	center.x = x;
	center.y = y;
	center.z = z;
}

SrVec SrCamera::getCenter()
{
	return center;
}

void SrCamera::setUpVector(SrVec u)
{
	up = u;
}

SrVec SrCamera::getUpVector()
{
	return up;
}

void SrCamera::setFov(float fov)
{
	fovy = fov;
}

float SrCamera::getFov()
{
	return fovy;
}

void SrCamera::setNearPlane(float n)
{
	znear = n;
}

float SrCamera::getNearPlane()
{
	return znear;
}

void SrCamera::setFarPlane(float f)
{
	zfar = f;
}

float SrCamera::getFarPlane()
{
	return zfar;
}

void SrCamera::setAspectRatio(float a)
{
	aspect = a;
}

float SrCamera::getAspectRatio()
{
	return aspect;
}

void SrCamera::setTrack(const std::string& characterName, const std::string& jointName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPawn* pawn = mcu.getPawn(characterName);
	if (!pawn)
	{
		LOG("Object %s was not found, cannot track.", characterName.c_str());
		return;
	}
	if (jointName == "")
	{
		LOG("Need to specify a joint to track.");
		return;
	}

	SkSkeleton* skeleton = NULL;
	skeleton = pawn->getSkeleton();

	SkJoint* joint = pawn->getSkeleton()->search_joint(jointName.c_str());
	if (!joint)
	{
		LOG("Could not find joint %s on object %s.", jointName.c_str(), characterName.c_str());
		return;
	}

	joint->skeleton()->update_global_matrices();
	joint->update_gmat();
	const SrMat& jointMat = joint->gmat();
	SrVec jointPos(jointMat[12], jointMat[13], jointMat[14]);
	CameraTrack* cameraTrack = new CameraTrack();
	cameraTrack->joint = joint;
	cameraTrack->jointToCamera = mcu.camera_p->eye - jointPos;
	LOG("Vector from joint to target is %f %f %f", cameraTrack->jointToCamera.x, cameraTrack->jointToCamera.y, cameraTrack->jointToCamera.z);
	cameraTrack->targetToCamera = mcu.camera_p->eye - mcu.camera_p->center;
	LOG("Vector from target to eye is %f %f %f", cameraTrack->targetToCamera.x, cameraTrack->targetToCamera.y, cameraTrack->targetToCamera.z);				
	mcu.cameraTracking.push_back(cameraTrack);
	LOG("Object %s will now be tracked at joint %s.", characterName.c_str(), jointName.c_str());
}

void SrCamera::removeTrack()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (mcu.cameraTracking.size() > 0)
	{
		for (std::vector<CameraTrack*>::iterator iter = mcu.cameraTracking.begin();
			 iter != mcu.cameraTracking.end();
			 iter++)
		{
			CameraTrack* cameraTrack = (*iter);
			delete cameraTrack;
		}
		mcu.cameraTracking.clear();
		LOG("Removing current tracked object.");
	}
}

void SrCamera::init () 
 { 
   eye.set ( 0, 0, 2.0f ); 
   center = SrVec::null;
   up = SrVec::j;
   fovy  = SR_TORAD(60);
   znear = 1.f; 
   zfar  = 10000.0f; 
   aspect = 1.0f;
   scale = 1.0f;
 }

SrMat& SrCamera::get_view_mat ( SrMat &m ) const
 {
   m.look_at ( eye, center, up );
   return m;
 }

SrMat& SrCamera::get_perspective_mat ( SrMat &m ) const
 {
   m.perspective ( fovy, aspect, znear, zfar );
   return m;
 }

// screenpt coords range in [-1,1]
void SrCamera::get_ray ( float winx, float winy, SrVec &p1, SrVec &p2 ) const
 {
   p1.set ( winx, winy, znear ); // p1 is in the near clip plane

   SrMat M(SrMat::NotInitialized), V(SrMat::NotInitialized), P(SrMat::NotInitialized);

   V.look_at ( eye, center, up );
   P.perspective ( fovy, aspect, znear, zfar );

   M.mult ( V, P ); // equiv to M = V * P

   M.invert();

   p1 = p1 * M; 
   p2 = p1-eye; // ray is in object coordinates, but before the scaling

   p2.normalize();
   p2 *= (zfar-znear);
   p2 += p1;

   float inv_scale = 1.0f/scale;
   p1*= inv_scale;
   p2*= inv_scale;

   //SR_TRACE1 ( "Ray: "<< p1 <<" : "<< p2 );
 }

/* - --------            \
   | |      |             \
   h | bbox |--------------.eye
   | |      |   dist      /
   - --------            /    tan(viewang/2)=(h/2)/dist
*/
void SrCamera::view_all ( const SrBox &box, float fovy_radians )
 {
   SrVec size = box.size();
   float h = SR_MAX(size.x,size.y);

   fovy = fovy_radians;
   up = SrVec::j;
   center = box.center();
   eye = center;
  
   float dist = (h/2)/tanf(fovy/2);
   eye.z = box.b.z + dist;

   // do not update z-far
   //float delta = box.max_size() + 0.0001f;
   //zfar = SR_ABS(eye.z)+delta;

   scale = 1.0f;
 }

void SrCamera::apply_translation_from_mouse_motion ( float lwinx, float lwiny, float winx, float winy )
 {
   SrVec p1, p2, x, inc;

   SrPlane plane ( center, eye-center );

   get_ray ( lwinx, lwiny, p1, x );
   p1 = plane.intersect ( p1, x );
   get_ray ( winx, winy, p2, x );
   p2 = plane.intersect ( p2, x );

   inc = p1-p2;

   inc *= scale;

   *this += inc;
 }

void SrCamera::operator*= ( const SrQuat& q )
 {
   eye -= center;
   eye = eye * q;
   eye += center;
   up -= center;
   up = up * q;
   up += center;
 }

void SrCamera::operator+= ( const SrVec& v )
 {
   eye += v;
   center += v;
 }

void SrCamera::operator-= ( const SrVec& v )
 {
   eye -= v;
   center -= v;
 }

//=============================== friends ==========================================

SrCamera operator* ( const SrCamera& c, const SrQuat& q )
 {
   SrCamera cam(c);
   cam *= q;
   return cam;
 }

SrCamera operator+ ( const SrCamera& c, const SrVec& v )
 {
   SrCamera cam(c);
   cam += v;
   return cam;
 }

SrOutput& operator<< ( SrOutput& out, const SrCamera& c )
 {
//   out << "eye:" << c.eye << " center:" << c.center << " up:" << c.up << srnl;

   out << "eye    " << c.eye << srnl <<
          "center " << c.center << srnl <<
          "up     " << c.up << srnl <<
          "fovy   " << c.fovy << srnl <<
          "znear  " << c.znear << srnl <<
          "zfar   " << c.zfar << srnl <<
          "aspect " << c.aspect << srnl <<
          "scale  " << c.scale << srnl;

   return out;
 }

SrInput& operator>> ( SrInput& inp, SrCamera& c )
 {
   while ( 1 )
    { if ( inp.get_token()==SrInput::EndOfFile ) break;
      if ( inp.last_token()=="eye" ) inp>>c.eye;
      else if ( inp.last_token()=="center" ) inp>>c.center;
      else if ( inp.last_token()=="up" ) inp>>c.up;
      else if ( inp.last_token()=="fovy" ) inp>>c.fovy;
      else if ( inp.last_token()=="znear" ) inp>>c.znear;
      else if ( inp.last_token()=="zfar" ) inp>>c.zfar;
      else if ( inp.last_token()=="aspect" ) inp>>c.aspect;
      else if ( inp.last_token()=="scale" ) inp>>c.scale;
      else { inp.unget_token(); break; }
    }
   return inp;
 }

void SrCamera::print()
{
	std::stringstream strstr;
	strstr << "   Camera Info " << std::endl;
	strstr << "-> eye position:	" << "(" << eye.x << ", " << eye.y << ", " << eye.z << ")" << std::endl;
	strstr << "-> center position:	" << "(" << center.x << ", " << center.y << ", " << center.z << ")" << std::endl;
	strstr << "-> up vector:		" << "(" << up.x << ", " << up.y << ", " << up.z << ")" << std::endl;
	strstr << "-> fovy:		" << fovy << std::endl;
	strstr << "-> near plane:		" << znear << std::endl;
	strstr << "-> far plane:		" << zfar << std::endl;
	strstr << "-> aspect:		" << aspect << std::endl;
	strstr << "-> scale:		" << scale << std::endl;
	LOG(strstr.str().c_str());
}

void SrCamera::reset()
{
	init();
	setEye(0, 166, 185);
	setCenter(0, 92, 0);
}

//================================ End of File =========================================
