/*
*  me_ct_constraint.h - part of Motion Engine and SmartBody-lib
*  Copyright (C) 2008  University of Southern California
*
*  SmartBody-lib is free software: you can redistribute it and/or
*  modify it under the terms of the Lesser GNU General Public License
*  as published by the Free Software Foundation, version 3 of the
*  license.
*
*  SmartBody-lib is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  Lesser GNU General Public License for more details.
*
*  You should have received a copy of the Lesser GNU General Public
*  License along with SmartBody-lib.  If not, see:
*      http://www.gnu.org/licenses/lgpl-3.0.txt
*
*  CONTRIBUTORS:
*      Wei-Wen Feng, USC
*/

#pragma once
#include <SK/sk_skeleton.h>
#include <ME/me_controller.h>
#include "me_ct_limb.hpp"
#include "me_ct_jacobian_IK.hpp"
#include "gwiz_math.h"

typedef std::set<SkMotion*> MotionDataSet;


class MeCtConstraint : public MeController
{
private:
	static const char* CONTROLLER_TYPE;
public:	
	static bool useBalance, useReferenceJoint, useIKConstraint;

	enum ConstraintFadeMode	{
		FADING_MODE_OFF = 0,
		FADING_MODE_IN,
		FADING_MODE_OUT
	};
public:	
	MeCtConstraint(SkSkeleton* skeleton);
	~MeCtConstraint(void);

protected:	
	MeCtJacobianIK       ik;
	MeCtIKTreeScenario   ik_scenario;
	SkSkeleton*     _skeleton;
	float 			_duration;
	SkChannelArray	_channels;
	SkJoint        *target_joint_ref;
	std::vector<SkJoint*> targetJointList;
		
	float           prev_time; // to get dt
	float           blendWeight;
	float           fadeRemainTime, fadeInterval;
	ConstraintFadeMode fadeMode;

public:			
	void init ();
	bool addJointEndEffectorPair(SkJoint* targetJoint, const char* effectorName);
	virtual void controller_map_updated();
	virtual void controller_start();	
	virtual bool controller_evaluate( double t, MeFrameData& frame );
	virtual SkChannelArray& controller_channels()	{ return( _channels ); }
	virtual double controller_duration()			{ return( (double)_duration ); }
	void set_duration(float duration) { _duration = duration; }
	virtual const char* controller_type() const		{ return( CONTROLLER_TYPE ); }
	virtual void print_state( int tabs );

	void setFadeIn(float interval);
	void setFadeOut(float interval);
	bool updateFading(float dt);

protected:
	SrVec get_reach_target(SkJoint* joint);
	void  updateChannelBuffer(MeFrameData& frame, std::vector<SrQuat>& quatList, bool bRead = false);
};
