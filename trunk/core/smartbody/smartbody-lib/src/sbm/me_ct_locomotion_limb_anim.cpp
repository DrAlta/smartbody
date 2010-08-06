/*
 *  me_locomotion_limb_anim.cpp - part of SmartBody-lib's Motion Engine
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

#include "me_ct_locomotion_limb_anim.hpp"

#include "sbm_character.hpp"
#include "gwiz_math.h"
#include "limits.h"


const char* MeCtLocomotionLimbAnim::TYPE = "MeCtLocomotionLimbAnim";

void clear_buffer(SrArray<SrQuat*>* dest);


/** Constructor */
MeCtLocomotionLimbAnim::MeCtLocomotionLimbAnim() 
{

}

/** Destructor */
MeCtLocomotionLimbAnim::~MeCtLocomotionLimbAnim() 
{
	// Nothing allocated to the heap
	for (int x = 0; x < displacement_list.size(); x++)
		delete displacement_list[x];
}

int MeCtLocomotionLimbAnim::get_support_joint_num()
{
	return support_joint_list->size();
}

void MeCtLocomotionLimbAnim::set_support_joint_list(SrArray<SrString*>* joint_list)
{
	support_joint_list = joint_list;
}

SrArray<SrString*>* MeCtLocomotionLimbAnim::get_support_joint_list()
{
	return support_joint_list;
}

void MeCtLocomotionLimbAnim::init_displacement_list(int num)
{
	displacement_list.size(0);
	displacement_list.capacity(walking->frames());
	SrVec* displacement;
	for(int i = 0; i < num; ++i)
	{
		displacement = new SrVec(0.0f, 0.0f, 0.0f);
		displacement_list.push() = displacement;
	}
}

SrArray<SrVec*>* MeCtLocomotionLimbAnim::get_displacement_list()
{
	return &displacement_list;
}

int MeCtLocomotionLimbAnim::get_next_frame(int frame)
{
	if(frame >= walking->frames()-1) return 0;
	return frame +1; 
}

int MeCtLocomotionLimbAnim::get_prev_frame(int frame)
{
	if(frame <= 0) return walking->frames()-1;
	return frame -1;
}

void MeCtLocomotionLimbAnim::set_anim(SkMotion* walking)
{
	this->walking = walking;
}

/*void MeCtLocomotionLimbAnim::estimate_direction(int* count)
{
	SrVec direction(0,0,0);
	for(int i = 0; i < displacement_list.size(); ++i)
	{
		if(count[i] <= 0) continue;
		direction += *(displacement_list.get(i));
	}
	for(int i = 0; i < displacement_list.size(); ++i)
	{
		if(dot(*displacement_list.get(i), direction)<=0.0f) 
		{
			displacement_list.get(i)->set(0,0,0);
			count[i] = 0;
		}
	}
}*/

//temp function for test, to be deleted......
void MeCtLocomotionLimbAnim::print_info()
{
	SrVec* velocity = NULL;
	LOG("\n Printing info for walking animation %s:", walking->name());
	/*for(int i = 0; i < get_support_joint_num(); ++i)
	{
		LOG("\n land_frame[%s]: %d", (const char*)*(support_joint_list->get(i)), land_frame.get(i));
		LOG("\n lift_frame[%s]: %d", (const char*)*(support_joint_list->get(i)), lift_frame.get(i));
	}*/
	for(int j = 0; j < walking->frames() && j < displacement_list.size(); ++j)
	{
		velocity = displacement_list.get(j);
		LOG("\n%d (%.2f, %.2f, %.2f)", j, velocity->x, velocity->y, velocity->z);
	}
}

void MeCtLocomotionLimbAnim::init_skeleton(SkSkeleton* standing_skeleton, SkSkeleton* walking_skeleton)
{
	this->walking_skeleton = walking_skeleton;
	this->standing_skeleton = standing_skeleton;
}

void MeCtLocomotionLimbAnim::init_quat_buffers(int num)
{
	quat_buffer.capacity(num);
	quat_buffer.size(num);
	quat_buffer_key_frame1.capacity(num);
	quat_buffer_key_frame1.size(num);
	quat_buffer_key_frame2.capacity(num);
	quat_buffer_key_frame2.size(num);
}

// pass a float index in 
void MeCtLocomotionLimbAnim::get_frame(float frame, char* limb_base, SrArray<int>* index_buff)
{
	//temp int, must be changed
	walking->connect(walking_skeleton);
	SkJoint* base = walking_skeleton->search_joint(limb_base);
	walking->apply_frame((int)frame);
	iterate_set(base, 0, 0, &quat_buffer_key_frame1, index_buff);
	walking->apply_frame((int)frame+1);
	iterate_set(base, 0, 0, &quat_buffer_key_frame2, index_buff);
	SrQuat q;
	for(int i = 0; i < quat_buffer.size(); ++i)
	{
		q = slerp(quat_buffer_key_frame1.get(i), quat_buffer_key_frame2.get(i), frame-(int)frame);
		quat_buffer.set(i,q);
	}
}

/*int MeCtLocomotionLimbAnim::iterate_set(SkJoint* base, int index, SrArray<SrQuat>* buff)
{
	SrQuat q = base->quat()->value();
	buff->set(index, q);
	for(int i = 0; i < base->num_children(); ++i)
	{
		index = iterate_set(base->child(i), index+1, buff);
	}
	return index;
}*/

SrArray<SrQuat>* MeCtLocomotionLimbAnim::get_buffer()
{
	return &quat_buffer;
}

void get_blended_anim(MeCtLocomotionLimbAnim* dest, MeCtLocomotionLimbAnim* anim1, MeCtLocomotionLimbAnim* anim2, float weight, float stride)
{
	get_blended_timing_space(dest->get_timing_space(), anim1->get_timing_space(), anim2->get_timing_space(), weight);
}