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

#include "me_ct_locomotion.hpp"
#include "mcontrol_util.h"
#include "sbm_character.hpp"
#include "gwiz_math.h"
#include "limits.h"
#include <vhcl_log.h>
#include <iostream>
#include <string>


const char* MeCtLocomotion::TYPE = "MeCtLocomotion";


/** Constructor */
MeCtLocomotion::MeCtLocomotion() {
	channels_valid = false;
	limb_joint_num = 0;
	last_time = std::numeric_limits<float>::quiet_NaN();
	nonlimb_joint_info.joint_num = 0;
	nonlimb_joint_info.buff_index.capacity(0);
	nonlimb_joint_info.joint_index.capacity(0);
	nonlimb_joint_info.quat.capacity(0);
	last_time = 0.0f;
	//last_t = -1.0f;
	//ratio = 0.0f;
	dominant_limb = 0;
//	automate = false;
	reset = false;
	dis_initialized = false;
	initialized = false;
	ik_enabled = true;
	enabled = false;
	joints_indexed = false;
	motions_loaded = true;
	base_name = NULL;
	locomotion_anims.capacity(0);
	pre_blended_base_height = 0.0f;
	r_blended_base_height = 0.0f;
	style = 0;
	motion_time = -1.0f;
	last_time = 0.0f;
	translation_joint_height = 0.0f;
	valid = false;
	
}

/** Destructor */
MeCtLocomotion::~MeCtLocomotion() {
	// Nothing allocated to the heap

	int num = limb_list.size();
	for (int i = 0; i < num; i++)
	{
		MeCtLocomotionLimb* limb = limb_list.get(i);
		delete limb;
	}

	for (int i = 0; i < anim_global_info.size(); i++)
	{
		delete anim_global_info[i];
	}
}

// Look up the context indices, and check to make sure it isn't -1
/*#define LOOKUP_BUFFER_INDEX( var_name, index ) \
	var_name = _context->toBufferIndex( _toContextCh[ ( index ) ] );  \
	is_valid &= ( var_name != -1 );*/

int MeCtLocomotion::LOOKUP_BUFFER_INDEX(int var_name, int index )
{
	var_name = _context->toBufferIndex( _toContextCh[ ( index ) ] );
	channels_valid &= ( var_name != -1 );
	return var_name;
}

// Implements MeController::controller_channels().
SkChannelArray& MeCtLocomotion::controller_channels() {
	if( request_channels.size() == 0 ) 
	{
		get_translation_base_joint_index();
		joint_channel_start_ind = navigator.controller_channels(&request_channels);

		limb_joint_num = 0;
		for(int i = 0; i < limb_list.size(); ++i)
		{
			MeCtLocomotionLimb* limb = limb_list.get(i);
			SkJoint* base = standing_skeleton->search_joint(limb->get_limb_base_name());
			//limb_joint_num += iterate_limb_joints(base, 0); // starting from joint_channel_start_ind
			iterate_joints(&(limb->limb_joint_info));
			limb_joint_num += limb->limb_joint_info.joint_num;
		}

		iterate_joints(&nonlimb_joint_info);

		t_joint_quats1.capacity(nonlimb_joint_info.joint_num);
		t_joint_quats2.capacity(nonlimb_joint_info.joint_num);
		t_joint_quats1.size(nonlimb_joint_info.joint_num);
		t_joint_quats2.size(nonlimb_joint_info.joint_num);
		joint_quats1.capacity(nonlimb_joint_info.joint_num);
		joint_quats2.capacity(nonlimb_joint_info.joint_num);
		joint_quats1.size(nonlimb_joint_info.joint_num);
		joint_quats2.size(nonlimb_joint_info.joint_num);
	}
	return request_channels;
}

void MeCtLocomotion::get_translation_base_joint_index()
{
	/*for(int i = 0 ; i < nonlimb_joint_info.joint_name.size(); ++i)
	{
		SkJoint* joint = walking_skeleton->search_joint(nonlimb_joint_info.joint_name.get(i));
		if(!joint->pos()->frozen(1))
		{
			translation_joint_index = i;
			return;
		}
	}*/
	for(int i = 0 ; i < nonlimb_joint_info.joint_name.size(); ++i)
	{
		if(nonlimb_joint_info.joint_name.get(i) == translation_joint_name)
		{
			translation_joint_index = i;
			return;
		}
	}
	translation_joint_index = 0;
}

int MeCtLocomotion::iterate_limb_joints(SkJoint* base, int depth)
{
	const char* name = base->name().get_string();
	int sum = 0;

	if(base->quat()->active() == true)
	{
		request_channels.add( SkJointName(name), SkChannel::Quat );
		sum = 1;
	}
	
	for(int i = 0; i < base->num_children(); ++i)
	{
		sum += iterate_limb_joints(base->child(i), depth+1);
	}
	return sum;
}

void MeCtLocomotion::iterate_joints(MeCtLocomotionJointInfo* joint_info)
{
	for(int i = 0; i < joint_info->joint_num; ++i)
	{
		const char* name = joint_info->joint_name.get(i);
		request_channels.add( SkJointName(name), SkChannel::Quat );
	}
}

// Implements MeController::context_updated(..).
void MeCtLocomotion::context_updated() {
	if( _context == NULL )
		channels_valid = false;
}

