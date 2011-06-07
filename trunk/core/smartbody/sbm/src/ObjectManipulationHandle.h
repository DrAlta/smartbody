#pragma once
#include "PawnPosControl.h"

class ObjectManipulationHandle
{
public:
	enum ControlType { CONTROL_POS = 0, CONTROL_ROT };
protected:
	PawnPosControl pawnPosControl;	
	PawnRotationControl pawnRotControl;
	PawnControl  *active_control;
	bool bHasPicking;
	SrVec2 pick_loc;
	ControlType pickType;
	
public:
	ObjectManipulationHandle(void);
	~ObjectManipulationHandle(void);

	void setPicking(SrVec2& loc) { pick_loc = loc; bHasPicking = true; }
	void setPickingType(ControlType type) { pickType = type; }

	bool hasPicking() { return bHasPicking; }
	SrVec2& getPickLoc() { bHasPicking = false; return pick_loc; }

	//bool has_active_control() { return (active_control!=NULL); }
	PawnControl* get_active_control() { return active_control; }
	SbmPawn* get_selected_pawn();

	SbmPawn* getPickingPawn(float x, float y, SrCamera& cam, std::vector<int>& hitNames);
	void picking(float x,float y,SrCamera& cam);
	void drag(SrCamera& cam,  float fx, float fy, float tx, float ty);
	void draw(SrCamera& cam);

	static void get_pawn_list(SrArray<SbmPawn*>& pawn_list);
	PawnControl* getPawnControl(ControlType type);
protected:	
	std::vector<int>  process_hit(unsigned int *pickbuffer,int nhits);
	
};
