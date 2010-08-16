/*
 *  me_ct_locomotion_analysis.cpp - part of SmartBody-lib's Motion Engine
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

#include <vhcl_log.h>

#include "me_ct_locomotion_analysis.hpp"

#include "sbm_character.hpp"
#include "me_utilities.hpp"
#include "gwiz_math.h"
#include "limits.h"


const char* MeCtLocomotionAnalysis::TYPE = "MeCtLocomotionAnalysis";


/** Constructor */
MeCtLocomotionAnalysis::MeCtLocomotionAnalysis() {
	//is_valid = true;
	_ct_locomotion = NULL;
	walking_skeleton = NULL;
	standing_skeleton = NULL;
	motion_standing = NULL;

}

/** Destructor */
MeCtLocomotionAnalysis::~MeCtLocomotionAnalysis() {
	// Nothing allocated to the heap
}

void MeCtLocomotionAnalysis::set_ct(MeCtLocomotion* controller)
{
	_ct_locomotion = controller;
	if (walking_skeleton)
		walking_skeleton->unref();
	walking_skeleton = _ct_locomotion->walking_skeleton;
	walking_skeleton->ref();

	if (standing_skeleton)
		standing_skeleton->unref();
	standing_skeleton = _ct_locomotion->standing_skeleton;
	standing_skeleton->ref();
}

MeCtLocomotion* MeCtLocomotionAnalysis::get_ct()
{
	return _ct_locomotion;

}

void MeCtLocomotionAnalysis::init(SkMotion* standing, srPathList &me_paths) //temp hard-coded init for human characters
{
	if(_ct_locomotion == NULL) 
	{
		LOG("Error: no locomotion controller attached.");
		return;
	}
	if (this->motion_standing)
		this->motion_standing->unref();
	this->motion_standing = standing;
	const char* base_name = standing_skeleton->root()->name().get_string();

	get_ct()->set_base_name(base_name);
	get_ct()->set_nonlimb_blending_base_name(base_name);

	MeCtLocomotionLimb* limb = new MeCtLocomotionLimb();
	limb->init_skeleton(standing_skeleton, walking_skeleton);
	get_ct()->get_limb_list()->push() = limb;
	limb->set_skeleton_name("common.sk");
	limb->set_limb_base("l_hip");
	limb->add_support_joint("l_ankle");
	limb->add_support_joint("l_forefoot");
	limb->add_support_joint("l_toe");
	limb->set_joint_type(0, JOINT_TYPE_BALL);
	limb->set_joint_type(1, JOINT_TYPE_HINGE);
	limb->set_joint_type(2, JOINT_TYPE_BALL);
	limb->set_joint_type(3, JOINT_TYPE_HINGE);
	limb->set_joint_type(4, JOINT_TYPE_HINGE);

	limb = new MeCtLocomotionLimb();
	limb->init_skeleton(standing_skeleton, walking_skeleton);
	get_ct()->get_limb_list()->push() = limb;
	limb->set_skeleton_name("common.sk");
	limb->set_limb_base("r_hip");
	limb->add_support_joint("r_ankle");
	limb->add_support_joint("r_forefoot");
	limb->add_support_joint("r_toe");
	limb->set_joint_type(0, JOINT_TYPE_BALL);
	limb->set_joint_type(1, JOINT_TYPE_HINGE);
	limb->set_joint_type(2, JOINT_TYPE_BALL);
	limb->set_joint_type(3, JOINT_TYPE_HINGE);
	limb->set_joint_type(4, JOINT_TYPE_HINGE);

	_ct_locomotion->init_nonlimb_joint_info();

	limb = NULL;

	MeCtLocomotionAnimGlobalInfo* info = new MeCtLocomotionAnimGlobalInfo();
	info->speed = 0.0f;
	info->direction.set(0,0,0);
	_ct_locomotion->get_anim_global_info()->push() = info;
	for(int i = 0; i < _ct_locomotion->get_limb_list()->size(); ++i)
	{
		limb = _ct_locomotion->get_limb_list()->get(i);
		limb->set_height_bound(0.5f);
		analyze_standing(limb, standing);
		limb->walking_list.get(limb->walking_list.size()-1)->global_info = info;
	}
	SkJoint* joint = NULL;
	SkJoint* tjoint = NULL;
	for(int i = 0; i < _ct_locomotion->get_limb_list()->size(); ++i)
	{
		limb = _ct_locomotion->get_limb_list()->get(i);
		joint = standing_skeleton->search_joint(limb->limb_base_name);
		for(int j = 0; j < limb->joint_num; ++j)
		{
			limb->ik.joint_info_list.get(j).is_support_joint = 0;
			limb->ik.joint_info_list.get(j).sk_joint = joint;
			for(int k = 0; k < limb->get_support_joint_num(); ++k)
			{
				SrString* a = limb->support_joint_list.get(k);
				const char* c = (const char *)*a;
				tjoint = standing_skeleton->search_joint(c);
				if(joint == tjoint)
				{
					limb->ik.joint_info_list.get(j).is_support_joint = 1;
					limb->ik.joint_info_list.get(j).support_joint_height = limb->support_height.get(k);
					limb->ik.joint_info_list.get(j).support_joint_comp = 0.0f;
				}
			}
			if(joint->num_children() > 0)
			{
				joint = joint->child(0);
			}
		}
	}

	
}

