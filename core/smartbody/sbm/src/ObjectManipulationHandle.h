#pragma once
#include "PawnPosControl.h"

class ObjectManipulationHandle
{
protected:
	PawnPosControl pawn_control;	
	PositionControl *active_control;
public:
	ObjectManipulationHandle(void);
	~ObjectManipulationHandle(void);

	//bool has_active_control() { return (active_control!=NULL); }
	PositionControl* get_active_control() { return active_control; }
	SbmPawn* get_selected_pawn();

	void picking(float x,float y,SrCamera& cam);
	void drag(SrCamera& cam,  float fx, float fy, float tx, float ty);
	void draw();
	static void get_pawn_list(SrArray<SbmPawn*>& pawn_list);

protected:
	
	int  process_hit(unsigned int *pickbuffer,int nhits);
	
};
