#include "PawnPosControl.h"
#include <sbm/mcontrol_util.h>
#include <sbm/sbm_pawn.hpp>
#include "vhcl.h"
#include <FL/gl.h>
#include <GL/glu.h>
#include <sr/sr_euler.h>
#include <sr/sr_plane.h>
#include <sr/sr_sphere.h>
#include <sr/sr_sn.h>
#include <sr/sr_sn_group.h>
#include <sr/sr_sa_gl_render.h>
#include <sr/sr_gl_render_funcs.h>
#include <sb/SBScene.h>

void PawnControl::init_font()
{	
	GLuint textureName;	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glGenTextures(0, &textureName);
#ifdef WIN32
	std::string mediaPath = SmartBody::SBScene::getScene()->getMediaPath();
	std::string fontPath = mediaPath + "/" +  "fonts/font.glf";
	if (!label.Create(fontPath.c_str(), 0))
	{
		if(!label.Create(".font.glf", 0))
			LOG("PawnPosControl::InitFont(): Error: Cannot load font file\n");
	}	
#endif
}

void PawnControl::attach_pawn(SbmPawn* ap)
{
	pawn = ap;
	if (pawn)
		pawn->registerObserver(this);
}

void PawnControl::detach_pawn()
{
	if (pawn)
		pawn->unregisterObserver(this);
	pawn = NULL;
}

SrVec PawnControl::get_pawn_pos(SbmPawn* pawn)
{
	SrVec pos;
	pawn->getSkeleton()->update_global_matrices();	
	float x,y,z,h,p,r;
	pawn->get_world_offset(x,y,z,h,p,r);	
	pos.x = x; pos.y = y; pos.z = z;

	return pos;
}

void PawnControl::set_pawn_pos(SbmPawn* pawn, SrVec& pos)
{

	float x,y,z,h,p,r;
	pawn->get_world_offset(x,y,z,h,p,r);
	//printf("h = %f, p = %f, r = %f\n",h,p,r);
	
	pawn->set_world_offset(pos.x,pos.y,pos.z,h,p,r);
	pawn->updateToColObject();	
	mcuCBHandle& mcu = mcuCBHandle::singleton();	
	mcu.resourceDataChanged = true;
}

SrQuat PawnControl::get_pawn_rot( SbmPawn* pawn )
{
	SrQuat rot;
	pawn->getSkeleton()->update_global_matrices();	
	float x,y,z,h,p,r;
	pawn->get_world_offset(x,y,z,h,p,r);	
	gwiz::quat_t q = gwiz::euler_t(p,h,r);	
	rot = SrQuat((float)q.w(),(float)q.x(),(float)q.y(),(float)q.z());
	return rot;
}

void PawnControl::set_pawn_rot( SbmPawn* pawn, SrQuat& quat )
{
	SrQuat rot;
	pawn->getSkeleton()->update_global_matrices();	
	float x,y,z,h,p,r;
	pawn->get_world_offset(x,y,z,h,p,r);
	gwiz::quat_t q = gwiz::quat_t(quat.w,quat.x,quat.y,quat.z);
	gwiz::euler_t e = gwiz::euler_t(q);
	pawn->set_world_offset(x,y,z,(float)e.h(),(float)e.p(),(float)e.r());	
	pawn->updateToColObject();
	mcuCBHandle& mcu = mcuCBHandle::singleton();	
	mcu.resourceDataChanged = true;
}

void PawnControl::notify(SmartBody::SBSubject* subject)
{
	SbmPawn* notifyPawn = dynamic_cast<SbmPawn*>(subject);
	if (notifyPawn && notifyPawn == pawn)
	{
		detach_pawn();
	}
}

/************************************************************************/
/* Pawn Pos Control                                                     */
/************************************************************************/


PawnPosControl::PawnPosControl(void) : PositionControl()
{
	/*
	dragging = false;
	active = false;
	base=80;
	r=6;
	len=r*4;
	s_len=10;
	ss_len=3;
	opdir = 3;

	colors[3]=SrVec(100.0/255.0, 220.0/255.0, 1.);
	colors[0]=SrVec(1,0,0);
	colors[1]=SrVec(0,154.0/255.0,82.0/255.0);
	colors[2]=SrVec(0,0,1);
	worldPt = SrVec(0,170,100);
	*/

	//label = NULL;	
	pawn = NULL;
	//init_font();
}

PawnPosControl::~PawnPosControl(void)
{
}

void PawnPosControl::renderControl(SrCamera& cam)
{
	// draw text from the pawn
	/*
	if (pawn)
	{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef WIN32
	label.Begin();
	label.DrawString("pawn", 0.003f,0.0,0.0);		
#endif
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);		
	}
	*/
	PositionControl::draw(cam);
}

void PawnPosControl::updateControl( SrCamera& cam, float fx, float fy, float tx, float ty )
{
	PositionControl::drag(cam,fx,fy,tx,ty);
}

SrVec PawnPosControl::getWorldPt()
{
	assert(pawn);
	return get_pawn_pos(pawn);
}

void PawnPosControl::setWorldPt(SrVec& newPt)
{
	//worldPt = newPt;	
	assert(pawn);
	set_pawn_pos(pawn,newPt);
}



void PawnRotationControl::renderControl( SrCamera& cam )
{
	RotationControl::draw(cam);
}

void PawnRotationControl::updateControl( SrCamera& cam, float fx, float fy, float tx, float ty )
{
	RotationControl::drag(cam,fx,fy,tx,ty);
}

SrQuat PawnRotationControl::getWorldRot()
{
	assert(pawn);
	return get_pawn_rot(pawn);
}

void PawnRotationControl::setWorldRot( SrQuat& newRot )
{
	assert(pawn);
	return set_pawn_rot(pawn,newRot);
}

SrVec PawnRotationControl::getWorldPt()
{
	assert(pawn);
	return get_pawn_pos(pawn);
}