int MeCtLocomotionAnalysis::get_descendant_num(char* base_name)
{
	//motion_locomotion->connect(walking_skeleton);
	SkJoint* base = walking_skeleton->search_joint(base_name);
	return get_descendant_num(base);
}

int MeCtLocomotionAnalysis::get_descendant_num(SkJoint* base)
{
	int sum = base->num_children();
	for(int i = 0; i < base->num_children(); ++i)
	{
		sum += get_descendant_num(base->child(i));
	}
	return sum;
}

void MeCtLocomotionAnalysis::analyze_standing(MeCtLocomotionLimb* limb, SkMotion* standing)
{
	limb->support_height.size(0);
	limb->support_height.capacity(limb->get_support_joint_num());

	SkSkeleton* skeleton = standing_skeleton;
	if(standing) standing->connect(skeleton);

	SkJoint* joint = NULL;
	SrMat mat;
	float* pos = NULL;

	if(standing) // if standard standing motion is available
	{
		// get ground height by picking the lowest y value of each joint
		standing->connect(skeleton);
		for(int k = 0; k < 1; ++k)
		{
			standing->apply_frame(k);
			analyze_standing_core(limb, skeleton);
		}
	}
	else // if default posture (usually in 'T' shape)
	{
		analyze_standing_core(limb, skeleton);
	}

	//_ct_locomotion->nonlimb_joint_info.Init(, base_name);
	MeCtLocomotionLimbAnim* anim = new MeCtLocomotionLimbAnim();
	limb->walking_list.push() = anim;
	anim->set_anim(standing);
	anim->init_skeleton(standing_skeleton, walking_skeleton);
	anim->get_timing_space()->add_ref_time_name("stance_time");
	anim->get_timing_space()->add_ref_time_name("lift_time");
	anim->get_timing_space()->add_ref_time_name("land_time");
	limb->joint_num = get_descendant_num(limb->limb_base_name)+1;
	limb->pos_buffer.capacity(limb->joint_num);
	limb->pos_buffer.size(limb->joint_num);
	anim->init_quat_buffers(limb->joint_num);
	anim->get_frame(0.0f, limb->limb_base_name, &(limb->limb_joint_info.joint_index));
	for(int i = 0; i < _ct_locomotion->limb_list.size(); ++i)
	{
		_ct_locomotion->limb_list.get(i)->direction_planner.ref_time_num = anim->get_timing_space()->get_ref_time_num();
	}
	limb->blended_anim.init_quat_buffers(limb->joint_num);
	//limb->init_skeleton(standing, standing);
}

