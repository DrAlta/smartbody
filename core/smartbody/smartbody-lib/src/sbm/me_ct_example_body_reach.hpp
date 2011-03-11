#pragma once
#include "me_ct_data_interpolation.h"
#include "me_ct_motion_parameter.h"
#include "me_ct_data_driven_reach.hpp"
#include "me_ct_jacobian_IK.hpp"

using namespace std;


class MeCtExampleBodyReach :
	public MeController
{
private:
	static const char* CONTROLLER_TYPE;
protected:
	SkSkeleton*           skeletonRef;
	
	SkJoint*              reachTargetJoint;		
	vector<SkJoint*>      affectedJoints; // list of joints that are affected by motion interpolation & IK. 
	                                      // set to the full skeleton by default ( excluding fingers & face bones ).

	MotionDataSet         motionData;
	SkMotion*             refMotion;

	MotionExampleSet      motionExamples;
	MotionParameter*      motionParameter;	
	
	DataInterpolator*     dataInterpolator;
	ResampleMotion*       interpMotion; // pointer to motion interface for generating motion example

	VecOfInterpWeight     blendWeight;

	vector<InterpolationExample*>* interpExampleData;
	vector<InterpolationExample*>* resampleData;
	
	float                 reachTime; // local time for the reach motion	
	float                 prev_time; // to get dt
	float                 reachRefTime;

	double                prevPercentTime, curPercentTime;
	SrVec                 reachError;
	SrVec                 curReachOffset;
	
	SkJoint*              reachEndEffector;
	MeCtJacobianIK        ik;
	MeCtIKTreeScenario    ikScenario;
	

	float 			_duration;
	SkChannelArray	_channels;

public:
	// these data are exposed directly for visualization/UI purpose
	vector<SrVec>         examplePts;
	vector<SrVec>         resamplePts;
	SrVec                 curReachTrajectory;
	bool                  useIKConstraint;

public:
	MeCtExampleBodyReach(SkSkeleton* sk);
	~MeCtExampleBodyReach(void);	

	virtual void controller_map_updated();
	virtual void controller_start();	
	virtual bool controller_evaluate( double t, MeFrameData& frame );

	virtual SkChannelArray& controller_channels()	{ return( _channels ); }
	virtual double controller_duration()			{ return( (double)_duration ); }
	void set_duration(float duration) { _duration = duration; }
	virtual const char* controller_type() const		{ return( CONTROLLER_TYPE ); }
	virtual void print_state( int tabs );
	
	void setReachTarget(SrVec& reachPos);
	void setReachTargetJoint(SkJoint* val) { reachTargetJoint = val; }
	void init();
	void updateMotionExamples(const MotionDataSet& inMotionSet);
protected:
	// get motion parameter at t, where t is the reference time
	void getParameter(VecOfSrQuat& quatList, float time, VecOfDouble& outPara);
	void updateChannelBuffer(MeFrameData& frame, BodyMotionFrame& motionFrame, bool bRead = false);
	DataInterpolator* createInterpolator();
	ResampleMotion*   createInterpMotion();
};







