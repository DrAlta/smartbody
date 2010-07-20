/*
 *  me_ct_locomotion.hpp - part of SmartBody-lib's Motion Engine
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

#ifndef ME_CT_LOCOMOTION_HPP
#define ME_CT_LOCOMOTION_HPP

#include <ME/me_controller.h>

#include "me_ct_locomotion_limb.hpp"
#include "me_ct_locomotion_speed_accelerator.hpp"
#include "me_ct_locomotion_navigator.hpp"
#include "me_ct_locomotion_func.hpp"
#include "me_ct_locomotion_joint_info.hpp"
#include "me_ct_IK.hpp"


class MeCtLocomotionLimb;

#pragma once

/*struct MeCtLocomotionJointInfo
{
	int joint_num;
	SrArray<const char*> joint_name;
	SrArray<int> buff_index;
	SrArray<int> joint_index;
	SrArray<SrQuat> quat;

	int iterate(SkJoint* joint, SrArray<char*>* limb_joint_name)
	{
		int sum = 0;
		const char* name = joint->name().get_string();
		if(limb_joint_name != NULL)
		{
			for(int i = 0; i < limb_joint_name->size(); ++i)
			{
				if(strcmp(limb_joint_name->get(i), name) == 0) return 0;
			}
		}
		if(joint->quat()->active() == true)
		{
			joint_name.push() = name;
			joint_index.push() = joint->index();
			sum = 1;
		}

		for(int i = 0; i < joint->num_children(); ++i)
		{
			sum += iterate(joint->child(i), limb_joint_name);
		}
		return sum;
	}

	void Init(SkSkeleton* skeleton, char* base_name, SrArray<char*>* limb_joint_name)
	{
		SkJoint* tjoint = skeleton->search_joint(base_name);
		joint_num = iterate(tjoint, limb_joint_name);
		quat.capacity(joint_num);
		quat.size(joint_num);
		buff_index.capacity(joint_num);
		buff_index.size(joint_num);
	}
};*/


class MeCtLocomotion : public MeController {
public:
	// Public Constants
	static const char* TYPE;

public:
	//SrArray<int>	nonlimb_joint_index;
	MeCtLocomotionJointInfo nonlimb_joint_info;
	SrArray<char*> limb_base_name;

protected:

	MeCtIK ik;
	// Data
	MeCtLocomotionNavigator navigator;
	MeCtLocomotionSpeedAccelerator speed_accelerator; // to be moved to MeCtLocomotionLimb

	SkChannelArray  request_channels;

	SkSkeleton* _skeleton_ref_p;

	SkMotion *_motion;

	SrArray<MeCtLocomotionAnimGlobalInfo*> anim_global_info;

	SrArray<SkMotion*> locomotion_anims;

	int dominant_limb;

	SrMat mat;

	bool is_valid;  // All necessary channels are present
	
	//SrQuat base_quat;

	bool joints_indexed;
	//SrArray<MeCtLocomotionJointInfo> nonlimb_joint_info;
	//SrArray<int> nonlimb_joint_buff_index;
	//int nonlimb_joint_num;
	//SrArray<SrQuat> nonlimb_joint_quats;



	//temp
	SrArray<SrQuat> t_joint_quats1;
	SrArray<SrQuat> t_joint_quats2;
	SrArray<SrQuat> joint_quats1;
	SrArray<SrQuat> joint_quats2;

	//SrArray<int> limb_joint_index;
	int limb_joint_num;

	float last_time;
	float last_t;
	float curr_t;

	char* base_name;
	char* nonlimb_blending_base_name;

	bool dis_initialized;
	bool initialized;

	int joint_channel_start_ind;

	//temp, to be deleted=================
public:
	SkSkeleton* walking_skeleton;
	SkSkeleton* standing_skeleton;
	float ratio;
	float dom_ratio;

	SrArray<MeCtLocomotionLimb*> limb_list; //limbs
	
	//SrVec displacement;

	bool automate;
	bool reset;
	bool ik_enabled;
	bool enabled;

	int temp;
	//SrArray<const char*> joint_name;

public:
	/** Constructor */
	MeCtLocomotion();

	/** Destructor */
	virtual ~MeCtLocomotion();

	const char* controller_type() const;


	/**
	 *  Implements MeController::controller_channels().
	 */
	SkChannelArray& controller_channels();

	int iterate_limb_joints(SkJoint* base);

	void iterate_joints(MeCtLocomotionJointInfo* joint_info);

	int iterate_limb_joints(SkJoint* base, int depth);

	int iterate_nonlimb_joints(SkJoint* base, int depth);

	/**
	 *  Implements MeController::controller_duration().  -1 means indefinite.
	 */
	double controller_duration() { return -1; }

	/*!
	 *  Implements MeController::context_updated(..).
	 */
	virtual void context_updated();

	/*!
	 *  Implements MeController::controller_map_updated(..).
	 *  Save channel indices after context remap.
	 */
	virtual void controller_map_updated();

	/**
	 *  Implements MeController::controller_evaluate(..).
	 */

	SrArray<MeCtLocomotionLimb*>* get_limb_list();

	SrArray<MeCtLocomotionAnimGlobalInfo*>* get_anim_global_info();

	void init_limbs();

	void init_skeleton(SkSkeleton* standing, SkSkeleton* walking);

	void init_nonlimb_joint_info();


	void update_pos();

	SrVec get_limb_pos(MeCtLocomotionLimb* limb);

	void update_facing();

	void update_heading();

	SrVec get_facing_vector();

	char* get_base_name();

	SrVec get_heading_direction();

	void set_turning_speed(float radians);

	bool controller_evaluate( double time, MeFrameData& frame );

	void set_skeleton(SkSkeleton* skeleton);

	void analyze_motion( SkMotion* motion );

	void blend();

	void update(float inc_frame);

	void update_facing(MeCtLocomotionLimb* limb);

	void normalize_proportion();

	void orthonormal_decompose(const SrVec& svec, const SrVec& direction, SrVec* dvec1, SrVec* dvec2);

	//temp. to be deleted or modified...
	//SrMat get_lmat (SkJoint* joint, SrQuat* quat);

	int get_dominant_limb();

	SrVec get_local_direction(SrVec* direction);

	int determine_dominant_limb();

	void get_IK();

	MeCtLocomotionNavigator* get_navigator();

	bool is_initialized();

	bool is_enabled();

	void set_base_name(const char* name);

	void set_nonlimb_blending_base_name(const char* name);

	void add_locomotion_anim(SkMotion* anim);

	int LOOKUP_BUFFER_INDEX(int var_name, int index );

	void update_limb_anim_standing();

	void update_limb_anim_standing(MeCtLocomotionLimbAnim* anim, int index, MeFrameData& frame);
};

#endif // ME_CT_LOCOMOTION_HPP