void MeCtLocomotion::controller_map_updated() 
{
	if(!joints_indexed)
	{
		int index = 0;
		int k = 0;
		channels_valid = navigator.controller_map_updated(_context, &_toContextCh);
		for(int j = 0; j < limb_list.size(); ++j)
		{
			MeCtLocomotionLimb* limb = limb_list.get(j);
			for(int i = 0; i < limb->limb_joint_info.joint_num; ++i)
			{
				index = LOOKUP_BUFFER_INDEX( index,  k+joint_channel_start_ind);
				if(index < 0)
				{
					LOG("\ni=%d failed to look up buffer index", k);
				}
				limb->limb_joint_info.buff_index.set(i, index);
				++k;
			}
		}

		for(int i = limb_joint_num; i < nonlimb_joint_info.joint_num + limb_joint_num; ++i)
		{
			index = LOOKUP_BUFFER_INDEX( index,  i+joint_channel_start_ind);
			if(index < 0)
			{
				LOG("\ni=%d failed to look up buffer index", i);
			}
			nonlimb_joint_info.buff_index.set(i-limb_joint_num, index);
		}
		joints_indexed = true;
	}

}

//temp function
void MeCtLocomotion::temp_update_for_footprint(MeFrameData& frame)
{
	MeCtLocomotionLimb* limb = NULL;
	int index;
	SrQuat quat;
	SrVec pos;
	SrQuat quat_buff;
	SrBuffer<float>& buffer = frame.buffer(); // convenience reference;

	navigator.controller_evaluate(0.0f, frame);

	for(int i = 0; i < nonlimb_joint_info.buff_index.size(); ++i)
	{
		index = nonlimb_joint_info.buff_index.get(i);
		quat = nonlimb_joint_info.quat.get(i);

		quat_buff.w = buffer[index+0];
		quat_buff.x = buffer[index+1];
		quat_buff.y = buffer[index+2];
		quat_buff.z = buffer[index+3];

		nonlimb_joint_info.quat.set(i, quat_buff);
	}

	update_nonlimb_mat(NULL, NULL, 0);

	for(int j = 0; j < limb_list.size(); ++j)
	{
		limb = limb_list.get(j);
		for(int i = 0; i < limb->limb_joint_info.buff_index.size(); ++i)
		{
			index = limb->limb_joint_info.buff_index.get(i);
			quat = limb->limb_joint_info.quat.get(i);

			quat_buff.w = buffer[index+0];
			quat_buff.x = buffer[index+1];
			quat_buff.y = buffer[index+2];
			quat_buff.z = buffer[index+3];

			limb->limb_joint_info.quat.set(i, quat_buff);
		}
		get_limb_pos(limb);
	}
	calc_rotational_displacement();
	for(int j = 0; j < limb_list.size(); ++j)
	{
		limb = limb_list.get(j);
		for(int i = 0; i < limb->pos_buffer.size(); ++i)
		{
			pos = limb->pos_buffer.get(i);
			pos += navigator.get_world_pos();
			pos -= world_offset_to_base;
			limb->pos_buffer.set(i, pos);
		}
	}

	/*

	navigator.update_world_offset();

	navigator.update_world_mat();

	update_nonlimb_mat_with_global_info();*/


}

bool MeCtLocomotion::is_motions_loaded()
{
	return motions_loaded;
}

bool MeCtLocomotion::controller_evaluate( double time, MeFrameData& frame ) {
	// TODO: Update MeController to pass in delta time.
	// Until then, fake it or compute it ourselves (but there are some gotchas)

	
	if(!valid) return false;
	if(!channels_valid ) return false;

	if(!enabled) return false;
	if(!motions_loaded) return motions_loaded;

	if(limb_list.get(dominant_limb)->walking_list.size() < 2) 
	{
		temp_update_for_footprint(frame);
		return false;
	}
	
	//curr_t = time;

	delta_time = time - last_time;

	if(delta_time > 0.03333333f) delta_time = 0.03333333f;

	if(motion_time > 0.0f)
	{
		motion_time -= delta_time;
		if(motion_time < 0.0f) motion_time = 0.0f;
	}
	if(motion_time == 0.0f)
	{
		navigator.set_reached_destination(frame);
		motion_time = -1.0f;
	}
	SrBuffer<float>& buffer = frame.buffer(); // convenience reference

	navigator.controller_evaluate(delta_time, frame);
	//if the character is stopped, locomotion controller will not take computational time.
	//if(navigator.check_stopped(&limb_list)) 
		//return true;

	float inc_frame;
	inc_frame = (float)(delta_time/0.03333333f);

	if(navigator.has_destination && navigator.get_destination_count() > navigator.get_curr_destination_index() && navigator.get_curr_destination_index()>=0)
	{
		SrVec dis_to_dest = navigator.get_dis_to_dest();
		if(2.0f * dis_to_dest.len() * abs(speed_accelerator.get_min_acceleration_neg()) <= speed_accelerator.get_curr_speed()*speed_accelerator.get_curr_speed())
		{
			if(navigator.get_destination_count() == navigator.get_curr_destination_index()+1) 
				navigator.set_reached_destination(frame);
			else navigator.next_destination(frame);
		}
	}

	MeCtLocomotionLimb* limb;
	if(navigator.get_local_velocity().len() != 0.0f)
	{
		for(int i = 0; i < limb_list.size(); ++i)
		{
			limb = limb_list.get(i);
			if(navigator.has_destination) limb->direction_planner.set_target_direction(&navigator.get_dis_to_dest_local());
			else limb->direction_planner.set_target_direction(&navigator.get_target_local_velocity());
		}
	}
	speed_accelerator.set_target_speed(navigator.get_target_local_velocity().len());

	update(inc_frame, frame);

	if(ik_enabled) apply_IK(); 

	balance.update(limb_list, SrVec(0.0f,1.0f,0.0f), &nonlimb_joint_info, navigator.get_facing_angle(), translation_joint_index, (float)delta_time);

	navigator.post_controller_evaluate(frame, limb_list.get(dominant_limb), reset);

	reset = false;

	int index = 0;
	SrQuat quat;
	SrQuat quat_buff;

	for(int i = 0; i < limb_list.size(); ++i)
	{
		MeCtLocomotionJointInfo* info = &(limb_list.get(i)->limb_joint_info);
		for(int j = 0; j < info->quat.size(); ++j)
		{
			index = info->buff_index.get(j);
			quat = info->quat.get(j);

			buffer[index+0] = (float)quat.w;
			buffer[index+1] = (float)quat.x;
			buffer[index+2] = (float)quat.y;
			buffer[index+3] = (float)quat.z;
		}
	}

	for(int i = 0; i < nonlimb_joint_info.joint_num; ++i)
	{
		index = nonlimb_joint_info.buff_index.get(i);
		quat = nonlimb_joint_info.quat.get(i);

		buffer[index+0] = (float)quat.w;
		buffer[index+1] = (float)quat.x;
		buffer[index+2] = (float)quat.y;
		buffer[index+3] = (float)quat.z;
	}
	
	last_time = time;
	return true;
}

