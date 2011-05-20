#pragma once
#include "me_ct_motion_parameter.h"
#include "me_ct_jacobian_IK.hpp"
#include "me_ct_ccd_IK.hpp"
#include "me_ct_constraint.hpp"

#include <SR/planner/sk_pos_cfg.h>

using namespace std;



class FingerChain
{
public:
	MeCtIKTreeNode* fingerTip;
	std::vector<MeCtIKTreeNode*> fingerNodes;	
	SrVec           fingerTarget;
	bool            isLock;
public:
	FingerChain() { isLock = false; fingerTip = NULL; fingerTarget = SrVec(); }
	~FingerChain() {}
	void init(MeCtIKTreeNode* figTip);	
	void unlockChain();
	void getLineSeg(std::vector<SrVec>& lineSeg);
	void testCollision(SbmColObject* colObj);
};


class MeCtHand :
	public MeController, public FadingControl
{
private:
	static const char* CONTROLLER_TYPE;
public:
	enum FingerID { F_THUMB = 0, F_INDEX, F_MIDDLE, F_RING, F_PINKY, F_NUM_FINGERS };
	enum GrabState { GRAB_START, GRAB_REACH, GRAB_FINISH, GRAB_RETURN };
protected:
	SkSkeleton*     skeletonRef;
	SkSkeleton*     skeletonCopy;
	SkJoint*        wristJoint;
	float 			_duration, prevTime;
	SkChannelArray	_channels;	
	GrabState             currentGrabState;
	BodyMotionFrame       releaseFrame, grabFrame, reachFrame, currentFrame, tempFrame;	
	
	vector<FingerChain>   fingerChains;
	vector<SkJoint*>      affectedJoints;
	
	ConstraintMap         handPosConstraint;
	ConstraintMap         handRotConstraint;

	MeCtIKTreeScenario    ikScenario;
	MeCtJacobianIK        ik;
	SbmColObject*         grabTarget; 

	std::string           attachJointName;
	SbmPawn*              attachedPawn;
	SrMat                 attachMat;

public:
	float                 grabVelocity;

public:
	MeCtHand(SkSkeleton* sk, SkJoint* wrist);
	~MeCtHand(void);	

	void init(const MotionDataSet& reachPose, const MotionDataSet& grabPose, const MotionDataSet& releasePose);

public:
	virtual void controller_map_updated();
	virtual void controller_start();	
	virtual bool controller_evaluate( double t, MeFrameData& frame );

	virtual SkChannelArray& controller_channels()	{ return( _channels ); }
	virtual double controller_duration()			{ return( (double)_duration ); }
	void set_duration(float duration) { _duration = duration; }
	virtual const char* controller_type() const		{ return( CONTROLLER_TYPE ); }
	virtual void print_state( int tabs );
	
	void setGrabState(GrabState state);
	void setGrabTargetObject(SbmColObject* targetObj);

	void attachPawnTarget(SbmPawn* pawn, std::string jointName);
	void releasePawn();
	void updateAttachedPawn();
protected:
	void solveIK(float dt);
	void updateChannelBuffer(MeFrameData& frame, BodyMotionFrame& handMotionFrame, bool bRead = false);
	void getPinchFrame(BodyMotionFrame& pinchFrame, SrVec& wristOffset);
	FingerID findFingerID(const char* jointName);
	void updateFingerChains( BodyMotionFrame& targetMotionFrame, float maxAngDelta);
	BodyMotionFrame& findTargetFrame(GrabState state);
};







