#include "ObjectManipulationHandle.h"
#include "fltk_viewer.h"
#include "vhcl.h"
#include <FL/gl.h>
#include <GL/glu.h>
#include <sbm/mcontrol_util.h>
#include <sb/SBCharacter.h>

ObjectManipulationHandle::ObjectManipulationHandle(void)
{
	active_control = NULL;
	bHasPicking = false;
	pickType = CONTROL_POS;
}

ObjectManipulationHandle::~ObjectManipulationHandle(void)
{
}

void ObjectManipulationHandle::get_pawn_list(std::vector<SbmPawn*>& pawn_list)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	for (std::map<std::string, SbmPawn*>::iterator iter = mcu.getPawnMap().begin();
		iter != mcu.getPawnMap().end();
		iter++)
	{
		SbmPawn* pawn = (*iter).second;
		pawn_list.push_back(pawn);
	}
}

SbmPawn* ObjectManipulationHandle::get_selected_pawn()
{
	if (active_control)
	{
		PawnPosControl* active_pawn_control = (PawnPosControl*) get_active_control();
		return active_pawn_control->get_attach_pawn();
	}
	return NULL;
}

void ObjectManipulationHandle::draw(SrCamera& cam)
{
	if (active_control)
		active_control->renderControl(cam);
	//pawn_control.draw();
}


void ObjectManipulationHandle::drag(SrCamera& cam,  float fx, float fy, float tx, float ty)
{
	if (active_control)
		active_control->updateControl(cam,fx,fy,tx,ty);
}

void ObjectManipulationHandle::set_selected_pawn( SbmPawn* pawn )
{
	if (active_control)
	{
		active_control->detach_pawn();
		active_control->attach_pawn(pawn);
	}
}

SbmPawn* ObjectManipulationHandle::getPickingPawn( float x, float y, SrCamera* cam, std::vector<int>& hitNames)
{
	GLint viewport[4];
	const int BUFSIZE = 512;
	GLuint selectBuf[BUFSIZE];
	float ratio;	
	SrMat mat;

	glSelectBuffer(BUFSIZE,selectBuf);
	glGetIntegerv(GL_VIEWPORT,viewport);
	glRenderMode(GL_SELECT);
	glInitNames();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	int cursorX,cursorY;
	cursorX = int((x+1.0)*viewport[2]*0.5);
	cursorY = int((y+1.0)*viewport[3]*0.5);

	//gluPickMatrix(cursorX,viewport[3]-cursorY,100,100,viewport);
	//printf("cursorX = %d, cursorY = %d\n",cursorX,cursorY);
	gluPickMatrix(cursorX,cursorY,5,5,viewport);
	ratio = (viewport[2]+0.0f) / viewport[3];	
	glMultMatrixf ( (const float*)cam->get_perspective_mat(mat) );	
	glMatrixMode(GL_MODELVIEW);

	// draw buffer object for each joints ?	
	// test for each joints
	//pawn_control.hitTest();
	std::vector<SbmPawn*> pawn_list;
	this->get_pawn_list(pawn_list);

	// determine the size of the pawns relative to the size of the characters
	float pawnSize = 1.0;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		iter != mcu.getCharacterMap().end();
		iter++)
	{
		SbmCharacter* character = (*iter).second;
		pawnSize = character->getHeight() / 30.0f;
		break;
	}

	for (unsigned int i=0;i<pawn_list.size();i++)
	{
		SbmPawn* pawn = pawn_list[i];
		SrVec pawn_pos = PawnPosControl::get_pawn_pos(pawn);
		glPushName(0xffffffff);
		glLoadName(i);
		SmartBody::SBCharacter* curChar = dynamic_cast<SmartBody::SBCharacter*>(pawn);
		//PositionControl::drawSphere(pawn_pos, pawnSize);		
		if (active_control && active_control->get_attach_pawn() == pawn)
		{			
			//pawnPosControl.hitOPS(cam);
			active_control->hitTest(*cam);
		}
		else if (curChar) // the selected pawn is actually a character
		{
			SrBox bbox = curChar->getBoundingBox();
			PositionControl::drawBox(bbox);						
		}
		else if (pawn->getPhysicsObject())
		{
			SrMat gmat = pawn->getPhysicsObject()->getGlobalTransform().gmat();
			//FltkViewer::drawColObject(pawn->getGeomObject(), gmat);
		}
		else
		{
			PositionControl::drawSphere(pawn_pos, pawnSize);				
		}		
		glPopName();			
	}
	// process picking
	GLint hits;
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glFlush();
	hits = glRenderMode(GL_RENDER);

	SbmPawn* selectPawn = NULL;
	if (hits != 0){
		//processHits2(hits,selectBuf,0);
		hitNames = this->process_hit(selectBuf,hits);		
		int pawnIdx = hitNames[0];
		selectPawn = pawn_list[pawnIdx];		
	}	
	return selectPawn;
}

