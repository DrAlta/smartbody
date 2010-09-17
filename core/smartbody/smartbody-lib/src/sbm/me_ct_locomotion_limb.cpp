/*
 *  me_ct_locomotion_limb.cpp - part of SmartBody-lib's Motion Engine
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

#include "me_ct_locomotion_limb.hpp"

#include "sbm_character.hpp"
#include "gwiz_math.h"
#include "limits.h"
#include <vhcl_log.h>


const char* MeCtLocomotionLimb::TYPE = "MeCtLocomotionLimb";


/** Constructor */
MeCtLocomotionLimb::MeCtLocomotionLimb() {
	height_bound = 0.0f;
	is_valid = false;
	ground_height = 0.0f;
	limb_base_name = NULL;
	skeleton_name = NULL;
	space_time = 0.0f;
	limb_joint_info.quat.capacity(0);
	limb_joint_info.joint_index.capacity(0);
	limb_joint_info.buff_index.capacity(0);
	curr_rotation = 0.0f;
	rotation_record = 0.0f;
	blended_anim.global_info = new MeCtLocomotionAnimGlobalInfo();
	walking_skeleton = NULL;
	standing_skeleton = NULL;
}

MeCtLocomotionLimb::MeCtLocomotionLimb(char* name) 
{
	MeCtLocomotionLimb();
	limb_name = name;
}

/** Destructor */
MeCtLocomotionLimb::~MeCtLocomotionLimb() {
	// Nothing allocated to the heap
	limb_base_name = NULL;
	for (int x = 0; x < walking_list.size(); x++)
	{
		MeCtLocomotionLimbAnim* anim = walking_list.get(x);
		delete anim;
	}
	walking_list.capacity(0);

	for (int x = 0; x < support_joint_list.size(); x++)
	{
		delete support_joint_list[x];
	}
	support_joint_list.capacity(0);

	free(limb_base_name);
	delete blended_anim.global_info;
	if (walking_skeleton)
		this->walking_skeleton->unref();
	if (standing_skeleton)
		this->standing_skeleton->unref();
}

int MeCtLocomotionLimb::get_descendant_num(SkJoint* joint)
{
	int num = joint->num_children();
	for (int i=0; i<joint->num_children(); i++ )
	{
		num += get_descendant_num(joint->child(i));
	}
	return num;
}

int MeCtLocomotionLimb::get_support_joint_num()
{
	return support_joint_list.size();
}

char* MeCtLocomotionLimb::get_limb_base_name()
{
	return limb_base_name;
}

void MeCtLocomotionLimb::set_limb_base(char* name)
{
	limb_base_name = (char*)malloc(sizeof(char)*(strlen(name)+1));
	strcpy(limb_base_name, name);
	int num = get_descendant_num(standing_skeleton->search_joint(name))+1;
	ik.joint_info_list.capacity(num);
	ik.joint_info_list.size(num);
	limb_joint_info.quat.capacity(num);
	SrQuat q(1,0,0,0);
	SrVec v(0,0,0);
	for(int i = 0; i < num; ++i)
	{
		limb_joint_info.quat.push(q);
	}
	limb_joint_info.Init(walking_skeleton, limb_base_name, NULL);
}

void MeCtLocomotionLimb::add_support_joint(char* joint_name)
{
	SrString* joint = new SrString(joint_name);
	support_joint_list.push() = joint;
}

void MeCtLocomotionLimb::set_skeleton_name(char* name)
{
	skeleton_name = name;
}

void MeCtLocomotionLimb::set_height_bound(float bound)
{
	height_bound = bound;
}

float MeCtLocomotionLimb::get_height_bound()
{
	return height_bound;
}

float MeCtLocomotionLimb::get_ground_height()
{
	return ground_height;
}

void MeCtLocomotionLimb::set_ground_height(float ground_height)
{
	this->ground_height = ground_height;
}

