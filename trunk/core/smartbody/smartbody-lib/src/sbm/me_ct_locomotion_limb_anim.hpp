/*
 *  me_locomotion_limb_anim.hpp - part of SmartBody-lib's Motion Engine
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

#ifndef ME_LOCOMOTION_LIMB_ANIM_HPP
#define ME_LOCOMOTION_LIMB_ANIM_HPP

#include "me_ct_locomotion_timing_space.hpp"
#include "me_ct_locomotion_anim_global_info.hpp"
#include <SK/sk_motion.h>

#pragma once

class MeCtLocomotionLimbAnim{
public:
	// Public Constants
	static const char* TYPE;

protected:
	// Data

	SkMotion*			walking;
	SrArray<SrString*>*	support_joint_list;
	SrVec				pos;
	bool				is_blended;
	MeCtLocomotionTimingSpace timing_space;
	

	//temp, to be deleted=================
public:
	SkSkeleton* walking_skeleton;
	SkSkeleton* standing_skeleton;

	MeCtLocomotionAnimGlobalInfo* global_info;

	SrArray<SrVec*>		displacement_list;
	SrArray<SrQuat>		quat_buffer;
	SrVec				local_direction;
	float				local_speed; //local walking speed of the limb.
	float				displacement;

protected:
	SrArray<SrQuat>		quat_buffer_key_frame1; // for key-frame interpolation
	SrArray<SrQuat>		quat_buffer_key_frame2; // for key-frame interpolation

public:
	MeCtLocomotionLimbAnim();
	~MeCtLocomotionLimbAnim();

	void					set_anim(SkMotion* walking);

	void					init_skeleton(SkSkeleton* standing_skeleton, SkSkeleton* walking_skeleton);

	void					init_quat_buffers(int num);

	void					set_skeleton_name(char* name);

	// support joint list functions
	void					set_support_joint_list(SrArray<SrString*>* joint_list);
	SrArray<SrString*>*		get_support_joint_list();
	int						get_support_joint_num();

	void					init_displacement_list(int num);
	SrArray<SrVec*>*		get_displacement_list();

	int						get_next_frame(int frame);
	int						get_prev_frame(int frame);
	void					get_stance_time(SkMotion* standing, char* limb_base);

	MeCtLocomotionTimingSpace* get_timing_space();

	void					get_frame(float frame, char* limb_base);

	SrArray<SrQuat>*		get_buffer();

	int						iterate_set(SkJoint* base, int index, SrArray<SrQuat>* buff);

	void					get_displacement(float start_frame, float end_frame);

	void					estimate_direction(int* count);

	void					set_stride_cycle_length(float length);
	//temp function for test, to be deleted......
	void					print_info();

};

SrArray<SrQuat>*	get_blended_quat_buffer(SrArray<SrQuat>* dest, SrArray<SrQuat>* quat_buffer1, SrArray<SrQuat>* quat_buffer2, float weight);
void				clear_buffer(SrArray<SrQuat*>* dest);
void				get_blended_anim(MeCtLocomotionLimbAnim* dest, MeCtLocomotionLimbAnim* anim1, MeCtLocomotionLimbAnim* anim2, float ratio, float stride);
#endif // ME_LOCOMOTION_LIMB_ANIM_HPP