void ObjectManipulationHandle::picking(float x,float y,SrCamera* cam)
{
	// start picking
	/*
	GLint viewport[4];
	const int BUFSIZE = 512;
	GLuint selectBuf[BUFSIZE];
	float ratio;	
	SrMat mat;
	
	glSelectBuffer(BUFSIZE,selectBuf);
	glGetIntegerv(GL_VIEWPORT,viewport);
	glRenderMode(GL_SELECT);
	glInitNames();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	int cursorX,cursorY;
	cursorX = int((x+1.0)*viewport[2]*0.5);
	cursorY = int((y+1.0)*viewport[3]*0.5);

	//gluPickMatrix(cursorX,viewport[3]-cursorY,100,100,viewport);
	//printf("cursorX = %d, cursorY = %d\n",cursorX,cursorY);
	gluPickMatrix(cursorX,cursorY,5,5,viewport);
	ratio = (viewport[2]+0.0f) / viewport[3];	
	glMultMatrixf ( (const float*)cam.get_perspective_mat(mat) );	
	glMatrixMode(GL_MODELVIEW);

	// draw buffer object for each joints ?	
	// test for each joints
	//pawn_control.hitTest();
	SrArray<SbmPawn*> pawn_list;
	this->get_pawn_list(pawn_list);

	// determine the size of the pawns relative to the size of the characters
	float pawnSize = 1.0;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.character_map.reset();
	while (SbmCharacter* character = mcu.character_map.next())
	{
		pawnSize = character->getHeight() / 30.0f;
		break;
	}

	for (int i=0;i<pawn_list.size();i++)
	{
		SbmPawn* pawn = pawn_list[i];
		SrVec pawn_pos = PawnPosControl::get_pawn_pos(pawn);
		glPushName(0xffffffff);
	    glLoadName(i);
		//PositionControl::drawSphere(pawn_pos, pawnSize);		
		if (active_control && active_control->get_attach_pawn() == pawn)
		{			
			//pawnPosControl.hitOPS(cam);
			active_control->hitTest(cam);
		}
		else
			PositionControl::drawSphere(pawn_pos, pawnSize);				
	    glPopName();			
	}
	// process picking
	GLint hits;
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glFlush();
	hits = glRenderMode(GL_RENDER);
	*/
	
	//printf("Hits = %d\n",hits);
	PawnControl* tempControl = getPawnControl(pickType);

	std::vector<int> hitNames;
	SbmPawn* selectPawn = getPickingPawn(x,y,cam,hitNames);
	
	if (selectPawn){		
		if (active_control != tempControl)
			active_control = tempControl;		
		active_control->attach_pawn(selectPawn);
		if (hitNames.size() > 1)
			active_control->processHit(hitNames);		
	}
	else
	{
		if (active_control)
		{
			active_control->detach_pawn();
			//pawnPosControl.active = false;
			active_control = NULL;
		}		
	}	
	// stop picking
}

std::vector<int> ObjectManipulationHandle::process_hit(unsigned int *pickbuffer,int nhits)
{
	GLuint d1,d2,i,n,zmin,zmax,sel=0;
	std::vector<int> hitNames;
	if(0<=nhits){    
		for(i=0,zmin=zmax=4294967295U; nhits>0; i+=n+3,nhits--)
		{      
			n=pickbuffer[i];
      		d1=pickbuffer[1+i];
			d2=pickbuffer[2+i];
			if(d1<zmin || (d1==zmin && d2<=zmax)){
				zmin=d1;
				zmax=d2;
				sel=i;
			}
		}
		//return pickbuffer[3+sel];
		int n = pickbuffer[sel];
		for (int k=0;k<n;k++)
			hitNames.push_back(pickbuffer[3+sel+k]);
	}
	return hitNames;
}

PawnControl* ObjectManipulationHandle::getPawnControl( ControlType type )
{
	if (type == CONTROL_POS)
		return &pawnPosControl;
	if (type == CONTROL_ROT)
		return &pawnRotControl;

	return &pawnPosControl;
}
