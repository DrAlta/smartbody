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
	is_valid = false;
	limb_joint_num = 0;
	last_time = std::numeric_limits<float>::quiet_NaN();
	nonlimb_joint_info.joint_num = 0;
	nonlimb_joint_info.buff_index.capacity(0);
	nonlimb_joint_info.joint_index.capacity(0);
	nonlimb_joint_info.quat.capacity(0);
	last_time = 0.0f;
	//last_t = -1.0f;
	ratio = 0.0f;
	dominant_limb = 0;
	automate = false;
	reset = false;
	dis_initialized = false;
	initialized = false;
	ik_enabled = true;
	enabled = false;
	joints_indexed = false;
	motions_loaded = true;
	base_name = NULL;
	nonlimb_blending_base_name = NULL;
	locomotion_anims.capacity(0);
	pre_blended_base_height = 0.0f;
	r_blended_base_height = 0.0f;
	style = 0;
	motion_time = -1.0f;

	abs_ground_height = 0.0f;
	
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

/*void MeCtLocomotion::init_limbs()
{
	MeCtLocomotionLimb* limb = NULL;
	for(int i = 0; i < limb_list.size(); ++i)
	{
		limb = limb_list.get(i);
		limb->pos = -get_limb_pos(limb);
	}
}*/

// Look up the context indices, and check to make sure it isn't -1
/*#define LOOKUP_BUFFER_INDEX( var_name, index ) \
	var_name = _context->toBufferIndex( _toContextCh[ ( index ) ] );  \
	is_valid &= ( var_name != -1 );*/

int MeCtLocomotion::LOOKUP_BUFFER_INDEX(int var_name, int index )
{
	var_name = _context->toBufferIndex( _toContextCh[ ( index ) ] );
	is_valid &= ( var_name != -1 );
	return var_name;
}

