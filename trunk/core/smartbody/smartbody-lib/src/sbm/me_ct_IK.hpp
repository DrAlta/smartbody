/*
 *  me_ct_IK.hpp - part of SmartBody-lib's Motion Engine
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

#ifndef ME_CT_IK_HPP
#define ME_CT_IK_HPP


#include <ME/me_controller.h>
#include "me_ct_locomotion_func.hpp"
#include "me_ct_IK_scenario.hpp"

#pragma once


///////////////////////////////////////////////

class MeCtIK{
public:
	// Public Constants
	static const char* TYPE;


protected:
	// Data
	MeCtIKScenario*				scenario;
	SkJoint*					base_joint;
	int							max_iteration;
	SrArray<SrVec>				joint_pos_list;
	SrArray<SrVec>				joint_axis_list;
	SrArray<SrMat>				joint_global_mat_list;
	//SrArray<SrMat>			joint_local_mat_list;

	SrArray<SrMat>				joint_init_mat_list;

	SrMat						end_mat;
	
	SrVec						target;
	float						threshold;

	MeCtIKScenarioJointInfo*	manipulated_joint;
	int							manipulated_joint_index;
	int							support_joint_num;

	//temp
	int							recrod_endmat;
	SrMat						lm;
	SrMat						pm;

	

public:
	MeCtIK();
	~MeCtIK();

public:
	void update(MeCtIKScenario* scenario);
	SrVec get_joint_pos(int index);

protected:
	void set_max_iteration(int iter);

	SrVec upright_point_to_plane(SrVec& point, SrVec& plane_normal, SrVec& plane_point);

	float distance_to_plane(SrVec& point, SrVec& plane_normal, SrVec& plane_point);

	bool cross_point_with_plane(SrVec* cross_point, SrVec& line_point, SrVec& direction, SrVec& plane_normal, SrVec& plane_point);

	SrVec cross_point_on_plane(SrVec& point, SrVec& line, SrVec& plane_normal, SrVec& plane_point);

	void update_limb_section_local_pos(int start_index);

	void rotate(SrVec& src, int start_index);

	void get_limb_section_local_pos(int start_index, int end_index);
	
	void init();

	int reach_destination();

	int check_constraint(SrQuat* quat, int index);

	void get_next_support_joint();

	int get_support_joint_num();

	//void adjust_support_joints();

	void calc_target(SrVec& orientation, SrVec& offset);

	void get_init_mat_list();

	void update_manipulated_joint_pos(int index);

};

#endif // ME_CT_IK_HPP