void MeCtLocomotionAnalysis::analyze_standing_core(MeCtLocomotionLimb* limb, SkSkeleton* skeleton)
{
	SkJoint* joint = NULL;
	SrMat mat;
	float* pos = NULL;

	float ground_height = 0.0f;

	// get ground height by picking the lowest y value of each joint
	skeleton->update_global_matrices();
	for(int i = 0; i < limb->get_support_joint_num(); ++i)
	{
		joint = skeleton->search_joint(*(limb->support_joint_list.get(i)));
		mat = joint->gmat();
		pos = mat.pt(12);
		pos[1] += 100.0f;
		if(i == 0 || ground_height > *(pos+1))
		{
			ground_height = *(pos+1);
		}
	}
	//calculate the distance from support joint to ground, 
	//use these values to calculate the time support joints hit the ground
	for(int i = 0; i < limb->get_support_joint_num(); ++i)
	{
		joint = skeleton->search_joint(*(limb->support_joint_list.get(i)));
		mat = joint->gmat();
		pos = mat.pt(12);
		pos[1] += 100.0f;
		limb->support_height.push() = *(pos+1)- ground_height;
	}

	limb->set_ground_height(ground_height);

}

void MeCtLocomotionAnalysis::analyze_limb_anim(MeCtLocomotionLimbAnim* anim, SkMotion* walking, SkMotion* standing, char* limb_base, int land_time, int stance_time, int lift_time)
{
	//printf("\nstart analysis......");
	anim->set_anim(walking);

	motion_locomotion = walking;

	anim->init_displacement_list(walking->frames());

	float* height = (float*)malloc(sizeof(float)*walking->frames());
	float* pos_x = (float*)malloc(sizeof(float)*walking->frames());
	float* pos_z = (float*)malloc(sizeof(float)*walking->frames());
	SkSkeleton* skeleton = walking_skeleton;
	SrMat mat, base_mat;

	walking->connect(skeleton);

	SkJoint* joint = NULL;
	SkJoint* base_joint = NULL;
	SrVec* velocity = NULL;
	float* pos = NULL;
	float* base_pos = NULL;
	int mode = 0;
	int i,j;
	SrVec walking_direction(0,0,0);
	//SrVec displacement(0,0,0);
	SrVec prev_base_pos(0,0,0);
	float limb_stride = 0.0f;

	int* section = (int*)malloc(sizeof(int)*(walking->frames()+1));
	int* count = (int*)malloc(sizeof(int)*(walking->frames()));
	SrVec* temp_axis = (SrVec*)malloc(sizeof(SrVec)*_limb->joint_num);
	int* temp_axis_num = (int*)malloc(sizeof(int)*_limb->joint_num);
	memset(temp_axis, 0, sizeof(SrVec)*_limb->joint_num);
	memset(temp_axis_num, 0, sizeof(int)*_limb->joint_num);
	memset(count, 0, sizeof(int)*(walking->frames()));
	int sec_num = -1;

	int land_frame_i = 0;
	int lift_frame_i = 0;

	SrVec taxis;

	base_joint = skeleton->search_joint(this->_ct_locomotion->get_base_name());

	j = 2;
	//for(j = 0; j < anim->get_support_joint_num(); ++j)
	{
		SrString* a = anim->get_support_joint_list()->get(j);
		const char* c = (const char *)*a;
		joint = skeleton->search_joint(c);
		pos = mat.pt(12);
		base_pos = base_mat.pt(12);
		for(int i = 0; i < walking->frames(); ++i)
		{
			prev_base_pos = *((SrVec*)base_pos);
			walking->apply_frame(i);
			skeleton->update_global_matrices();
			mat = joint->gmat();
			base_mat = base_joint->gmat();
			if(i == 0) prev_base_pos = *((SrVec*)base_pos);
			pos_x[i] = pos[0] - base_pos[0];
			pos_z[i] = pos[2] - base_pos[2];

			walking_direction += *((SrVec*)base_pos)-prev_base_pos;
		}
		//the direction from last frame to first frame
		prev_base_pos = *((SrVec*)base_pos);
		walking->apply_frame(0);
		skeleton->update_global_matrices();
		mat = joint->gmat();
		base_mat = base_joint->gmat();
		walking_direction += *((SrVec*)base_pos)-prev_base_pos;

		int ind = land_time;
		for(int i = 0; i < walking->frames(); ++i)
		{
			if(ind == lift_time) break;
			int next = anim->get_next_frame(ind); 
			velocity = anim->get_displacement_list()->get(ind);

			if(count[i] == 0)
			{
				velocity->set(0,0,0);
			}
			++count[i];
			velocity->x += pos_x[next]-pos_x[ind];
			velocity->z += pos_z[next]-pos_z[ind];
			++ind;
			if(ind > walking->frames()-1) ind = 0;
		}
	}

	for(int i = 0; i < walking->frames(); ++i)
	{
		//printf("\ncount[%d]:%d", i, count[i]);
		//printf("\ncount[%d]:%f", i, anim->get_displacement_list()->get(i)->len());
		if(count[i] <= 0) continue;
		velocity = anim->get_displacement_list()->get(i);
		velocity->x /= count[i];
		velocity->z /= count[i];

		limb_stride += velocity->len();
	}

	if(walking_direction.len()*4 < limb_stride) estimate_direction(anim, count);
	else 
	{
		SrVec dir(0,0,0);
		anim->local_direction = walking_direction;
		anim->local_direction.normalize();
	}
	
	stance_frame.capacity(0);
	stance_frame.push() = stance_time;
	//calc_stance_time(anim, limb_base);

	anim->get_timing_space()->add_ref_time_name("stance_time");
	anim->get_timing_space()->set_ref_time(0, (float)stance_time);

	anim->get_timing_space()->add_ref_time_name("lift_time");
	anim->get_timing_space()->set_ref_time(1, (float)lift_time);

	anim->get_timing_space()->add_ref_time_name("land_time");
	anim->get_timing_space()->set_ref_time(2, (float)land_time);

	anim->get_timing_space()->set_frame_num((float)motion_locomotion->frames());

	//add_ref_times(anim, count);
	
	//printf("\nend analysis......");
}

