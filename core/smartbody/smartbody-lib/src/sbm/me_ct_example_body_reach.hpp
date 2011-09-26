
#ifndef _ME_BODY_REACH_CONTROLLER_
#define _ME_BODY_REACH_CONTROLLER_

#include "me_ct_data_interpolation.h"
#include "me_ct_barycentric_interpolation.h"
#include "me_ct_motion_parameter.h"
#include "me_ct_jacobian_IK.hpp"
#include "me_ct_ccd_IK.hpp"
#include "me_ct_constraint.hpp"

#include <sr/planner/sk_pos_planner.h>
#include <sr/planner/sk_blend_planner.h>
#include <sbm/Physics/SbmColObject.h>
#include <sbm/sbm_pawn.hpp>

#include <sbm/MeCtReachEngine.h>
#include <sbm/SBController.h>

class ReachStateData;
class ReachStateInterface;
class ReachHandAction;

using namespace std;
// used when we want exact control for an end effector

typedef std::map<int,MeCtReachEngine*> ReachEngineMap; 

class MeCtExampleBodyReach :
	public SmartBody::SBController, public FadingControl
{
private:
	static std::string CONTROLLER_TYPE;
public:	
	//enum HandActionState { PICK_UP_OBJECT = 0, TOUCH_OBJECT, PUT_DOWN_OBJECT };

protected:	
	int                   defaultReachType;
	ReachEngineMap        reachEngineMap;
	MeCtReachEngine*      currentReachEngine;
	std::string           characterName;		
	bool                  footIKFix;
	bool                  useProfileInterpolation;
	bool                  useRetiming;
	bool                  isMoving;
	bool                  startReach;
	bool                  endReach;
	float                 autoReturnDuration;
	float                 reachVelocityScale;
	vector<SkJoint*>      affectedJoints;		
	BodyMotionFrame       inputMotionFrame;		
	float 			      _duration;
	SkChannelArray	      _channels;

public:	
	ReachStateData*       currentReachData;		

public:	
	MeCtExampleBodyReach(std::map<int,MeCtReachEngine*>& reMap);
	virtual ~MeCtExampleBodyReach(void);		
	virtual void controller_map_updated();
	virtual void controller_start();	
	virtual bool controller_evaluate( double t, MeFrameData& frame );

	virtual SkChannelArray& controller_channels()	{ return( _channels ); }
	virtual double controller_duration()			{ return( (double)_duration ); }	
	virtual const std::string& controller_type() const		{ return( CONTROLLER_TYPE ); }
	virtual void print_state( int tabs );

	MeCtReachEngine* getReachEngine() const { return currentReachEngine; }	
	void set_duration(float duration) { _duration = duration; }

	void setHandActionState(MeCtReachEngine::HandActionState newState);
	void setLinearVelocity(float vel);
	void setReachCompleteDuration(float duration);	
	bool addHandConstraint(SkJoint* targetJoint, const char* effectorName);
	void setReachTargetPawn(SbmPawn* targetPawn);
	void setReachTargetJoint(SkJoint* targetJoint);
	void setReachTargetPos(SrVec& targetPos);
	void setFinishReaching(bool isFinish);
	void setFootIK(bool useIK);
	void setDefaultReachType(const std::string& reachTypeName);
	void init(SbmPawn* pawn);	
protected:			
	void updateChannelBuffer(MeFrameData& frame, BodyMotionFrame& motionFrame, bool bRead = false);
	bool updateLocomotion(); // return true if locomotion is finished
	void updateReachType(SrVec& targetPos); // update the correct reach engine 
	int  determineReachType(SrVec& targetPos);
	void setNewReachEngine(MeCtReachEngine* newReachEngine);
	bool isValidReachEngine(int reachType);
};

#endif





