#pragma once
#include "me_ct_data_interpolation.h"
#include "me_ct_barycentric_interpolation.h"
#include "me_ct_motion_parameter.h"
#include "me_ct_jacobian_IK.hpp"
#include "me_ct_ccd_IK.hpp"
#include "me_ct_constraint.hpp"

#include <SR/planner/sk_pos_planner.h>
#include <SR/planner/sk_blend_planner.h>
#include <SBM/Physics/SbmColObject.h>
#include <SBM/sbm_pawn.hpp>

#include <SBM/MeCtReachEngine.h>

class ReachStateData;
class ReachStateInterface;
class ReachHandAction;

using namespace std;
// used when we want exact control for an end effector

class MeCtExampleBodyReach :
	public MeController, public FadingControl
{
private:
	static const char* CONTROLLER_TYPE;
public:	
	//enum HandActionState { PICK_UP_OBJECT = 0, TOUCH_OBJECT, PUT_DOWN_OBJECT };
protected:
	MeCtReachEngine*      reachEngine;	
	std::string           characterName;		
	bool                  footIKFix;
	vector<SkJoint*>      affectedJoints;		
	BodyMotionFrame       inputMotionFrame;		
	float 			      _duration;
	SkChannelArray	      _channels;

public:	
	ReachStateData*       reachData;		

public:	
	MeCtExampleBodyReach(MeCtReachEngine* re);
	virtual ~MeCtExampleBodyReach(void);		
	virtual void controller_map_updated();
	virtual void controller_start();	
	virtual bool controller_evaluate( double t, MeFrameData& frame );

	virtual SkChannelArray& controller_channels()	{ return( _channels ); }
	virtual double controller_duration()			{ return( (double)_duration ); }	
	virtual const char* controller_type() const		{ return( CONTROLLER_TYPE ); }
	virtual void print_state( int tabs );

	MeCtReachEngine* getReachEngine() const { return reachEngine; }	
	void set_duration(float duration) { _duration = duration; }
	
	void setHandActionState(MeCtReachEngine::HandActionState newState);
	void setLinearVelocity(float vel);
	void setReachCompleteDuration(float duration);	
	bool addHandConstraint(SkJoint* targetJoint, const char* effectorName);
	void setReachTargetPawn(SbmPawn* targetPawn);	
	void setReachTargetPos(SrVec& targetPos);
	void setFinishReaching(bool isFinish);
	void setFootIK(bool useIK);
	void init();	
protected:			
	void updateChannelBuffer(MeFrameData& frame, BodyMotionFrame& motionFrame, bool bRead = false);			
};







