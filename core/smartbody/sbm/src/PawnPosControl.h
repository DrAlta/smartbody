#pragma once
#include "PositionControl.h"
#include "RotationControl.h"
#include <sbm/sbm_pawn.hpp>
#ifdef WIN32
#include "glfont2.h"

using namespace glfont;
#endif

class PawnControl : public DObserver
{
protected:
	SbmPawn* pawn;
#ifdef WIN32
	GLFont   label;
#endif

public:	
	void attach_pawn(SbmPawn* ap);
	void detach_pawn();
	SbmPawn* get_attach_pawn() { return pawn; }
	static SrVec get_pawn_pos(SbmPawn* pawn);
	static void  set_pawn_pos(SbmPawn* pawn, SrVec& pos);
	static SrQuat get_pawn_rot(SbmPawn* pawn);
	static void  set_pawn_rot(SbmPawn* pawn, SrQuat& quat);
	virtual void renderControl(SrCamera& cam) = 0;
	virtual void updateControl(SrCamera& cam, float fx, float fy, float tx, float ty) = 0;

	virtual void hitTest(SrCamera& cam) = 0;
	virtual void processHit(std::vector<int>& hitNames) = 0;

	virtual void notify(DSubject* subject);

protected:
	void init_font();
};


class PawnPosControl : public PositionControl, public PawnControl
{
public:
	PawnPosControl(void);
	~PawnPosControl(void);

	virtual void renderControl(SrCamera& cam);
	virtual void updateControl(SrCamera& cam, float fx, float fy, float tx, float ty);
	virtual SrVec getWorldPt(); // get world position ( from the attached object ? )
	virtual void setWorldPt(SrVec& newPt);	
	virtual void hitTest(SrCamera& cam)  { hitOPS(cam); }
	virtual void processHit(std::vector<int>& hitNames) { identify(hitNames); }
};

class PawnRotationControl : public RotationControl, public PawnControl
{
public:
	virtual void renderControl(SrCamera& cam);
	virtual void updateControl(SrCamera& cam, float fx, float fy, float tx, float ty);	
	virtual SrQuat getWorldRot();
	virtual void setWorldRot(SrQuat& newRot);
	virtual SrVec getWorldPt(); // get world position ( from the attached object ? )
	virtual void hitTest(SrCamera& cam)  { hitOPS(cam); }
	virtual void processHit(std::vector<int>& hitNames) { identify(hitNames); }
};