void MeCtLocomotionAnalysis::analyze_limb_anim(MeCtLocomotionLimbAnim* anim, SkMotion* walking, SkMotion* standing, char* limb_base, SrArray<float>* support_height, 
								   float ground_height, float height_bound)
{

	//printf("\nstart analysis......");

	anim->set_anim(walking);

	motion_locomotion = walking;

	anim->init_displacement_list(walking->frames());

	float* height = (float*)malloc(sizeof(float)*walking->frames());
	float* pos_x = (float*)malloc(sizeof(float)*walking->frames());
	float* pos_z = (float*)malloc(sizeof(float)*walking->frames());
	SkSkeleton* skeleton = walking_skeleton;
	SrMat mat, base_mat;

	walking->connect(skeleton);

	SkJoint* joint = NULL;
	SkJoint* base_joint = NULL;
	SrVec* velocity = NULL;
	float* pos = NULL;
	float* base_pos = NULL;
	int mode = 0;
	int i,j;
	SrVec walking_direction(0,0,0);
	//SrVec displacement(0,0,0);
	SrVec prev_base_pos(0,0,0);
	float limb_stride = 0.0f;

	int* section = (int*)malloc(sizeof(int)*(walking->frames()+1));
	int* count = (int*)malloc(sizeof(int)*(walking->frames()));
	SrVec* temp_axis = (SrVec*)malloc(sizeof(SrVec)*_limb->joint_num);
	int* temp_axis_num = (int*)malloc(sizeof(int)*_limb->joint_num);
	memset(temp_axis, 0, sizeof(SrVec)*_limb->joint_num);
	memset(temp_axis_num, 0, sizeof(int)*_limb->joint_num);
	memset(count, 0, sizeof(int)*(walking->frames()));
	int sec_num = -1;

	int land_frame_i = 0;
	int lift_frame_i = 0;

	SrVec taxis;

	base_joint = skeleton->search_joint(this->_ct_locomotion->get_base_name());

	// analysis for IK.........................................................................

	// analysis for IK.........................................................................

	for(j = 0; j < anim->get_support_joint_num(); ++j)
	{
		sec_num = -1;
		mode = 0;
		SrString* a = anim->get_support_joint_list()->get(j);
		const char* c = (const char *)*a;
		joint = skeleton->search_joint(c);
		pos = mat.pt(12);
		base_pos = base_mat.pt(12);
		for(i = 0; i < walking->frames(); ++i)
		{
			prev_base_pos = *((SrVec*)base_pos);
			walking->apply_frame(i);
			skeleton->update_global_matrices();
			mat = joint->gmat();
			base_mat = base_joint->gmat();
			if(i == 0) prev_base_pos = *((SrVec*)base_pos);
			pos_x[i] = pos[0] - base_pos[0];
			height[i] = pos[1];
			pos_z[i] = pos[2] - base_pos[2];

			walking_direction += *((SrVec*)base_pos)-prev_base_pos;
			//displacement += *((SrVec*)base_pos)-prev_base_pos;
			//LOG("\nheight[frame%2d]:%f", i, height[i]);
		}
		int next = -1;

		// calculate the velocity if the base joint is fixed
		for(i = 0; i < walking->frames(); ++i)
		{
			float a = support_height->get(j);
			next = anim->get_next_frame(i); 
			velocity = anim->get_displacement_list()->get(i);
			if(height[i]-a <= ground_height + height_bound)
			{
				SrVec dis(pos_x[next]-pos_x[i], 0, pos_z[next]-pos_z[i]);
				if(dot(dis, walking_direction) > 0.0f) continue;
				if(count[i] == 0)
				{
					velocity->set(0,0,0);
				}
				++count[i];
				velocity->x += pos_x[next]-pos_x[i];
				velocity->z += pos_z[next]-pos_z[i];
			}
			else
			{
				/*if(count[i] == 0)
				{
					velocity->x += (pos_x[next]-pos_x[i])/anim->get_support_joint_num();
					velocity->z += (pos_z[next]-pos_z[i])/anim->get_support_joint_num();
				}*/
			}
		}
		//displacement /= (float)anim->get_support_joint_num();
	}

	walking_direction /= (float)anim->get_support_joint_num();

	// calculate the velocity if the base joint is fixed
	for(i = 0; i < walking->frames(); ++i)
	{
		if(count[i] <= 0) continue;
		velocity = anim->get_displacement_list()->get(i);
		velocity->x /= count[i];
		velocity->z /= count[i];

		limb_stride += velocity->len();
	}

	if(walking_direction.len()*4 < limb_stride) 
		estimate_direction(anim, count);
	else 
	{
		SrVec dir(0,0,0);
		anim->local_direction = walking_direction;
		anim->local_direction.normalize();

		for(i = 0; i < walking->frames(); ++i)
		{
			dir = *(anim->get_displacement_list()->get(i));
			float va = dot(dir, anim->local_direction);
			if(dot(dir, anim->local_direction)>0.0f) count[i] = 0;
			printf("\n count[%d]: %d", i, count[i]);
		}
	}

	for(i = 0; i < walking->frames(); ++i)
	{
		if(count[i]> 0)
		{
			mode = 1;
		}
		else
		{
			if(mode == 0 || mode == 1)
			{
				++sec_num;
				section[sec_num*2] = i;
			}
			section[sec_num*2+1] = i;
			mode = 2;
		}
	}

	if(sec_num != -1) //no frame when foot is in the air
	{
		if(section[0] == 0 && section[sec_num*2+1] == walking->frames()-1)
		{
			section[0] = section[sec_num*2];
			--sec_num;
		}
		if(sec_num > -1)
		{
			float sum = 0, tsum = 0;
			int index = 0;
			for(i = 0; i <= sec_num; ++i)
			{
				tsum = 0;
				for(int k = section[i*2]; k != anim->get_next_frame(section[i*2+1]); k = anim->get_next_frame(k))
				{
					tsum += height[k]-support_height->get(j)-ground_height;
				}
				if(tsum > sum) 
				{
					sum = tsum;
					index = i;
				}
			}
			land_frame_i = section[index*2+1];
			lift_frame_i = section[index*2];

			for(i = land_frame_i; i != lift_frame_i; i = anim->get_next_frame(i))
			{
				count[i] = anim->get_support_joint_num();
			}
			for(i = 0;i < walking->frames(); ++i)
			{
				if(count[i] == 0) anim->get_displacement_list()->get(i)->set(0,0,0);
			}
		}
	}

	/*anim->global_info->direction.set(0,0,0);
	int sum = 0;
	for(int i = 0; i < anim->get_velocity_list()->size(); ++i)
	{
		anim->direction += *(anim->get_velocity_list()->get(i));
		if(!anim->get_velocity_list()->get(i)->iszero())
		{
			++sum;
		}
	}
	//anim->direction = - anim->direction;
	anim->speed = anim->direction.len()/(0.0333f*sum);
	anim->direction.normalize();
	anim->average_speed_factor = 1.0f; // temp, must be modified later*/

	calc_stance_time(anim, limb_base);

	add_ref_times(anim, count);

	free(height);
	free(pos_x);
	free(pos_z);
	free(count);
	free(temp_axis);
	//LOG("\nend analysis......");
}

