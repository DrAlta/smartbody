#pragma once
#include "me_ct_data_interpolation.h"
#include "me_ct_barycentric_interpolation.h"
#include "me_ct_motion_parameter.h"
#include "me_ct_jacobian_IK.hpp"
#include "me_ct_ccd_IK.hpp"
#include "me_ct_constraint.hpp"

#include <SR/planner/sk_pos_planner.h>
#include <SR/planner/sk_blend_planner.h>
#include <SBM/Collision/SbmColObject.h>
#include <SBM/sbm_pawn.hpp>


using namespace std;

// used when we want exact control for an end effector
class EffectorConstantConstraint : public EffectorConstraint
{
public:		
	SrQuat          targetRot;
	SrVec           targetPos;	
public:
	EffectorConstantConstraint() {}
	~EffectorConstantConstraint() {}

	EffectorConstantConstraint& operator=(const EffectorConstantConstraint& rhs);

	virtual SrVec getPosConstraint() { return targetPos; }
	virtual SrQuat getRotConstraint()  { return targetRot; }
};

class MeCtExampleBodyReach :
	public MeController, public FadingControl
{
private:
	static const char* CONTROLLER_TYPE;
public:
	enum ReachState { REACH_START = 0, REACH_COMPLETE, REACH_RETURN, REACH_IDLE };
	enum GrabState { PICK_UP_OBJECT = 0, TOUCH_OBJECT, PUT_DOWN_OBJECT };
protected:
	std::string           characterName;
	SkSkeleton*           skeletonCopy, *skeletonRef;	
	SbmPawn*              reachTargetPawn;

	SrMat                 pawnAttachMat;
	SbmPawn*              attachedPawn;
	//SkJoint*              reachTargetJoint;		
	SrVec                 reachTargetPos;	
	vector<SkJoint*>      affectedJoints; // list of joints that are affected by motion interpolation & IK. 
	                                      // set to the full skeleton by default ( excluding fingers & face bones ).

	MotionDataSet         motionData;

	SkMotion*             refMotion;   // reference motion for time warping
	MotionExampleSet      motionExamples;
	
	// for motion interpolation
	MotionParameter*      motionParameter;		
	DataInterpolator*     dataInterpolator;
	BarycentricInterpolator* testDataInterpolator;
	ResampleMotion*       interpMotion; // pointer to motion interface for generating motion example
	ResampleMotion*       blendPlannerMotion;
	vector<InterpolationExample*>* interpExampleData;
	vector<InterpolationExample*>* resampleData;
	

	float                 reachTime; // local time for the reach motion	
	float                 reachCompleteTime; 
	float                 prev_time; // to get dt		
	double                prevPercentTime, curPercentTime;	

	bool                  finishReaching, startReaching, interactiveReach, sendGrabMsg;
	SrVec                 reachError, reachTarget, returnTarget;
	ReachState            curReachState;
	GrabState             curGrabState;
	SrVec                 curReachIKOffset;
	
	SkJoint*              reachEndEffector;
	ConstraintMap         reachPosConstraint;
	ConstraintMap         reachRotConstraint;
	ConstraintMap         reachNoRotConstraint;
	ConstraintMap         leftFootConstraint, rightFootConstraint;	

	ConstraintMap         handConstraint;

	BodyMotionFrame       inputMotionFrame,ikMotionFrame, idleMotionFrame, interpMotionFrame, initMotionFrame;
	BodyMotionFrame       interpStartFrame;

	double                ikDamp;
	float                 ikReachRegion, ikMaxOffset;
	MeCtJacobianIK        ik;
	MeCtCCDIK             ikCCD;
	MeCtIKTreeScenario    ikScenario, ikCCDScenario;
	SrBox                 paraBound;
	SkPosCfg              *cfgStart, *cfgEnd, *cfgPath;
	SkBlendCfg            *cfgBlendStart, *cfgBlendEnd, *cfgBlendPath;
	
	
	std::map<std::string, SbmColObject*> obstacleMap;
	std::map<std::string, SbmColObject*> jointColliderMap;
	float obstacleScale, planStep, planError;
	int   planNumTries;

	float 			_duration;
	SkChannelArray	_channels;

public:
	// these data are exposed directly for visualization/UI purpose	
	int                   simplexIndex;
	VecOfSimplex          simplexList;
	vector<SrVec>         examplePts;
	vector<SrVec>         resamplePts;
	SkPosPlanner*         posPlanner;
	SkBlendPlanner*       blendPlanner;

	// parameters and intermediate variables for debug/visualization
	SrVec                 curReachIKTrajectory, ikTarget, ikOffset, interpPos, curEffectorPos, currentInterpTarget, ikFootTarget;
	SrQuat                ikRotTarget, ikRotTrajectory, ikRotError;
	SrVec                 curOffsetDir;
	float                 reachVelocity, reachCompleteDuration;
	float                 characterHeight;
	bool                  useIKConstraint, useInterpolation, useTargetPawn;

public:
	MeCtExampleBodyReach(std::string charName, SkSkeleton* sk, SkJoint* effectorJoint);
	virtual ~MeCtExampleBodyReach(void);	

	void addObstacle(const char* name, SbmColObject* colObj);	
	void setGrabState(GrabState newState) { curGrabState = newState; }

	virtual void controller_map_updated();
	virtual void controller_start();	
	virtual bool controller_evaluate( double t, MeFrameData& frame );

	virtual SkChannelArray& controller_channels()	{ return( _channels ); }
	virtual double controller_duration()			{ return( (double)_duration ); }
	void set_duration(float duration) { _duration = duration; }
	virtual const char* controller_type() const		{ return( CONTROLLER_TYPE ); }
	virtual void print_state( int tabs );


	SrVec getIKTarget();
	void findReachTarget(SrVec& reachTarget, SrVec& reachError, double time);	
	bool updateInterpolation(float time,float dt, BodyMotionFrame& outFrame, float& du); 
	void updateIK(SrVec& rTrajectory, BodyMotionFrame& refFrame, BodyMotionFrame& outFrame);
	void updateState(double time);


	bool addHandConstraint(SkJoint* targetJoint, const char* effectorName);


	void setReachTargetPawn(SbmPawn* targetPawn);
	void setReachTargetJoint(SkJoint* val); 
	void setReachTargetPos(SrVec& targetPos);
	void setEndEffectorRoot(const char* rootName);	
	void setFinishReaching(bool isFinish) { finishReaching = isFinish; }
	void init();

	void updateMotionExamples(const MotionDataSet& inMotionSet);
protected:	
	bool updatePlannerPath(SrVec& curPos, SrVec& targetPos);
	bool updatePlannerBlendPath(VecOfInterpWeight& curWeight, float curRefTime, VecOfInterpWeight& targetWeight, float targetRefTime);
	void updateSkeletonCopy();
	void updateChannelBuffer(MeFrameData& frame, BodyMotionFrame& motionFrame, bool bRead = false);
	
	SrVec getCurrentHandPos(BodyMotionFrame& motionFrame);
	SkJoint* findRootJoint(SkSkeleton* sk);
	DataInterpolator* createInterpolator();
	ResampleMotion*   createInterpMotion();
};







