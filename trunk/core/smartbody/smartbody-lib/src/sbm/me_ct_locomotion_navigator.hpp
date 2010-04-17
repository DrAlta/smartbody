/*
 *  me_ct_locomotion_navigator.hpp - part of SmartBody-lib's Motion Engine
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

#ifndef ME_CT_LOCOMOTION_NAVIGATOR_HPP
#define ME_CT_LOCOMOTION_NAVIGATOR_HPP

//#include <ME/me_controller.h>
#include "me_ct_locomotion_limb.hpp"
#include "me_ct_locomotion_routine.hpp"
#include "me_ct_locomotion_speed_accelerator.hpp"
#include "me_ct_locomotion_limb_direction_planner.hpp"

#pragma once

class MeCtLocomotionNavigator{
public:
	// Public Constants
	static const char* TYPE;
	float standing_factor;
	float framerate_accelerator;
	SkChannelArray  request_channels;
	bool is_valid;

	SrVec base_offset;
	SrQuat base_rot;

protected:
	int bi_world_x, bi_world_y, bi_world_z, bi_world_rot; // World offset position and rotation
	int bi_loco_vel_x, bi_loco_vel_y, bi_loco_vel_z;      // Locomotion velocity
	int bi_loco_rot_global_y;                             // Rotational velocity around Y
	int bi_loco_rot_local_y;                             // Rotational velocity around Y
	int bi_id;											 // ID
	int bi_base_offset_x, bi_base_offset_y, bi_base_offset_z, bi_base_rot; // World offset position and rotation

	SrArray<MeCtLocomotionRoutine> routine_stack;

	SrVec target_world_pos;
	SrQuat target_world_rot;

	SrVec world_pos;
	SrQuat world_rot;

	SrVec local_vel;
	SrVec global_vel;
	SrVec displacement;

	float facing_angle;
	float pre_facing_angle;

	float local_rps;

	double delta_time;

	int routine_channel_num;


public:
	/** Constructor */
	MeCtLocomotionNavigator();

	/** Destructor */
	virtual ~MeCtLocomotionNavigator();

public:
	const char* controller_type();
	SkChannelArray& controller_channels();
	double controller_duration() { return -1; }
	void context_updated(MeControllerContext* _context);
	bool controller_map_updated(MeControllerContext* _context, SrBuffer<int>* _toContextCh);
	bool controller_evaluate(double delta_time, MeCtLocomotionLimbDirectionPlanner* direction_planner, MeCtLocomotionSpeedAccelerator* acc, MeFrameData& frame);
	void post_controller_evaluate(MeFrameData& frame, MeCtLocomotionLimb* limb, bool reset);
	SrVec get_local_velocity();
	float get_facing_angle();
	float get_pre_facing_angle();
	int controller_channels(SkChannelArray* request_channels);
	void CheckNewRoutine(MeFrameData& frame);
	void AddChannel(SkChannelArray* request_channels, const char* name, SkChannel::Type type);

public:
	void update_framerate_accelerator(float accelerator, SrArray<MeCtLocomotionLimb*>* limb_list);
	void update(SrBuffer<float>* buffer);
	void update_facing(MeCtLocomotionLimb* limb, bool dominant_limb);
	void update_displacement(SrVec* displacement);
	void AddRoutine(MeCtLocomotionRoutine& routine);
	void DelRoutine(char* name);

public://temp
	void print_foot_pos(MeFrameData& frame, MeCtLocomotionLimb* limb);
	//SrMat get_lmat (SkJoint* joint, SrQuat* quat);
};

#endif // ME_CT_LOCOMOTION_NAVIGATOR_HPP