bool MeCtLocomotion::is_initialized()
{
	return initialized;
}

bool MeCtLocomotion::is_enabled()
{
	return enabled;
}

bool MeCtLocomotion::is_valid()
{
	return valid;
}

bool MeCtLocomotion::is_channels_valid()
{
	return channels_valid;
}

void MeCtLocomotion::set_valid(bool valid)
{
	this->valid = valid;
}

SrArray<MeCtLocomotionLimb*>* MeCtLocomotion::get_limb_list()
{
	return &limb_list;
}

SrArray<MeCtLocomotionAnimGlobalInfo*>* MeCtLocomotion::get_anim_global_info()
{
	return &anim_global_info;
}

void MeCtLocomotion::set_balance_factor(float factor)
{
	balance.set_factor(factor);
}

SrVec MeCtLocomotion::calc_rotational_displacement()
{
	SrVec v;
	SkJoint* tjoint = walking_skeleton->search_joint(nonlimb_joint_info.joint_name.get(0));
	SrMat pmat;
	/*SrMat gmat, pmat, lmat;
	gmat.identity();
	for(int i = 0; i <= translation_joint_index; ++i)
	{
		pmat = gmat;
		lmat = get_lmat(tjoint, &(nonlimb_joint_info.quat.get(i)));
		gmat = lmat * pmat;
		if(tjoint->num_children()>0)
		{ 
			tjoint = tjoint->child(0);
		}
		else break;
	}
	
	v.set(-gmat.e41(), -gmat.e42(), -gmat.e43());*/

	//v = -navigator.get_base_pos();

	v = -get_offset(walking_skeleton->root(), translation_joint_index+1, nonlimb_joint_info.quat);

	v -= navigator.get_base_pos();

	pmat.roty(navigator.get_facing_angle());

	world_offset_to_base = v*pmat;
	v = world_offset_to_base - pre_world_offset_to_base;
	v.y = 0.0f;

	pre_world_offset_to_base = world_offset_to_base;

	if(!dis_initialized) 
	{
		v.set(0.0f, 0.0f, 0.0f);
		return v;
	}
	return v;
}

SrVec MeCtLocomotion::get_base_pos()
{
	return navigator.get_world_pos()-world_offset_to_base;
}

void MeCtLocomotion::init_nonlimb_joint_info()
{
	SrArray<char*> limb_base_name;
	limb_base_name.capacity(limb_list.size());
	limb_base_name.size(limb_list.size());
	for(int i = 0; i < limb_list.size(); ++i)
	{
		limb_base_name.set(i, limb_list.get(i)->get_limb_base_name());
	}
	const char* base_name = walking_skeleton->root()->name().get_string();
	nonlimb_joint_info.Init(walking_skeleton, base_name, &limb_base_name);
	SkJoint* joint = NULL;
	int index = -1;
	for(int i = 0; i < limb_list.size(); ++i)
	{
		joint = walking_skeleton->search_joint(limb_list.get(i)->get_limb_base_name());
		while(true)
		{
			joint = joint->parent();
			if(joint == NULL) break;
			index = nonlimb_joint_info.get_index_by_name(joint->name().get_string());
			nonlimb_joint_info.mat_valid.set(index, 1);
		}
	}
}

void MeCtLocomotion::init_skeleton(SkSkeleton* standing, SkSkeleton* walking)
{
	walking_skeleton = walking;
	standing_skeleton = standing;
	initialized = true;
}

const char* MeCtLocomotion::controller_type( void )	const {

	return TYPE;
}

void MeCtLocomotion::set_base_name(const char* name)
{
	if(base_name != NULL) free(base_name);
	base_name = (char*)malloc(sizeof(char)*(strlen(name)+1));
	strcpy(base_name, name);
}

/*void MeCtLocomotion::set_nonlimb_blending_base_name(const char* name)
{
	if(nonlimb_blending_base_name != NULL) free(nonlimb_blending_base_name);
	nonlimb_blending_base_name = (char*)malloc(sizeof(char)*(strlen(name)+1));
	strcpy(nonlimb_blending_base_name, name);
}*/

void MeCtLocomotion::set_skeleton(SkSkeleton* skeleton)
{
	_skeleton_ref_p = skeleton;
}

void MeCtLocomotion::set_turning_speed(float radians)
{
	for(int i = 0; i < limb_list.size(); ++i)
	{
		limb_list.get(i)->direction_planner.set_turning_speed(radians);
	}
}

int MeCtLocomotion::determine_dominant_limb()
{
	SrVec pos;
	SrMat mat;
	mat.rot(navigator.get_local_velocity(), SrVec(0,0,1));
	int min_ind = -1;
	float min = 0.0f;
	for(int i = 0; i < limb_list.size(); ++i)
	{
		pos = get_limb_pos(limb_list.get(i));
		pos = pos*mat;
		if(min_ind == -1 || pos.z < min) 
		{
			min_ind = i;
			min = pos.z;
		}
		/*else
		{
			if(pos.z < min) 
			{
				min_ind = i;
				min = pos.z;
			}
		}*/
	}
	return min_ind;
}

