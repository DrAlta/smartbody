#pragma once
#include "PositionControl.h"
#include <sbm/sbm_pawn.hpp>
#include "glfont2.h"

using namespace glfont;

class PawnPosControl : public PositionControl
{
protected:
	SbmPawn* pawn;
	GLFont   label;
public:
	bool dragging;
public:
	PawnPosControl(void);
	~PawnPosControl(void);

	void attach_pawn(SbmPawn* ap);
	void detach_pawn();
	SbmPawn* get_attach_pawn() { return pawn; }

	virtual void draw(); 
	virtual SrVec getWorldPt(); // get world position ( from the attached object ? )
	virtual void setWorldPt(SrVec& newPt);	

	static SrVec get_pawn_pos(SbmPawn* pawn);
	static void  set_pawn_pos(SbmPawn* pawn, SrVec& pos);
protected:
	void init_font();
};

