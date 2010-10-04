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
#include "me_ct_locomotion_height_offset.hpp"
#include "me_ct_locomotion_IK.hpp"
#include "me_ct_locomotion_balance.hpp"

class MeCtLocomotionLimb;

#pragma once


class MeCtLocomotion : public MeController {
public:
	// Public Constants
	static const char* TYPE;

public:
	MeCtLocomotionJointInfo nonlimb_joint_info;
	int translation_joint_index;

	SrArray<char*> limb_base_name;
	float pre_blended_base_height;
	float r_blended_base_height;

	SrVec pre_world_offset_to_base;
	SrString translation_joint_name;
	float translation_joint_height;

	//int base_index;

	int r_anim1_index;
	int r_anim2_index;
	int r_anim1_index_dominant;
	int r_anim2_index_dominant;
	int style;

	bool motions_loaded;

protected:
	MeCtLocomotionIK ik;

	MeCtLocomotionNavigator navigator;

	MeCtLocomotionHeightOffset height_offset;

	MeCtLocomotionBalance balance;

	MeCtLocomotionSpeedAccelerator speed_accelerator; // to be moved to MeCtLocomotionLimb

	SkChannelArray  request_channels;

	SkSkeleton* _skeleton_ref_p;

	SkMotion *_motion;

	SrArray<MeCtLocomotionAnimGlobalInfo*> anim_global_info;

	int dominant_limb;

	SrMat mat;

	bool is_valid;  // All necessary channels are present

	bool joints_indexed;

	//temp
	SrArray<SrQuat> t_joint_quats1;
	SrArray<SrQuat> t_joint_quats2;
	SrArray<SrQuat> joint_quats1;
	SrArray<SrQuat> joint_quats2;

	int limb_joint_num;

	double motion_time;

	float last_time;

	double last_t;
	double curr_t;

	double delta_time;

	char* base_name;
	//char* nonlimb_blending_base_name;

	bool dis_initialized; // limb joint positiona calculated
	bool initialized;

	int joint_channel_start_ind;

	//temp, to be deleted=================
public:
	SkSkeleton* walking_skeleton;
	SkSkeleton* standing_skeleton;
	float ratio;
	float dom_ratio;
	float abs_ground_height; // absolute height of base joint when standing.


	SrArray<MeCtLocomotionLimb*> limb_list; //limbs
	SrArray<SkMotion*> locomotion_anims;
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

public:
	MeCtLocomotionNavigator* get_navigator();

	void add_locomotion_anim(SkMotion* anim);

	SrArray<MeCtLocomotionAnimGlobalInfo*>* get_anim_global_info();

	SrVec get_facing_vector();

	void set_motion_time(float time);

	void init_skeleton(SkSkeleton* standing, SkSkeleton* walking);
	
	bool is_initialized();

	bool is_enabled();

	SrArray<MeCtLocomotionLimb*>* get_limb_list();

	void init_nonlimb_joint_info();

	void set_target_height_displacement(float displacement);

	void set_base_name(const char* name);

	void print_info(char* name);

	void set_balance_factor(float factor);

protected:

	void init_limbs();

	void get_translation_base_joint_index();

	void get_anim_indices(int limb_index, SrVec direction, int* anim1_index, int* anim2_index);

	void update_pos();

	SrVec get_limb_pos(MeCtLocomotionLimb* limb);

	void update_facing();

	void update_heading();

	char* get_base_name();

	SrVec get_heading_direction();

	void set_turning_speed(float radians);

	bool controller_evaluate( double time, MeFrameData& frame );

	void set_skeleton(SkSkeleton* skeleton);

	void analyze_motion( SkMotion* motion );

	void blend();

	void update(float inc_frame, MeFrameData& frame);

	void update_facing(MeCtLocomotionLimb* limb);

	void normalize_proportion();

	void blend_base_joint(MeFrameData& frame, float space_time, int anim_index1, int anim_index2, float weight);

	int get_dominant_limb();

	SrVec get_local_direction(SrVec* direction);

	int determine_dominant_limb();

	void apply_IK();

	//void set_nonlimb_blending_base_name(const char* name);

	int LOOKUP_BUFFER_INDEX(int var_name, int index );

	void update_limb_anim_standing();

	void update_limb_anim_standing(MeCtLocomotionLimbAnim* anim, int index, MeFrameData& frame);

	void update_nonlimb_mat();

	void update_nonlimb_mat(SkJoint* joint, SrMat* mat);

	void blend_standing(MeFrameData& frame);

	//float get_buffer_base_height(SrBuffer<float>& buffer);

	SrVec calc_rotational_displacement();

	void update_nonlimb_mat_with_global_info();

	void update_limb_mat_with_global_info();

};

#endif // ME_CT_LOCOMOTION_HPP
