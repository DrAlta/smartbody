#pragma once
#include "me_ct_data_interpolation.h"
//#include "me_ct_barycentric_interpolation.h"
#include "me_ct_motion_parameter.h"
#include "me_ct_jacobian_IK.hpp"
#include "me_ct_constraint.hpp"

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
	//BarycentricInterpolator* testDataInterpolator;
	ResampleMotion*       interpMotion; // pointer to motion interface for generating motion example	
	vector<InterpolationExample*>* interpExampleData;
	vector<InterpolationExample*>* resampleData;
	

	float                 reachTime; // local time for the reach motion	
	float                 reachCompleteTime; 
	float                 prev_time; // to get dt		
	double                prevPercentTime, curPercentTime;	

	bool                  finishReaching, startReaching;
	SrVec                 reachError, reachTarget, returnTarget;
	ReachState            curReachState;
	SrVec                 curReachIKOffset;
	
	SkJoint*              reachEndEffector;
	ConstraintMap         reachPosConstraint;
	ConstraintMap         reachRotConstraint;
	BodyMotionFrame       inputMotionFrame,ikMotionFrame, idleMotionFrame, interpMotionFrame, initMotionFrame;
	BodyMotionFrame       interpStartFrame;

	double                ikDamp;
	float                 ikReachRegion, ikMaxOffset;
	MeCtJacobianIK        ik;
	MeCtIKTreeScenario    ikScenario;
	

	float 			_duration;
	SkChannelArray	_channels;

public:
	// these data are exposed directly for visualization/UI purpose	
	int                   simplexIndex;
	//VecOfSimplex          simplexList;
	vector<SrVec>         examplePts;
	vector<SrVec>         resamplePts;
	
	// parameters and intermediate variables for debug/visualization
	SrVec                 curReachIKTrajectory, ikTarget, interpPos, curEffectorPos, currentInterpTarget, ikRoot;
	float                 reachVelocity, reachCompleteDuration;
	float                 characterHeight;
	bool                  useIKConstraint, useInterpolation, useTargetJoint;

public:
	MeCtExampleBodyReach(SkSkeleton* sk, SkJoint* effectorJoint);
	~MeCtExampleBodyReach(void);	

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


	void setReachTargetJoint(SkJoint* val); 
	void setReachTargetPos(SrVec& targetPos);
	void setEndEffectorRoot(const char* rootName);	
	void setFinishReaching(bool isFinish) { finishReaching = isFinish; }
	void init();

	void updateMotionExamples(const MotionDataSet& inMotionSet);
protected:	
	void updateSkeletonCopy();
	void updateChannelBuffer(MeFrameData& frame, BodyMotionFrame& motionFrame, bool bRead = false);
	SrVec getCurrentHandPos(BodyMotionFrame& motionFrame);
	DataInterpolator* createInterpolator();
	ResampleMotion*   createInterpMotion();
};