void MeCtLocomotionAnalysis::estimate_direction(MeCtLocomotionLimbAnim* anim, int* count)
{
	//SrVec direction(0,0,0);
	anim->local_direction.set(0,0,0);
	for(int i = 0; i < anim->displacement_list.size(); ++i)
	{
		if(count[i] <= 0) continue;
		anim->local_direction += *(anim->displacement_list.get(i));
	}
	anim->local_direction = -anim->local_direction;
}

void MeCtLocomotionAnalysis::filter_displacement(MeCtLocomotionLimbAnim* anim, int* count)
{
	for(int i = 0; i < anim->displacement_list.size(); ++i)
	{
		if(dot(*anim->displacement_list.get(i), anim->local_direction)>0.0f) 
		{
			anim->displacement_list.get(i)->set(0,0,0);
			count[i] = 0;
		}
	}
}

void MeCtLocomotionAnalysis::calc_stance_time(MeCtLocomotionLimbAnim* anim, char* limb_base)
{
	stance_frame.capacity(0);
	motion_locomotion->connect(walking_skeleton);
	motion_standing->connect(standing_skeleton);

	motion_standing->apply_frame(0);
	standing_skeleton->update_global_matrices();

	float dis = -1.0f;
	int stance = -1;

	SkJoint* walking_base = walking_skeleton->search_joint(limb_base);
	SkJoint* standing_base = standing_skeleton->search_joint(limb_base);

	for(int i = 0; i < motion_locomotion->frames(); ++i)
	{
		motion_locomotion->apply_frame(i);
		walking_skeleton->update_global_matrices();
		float l = iterate_sub_joints(walking_base, standing_base);
		if(dis < 0.0f || dis > l)
		{
			dis = l;
			stance = i;
		}
	}
	stance_frame.push() = stance;

}

