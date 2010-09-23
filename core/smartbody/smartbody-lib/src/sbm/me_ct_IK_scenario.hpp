/*
 *  me_ct_IK_scenario.hpp - part of SmartBody-lib's Motion Engine
 *  Copyright (C) 2009  University of Southern California
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
 *      Jingqiao Fu, USC
 */

#ifndef ME_CT_IK_SCENARIO_HPP
#define ME_CT_IK_SCENARIO_HPP


#include <ME/me_controller.h>
#include "me_ct_locomotion_func.hpp"

#pragma once

#define JOINT_TYPE_UNKNOWN 0
#define JOINT_TYPE_HINGE 1
#define JOINT_TYPE_BALL 2

#define ORIENTATION_RES_UNCHANGED_LOCAL 0
#define ORIENTATION_RES_PLANE 1
#define ORIENTATION_RES_UNCHANGED_WORLD 2

///////////////////////////////////////////////

class MeCtIKScenarioJointInfo
{
public:
	int			type; // JOINT_TYPE_HINGE or JOINT_TYPE_BALL
	int			index;
	SkJoint*	sk_joint;
	//SrVec offset;
	int			is_support_joint;
	float		support_joint_height;
	float		support_joint_comp;
	union
	{
		struct{float max;}ball;
		struct{float min; float max;}hinge;
	}constraint;
	SrVec		axis;

public:
	MeCtIKScenarioJointInfo();
	~MeCtIKScenarioJointInfo();
};


class MeCtIKScenario{
public:
	// Public Constants
	static const char* TYPE;

public:
	// Data
	SrMat				mat;
	MeCtIKScenarioJointInfo*			start_joint;
	MeCtIKScenarioJointInfo*			end_joint;
	
	SrArray<MeCtIKScenarioJointInfo>	joint_info_list;
	SrArray<SrQuat>						quat_list;

	//definition for plane
	SrVec								plane_normal;
	SrVec								plane_point;

	SrVec								ik_offset;	// offset from the current position to the position which is 
													// used to calculate the target position.

	//the forced direction of 
	SrVec								ik_orientation;

	float								ik_compentate;

/*//??
protected:
	SrVec								orientation;
	int									orientation_type;*/

public:
	MeCtIKScenario();
	~MeCtIKScenario();

public:
	//void set_orientation_of_residual_type(int type, SrVec* orientation = NULL);
	//int get_orientation_of_residual_type();
	//SrVec& get_orientation_of_residual();

	void set_plane_normal(SrVec& normal);
	void set_plane_point(SrVec& point);
	void set_offset(SrVec& offset);
};

#endif // ME_CT_IK_SCENARIO_HPP