//temp function for test, to be deleted......
void MeCtLocomotionLimb::print_info()
{
	LOG("\n ground_height: %f", ground_height);
	LOG("\n height_bound: %f", height_bound);
	for(int i = 0; i < walking_list.size(); ++i)
	{
		SrVec walking_dir = walking_list.get(i)->local_direction;
		printf("\n walking animation %d direction: (%f, %f, %f)", i, walking_dir.x, walking_dir.y, walking_dir.z);
	}
	for(int i = 0; i < get_support_joint_num(); ++i)
	{
		LOG("\n support_height[%s]: %f", (const char*)*support_joint_list.get(i), support_height.get(i));
	}
	for(int j = 0; j < walking_list.size(); ++j)
	{
		walking_list.get(j)->print_info();
	}
}

void MeCtLocomotionLimb::blend_anim(float space_time, int anim_index1, int anim_index2, float weight, SrArray<int>* index_buff)
{
	MeCtLocomotionLimbAnim* anim1 = walking_list.get(anim_index1);
	MeCtLocomotionLimbAnim* anim2 = walking_list.get(anim_index2);
	anim1->get_frame(anim1->get_timing_space()->get_virtual_frame(space_time), limb_base_name, index_buff);
	anim2->get_frame(anim2->get_timing_space()->get_virtual_frame(space_time), limb_base_name, index_buff);
	get_blended_quat_buffer(&(limb_joint_info.quat), anim1->get_buffer(), anim2->get_buffer(), weight);
	//r_base_pos = anim1->get_base_pos()*weight + anim2->get_base_pos()*(1.0f-weight);
}

void MeCtLocomotionLimb::manipulate_turning(SrMat& root_mat)
{
	SrMat mat;
	SrVec y(0.0f, 1.0f, 0.0f);
	//root_mat.invert();
	//y = root_mat*y;
	//y = y*root_mat;
	//printf("\n%f", curr_rotation);
	mat.rot(y, curr_rotation);
	SrQuat quat = limb_joint_info.quat.get(0);
	quat = quat * mat;
	//quat = mat* quat;
	limb_joint_info.quat.set(0, quat);
}

void MeCtLocomotionLimb::calc_blended_anim_speed(MeCtLocomotionLimbAnim* anim1, MeCtLocomotionLimbAnim* anim2, float weight)
{
	SrVec v1 = anim1->global_info->direction * anim1->global_info->displacement * (float)anim1->get_timing_space()->get_mode() * weight;
	SrVec v2 = anim2->global_info->direction * anim2->global_info->displacement * (float)anim2->get_timing_space()->get_mode() * (1-weight);
	SrVec v = v1+v2;
	blended_anim.global_info->displacement = v.len();
	blended_anim.global_info->speed = v.len()/(blended_anim.get_timing_space()->get_frame_num()*0.033333f);
	v.normalize();
	blended_anim.global_info->direction = v;
}

void MeCtLocomotionLimb::blend_standing(MeCtLocomotionLimbAnim* anim, float weight)
{
	get_blended_quat_buffer(&(limb_joint_info.quat), &(limb_joint_info.quat), anim->get_buffer(), weight);
}

void MeCtLocomotionLimb::init_skeleton(SkSkeleton* standing_skeleton, SkSkeleton* walking_skeleton)
{
	this->walking_skeleton = walking_skeleton;
	this->walking_skeleton->ref();
	this->standing_skeleton = standing_skeleton;
	this->standing_skeleton->ref();
}

SrArray<MeCtLocomotionLimbAnim*>* MeCtLocomotionLimb::get_walking_list()
{
	return &walking_list;
}

void MeCtLocomotionLimb::set_joint_type(int index, int type)
{
	ik.joint_info_list.get(index).type = type;
}

void MeCtLocomotionLimb::set_joint_rotation_axis(int index, SrVec* axis)
{
	ik.joint_info_list.get(index).axis = *axis;
}