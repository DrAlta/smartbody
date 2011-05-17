#pragma once
#include "me_ct_example_body_reach.hpp"

class ReachTarget
{
public:
	SRT      targetState;
	SbmPawn* targetPawn;
	bool     useTargetPawn;
public:
	ReachTarget();
	~ReachTarget() {}
	bool targetIsPawn();
	void setTargetState(SRT& ts);
	void setTargetPawn(SbmPawn* tpawn);
	SRT getTargetState(); // the return state is based on target's state
	SRT getGrabTargetState(SRT& naturalState); 
	SbmPawn* getTargetPawn();
};

class ReachHandAction
{
public:
	// action interface, do nothing by default
	virtual void reachPreCompleteAction(ReachStateData* rd) {};
	virtual void reachCompleteAction(ReachStateData* rd) {};	
	virtual void reachNewTargetAction(ReachStateData* rd) {};
	virtual void reachReturnAction(ReachStateData* rd) {};
	virtual SRT getHandTargetStateOffset(ReachStateData* rd, SRT& naturalState);
	virtual bool pickUpNewPawn(ReachStateData* rd) { return false; }
protected:
	void sendReachEvent(std::string cmd, float time = 0.0);	
};

class ReachHandPickUpAction : public ReachHandAction
{
public:
	// action interface, do nothing by default
	virtual void reachPreCompleteAction(ReachStateData* rd);
	virtual void reachCompleteAction(ReachStateData* rd);		
	virtual void reachNewTargetAction(ReachStateData* rd);
	virtual SRT getHandTargetStateOffset(ReachStateData* rd, SRT& naturalState);
	virtual bool pickUpNewPawn(ReachStateData* rd);
};

class ReachHandPutDownAction : public ReachHandAction
{
public:
	// action interface, do nothing by default	
	virtual void reachCompleteAction(ReachStateData* rd);	
	virtual void reachReturnAction(ReachStateData* rd);	
	virtual SRT getHandTargetStateOffset(ReachStateData* rd, SRT& naturalState);
};

class EffectorState 
{
public:
	std::string  effectorName;
	SRT curTargetState, targetState;
	SRT curState; 
	SRT grabStateError;
	SrVec paraTarget;

	SbmPawn* attachedPawn;
	SrMat    attachMat;
public:
	EffectorState();
	~EffectorState() {}
	void attachPawnTarget(ReachTarget& target);
	void releasePawn();
	void updateAttachedPawn();
};

class ReachStateData
{
public:
	std::string     charName; // character name
	float           curTime, dt;
	float           curRefTime, du;	
	SrMat           gmat;
	// reference motion frames ( as motion interpolation output, or IK reference pose )
	BodyMotionFrame idleRefFrame, targetRefFrame, currentRefFrame;	
	BodyMotionFrame curMotionFrame; 
	EffectorState   effectorState;
	ReachTarget     reachTarget;	
	// flags for state transition
	float           autoReturnTime;
	bool            startReach, endReach;
	bool            useExample;	

	// for pick-up/put-down action
	ReachHandAction* curHandAction;
	
	// for motion interpolation
	ResampleMotion* interpMotion;
	MotionParameter* motionParameter;
	DataInterpolator* dataInterpolator;
public:
	float linearVel, angularVel;
	float reachRegion;
public:	
	ReachStateData();
	~ReachStateData();
	void updateReachState(const SrMat& worldOffset, BodyMotionFrame& motionFrame); // update corresponding parameters according to current body frame
	void updateBlendWeight(SrVec paraPos);
	void getInterpFrame(float refTime, BodyMotionFrame& outFrame);
	SRT getBlendPoseState(SrVec paraPos, float refTime);
	SRT getPoseState(BodyMotionFrame& frame);
	bool useInterpolation();	
};

class ReachStateInterface
{	
public:
	virtual void update(ReachStateData* rd) = 0;
	virtual void updateEffectorTargetState(ReachStateData* rd) = 0;	
	virtual float curStatePercentTime(ReachStateData* rd, float refTime)  { return 0.f; };
	virtual std::string nextState(ReachStateData* rd) = 0;
	virtual std::string curStateName() = 0;
protected:
	void updateReturnToIdle(ReachStateData* rd);
	void updateReachToTarget(ReachStateData* rd);
	void updateMotionIK(ReachStateData* rd);
	void updateMotionInterp(ReachStateData* rd);
	bool ikTargetReached(ReachStateData* rd);
	bool interpTargetReached(ReachStateData* rd);	
};

class ReachStateIdle : public ReachStateInterface
{
public:
	virtual void update(ReachStateData* rd);
	virtual void updateEffectorTargetState(ReachStateData* rd);	
	virtual std::string nextState(ReachStateData* rd);	
	virtual std::string curStateName() { return "Idle"; }
};


class ReachStateStart : public ReachStateInterface
{
public:
	virtual void update(ReachStateData* rd);
	virtual void updateEffectorTargetState(ReachStateData* rd);	
	virtual float curStatePercentTime(ReachStateData* rd, float refTime);
	virtual std::string nextState(ReachStateData* rd);	
	virtual std::string curStateName() { return "Start"; }
};

class ReachStateComplete : public ReachStateInterface
{
protected:
	float completeTime;
public:
	virtual void update(ReachStateData* rd);
	virtual void updateEffectorTargetState(ReachStateData* rd);	
	virtual std::string nextState(ReachStateData* rd);	
	virtual std::string curStateName() { return "Complete"; }
};

class ReachStateNewTarget : public ReachStateInterface
{
public:
	virtual void update(ReachStateData* rd);
	virtual void updateEffectorTargetState(ReachStateData* rd);	
	virtual std::string nextState(ReachStateData* rd);
	virtual std::string curStateName() { return "NewTarget"; }
};

class ReachStateReturn : public ReachStateInterface
{
public:
	virtual void update(ReachStateData* rd);
	virtual void updateEffectorTargetState(ReachStateData* rd);	
	virtual float curStatePercentTime(ReachStateData* rd, float refTime);
	virtual std::string nextState(ReachStateData* rd);
	virtual std::string curStateName() { return "Return"; }
};


