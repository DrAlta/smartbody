#pragma once
#include <SK/sk_skeleton.h>
#include <ME/me_controller.h>
#include "me_ct_limb.hpp"
#include "me_ct_reach_IK.hpp"
#include "gwiz_math.h"

class MeCtReach : public MeController
{
private:
	static const char* CONTROLLER_TYPE;

public:
	enum ReachMode { TARGET_POS = 1, TARGET_JOINT_POS };	
	enum ReachArm { REACH_LEFT_ARM = 1, REACH_RIGHT_ARM };

public:	
	MeCtReach(SkSkeleton* skeleton);
	~MeCtReach(void);

protected:
	ReachMode            reach_mode;
	ReachArm             reach_arm;	
	MeCtReachIK          ik;
	MeCtLimb             limb;
	float                limb_length;	
	SrArray<const char*> joint_name;	
	SrArray<MeCtIKJointLimit>        joint_limit;
	float 			_duration;
	SkChannelArray	_channels;
	SrVec   		target_pos;
	SkJoint        *target_joint_ref;
	SkJoint        *root_joint_ref; // root of target chain	
	SkSkeleton*     _skeleton;
	float           prev_time; // to get dt


public:	
	void set_target_pos(SrVec& target_pos);
	void set_target_joint(SkJoint* target_joint);	

	void init ();
	virtual void controller_map_updated();
	virtual void controller_start();	
	virtual bool controller_evaluate( double t, MeFrameData& frame );
	virtual SkChannelArray& controller_channels()	{ return( _channels ); }
	virtual double controller_duration()			{ return( (double)_duration ); }
	void set_duration(float duration) { _duration = duration; }
	virtual const char* controller_type() const		{ return( CONTROLLER_TYPE ); }
	virtual void print_state( int tabs );	

	void setReachArm(MeCtReach::ReachArm val) { reach_arm = val; }
	MeCtReach::ReachArm getReachArm() { return reach_arm; }
	float getLimbLength() const { return limb_length; }
	SkJoint* getRootJoint() const { return root_joint_ref; }

protected:
	SrVec get_reach_target();
};