float MeCtLocomotionAnalysis::iterate_sub_joints(SkJoint* walking_joint, SkJoint* standing_joint)
{
	SrVec vec;
	SrMat walking_mat = walking_joint->gmat();
	SrMat standing_mat = standing_joint->gmat();
	vec.x = walking_mat.get(12) - standing_mat.get(12);
	vec.y = walking_mat.get(13) - standing_mat.get(13);
	vec.z = walking_mat.get(14) - standing_mat.get(14);
	float len = vec.len();
	for(int i=0; i < walking_joint->num_children(); i++ )
    { 
		len += iterate_sub_joints(walking_joint->child(i), standing_joint->child(i));
    }
	return len;
}

void MeCtLocomotionAnalysis::add_ref_times(MeCtLocomotionLimbAnim* anim, int* count)
{
	int counter = 0;
	anim->get_timing_space()->add_ref_time_name("stance_time");
	anim->get_timing_space()->set_ref_time(counter++, (float)stance_frame.get(0));

	float val = 0.0f;
	int i = 0, j = -1, start = 0, end = 0;

	for(i = 0; i < anim->get_displacement_list()->size(); ++i)
	{
		if(count[i] > 0) break;
	}

	start = i;
	end = i;

	while(j!=i)
	{
		j = anim->get_next_frame(end);
		end = j;
		if(count[j] <= 0) 
		{
			end = anim->get_prev_frame(end);
			break;
		}
	}
	anim->get_timing_space()->add_ref_time_name("lift_time");
	anim->get_timing_space()->set_ref_time(counter++, (float)end);
	while(j!=i)
	{
		j = anim->get_prev_frame(start);
		if(count[j] <= 0) break;
		start = j;
	}
	anim->get_timing_space()->add_ref_time_name("land_time");
	anim->get_timing_space()->set_ref_time(counter++, (float)start);

	anim->get_timing_space()->set_frame_num((float)motion_locomotion->frames());
	//LOG("\nstance_time:%f; lift_time:%f; land_time:%f", anim->get_timing_space()->get_ref_time(0), anim->get_timing_space()->get_ref_time(1), anim->get_timing_space()->get_ref_time(2));

}