// Implements MeController::controller_channels().
SkChannelArray& MeCtLocomotion::controller_channels() {
	if( request_channels.size() == 0 ) 
	{
		get_translation_base_joint_index();

		//request_channels.add( SkJointName( nonlimb_joint_info.joint_name.get(translation_joint_index) ), SkChannel::XPos );  // 4
		//request_channels.add( SkJointName( nonlimb_joint_info.joint_name.get(translation_joint_index) ), SkChannel::YPos );  // 5
		//request_channels.add( SkJointName( nonlimb_joint_info.joint_name.get(translation_joint_index) ), SkChannel::ZPos );  // 6

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
	for(int i = 0 ; i < nonlimb_joint_info.joint_name.size(); ++i)
	{
		SkJoint* joint = walking_skeleton->search_joint(nonlimb_joint_info.joint_name.get(i));
		if(!joint->pos()->frozen(1))
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
		is_valid = false;
}

void MeCtLocomotion::controller_map_updated() 
{
	if(!joints_indexed)
	{
		int index = 0;
		int k = 0;
		is_valid = navigator.controller_map_updated(_context, &_toContextCh);
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
			//nonlimb_joint_info.get(i-limb_joint_num).buff_index = index;
			nonlimb_joint_info.buff_index.set(i-limb_joint_num, index);
		}
		joints_indexed = true;
	}

	//base_index = nonlimb_joint_info.buff_index.get(0);
}

char* MeCtLocomotion::get_base_name()
{
	return base_name;
}

bool MeCtLocomotion::controller_evaluate( double time, MeFrameData& frame ) {
	// TODO: Update MeController to pass in delta time.
	// Until then, fake it or compute it ourself (but there are some gotchas)

	if(!enabled) return false;
	if( !is_valid ) return is_valid;
	if(!motions_loaded) return motions_loaded;
	


	if(limb_list.size() == 0) 
	{
		return false;
	}

	curr_t = time;

	delta_time = curr_t - last_t;

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

	if(ik_enabled) 
		apply_IK();

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
	
	last_t = time;
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

SrArray<MeCtLocomotionLimb*>* MeCtLocomotion::get_limb_list()
{
	return &limb_list;
}

SrArray<MeCtLocomotionAnimGlobalInfo*>* MeCtLocomotion::get_anim_global_info()
{
	return &anim_global_info;
}

SrVec MeCtLocomotion::calc_rotational_displacement()
{
	SrVec v;
	SkJoint* tjoint = walking_skeleton->search_joint(nonlimb_joint_info.joint_name.get(0));
	SrMat gmat, pmat, lmat;
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
	
	v.set(-gmat.e41(), -gmat.e42(), -gmat.e43());

	v = -navigator.get_base_pos();
	pmat.roty(navigator.get_facing_angle());

	SrVec world_offset_to_base = v*pmat;
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

void MeCtLocomotion::init_nonlimb_joint_info()
{
	SrArray<char*> limb_base_name;
	limb_base_name.capacity(limb_list.size());
	limb_base_name.size(limb_list.size());
	for(int i = 0; i < limb_list.size(); ++i)
	{
		limb_base_name.set(i, limb_list.get(i)->get_limb_base_name());
	}
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

void MeCtLocomotion::set_nonlimb_blending_base_name(const char* name)
{
	if(nonlimb_blending_base_name != NULL) free(nonlimb_blending_base_name);
	nonlimb_blending_base_name = (char*)malloc(sizeof(char)*(strlen(name)+1));
	strcpy(nonlimb_blending_base_name, name);
}

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
		if(min_ind == -1) 
		{
			min_ind = i;
			min = pos.z;
		}
		else
		{
			if(pos.z < min) 
			{
				min = pos.z;
				min_ind = i;
			}
		}
	}
	return min_ind;
}

float MeCtLocomotion::get_buffer_base_height(SrBuffer<float>& buffer)
{
	SkJoint* tjoint = walking_skeleton->root();
	SrQuat quat;
	int index;
	SrMat gmat;
	SrMat pmat, lmat;
	bool found = false;
	while(true)
	{
		index = nonlimb_joint_info.get_index_by_name(tjoint->name().get_string());

		SrVec base_pos = navigator.get_base_pos();
		if(index == translation_joint_index)
		{
			tjoint->pos()->value(0, base_pos.x);
			tjoint->pos()->value(1, base_pos.y);
			tjoint->pos()->value(2, base_pos.z);
		}
		
		quat.w = buffer[index+0];
		quat.x = buffer[index+1];
		quat.y = buffer[index+2];
		quat.z = buffer[index+3];

		pmat = gmat;
		lmat = get_lmat(tjoint, &quat);
		gmat = lmat * pmat;

		if(strcmp(nonlimb_joint_info.joint_name.get(translation_joint_index), tjoint->name().get_string()) == 0)
		{
			found = true;
			break;
		}

		if(tjoint->num_children()>0)
		{ 
			tjoint = tjoint->child(0);
		}
		else break;
	}
	if(found == false)
	{
		LOG("\nError: MeCtLocomotion::get_buffer_base_height(). translation joint not found.");
	}
	return gmat.e42();
}

void MeCtLocomotion::blend_base_joint(MeFrameData& frame, float space_time, int anim_index1, int anim_index2, float weight)
{
	pre_blended_base_height = r_blended_base_height;
	SrMat mat;
	SrQuat rot1, rot2, rot3, rot4;
	float ratio = 0.0f;
	const float* pheight = mat.pt(13);
	float base_height = 0.0f;
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
	base->update_gmat();
	mat = base->gmat();
	r_blended_base_height = *pheight*(1.0f-ratio)*(weight);

	anim1->walking->apply_frame((int)frame1+1);
	base->update_gmat();
	mat = base->gmat();
	r_blended_base_height += *pheight*ratio*(weight);

	anim2->walking->connect(walking_skeleton);
	ratio = frame2 - (int)frame2;

	anim2->walking->apply_frame((int)frame2);
	base->update_gmat();
	mat = base->gmat();
	r_blended_base_height += *pheight*(1.0f-ratio)*(1.0f-weight);

	anim2->walking->apply_frame((int)frame2+1);
	base->update_gmat();
	mat = base->gmat();
	r_blended_base_height += *pheight*ratio*(1.0f-weight);

	r_blended_base_height += base_height;

	SrBuffer<float>& buffer = frame.buffer();
	float standing_height = get_buffer_base_height(buffer);

	r_blended_base_height = r_blended_base_height * (navigator.standing_factor) + standing_height * (1.0f-navigator.standing_factor);

	//printf("\nHeight: %f", r_blended_base_height);
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
	float frame_num; 

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
	limb_list.get(dominant_limb)->direction_planner.update_direction(delta_time, &limb_list.get(dominant_limb)->space_time, true);

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
			limb_list.get(i)->direction_planner.update_direction(delta_time, &limb_list.get(i)->space_time, false);
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
	get_frame(locomotion_anims.get(r_anim1_index_dominant - 1), walking_skeleton, anim1->get_timing_space()->get_virtual_frame(limb_list.get(0)->space_time), nonlimb_blending_base_name, &joint_quats1, &t_joint_quats1, &t_joint_quats2, &(nonlimb_joint_info.joint_index));
	get_frame(locomotion_anims.get(r_anim2_index_dominant - 1), walking_skeleton, anim2->get_timing_space()->get_virtual_frame(limb_list.get(0)->space_time), nonlimb_blending_base_name, &joint_quats2, &t_joint_quats1, &t_joint_quats2, &(nonlimb_joint_info.joint_index));
	get_blended_quat_buffer(&(nonlimb_joint_info.quat), &joint_quats1, &joint_quats2, dom_ratio);

	//recompute the dominant limb
	get_dominant_limb();

	//blend with standing animation is standing factor > 0
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

	update_nonlimb_mat();

	update_pos();

	navigator.update_world_offset();
	update_nonlimb_mat_with_global_info();
	
}

void MeCtLocomotion::update_nonlimb_mat_with_global_info()
{
	SrMat global_mat;
	global_mat.roty(navigator.get_facing_angle());
	global_mat.set(12, navigator.get_world_pos().x);
	global_mat.set(13, navigator.get_world_pos().y);
	global_mat.set(14, navigator.get_world_pos().z);
	for(int i = 0; i < nonlimb_joint_info.joint_name.size(); ++i)
	{
		nonlimb_joint_info.mat.set(i, nonlimb_joint_info.mat.get(i)*global_mat);
	}
}

void MeCtLocomotion::blend_standing(MeFrameData& frame)
{
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

void MeCtLocomotion::update_nonlimb_mat()//without global rotation
{
	SkJoint* joint = walking_skeleton->root();
	SrMat mat;
	mat = get_lmat(joint, &(nonlimb_joint_info.quat.get(0)));
	nonlimb_joint_info.mat.set(0, mat);
	update_nonlimb_mat(joint, &mat);
}

void MeCtLocomotion::update_nonlimb_mat(SkJoint* joint, SrMat* mat)
{
	SkJoint* tjoint = NULL;
	int index = -1;
	SrMat lmat;
	SrMat gmat;
	bool cont = false;
	for(int i = 0; i < joint->num_children(); ++i)
	{
		cont = false;
		tjoint = joint->child(i);
		for(int j = 0; j < limb_list.size(); ++j)
		{
			if(strcmp(tjoint->name().get_string(), limb_list.get(j)->limb_base_name) == 0) 
			{
				cont = true;
				break;
			}
		}
		if(cont) continue;
		index = nonlimb_joint_info.get_index_by_name(tjoint->name().get_string());
		if(nonlimb_joint_info.mat_valid.get(index) != 1) continue;
		lmat = get_lmat(tjoint, &(nonlimb_joint_info.quat.get(index)));
		gmat = lmat * *mat;
		nonlimb_joint_info.mat.set(index, gmat);
		update_nonlimb_mat(joint->child(i), &gmat);
	}
}

void MeCtLocomotion::set_target_height_displacement(float displacement)
{
	navigator.target_height_displacement = displacement;
}

void MeCtLocomotion::apply_IK()
{
	//if(navigator.target_height_displacement == 0.0f) return;
	MeCtIKScenario* ik_scenario = NULL;
	MeCtIKScenarioJointInfo* info = NULL;
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	float normal[3];
	

	for(int i = 0; i < limb_list.size(); ++i)
	{
		ik_scenario = &(limb_list.get(i)->ik);

		//temp...............
		//ik_scenario->joint_info_list.get(2).constraint.ball.max = 3.14159265f/4.0f;
		//temp...............

		SkJoint* tjoint = walking_skeleton->search_joint(limb_list.get(i)->get_limb_base_name());
		int parent_ind = nonlimb_joint_info.get_index_by_name(tjoint->parent()->name().get_string());
		ik_scenario->mat = nonlimb_joint_info.mat.get(parent_ind);
		ik_scenario->start_joint = &(ik_scenario->joint_info_list.get(0));
		ik_scenario->end_joint = &(ik_scenario->joint_info_list.get(ik_scenario->joint_info_list.size()-1));
		
		//if(i == 0) ik_scenario->plane_point = SrVec(0.0f, 20.0f-target_height_displacement, 0.0f);
		//else ik_scenario->plane_point = SrVec(0.0f, 40.0f-target_height_displacement, 00.0f);
		//ik_scenario->plane_point = SrVec(0.0f, -navigator.target_height_displacement*navigator.standing_factor, 00.0f);

		SrVec pos = limb_list.get(i)->pos_buffer.get(2);
		pos += navigator.get_world_pos();

		float height = mcu.query_terrain(pos.x, pos.z, normal);

		ik_scenario->set_plane_normal(SrVec(normal[0], normal[1], normal[2]));

		ik_scenario->plane_point = SrVec(pos.x, height, pos.z);

		ik_scenario->set_offset(limb_list.get(i)->ik_offset);
		
		ik_scenario->quat_list = limb_list.get(i)->limb_joint_info.quat;

		ik_scenario->ik_orientation.set(0.0f, -1.0f, 0.0f);
		for(int j = 0; j < ik_scenario->joint_info_list.size(); ++j)
		{
			info = &(ik_scenario->joint_info_list.get(j));
			//info->support_joint_comp = nonlimb_joint_info.mat.get(parent_ind).e42()+limb_list.get(i)->pos_buffer.get(j).y - info->support_joint_height;
			info->support_joint_comp = nonlimb_joint_info.mat.get(parent_ind).e42()-navigator.target_height_displacement- abs_ground_height + limb_list.get(i)->pos_buffer.get(j).y - info->support_joint_height;
		}
		ik.update(ik_scenario);
		limb_list.get(i)->limb_joint_info.quat = ik_scenario->quat_list;
		//ik_scenario->quat_list. = limb_list.get(i)->quat_buffer
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

int MeCtLocomotion::get_dominant_limb()
{
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
	gmat.set(11, 0.0f);
	gmat.set(12, 0.0f);
	gmat.set(13, 0.0f);
	//SrVec origin;
	//origin.set(gmat.e41(), gmat.e42(), gmat.e43());
	//pmat = nonlimb_joint_info.mat.get(translation_joint_index);
	//gmat.set(12, gmat.e41()-pmat.e41());
	//gmat.set(13, gmat.e42()-pmat.e42());
	//gmat.set(14, gmat.e43()-pmat.e43());

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
	SrVec displacement;

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
	navigator.update_displacement(&displacement);
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