void MeCtLocomotion::blend_base_joint(MeFrameData& frame, float space_time, int anim_index1, int anim_index2, float weight)
{
	pre_blended_base_height = r_blended_base_height;

	SrQuat rot1, rot2, rot3, rot4;
	float ratio = 0.0f;
	float pheight = 0.0f;
	char* translate_base;
	SkJoint* base;

	MeCtLocomotionLimbAnim* anim1 = limb_list.get(0)->walking_list.get(anim_index1);
	MeCtLocomotionLimbAnim* anim2 = limb_list.get(0)->walking_list.get(anim_index2);
	float frame1 = anim1->get_timing_space()->get_virtual_frame(space_time);
	float frame2 = anim2->get_timing_space()->get_virtual_frame(space_time);

	translate_base = (char*)nonlimb_joint_info.joint_name.get(translation_joint_index);
	base = walking_skeleton->search_joint(translate_base);

	anim1->walking->connect(walking_skeleton);
	ratio = frame1 - (int)frame1;

	anim1->walking->apply_frame((int)frame1);
	pheight = base->pos()->value(1);
	r_blended_base_height = pheight*(1.0f-ratio)*(weight);

	anim1->walking->apply_frame((int)frame1+1);
	pheight = base->pos()->value(1);
	r_blended_base_height += pheight*ratio*(weight);

	anim2->walking->connect(walking_skeleton);
	ratio = frame2 - (int)frame2;

	anim2->walking->apply_frame((int)frame2);
	pheight = base->pos()->value(1);
	r_blended_base_height += pheight*(1.0f-ratio)*(1.0f-weight);

	anim2->walking->apply_frame((int)frame2+1);
	pheight = base->pos()->value(1);
	r_blended_base_height += pheight*ratio*(1.0f-weight);

	SrBuffer<float>& buffer = frame.buffer();

	SrVec base_pos = navigator.get_base_pos();
	base->pos()->value(0, base_pos.x);
	base->pos()->value(1, base_pos.y);
	base->pos()->value(2, base_pos.z);

	r_blended_base_height = r_blended_base_height * (navigator.standing_factor) + base_pos.y * (1.0f-navigator.standing_factor);

}

void MeCtLocomotion::set_motion_time(float time)
{
	navigator.reached_destination = false;
	motion_time = time;
}

