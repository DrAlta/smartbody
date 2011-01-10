#include <fltk/gl.h>
#include <GL/glu.h>
#include <sr/sr_plane.h>
#include <sr/sr_sphere.h>
#include <sr/sr_sn.h>
#include <sr/sr_sn_group.h>
# include <SR/sr_sa_gl_render.h>
# include <SR/sr_gl_render_funcs.h>
#include "vhcl.h"
#include "glfont2.h"

#include "PawnPosControl.h"


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
	init_font();
}

PawnPosControl::~PawnPosControl(void)
{
}

void PawnPosControl::draw()
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
		label.Begin();
		label.DrawString("pawn", 0.003f,0.0,0.0);		
	    glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glDisable(GL_TEXTURE_2D);		
	}
	*/
	
	PositionControl::draw();
}

void PawnPosControl::init_font()
{	
	GLuint textureName;	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glGenTextures(0, &textureName);
	if (!label.Create("../../../../data/fonts/font.glf", 0))
	{
		if(!label.Create(".font.glf", 0))
			LOG("PawnPosControl::InitFont(): Error: Cannot load font file\n");
	}	
}

void PawnPosControl::attach_pawn(SbmPawn* ap)
{
	pawn = ap;
}

void PawnPosControl::detach_pawn()
{
	pawn = NULL;
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


SrVec PawnPosControl::get_pawn_pos(SbmPawn* pawn)
{
	SrVec pos;
	pawn->skeleton_p->update_global_matrices();
	SrArray<SkJoint*>& joints = pawn->skeleton_p->get_joint_array();		
	SrMat gmat = joints[0]->gmat();
	//pos = SrVec(gmat(
	//pos.set(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14));
	float x,y,z,h,p,r;
	pawn->get_world_offset(x,y,z,h,p,r);
	pos.x = x; pos.y = y; pos.z = z;

	return pos;
}

void PawnPosControl::set_pawn_pos(SbmPawn* pawn, SrVec& pos)
{
	float x,y,z,h,p,r;
	pawn->get_world_offset(x,y,z,h,p,r);
	pawn->set_world_offset(pos.x,pos.y,pos.z,h,p,r);
}