void MeCtLocomotionAnalysis::analyze_walking_limb(MeCtLocomotionLimb* limb, SkMotion* walking, SkMotion* standing, int walking_style)
{
	_limb = limb;
	MeCtLocomotionLimbAnim* anim = new MeCtLocomotionLimbAnim();
	anim->style = walking_style;
	anim->init_skeleton(limb->standing_skeleton, limb->walking_skeleton);
	//anim->set_skeleton_name(skeleton_name);
	anim->set_support_joint_list(&(limb->support_joint_list));
	int limb_joint_num = get_descendant_num(limb->limb_base_name)+1;
	anim->init_quat_buffers(limb_joint_num);
	analyze_limb_anim(anim, walking, standing, limb->get_limb_base_name(), &limb->support_height, limb->get_ground_height(), limb->get_height_bound());
	limb->walking_list.push() = anim;
}

//temp copy of analyze_walking_limb to enable preset of land time and lift time
void MeCtLocomotionAnalysis::analyze_walking_limb(MeCtLocomotionLimb* limb, SkMotion* walking, SkMotion* standing, int land_time, int stance_time, int lift_time, int walking_style)
{
	_limb = limb;
	MeCtLocomotionLimbAnim* anim = new MeCtLocomotionLimbAnim();
	anim->style = walking_style;
	anim->init_skeleton(limb->standing_skeleton, limb->walking_skeleton);
	//anim->set_skeleton_name(skeleton_name);
	anim->set_support_joint_list(&(limb->support_joint_list));
	int limb_joint_num = get_descendant_num(limb->limb_base_name)+1;
	anim->init_quat_buffers(limb_joint_num);
	analyze_limb_anim(anim, walking, standing, limb->get_limb_base_name(), land_time, stance_time, lift_time);
	limb->walking_list.push() = anim;
}

void MeCtLocomotionAnalysis::init_blended_anim()
{
	MeCtLocomotionLimb* limb = NULL;
	MeCtLocomotionLimbAnim* anim = NULL;
	for(int i = 0; i < _ct_locomotion->get_limb_list()->size();++i)
	{
		limb = _ct_locomotion->get_limb_list()->get(i);
		anim = limb->get_walking_list()->get(0);
		for(int j = 0; j < anim->get_timing_space()->get_ref_time_num(); ++j)
		{
			limb->blended_anim.get_timing_space()->add_ref_time_name(anim->get_timing_space()->get_ref_time_name(j));
		}
	}
}

void MeCtLocomotionAnalysis::print_info()
{
	MeCtLocomotionLimb* limb = NULL;
	for(int i = 0; i < _ct_locomotion->get_limb_list()->size();++i)
	{
		limb = _ct_locomotion->get_limb_list()->get(i);
		//limb->print_info();
	}
}

