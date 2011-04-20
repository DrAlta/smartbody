#pragma once
#include "me_ct_motion_parameter.h"
#include "me_ct_jacobian_IK.hpp"
#include "me_ct_ccd_IK.hpp"
#include "me_ct_constraint.hpp"

using namespace std;

// basic interface for the object to be grabbed
class GrabObject
{
public:
	virtual SrVec getCentroid() = 0; // get the center of the object
	virtual bool  isCollided(const SrVec& inPos) = 0; // check if a point is inside the object	
	virtual bool isCollided(const SrVec& p1, const SrVec& p2) = 0;
	virtual bool isCollided(std::vector<SrVec>& lineSeg) = 0;
};

class GrabSphere : public GrabObject
{
protected:
	float radius;
	SrVec center;
public:
	GrabSphere() {};
	~GrabSphere() {};
	void setSphere(const SrVec& pt, const float r);
	virtual SrVec getCentroid(); // get the center of the object
	virtual bool  isCollided(const SrVec& inPos); // check if a point is inside the object	
	virtual bool isCollided(const SrVec& p1, const SrVec& p2);
	virtual bool isCollided(std::vector<SrVec>& lineSeg);
};

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
	void getLineSeg(std::vector<SrVec>& lineSeg);
};


class MeCtHand :
	public MeController, public FadingControl
{
private:
	static const char* CONTROLLER_TYPE;
public:
	enum FingerID { F_THUMB = 0, F_INDEX, F_MIDDLE, F_RING, F_PINKY, F_NUM_FINGERS };
	enum GrabState { GRAB_START, GRAB_RETURN };
protected:
	SkSkeleton*     skeletonRef;
	SkSkeleton*     skeletonCopy;
	SkJoint*        wristJoint;
	float 			_duration, prevTime;
	SkChannelArray	_channels;	
	GrabState             currentGrabState;
	BodyMotionFrame       restFrame, currentFrame, targetFrame;	
	
	vector<FingerChain>   fingerChains;
	vector<SkJoint*>      affectedJoints;
	
	ConstraintMap         handPosConstraint;
	ConstraintMap         handRotConstraint;

	MeCtIKTreeScenario    ikScenario;
	MeCtJacobianIK        ik;
	GrabObject*           grabTarget;           
public:
	float                 grabVelocity;

public:
	MeCtHand(SkSkeleton* sk, SkJoint* wrist);
	~MeCtHand(void);	

	void init(const MotionDataSet& reachPose, const MotionDataSet& grabPose);

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
	void setGrabTargetPos(SrVec& targetPos);
protected:
	void solveIK(float dt);
	void updateChannelBuffer(MeFrameData& frame, BodyMotionFrame& handMotionFrame, bool bRead = false);
	void getPinchFrame(BodyMotionFrame& pinchFrame, SrVec& wristOffset);
	FingerID findFingerID(const char* jointName);
	void updateFingerChains( BodyMotionFrame& targetMotionFrame, float maxAngDelta);
};







