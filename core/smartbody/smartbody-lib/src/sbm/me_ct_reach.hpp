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
	MeCtReach(void);
	~MeCtReach(void);

protected:

	enum ReachMode { TARGET_POS = 1, TARGET_JOINT_POS };

	ReachMode            reach_mode;
	MeCtReachIK          ik;
	MeCtLimb             limb;
	SrArray<const char*> joint_name;
	SrArray<MeCtLimbJointConstraint> joint_constraint;
	float 			_duration;
	SkChannelArray	_channels;
	SrVec   		target_pos;
	SkJoint        *target_joint_ref;


public:	
	void set_target_pos(SrVec& target_pos);
	void set_target_joint(SkJoint* target_joint);

	void init ();
	virtual void controller_map_updated();
	virtual void controller_start();	
	virtual bool controller_evaluate( double t, MeFrameData& frame );
	virtual SkChannelArray& controller_channels()	{ return( _channels ); }
	virtual double controller_duration()			{ return( (double)_duration ); }
	virtual const char* controller_type() const		{ return( CONTROLLER_TYPE ); }
	virtual void print_state( int tabs );

protected:
	SrVec get_reach_target();
};
