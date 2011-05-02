#pragma once
#include "me_ct_data_interpolation.h"
#include "me_ct_barycentric_interpolation.h"
#include "me_ct_motion_parameter.h"
#include "me_ct_jacobian_IK.hpp"
#include "me_ct_ccd_IK.hpp"
#include "me_ct_constraint.hpp"

#include <SR/planner/sk_pos_planner.h>
#include <SBM/Collision/SbmColObject.h>


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
protected:
	SkSkeleton*           skeletonCopy, *skeletonRef;	
	SkJoint*              reachTargetJoint;		
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
	vector<InterpolationExample*>* interpExampleData;
	vector<InterpolationExample*>* resampleData;
	

	float                 reachTime; // local time for the reach motion	
	float                 reachCompleteTime; 
	float                 prev_time; // to get dt		
	double                prevPercentTime, curPercentTime;	

	bool                  finishReaching, startReaching, interactiveReach;
	SrVec                 reachError, reachTarget, returnTarget;
	ReachState            curReachState;
	SrVec                 curReachIKOffset;
	
	SkJoint*              reachEndEffector;
	ConstraintMap         reachPosConstraint;
	ConstraintMap         reachRotConstraint;
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
	
	
	std::map<std::string, SbmColObject*> obstacleMap;
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

	// parameters and intermediate variables for debug/visualization
	SrVec                 curReachIKTrajectory, ikTarget, interpPos, curEffectorPos, currentInterpTarget, ikFootTarget;
	SrVec                 curOffsetDir;
	float                 reachVelocity, reachCompleteDuration;
	float                 characterHeight;
	bool                  useIKConstraint, useInterpolation, useTargetJoint;

public:
	MeCtExampleBodyReach(SkSkeleton* sk, SkJoint* effectorJoint);
	virtual ~MeCtExampleBodyReach(void);	

	void addObstacle(const char* name, SbmColObject* colObj);

	virtual void controller_map_updated();
	virtual void controller_start();	
	virtual bool controller_evaluate( double t, MeFrameData& frame );

	virtual SkChannelArray& controller_channels()	{ return( _channels ); }
	virtual double controller_duration()			{ return( (double)_duration ); }
	void set_duration(float duration) { _duration = duration; }
	virtual const char* controller_type() const		{ return( CONTROLLER_TYPE ); }
	virtual void print_state( int tabs );


	void findReachTarget(SrVec& reachTarget, SrVec& reachError);	
	bool updateInterpolation(float dt, BodyMotionFrame& outFrame, float& du); 
	void updateIK(SrVec& rTrajectory, BodyMotionFrame& refFrame, BodyMotionFrame& outFrame);
	void updateState();


	bool addHandConstraint(SkJoint* targetJoint, const char* effectorName);

	void setReachTargetJoint(SkJoint* val); 
	void setReachTargetPos(SrVec& targetPos);
	void setEndEffectorRoot(const char* rootName);	
	void setFinishReaching(bool isFinish) { finishReaching = isFinish; }
	void init();

	void updateMotionExamples(const MotionDataSet& inMotionSet);
protected:	
	bool updatePlannerPath(SrVec& curPos, SrVec& targetPos);
	void updateSkeletonCopy();
	void updateChannelBuffer(MeFrameData& frame, BodyMotionFrame& motionFrame, bool bRead = false);
	SrVec getCurrentHandPos(BodyMotionFrame& motionFrame);
	SkJoint* findRootJoint(SkSkeleton* sk);
	DataInterpolator* createInterpolator();
	ResampleMotion*   createInterpMotion();
};







