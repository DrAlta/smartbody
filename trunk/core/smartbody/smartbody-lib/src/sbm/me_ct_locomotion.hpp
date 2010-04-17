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
#include "me_ct_IK.hpp"


class MeCtLocomotionLimb;

#pragma once


class MeCtLocomotion : public MeController {
public:
	// Public Constants
	static const char* TYPE;

protected:

	MeCtIK ik;
	// Data
	MeCtLocomotionNavigator navigator;
	MeCtLocomotionSpeedAccelerator speed_accelerator; // to be moved to MeCtLocomotionLimb

	SkChannelArray  request_channels;

	SkSkeleton* _skeleton_ref_p;

	SkMotion *_motion;

	SrArray<MeCtLocomotionAnimGlobalInfo*> anim_global_info;

	int dominant_limb;

	//float facing_angle;
	//float pre_facing_angle;
	//float heading_angle;

	SrMat mat;

	//float turning_speed; // circles per second.
	//float turning_speed_limit; // temp

	bool is_valid;  // All necessary channels are present
	bool is_initialized;

	// Buffer indices ("bi_") to the requested channels
	//int bi_world_x, bi_world_y, bi_world_z, bi_world_rot; // World offset position and rotation
	//int bi_loco_vel_x, bi_loco_vel_y, bi_loco_vel_z;      // Locomotion velocity
	//int bi_loco_rot_y;                                    // Rotational velocity around Y

	
	SrQuat base_quat;

	SrArray<int> bi_joint_quats;
	int joint_num;

	float last_time;
	float last_t;
	float curr_t;

	char* base_name;

	//SrVec world_pos;
	//SrVec loco_vel;

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

	int temp;

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

	int iterate_children(SkJoint* base);

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

	bool is_enabled();

	SrArray<MeCtLocomotionLimb*>* get_limb_list();

	SrArray<MeCtLocomotionAnimGlobalInfo*>* get_anim_global_info();

	void init_limbs();

	void init_skeleton(SkSkeleton* standing, SkSkeleton* walking);

	void update_pos();

	SrVec get_limb_pos(MeCtLocomotionLimb* limb);

	void update_facing();

	void update_heading();

	SrVec get_facing();

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
};

#endif // ME_CT_LOCOMOTION_HPP
