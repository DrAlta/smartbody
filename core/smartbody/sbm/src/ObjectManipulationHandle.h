#pragma once
#include "PawnPosControl.h"

class ObjectManipulationHandle
{
protected:
	PawnPosControl pawn_control;	
	PositionControl *active_control;
	bool bHasPicking;
	SrVec2 pick_loc;
	
public:
	ObjectManipulationHandle(void);
	~ObjectManipulationHandle(void);

	//bool has_active_control() { return (active_control!=NULL); }
	PositionControl* get_active_control() { return active_control; }
	SbmPawn* get_selected_pawn();
	void setPicking(SrVec2& loc) { pick_loc = loc; bHasPicking = true; }
	bool hasPicking() { return bHasPicking; }
	SrVec2& getPickLoc() { bHasPicking = false; return pick_loc; }

	void picking(float x,float y,SrCamera& cam);
	void drag(SrCamera& cam,  float fx, float fy, float tx, float ty);
	void draw();
	static void get_pawn_list(SrArray<SbmPawn*>& pawn_list);

protected:
	
	int  process_hit(unsigned int *pickbuffer,int nhits);
	
};