// main stream
void MeCtLocomotion::update(float inc_frame, MeFrameData& frame)
{
	if(inc_frame < 0) inc_frame = 0.0f;
	float frame_num = 0.0f; 
	float ratio = 0.0f;
	float dom_ratio = 0.0f;

	// set the after limb the dominant limb and the the space value to 0 when starting the locomotion.
	if(navigator.standing_factor == 0.0f)
	{
		//if starting locomotion from standing pose
		if(speed_accelerator.get_target_speed() != 0.0f)
		{
			dominant_limb = determine_dominant_limb();
			limb_list.get(dominant_limb)->space_time = 0.0f;
		}
	}

	//if(speed_accelerator.get_target_speed() > speed_accelerator.get_curr_speed() || limb_list.get(dominant_limb)->space_time > 2.0f) 
	speed_accelerator.update_speed(delta_time);

	//get current direction
	limb_list.get(dominant_limb)->direction_planner.update_direction(delta_time, &limb_list.get(dominant_limb)->space_time, 3, true);

	// set r_anim1_index_dominant amd r_anim2_index_dominant
	get_anim_indices(dominant_limb, limb_list.get(dominant_limb)->direction_planner.get_curr_direction(), &r_anim1_index_dominant, &r_anim2_index_dominant);

	//SrVec v = limb_list.get(dominant_limb)->direction_planner.get_curr_direction();
	//printf("\n(%f, %f, %f)", v.x, v.y, v.z);

	MeCtLocomotionLimbAnim* anim1 = limb_list.get(dominant_limb)->get_walking_list()->get(r_anim1_index_dominant);
	MeCtLocomotionLimbAnim* anim2 = limb_list.get(dominant_limb)->get_walking_list()->get(r_anim2_index_dominant);
	MeCtLocomotionLimbAnim* blended_anim = &limb_list.get(dominant_limb)->blended_anim;

	// get current acceleration
	float acc = speed_accelerator.update(&(limb_list.get(dominant_limb)->direction_planner.get_curr_direction()), limb_list.get(dominant_limb));

	// get the ratio of the two animations
	dom_ratio = limb_list.get(dominant_limb)->direction_planner.get_ratio(anim1, anim2);

	navigator.update_framerate_accelerator(acc, &limb_list);

	get_blended_timing_space(blended_anim->get_timing_space(), anim1->get_timing_space(), anim2->get_timing_space(), dom_ratio);


	//the current frame number is the addition of previous frame number + increased frame num * acceleration
	frame_num = blended_anim->get_timing_space()->get_virtual_frame(limb_list.get(dominant_limb)->space_time) + inc_frame * navigator.framerate_accelerator;
	frame_num = blended_anim->get_timing_space()->get_normalized_frame(frame_num); // in case new frame number is beyond the range. 

	//space time was computed with current frame number
	if(navigator.standing_factor != 0.0f) 
		limb_list.get(dominant_limb)->space_time = blended_anim->get_timing_space()->get_space_value(frame_num);
	
	// update the current orientation of dominant limb
	navigator.update_facing(limb_list.get(dominant_limb), true);

	// blend the two animations
	limb_list.get(dominant_limb)->blend_anim(limb_list.get(dominant_limb)->space_time, r_anim1_index_dominant, r_anim2_index_dominant, dom_ratio, &(limb_list.get(dominant_limb)->limb_joint_info.joint_index));

	// update acceleration based on current timing space and space time.
	speed_accelerator.update_acceleration(limb_list.get(dominant_limb), blended_anim->get_timing_space());
 
	for(int i = 0; i < limb_list.size(); ++i)
	{
		if(i != dominant_limb)
		{
			//the following code computes the space value in accordance with that of the dominant limb:
			//get a subordinate timing space which has the same scale as the dominant timing space,
			//then map the frame number onto the space value of this timing space.
			//the reason for this is that provided the freedom of all the limbs, the animation can be 
			//unsmooth if frame number is not mapped properly onto the dominant limb's timing space
			get_anim_indices(i, limb_list.get(dominant_limb)->direction_planner.get_curr_direction(), &r_anim1_index, &r_anim2_index);
			anim1 = limb_list.get(i)->get_walking_list()->get(r_anim1_index);
			anim2 = limb_list.get(i)->get_walking_list()->get(r_anim2_index);
			blended_anim = &limb_list.get(i)->blended_anim;
			get_blended_timing_space(blended_anim->get_timing_space(), anim1->get_timing_space(), anim2->get_timing_space(), dom_ratio);
			
			if(navigator.standing_factor != 0.0f) 
				limb_list.get(i)->space_time = blended_anim->get_timing_space()->get_space_value(frame_num);
			
			//compute the direction and orientation based on the real timing space
			limb_list.get(i)->direction_planner.update_direction(delta_time, &limb_list.get(i)->space_time, blended_anim->get_timing_space()->get_ref_time_num(), false);
			get_anim_indices(i, limb_list.get(i)->direction_planner.get_curr_direction(), &r_anim1_index, &r_anim2_index);
			anim1 = limb_list.get(i)->get_walking_list()->get(r_anim1_index);
			anim2 = limb_list.get(i)->get_walking_list()->get(r_anim2_index);
			ratio = limb_list.get(i)->direction_planner.get_ratio(anim1, anim2);
			get_blended_timing_space(blended_anim->get_timing_space(), anim1->get_timing_space(), anim2->get_timing_space(), ratio);
			navigator.update_facing(limb_list.get(i), false);
			limb_list.get(i)->blend_anim(limb_list.get(i)->space_time, r_anim1_index, r_anim2_index, ratio, &(limb_list.get(i)->limb_joint_info.joint_index));
		}
	}

	//blend non-limb joints
	anim1 = limb_list.get(0)->get_walking_list()->get(r_anim1_index_dominant);
	anim2 = limb_list.get(0)->get_walking_list()->get(r_anim2_index_dominant);
	get_frame(locomotion_anims.get(r_anim1_index_dominant - 1), walking_skeleton, anim1->get_timing_space()->get_virtual_frame(limb_list.get(0)->space_time), base_name, &joint_quats1, &t_joint_quats1, &t_joint_quats2, &(nonlimb_joint_info.joint_index));
	get_frame(locomotion_anims.get(r_anim2_index_dominant - 1), walking_skeleton, anim2->get_timing_space()->get_virtual_frame(limb_list.get(0)->space_time), base_name, &joint_quats2, &t_joint_quats1, &t_joint_quats2, &(nonlimb_joint_info.joint_index));
	get_blended_quat_buffer(&(nonlimb_joint_info.quat), &joint_quats1, &joint_quats2, dom_ratio);

	//recompute the dominant limb
	determine_dominant_limb_index();

	//blend with standing animation if standing factor > 0
	blend_standing(frame);

	for(int i = 0; i < limb_list.size(); ++i)
	{
		if(limb_list.get(i)->curr_rotation == 0.0f) continue;
		SkJoint* tjoint = walking_skeleton->search_joint(limb_list.get(i)->get_limb_base_name());
		int parent_ind = nonlimb_joint_info.get_index_by_name(tjoint->parent()->name().get_string());
		SrMat mat = nonlimb_joint_info.mat.get(parent_ind);
		limb_list.get(i)->manipulate_turning(mat);
	}

	last_time = limb_list.get(dominant_limb)->space_time;

	blend_base_joint(frame, limb_list.get(0)->space_time, r_anim1_index_dominant, r_anim2_index_dominant, dom_ratio);

	update_nonlimb_mat(NULL, NULL, 0);

	update_pos();

	navigator.update_world_offset(displacement);

	navigator.update_world_mat();

	if(ik_enabled)
	{
		height_offset.set_limb_list(&limb_list);
		height_offset.set_translation_base_joint_height(translation_joint_height);
		//SrVec displacement(0,0,0);
		SrMat w_mat = navigator.get_world_mat();

		w_mat.set(12, w_mat.get(12)-world_offset_to_base.x);
		w_mat.set(13, w_mat.get(13)-world_offset_to_base.y);
		w_mat.set(14, w_mat.get(14)-world_offset_to_base.z);
		//SrMat w_mat = get_gmat(walking_skeleton->root(), translation_joint_index+1, nonlimb_joint_info.quat);
		//w_mat = w_mat * navigator.get_world_mat();

		//height_offset.update_height_offset(nonlimb_joint_info.mat.get(translation_joint_index) * w_mat, r_blended_base_height);
		height_offset.update_height_offset(w_mat, r_blended_base_height, (float)delta_time);
		w_mat.set(13, navigator.get_world_mat().get(13) + height_offset.get_height_offset());
		navigator.set_world_mat(w_mat);
		navigator.world_pos.y += height_offset.get_height_offset();
	}

	update_nonlimb_mat_with_global_info();

	//printf("\ndominant: %d", dominant_limb);

}

float MeCtLocomotion::get_current_speed()
{
	return speed_accelerator.get_curr_speed();
}

