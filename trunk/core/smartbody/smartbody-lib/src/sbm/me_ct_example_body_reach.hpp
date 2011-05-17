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

class ReachStateData;
class ReachStateInterface;
class ReachHandAction;

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
	enum GrabStateID { PICK_UP_OBJECT = 0, TOUCH_OBJECT, PUT_DOWN_OBJECT };
protected:
	std::string           characterName;
	SkSkeleton*           skeletonCopy, *skeletonRef;
	MotionDataSet         motionData;
	SkMotion*             refMotion;   // reference motion for time warping
	MotionExampleSet      motionExamples;

	// for motion interpolation
	MotionParameter*      motionParameter;		
	DataInterpolator*     dataInterpolator;
	ResampleMotion*       interpMotion; // pointer to motion interface for generating motion example

	vector<InterpolationExample*>* interpExampleData;
	vector<InterpolationExample*>* resampleData;
	vector<SkJoint*>      affectedJoints; // list of joints that are affected by motion interpolation & IK. 
	                                      // set to the full skeleton by default ( excluding fingers & face bones ).

	std::map<std::string,ReachStateInterface*> stateTable;
	std::map<GrabStateID,ReachHandAction*>     handActionTable;
	
	ReachStateInterface*  curState;
	
	GrabStateID           curGrabState;

	float                 reachCompleteTime; 
	float                 prev_time; // to get dt
	bool                  interactiveReach;	
		
	SkJoint*              reachEndEffector;
	ConstraintMap         reachPosConstraint;
	ConstraintMap         reachRotConstraint;
	ConstraintMap         reachNoRotConstraint;

	ConstraintMap         leftFootConstraint, rightFootConstraint;	
	ConstraintMap         handConstraint;

	BodyMotionFrame       inputMotionFrame,ikMotionFrame, idleMotionFrame;
	

	double                ikDamp;
	float                 ikReachRegion, ikMaxOffset;
	MeCtJacobianIK        ik;
	MeCtCCDIK             ikCCD;
	MeCtIKTreeScenario    ikScenario, ikCCDScenario;	

	float 			_duration;
	SkChannelArray	_channels;

public:
	// these data are exposed directly for visualization/UI purpose	
	vector<SrVec>         examplePts;
	vector<SrVec>         resamplePts;

	ReachStateData*       reachData;	
	// parameters and intermediate variables for debug/visualization
	SrVec                 curReachIKTrajectory, ikTarget, ikOffset, interpPos, curEffectorPos, currentInterpTarget, ikFootTarget;
		
	float                 defaultVelocity, reachCompleteDuration;
	float                 characterHeight;

public:
	MeCtExampleBodyReach(std::string charName, SkSkeleton* sk, SkJoint* effectorJoint);
	virtual ~MeCtExampleBodyReach(void);	

	void setGrabState(GrabStateID newState);
	void setLinearVelocity(float vel);

	virtual void controller_map_updated();
	virtual void controller_start();	
	virtual bool controller_evaluate( double t, MeFrameData& frame );

	virtual SkChannelArray& controller_channels()	{ return( _channels ); }
	virtual double controller_duration()			{ return( (double)_duration ); }
	void set_duration(float duration) { _duration = duration; }
	virtual const char* controller_type() const		{ return( CONTROLLER_TYPE ); }
	virtual void print_state( int tabs );

	
	void solveIK(ReachStateData* rd, BodyMotionFrame& outFrame );
	void setReachTargetPawn(SbmPawn* targetPawn);	
	void setReachTargetPos(SrVec& targetPos);
	void setEndEffectorRoot(const char* rootName);	
	void setFinishReaching(bool isFinish);
	void init();

	void updateMotionExamples(const MotionDataSet& inMotionSet);
protected:		
	void updateSkeletonCopy();
	void updateChannelBuffer(MeFrameData& frame, BodyMotionFrame& motionFrame, bool bRead = false);		
	ReachStateInterface* getState(std::string stateName);
	SkJoint* findRootJoint(SkSkeleton* sk);
	DataInterpolator* createInterpolator();
	ResampleMotion*   createInterpMotion();
};







