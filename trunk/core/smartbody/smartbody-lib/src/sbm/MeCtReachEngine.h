#pragma once
#include "me_ct_data_interpolation.h"
#include "me_ct_barycentric_interpolation.h"
#include "me_ct_motion_parameter.h"
#include "me_ct_jacobian_IK.hpp"
#include "me_ct_ccd_IK.hpp"
#include "me_ct_constraint.hpp"
#include <SBM/sbm_pawn.hpp>

class SbmCharacter;
class ReachStateData;
class ReachStateInterface;
class ReachHandAction;

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

class MeCtReachEngine
{
public:
	enum HandActionState { PICK_UP_OBJECT = 0, TOUCH_OBJECT, PUT_DOWN_OBJECT };
protected:
	SbmCharacter* character;
	SkSkeleton*   skeletonCopy, *skeletonRef;
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
	std::map<HandActionState,ReachHandAction*> handActionTable;
	ReachStateInterface*  curReachState;	
	

	SkJoint*              reachEndEffector;
	ConstraintMap         reachPosConstraint;
	ConstraintMap         reachRotConstraint;
	ConstraintMap         reachNoRotConstraint;

	ConstraintMap         leftFootConstraint, rightFootConstraint;	
	ConstraintMap         handConstraint;

	BodyMotionFrame       inputMotionFrame,ikMotionFrame, idleMotionFrame;

	bool                  initStart;
	double                ikDamp;
	float                 ikReachRegion, ikMaxOffset, ikDefaultVelocity;
	float                 reachCompleteDuration;
	MeCtJacobianIK        ik;
	MeCtCCDIK             ikCCD;
	MeCtIKTreeScenario    ikScenario, ikCCDScenario;
	ReachStateData*       reachData;	
public:
	vector<SrVec>         examplePts,resamplePts;
	HandActionState       curHandActionState;
	bool                  footIKFix;

public:
	MeCtReachEngine(SbmCharacter* sbmChar, SkSkeleton* sk, SkJoint* effector);
	virtual ~MeCtReachEngine(void);
	ReachStateData* getReachData() { return reachData; }
	MotionParameter* getMotionParameter() { return motionParameter; }
	BodyMotionFrame& outputMotion() { return ikMotionFrame; }
	IKTreeNodeList& ikTreeNodes() { return ikScenario.ikTreeNodes; }
	ConstraintMap&  getHandConstraint() { return handConstraint; }
	bool addHandConstraint(SkJoint* targetJoint, const char* effectorName);

	void updateReach(float t, float dt, BodyMotionFrame& inputFrame);
	void init();
	void updateMotionExamples(const MotionDataSet& inMotionSet);
	void solveIK(ReachStateData* rd, BodyMotionFrame& outFrame );
protected:
	void updateSkeletonCopy();
	ReachStateInterface* getState(std::string stateName);
	SkJoint* findRootJoint(SkSkeleton* sk);
	DataInterpolator* createInterpolator();
	ResampleMotion*   createInterpMotion();	
	bool hasEffectorRotConstraint(ReachStateData* rd);
};