void MeCtLocomotion::update_nonlimb_mat_with_global_info()
{
	SrMat mat = navigator.get_world_mat();
	for(int i = 0; i < nonlimb_joint_info.joint_name.size(); ++i)
	{
		nonlimb_joint_info.mat.set(i, nonlimb_joint_info.mat.get(i)*mat);
	}
}

int MeCtLocomotion::get_dominant_limb_index()
{
	return dominant_limb;
}

void MeCtLocomotion::update_limb_mat_with_global_info()
{
	SrMat mat = navigator.get_world_mat();
	for(int j = 0; j < limb_list.size(); ++j)
	{
		MeCtLocomotionLimb* limb = limb_list.get(j);
		for(int i = 0; i < nonlimb_joint_info.joint_name.size(); ++i)
		{
			limb->limb_joint_info.mat.set(i, limb->limb_joint_info.mat.get(i)*mat);
		}
	}
}

void MeCtLocomotion::blend_standing(MeFrameData& frame)
{
	if(navigator.standing_factor == 1.0f) return;
	MeCtLocomotionLimb* limb = NULL;
	int index;
	SrQuat quat;
	SrQuat quat_buff;
	SrBuffer<float>& buffer = frame.buffer();
	for(int j = 0; j < limb_list.size(); ++j)
	{
		limb = limb_list.get(j);
		for(int i = 0; i < limb->limb_joint_info.buff_index.size(); ++i)
		{
			index = limb->limb_joint_info.buff_index.get(i);
			quat = limb->limb_joint_info.quat.get(i);

			quat_buff.w = buffer[index+0];
			quat_buff.x = buffer[index+1];
			quat_buff.y = buffer[index+2];
			quat_buff.z = buffer[index+3];

			quat_buff = slerp(quat_buff, quat, navigator.standing_factor);
			limb->limb_joint_info.quat.set(i, quat_buff);
		}
	}
	for(int i = 0; i < nonlimb_joint_info.buff_index.size(); ++i)
	{
		index = nonlimb_joint_info.buff_index.get(i);
		quat = nonlimb_joint_info.quat.get(i);

		quat_buff.w = buffer[index+0];
		quat_buff.x = buffer[index+1];
		quat_buff.y = buffer[index+2];
		quat_buff.z = buffer[index+3];

		quat_buff = slerp(quat_buff, quat, navigator.standing_factor);
		nonlimb_joint_info.quat.set(i, quat_buff);
	}
}

/*void MeCtLocomotion::update_nonlimb_mat()//without global rotation
{
	SkJoint* joint = walking_skeleton->root();
	SrMat mat;
	mat = get_lmat(joint, &(nonlimb_joint_info.quat.get(0)));
	mat.set(12, 0.0f);
	mat.set(13, 0.0f);
	mat.set(14, 0.0f);
	nonlimb_joint_info.mat.set(0, mat);
	update_nonlimb_mat(joint, &mat);
}*/

void MeCtLocomotion::update_nonlimb_mat(SkJoint* joint, SrMat* mat, int depth)
{
	if(joint == NULL) joint = walking_skeleton->root();
	//SkJoint* tjoint = NULL;
	int index = -1;
	SrMat lmat;
	SrMat gmat;
	if(mat == NULL) mat = &gmat;

	for(int j = 0; j < limb_list.size(); ++j)
	{
		if(strcmp(joint->name().get_string(), limb_list.get(j)->limb_base_name) == 0) 
		{
			return;
		}
	}

	index = nonlimb_joint_info.get_index_by_name(joint->name().get_string());
	if(nonlimb_joint_info.mat_valid.get(index) != 1) return;
	lmat = get_lmat(joint, &(nonlimb_joint_info.quat.get(index)));
	if(depth <= translation_joint_index)
	{
		lmat.set(12, 0.0f);
		lmat.set(13, 0.0f);
		lmat.set(14, 0.0f);
	}
	gmat = lmat * *mat;
	nonlimb_joint_info.mat.set(index, gmat);
	for(int i = 0; i < joint->num_children(); ++i)
	{
		update_nonlimb_mat(joint->child(i), &gmat, depth+1);
	}
}

void MeCtLocomotion::set_target_height_displacement(float displacement)
{
	navigator.target_height_adjustment = displacement;
}