void MeCtLocomotionAnalysis::add_locomotion(SkMotion* motion_locomotion, int type, int walking_style)
{
	MeCtLocomotionLimb* limb = NULL;
	float lower_bound = 0.0f;
	int i = 0;
	int land_time, lift_time, stance_time;
	for(i = 0; i < _ct_locomotion->get_limb_list()->size();++i)
	{
		limb = _ct_locomotion->get_limb_list()->get(i);


		/*if(type == 1) // forward
		{
			if(i == 1) //right leg
			{
				land_time = 1;
				stance_time = 6;
				lift_time = 16;
			}
			else //left leg
			{
				land_time = 14;
				stance_time = 21;
				lift_time = 1;
			}
		}
		else if(type == 2) //strafe right
		{
			if(i == 1) //right leg
			{
				land_time = 16;
				stance_time = 34;
				lift_time = 0;
			}
			else //left leg
			{
				land_time = 41;
				stance_time = 7;
				lift_time = 28;
			}
		}*/

		if(type == 1)// forward
		{
			if(i == 1) //right leg
			{
				land_time = 1;
				stance_time = 17;
				lift_time = 33;
			}
			else//left leg
			{
				land_time = 29;
				stance_time = 45;
				lift_time = 6;
			}
		}
		else if(type == 2)//strafe right
		{
			if(i == 1) //right leg
			{
				land_time = 24;
				stance_time = 51;
				lift_time = 3;
			}
			else//left leg
			{
				land_time = 50;
				stance_time = 2;
				lift_time = 34;
			}
		}

		if (motion_locomotion->frames() <= land_time)
		{
			LOG("Land time for motion %s at frame %d, only %d frames. Setting to %d.", motion_locomotion->name(), land_time, motion_locomotion->frames(), motion_locomotion->frames() - 1);
			land_time = motion_locomotion->frames() - 1;
		}
		if (motion_locomotion->frames() <= stance_time)
		{
			LOG("Stance time for motion %s at frame %d, only %d frames. Setting to %d.", motion_locomotion->name(), stance_time, motion_locomotion->frames(), motion_locomotion->frames() - 1);
			stance_time = motion_locomotion->frames() - 1;
		}
		if (motion_locomotion->frames() <= lift_time)
		{
			LOG("Lift time for motion %s at frame %d, only %d frames. Setting to %d.", motion_locomotion->name(), lift_time, motion_locomotion->frames(), motion_locomotion->frames() - 1);
			lift_time = motion_locomotion->frames() - 1;
		}

		analyze_walking_limb(limb, motion_locomotion, motion_standing, land_time, stance_time, lift_time, 0);
		//limb->print_info();
		if(i == 0) // let the first limb be the leading limb during analysis process
		{
			lower_bound = limb->get_walking_list()->get(limb->get_walking_list()->size()-1)->get_timing_space()->get_virtual_frame(0);
		}
		limb->get_walking_list()->get(limb->get_walking_list()->size()-1)->get_timing_space()->set_lower_bound(lower_bound);
	}

	//calculate direction & complete velocity list
	SrVec* vel = NULL;
	SrVec sum(0.0f, 0.0f, 0.0f);
	SrVec presum(0.0f, 0.0f, 0.0f);
	SrVec direction(0.0f, 0.0f, 0.0f);
	int count = 0;
	float t = 0.0f;
	float sum_t = 0.0f;
	for(int j = 0; j < motion_locomotion->frames(); ++j)
	{
		count = 0;
		sum.set(0.0f, 0.0f, 0.0f);
		t = 0.0f;
		for(int i = 0; i < _ct_locomotion->get_limb_list()->size();++i)
		{
			limb = _ct_locomotion->get_limb_list()->get(i);
			vel = limb->walking_list.get(limb->walking_list.size()-1)->get_displacement_list()->get(j);
			if(!vel->iszero())
			{
				++count;
				sum += *vel;
			}
		}
		if(count == 0) sum = presum; // if all limbs are in the air, speed and direction remain the same as lift time.
		else sum /= (float)count; // averaqge of all the supporting limbs.
		if(j < motion_locomotion->frames()-1) t = motion_locomotion->keytime(j+1)- motion_locomotion->keytime(j);
		sum_t += t;
		direction += sum;
		presum = sum;
	}

	MeCtLocomotionAnimGlobalInfo* info = new MeCtLocomotionAnimGlobalInfo();
	info->displacement = direction.len();
	direction /= sum_t;
	info->speed = direction.len();
	info->direction = -direction/direction.len();
	_ct_locomotion->get_anim_global_info()->push() = info;

	for(int i = 0; i < _ct_locomotion->get_limb_list()->size();++i)
	{
		limb = _ct_locomotion->get_limb_list()->get(i);
		//limb->walking_list.get(limb->walking_list.size()-1)->direction = -*direction;
		limb->walking_list.get(limb->walking_list.size()-1)->global_info = info;
	}
}

void MeCtLocomotionAnalysis::calc_velocity()
{


}

/*void MeCtLocomotionAnalysis::test_facing(SkMotion* walking)
{
	SkSkeleton* skeleton = walking->connected_skeleton();
	get_ct()->set_skeleton(skeleton);
	SrVec vec;
	for(int i = 0; i < walking->frames(); ++i)
	{
		walking->apply_frame(i);
		walking->connected_skeleton()->update_global_matrices();
		get_ct()->update_facing();
		vec = get_ct()->get_facing();
		LOG("\nfaceing[%d]: <%f, %f, %f>", i, vec.x, vec.y, vec.z);
	}
}*/