// temp function
SrVec MeCtLocomotion::get_supporting_joint_pos(int joint_index, int limb_index, SrVec* orientation, SrVec* normal)
{
	//SrVec pos = limb_list.get(index)->pos_buffer.get(2) * navigator.get_world_mat();
	//pos.y -= limb_list.get(index)->support_height.get(0);

	int joint_index_plus = joint_index+1;
	if(joint_index_plus >= limb_list.get(limb_index)->support_joint_list.size())
	{
		joint_index_plus = joint_index-1;
	}
	float tnormal[3];
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SrVec pos = limb_list.get(limb_index)->pos_buffer.get(2+joint_index);
	float height = mcu.query_terrain(pos.x, pos.z, tnormal);
	pos.y = height;
	SrVec pos1 = limb_list.get(limb_index)->pos_buffer.get(2+joint_index_plus);
	height = mcu.query_terrain(pos1.x, pos1.z, NULL);
	pos1.y = height;
	if(joint_index_plus > joint_index) *orientation = pos1-pos;
	else *orientation = pos-pos1;
	normal->set(tnormal[0], tnormal[1], tnormal[2]);
	return pos;
}
// temp function
void MeCtLocomotion::apply_IK()
{
	//if(navigator.target_height_displacement == 0.0f) return;
	MeCtIKScenario* ik_scenario = NULL;
	MeCtIKScenarioJointInfo* info = NULL;
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	SrMat global_mat;
	global_mat = navigator.get_world_mat();

	global_mat.set(12, global_mat.get(12) - world_offset_to_base.x);
	global_mat.set(13, global_mat.get(13) - world_offset_to_base.y);
	global_mat.set(14, global_mat.get(14) - world_offset_to_base.z);
	//global_mat = get_gmat(walking_skeleton->root(), translation_joint_index+1, nonlimb_joint_info.quat);

	//global_mat = global_mat * navigator.get_world_mat();

	for(int i = 0; i < limb_list.size(); ++i)
	{
		ik_scenario = &(limb_list.get(i)->ik);

		//temp...............
		//ik_scenario->joint_info_list.get(2).constraint.ball.max = 3.14159265f/4.0f;
		//temp...............

		SkJoint* tjoint = walking_skeleton->search_joint(limb_list.get(i)->get_limb_base_name());
		int parent_ind = nonlimb_joint_info.get_index_by_name(tjoint->parent()->name().get_string());
		ik_scenario->mat = nonlimb_joint_info.mat.get(parent_ind);
		ik_scenario->mat.set(12, ik_scenario->mat.get(12) - world_offset_to_base.x);
		ik_scenario->mat.set(13, ik_scenario->mat.get(13) - world_offset_to_base.y);
		ik_scenario->mat.set(14, ik_scenario->mat.get(14) - world_offset_to_base.z);
		ik_scenario->start_joint = &(ik_scenario->joint_info_list.get(0));
		ik_scenario->end_joint = &(ik_scenario->joint_info_list.get(ik_scenario->joint_info_list.size()-1));
		
		SrVec pos = limb_list.get(i)->pos_buffer.get(2);
		pos  = pos * global_mat;


		//printf("\n(%f, %f)", pos.x, pos.z);

		float normal[3] = {0.0f, 0.0f, 0.0f};

		float height = mcu.query_terrain(pos.x, pos.z, normal);

		ik_scenario->set_offset(limb_list.get(i)->ik_offset);
		
		ik_scenario->quat_list = limb_list.get(i)->limb_joint_info.quat;

		ik_scenario->ik_orientation.set(0.0f, -1.0f, 0.0f);

		ik_scenario->set_plane_normal(SrVec(limb_list.get(i)->ik_terrain_normal.x, limb_list.get(i)->ik_terrain_normal.y, limb_list.get(i)->ik_terrain_normal.z));
		ik_scenario->plane_point = SrVec(pos.x, height, pos.z);

		SrVec v1 = -limb_list.get(i)->ik_terrain_normal;
		ik_scenario->ik_compensate_factor = dot(v1, ik_scenario->ik_orientation);
		for(int j = 0; j < ik_scenario->joint_info_list.size(); ++j)
		{
			info = &(ik_scenario->joint_info_list.get(j));
			info->support_joint_comp = translation_joint_height + r_blended_base_height + limb_list.get(i)->pos_buffer.get(j).y - info->support_joint_height;
			if(info->support_joint_comp < 0.0f) info->support_joint_comp = 0.0f;// mannually check if the compensation is valid
		}

		ik.update(ik_scenario);
		limb_list.get(i)->limb_joint_info.quat = ik_scenario->quat_list;
		limb_list.get(i)->pos_buffer = ik.joint_pos_list;
	}
}

void MeCtLocomotion::get_anim_indices(int limb_index, SrVec direction, int* anim1_index, int* anim2_index)
{
	MeCtLocomotionLimb* limb = limb_list.get(limb_index);
	float angle1 = -1.0f, angle2 = -1.0f;
	float angle = 0.0f;
	SrVec dir1(0.0f, 1.0f, 0.0f);
	SrVec dir2(0.0f, -1.0f, 0.0f);
	SrVec d;
	int mode1 = 0, mode2 = 0;
	int index1  = -1, index2 = -1;
	for(int i = 1; i < limb->get_walking_list()->size(); ++i)
	{
		MeCtLocomotionLimbAnim* anim = limb->get_walking_list()->get(i);
		if(style != anim->style) continue;
		d = cross(direction, anim->global_info->direction);
		angle = dot(direction, anim->global_info->direction);
		if(dot(d, dir1) > 0.0f)
		{
			if(angle1 < angle)
			{
				angle1 = angle;
				mode1 = 1;
				index1 = i;
			}
			if(angle2 < -angle)
			{
				angle2 = -angle;
				mode2 = -1;
				index2 = i;
			}
		}
		else
		{
			if(angle1 < -angle)
			{
				angle1 = -angle;
				mode1 = -1;
				index1 = i;
			}
			if(angle2 < angle)
			{
				angle2 = angle;
				mode2 = 1;
				index2 = i;
			}
		}
	}

	*anim1_index = index1;
	*anim2_index = index2;
	limb->get_walking_list()->get(index1)->get_timing_space()->set_mode(mode1);
	limb->get_walking_list()->get(index2)->get_timing_space()->set_mode(mode2);
}

int MeCtLocomotion::determine_dominant_limb_index()
{
	if(navigator.standing_factor == 0.0f)
	{
		++dominant_limb;
		if(dominant_limb >= limb_list.size()) dominant_limb = 0;
		return dominant_limb;
	}
	float remnant = 0.0f;
	float r = -1;
	for(int i = 0; i < limb_list.size(); ++i)
	{
		if(limb_list.get(i)->space_time > 1.0f && limb_list.get(i)->space_time < 2.0f) continue;
		else if(limb_list.get(i)->space_time <= 1.0f) 
		{
			r = 1- limb_list.get(i)->space_time;
		}
		else if(limb_list.get(i)->space_time >= 2.0f) 
		{
			r = limb_list.get(i)->space_time - 1;
		}
		if(r < 0.0f) 
		{
			LOG("Error: can not determine dominant limb. \n space_time_limb1=%f\nspace_time_limb2=%f", limb_list.get(0)->space_time, limb_list.get(1)->space_time);
			r = 0;
		}
		if(r > remnant) 
		{
			remnant = r;
			dominant_limb = i;
		}
	}

	return dominant_limb;
}

SrVec MeCtLocomotion::get_limb_pos(MeCtLocomotionLimb* limb)
{
	SrMat gmat;
	SrMat pmat;
	SrMat lmat;
	SrVec pos;
	SkJoint* tjoint = NULL;
	SkJoint* tjoint_base = NULL;
	//float* ppos;
	SkSkeleton* skeleton = limb->walking_skeleton;

	tjoint = skeleton->search_joint(limb->get_limb_base_name());
	tjoint_base = skeleton->search_joint(tjoint->parent()->name().get_string());
	int parent_ind = nonlimb_joint_info.get_index_by_name(tjoint->parent()->name().get_string());
	gmat = nonlimb_joint_info.mat.get(parent_ind);
	//gmat.set(12, 0.0f);
	//gmat.set(13, 0.0f);
	//gmat.set(14, 0.0f);


	for(int j  = 0; j <= limb->limb_joint_info.quat.size()-1; ++j)
	{
		pmat = gmat;
		lmat = get_lmat(tjoint, &(limb->limb_joint_info.quat.get(j)));
		gmat = lmat * pmat;
		pos.set(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14));
		limb->pos_buffer.set(j, pos);
		if(tjoint->num_children()>0)
		{ 
			tjoint = tjoint->child(0);
		}
		else break;
	}

	pos.set(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14));
	//pos.set(*gmat.pt(12)-origin.x, *gmat.pt(13)-origin.y, *gmat.pt(14)-origin.z);

	//if(abs_ground_height == 0.0f) abs_ground_height = pos.y+navigator.get_world_pos().y;

	return pos;
}

void MeCtLocomotion::update_pos()
{
	SkJoint* tjoint = NULL;
	SrVec currpos;
	
	SrVec dis[2];//temp;
	float ratio[2];//temp
	float sum = 0.0f;
	displacement.set(0,0,0);
	int y = 0;
	for(int i = 0; i < limb_list.size(); ++i)
	{
		MeCtLocomotionLimb* limb = limb_list.get(i);
		currpos = get_limb_pos(limb);
		ratio[i] = 0;
		dis[i].set(0,0,0);
		//if(i == dominant_limb)
		if((limb->space_time >= 2.0f || limb->space_time <= 1.0f))
		{
			if(limb->space_time >= 2.0f) ratio[i] = limb->space_time - 2.0f;
			else if(limb->space_time <= 1.0f) ratio[i] = 1.0f - limb->space_time;
			//ratio[i] *= ratio[i];
			SrMat mat;

			mat.roty(navigator.get_pre_facing_angle());

			SrVec v1 = limb->pos * mat;

			mat.roty(navigator.get_facing_angle());

			SrVec v2 = currpos * mat;

			dis[i] = v1 - v2;

			dis[i].y = 0.0f;
			sum += ratio[i];
			++y;
		}
		limb->pos = currpos;

	}

	if(dis_initialized)
	{
		if(sum != 0.0f)
		{
			for(int i = 0; i < 2; ++i)
			{
				displacement += dis[i]*ratio[i]/sum;
			}
			
			if(navigator.standing_factor != 0.0f)
			{
				// update the ik_offset for each limb
				for(int i = 0; i < 2; ++i)
				{
					MeCtLocomotionLimb* limb = limb_list.get(i);
					if(limb->space_time >= 2.0f || limb->space_time <= 1.0f)
					{
						limb->ik_offset += (dis[i]-displacement)/3.0f;
						limb->ik_offset_record = limb->ik_offset;
						//printf("\n(%f, %f, %f)", limb->ik_offset.x, limb->ik_offset.y, limb->ik_offset.z);
					}
					else if(limb->space_time > 1.0f && limb->space_time <= 1.5f)
					{
						limb->ik_offset = limb->ik_offset_record * (1.5f - limb->space_time)*2.0f;
					}
					else
						limb->ik_offset.set(0,0,0);
				}
			}

			displacement += calc_rotational_displacement();
			//displacement.x = 0.0f;
			displacement.y = r_blended_base_height-pre_blended_base_height;
			//displacement.z = 0.0f;
		}
		else
		{
			LOG("No limb touches the ground");
		}
	}
	else dis_initialized = true;
	//navigator.update_displacement(&displacement);
}


MeCtLocomotionNavigator* MeCtLocomotion::get_navigator()
{
	return &navigator;
}

void MeCtLocomotion::add_locomotion_anim(SkMotion* anim)
{
	locomotion_anims.push() = anim;
}

SrVec MeCtLocomotion::get_facing_vector()
{
	SrMat mat;
	float angle = navigator.get_facing_angle();
	mat.roty(angle);
	SrVec direction(0.0, 0.0, 1.0f);
	direction = direction*mat;
	return direction;
}

void MeCtLocomotion::print_info(char* name)
{
	LOG("Locomotion status of character: %s", name);
	LOG("Animations loaded:");
	for(int i = 0; i < locomotion_anims.size(); ++i)
	{
		LOG("\t[%d] %s", i, locomotion_anims.get(i)->name());
	}

	LOG("Limbs:");
	LOG("  Total number: %d", limb_list.size());
	MeCtLocomotionLimb* limb;
	for(int i = 0; i < limb_list.size(); ++i)
	{
		limb = limb_list.get(i);
		LOG("\t%s:", limb->limb_name.get(0));
		//limb->print_info();
		LOG("\tSupport joints:");
		for(int j = 0; j < limb->get_support_joint_num(); ++j)
		{
			LOG("\t\t%s", (const char*)*(limb->support_joint_list.get(j)));
		}
	}

	if(initialized) LOG("Initialized: Yes");
	else LOG("Initialized: No");

	if(enabled) LOG("Enabled: Yes");
	else LOG("Enabled: No");
}
