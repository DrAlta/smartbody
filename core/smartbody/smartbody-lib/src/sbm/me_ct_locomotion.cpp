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

#include "sbm_character.hpp"
#include "gwiz_math.h"
#include "limits.h"


const char* MeCtLocomotion::TYPE = "MeCtLocomotion";


/** Constructor */
MeCtLocomotion::MeCtLocomotion() {
	is_valid = false;
	limb_joint_num = 0;

	last_time = std::numeric_limits<float>::quiet_NaN();
	//limb_joint_index.capacity(0);

	//nonlimb_joint_buff_index.capacity(0);
	//nonlimb_joint_quats.capacity(0);
	nonlimb_joint_info.joint_num = 0;
	nonlimb_joint_info.buff_index.capacity(0);
	nonlimb_joint_info.joint_index.capacity(0);
	nonlimb_joint_info.quat.capacity(0);
	last_time = 0.0f;
	last_t = -1.0f;
	ratio = 0.0f;
	dominant_limb = 0;
	automate = false;
	reset = false;
	dis_initialized = false;
	initialized = false;
	ik_enabled = false;
	enabled = false;
	joints_indexed = false;
	base_name = NULL;
	nonlimb_blending_base_name = NULL;
	locomotion_anims.capacity(0);
}

/** Destructor */
MeCtLocomotion::~MeCtLocomotion() {
	// Nothing allocated to the heap

	int num = limb_list.size();
	for (int x = 0; x < num; x++)
	{
		MeCtLocomotionLimb* limb = limb_list.get(x);
		delete limb;
	}

	for (int x = 0; x < anim_global_info.size(); x++)
	{
		delete anim_global_info[x];
	}
}

void MeCtLocomotion::init_limbs()
{
	MeCtLocomotionLimb* limb = NULL;
	for(int i = 0; i < limb_list.size(); ++i)
	{
		limb = limb_list.get(i);
		limb->pos = -get_limb_pos(limb);
	}
}

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
		// Initialize Requested Channels                                                           // Indices
		request_channels.add( SkJointName( SbmPawn::WORLD_OFFSET_JOINT_NAME ), SkChannel::XPos );  // #0
		request_channels.add( SkJointName( SbmPawn::WORLD_OFFSET_JOINT_NAME ), SkChannel::YPos );  //  1
		request_channels.add( SkJointName( SbmPawn::WORLD_OFFSET_JOINT_NAME ), SkChannel::ZPos );  //  2
		request_channels.add( SkJointName( SbmPawn::WORLD_OFFSET_JOINT_NAME ), SkChannel::Quat );  //  3

		joint_channel_start_ind = 4;
		joint_channel_start_ind += navigator.controller_channels(&request_channels);

		limb_joint_num = 0;
		for(int i = 0; i < limb_list.size(); ++i)
		{
			MeCtLocomotionLimb* limb = limb_list.get(i);
			SkJoint* base = standing_skeleton->search_joint(limb->get_limb_base_name());
			//limb_joint_num += iterate_limb_joints(base, 0); // starting from joint_channel_start_ind
			iterate_joints(&(limb->limb_joint_info));
			limb_joint_num += limb->limb_joint_info.joint_num;
		}
		//limb_joint_index.capacity(limb_joint_num);
		//limb_joint_index.size(limb_joint_num);

		//SkJoint* base = standing_skeleton->search_joint(nonlimb_blending_base_name);

		iterate_joints(&nonlimb_joint_info);
		/*nonlimb_joint_info.quat.capacity(nonlimb_joint_info.joint_num);
		nonlimb_joint_info.quat.size(nonlimb_joint_info.joint_num);
		nonlimb_joint_info.buff_index.capacity(nonlimb_joint_info.joint_num);
		nonlimb_joint_info.buff_index.size(nonlimb_joint_info.joint_num);
		nonlimb_joint_info.joint_index.capacity(nonlimb_joint_info.joint_num);
		nonlimb_joint_info.joint_index.size(nonlimb_joint_info.joint_num);*/

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

/*int MeCtLocomotion::iterate_nonlimb_joints(SkJoint* base, int depth)
{
	//if(request_channels.size() >= 90-10-14) return 0;
	const char* name = base->name().get_string();
	int i = 0;
	// check if current joint is the base of a limb
	for(i = 0; i < limb_list.size(); ++i)
	{
		MeCtLocomotionLimb* limb = limb_list.get(i);
		if(strcmp(name, limb->get_limb_base_name())==0 ) 
			return 0;
	}
	int sum = 0;
	//if(strcmp(name, base_name) != 0) 
	{
		if(base->quat()->active() == true)
		{
			request_channels.add( SkJointName(name), SkChannel::Quat );
			//joint_name.push() = name;
			//MeCtLocomotionJointInfo info;
			//info.joint_index = base->index();
			//nonlimb_joint_info.push() = info;
			LOG("[%s] %d\n", name, request_channels.size()-1);
			sum = 1;
		}
		else
		{
			LOG(">>>>>>>>>>>>>>>>>>>>[%s] %d\n", name, request_channels.size()-1);
		}
	}
	for(i = 0; i < base->num_children(); ++i)
	{
		for(int j = 0; j < depth; ++j) LOG(" ");
		sum += iterate_nonlimb_joints(base->child(i), depth+1);
	}
	return sum;
}*/

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
}

char* MeCtLocomotion::get_base_name()
{
	return base_name;
}

/*void MeCtLocomotion::update_limb_anim_standing(MeCtLocomotionLimbAnim* anim, int index, MeFrameData& frame)
{
	SrBuffer<float>& buffer = frame.buffer();
	SrArray<SrQuat>* anim_buff = anim->get_buffer();
	SrQuat quat;
	for(int i = 0; i < anim_buff->size(); ++i)
	{
		index = limb_joint_index.get(i+index);
		quat.w = buffer[index+0];
		quat.x = buffer[index+1];
		quat.y = buffer[index+2];
		quat.z = buffer[index+3];
		anim_buff->set(i, quat);
	}
}*/

/*void MeCtLocomotion::update_limb_anim_standing()
{
	for(int i = 0; i < limb_list.size(); ++i)
	{
		limb_list.get(i).
		update_limb_anim_standing(, int index, MeFrameData& frame)
	}
}*/

bool MeCtLocomotion::controller_evaluate( double time, MeFrameData& frame ) {
	// TODO: Update MeController to pass in delta time.
	// Until then, fake it or compute it ourself (but there are some gotchas)

	//is_valid = false;

	if(!enabled) return false;
	//return false;
	if( !is_valid ) return is_valid;

	float time_delta = 0.03333333f;

	if(last_t == -1.0f) last_t = (float)time-0.033333f;

	curr_t = (float)time;

	if(curr_t - last_t > 0.033333f)  curr_t = last_t + 0.033333f;

	if(curr_t < last_t) curr_t = last_t;
	
	float inc_frame = (float)((curr_t-last_t)/time_delta);

	//SkJoint* base = _skeleton_ref_p->search_joint(base_name);

	const vector3_t UP_VECTOR( 0, 1, 0 );

	SrBuffer<float>& buffer = frame.buffer(); // convenience reference
 
	MeCtLocomotionLimb* limb = limb_list.get(dominant_limb);

	navigator.controller_evaluate(curr_t-last_t, &(limb->direction_planner), &speed_accelerator, frame);

	if(navigator.has_destination && navigator.get_destination_count() > navigator.get_curr_destinatio_index() && navigator.get_curr_destinatio_index()>=0)
	{
		SrVec dis_to_dest = navigator.get_dis_to_dest();
		if(2.0f * dis_to_dest.len() * speed_accelerator.get_target_acceleration() <= speed_accelerator.get_target_speed()*speed_accelerator.get_target_speed())
		{
			if(navigator.get_destination_count() == navigator.get_curr_destinatio_index()+1) 
				navigator.set_reached_destination(frame);
			else navigator.next_destination(frame);
		}
		//LOG("\n[%f, %f]", 2.0f * dis_to_dest.len() * speed_accelerator.get_target_acceleration(), speed_accelerator.get_curr_speed()*speed_accelerator.get_curr_speed());
	}

	if(navigator.get_local_velocity().len() != 0.0f)
	{
		for(int i = 0; i < limb_list.size(); ++i)
		{
			limb = limb_list.get(i);
			if(navigator.has_destination) limb->direction_planner.set_target_direction(&navigator.get_dis_to_dest_local());
			else limb->direction_planner.set_target_direction(&navigator.get_target_local_velocity());
		}
		//speed_accelerator.set_target_speed(navigator.get_local_velocity().len());
	}
	speed_accelerator.set_target_speed(navigator.get_target_local_velocity().len());

	update(inc_frame);

	navigator.post_controller_evaluate(frame, limb_list.get(dominant_limb), reset);

	reset = false;

	int index = 0;
	SrQuat quat;
	SrQuat quat_buff;

	//LOG("\n");
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

			//LOG("\n%d", index);
		}
	}


	for(int i = 0; i < nonlimb_joint_info.joint_num; ++i)
	{
		index = nonlimb_joint_info.buff_index.get(i);
		quat = nonlimb_joint_info.quat.get(i);
		//index = nonlimb_joint_info.get(i).buff_index;
		//quat = nonlimb_joint_info.get(i).quat;

		quat_buff.w = buffer[index+0];
		quat_buff.x = buffer[index+1];
		quat_buff.y = buffer[index+2];
		quat_buff.z = buffer[index+3];

		quat_buff = slerp(quat_buff, quat, navigator.standing_factor);
		buffer[index+0] = (float)quat_buff.w;
		buffer[index+1] = (float)quat_buff.x;
		buffer[index+2] = (float)quat_buff.y;
		buffer[index+3] = (float)quat_buff.z;
	}
	
	last_t = (float)time;
	//navigator.print_foot_pos(frame, limb_list.get(dominant_limb));

	//if(dominant_limb == 0) LOG("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> yes    (%f, %f, %f)", limb_list.get(dominant_limb)->pos.x+navigator.x, limb_list.get(dominant_limb)->pos.y+navigator.y, limb_list.get(dominant_limb)->pos.z+navigator.z);
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

// main stream
void MeCtLocomotion::update(float inc_frame)
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


	speed_accelerator.update_speed(curr_t - last_t);

	//get current direction
	limb_list.get(dominant_limb)->direction_planner.update_direction(curr_t - last_t, &limb_list.get(dominant_limb)->space_time);

	MeCtLocomotionLimbAnim* anim1 = limb_list.get(dominant_limb)->get_walking_list()->get(1);
	MeCtLocomotionLimbAnim* anim2 = limb_list.get(dominant_limb)->get_walking_list()->get(2);
	MeCtLocomotionLimbAnim* blended_anim = &limb_list.get(dominant_limb)->blended_anim;

	// adjust key-frame animation mode according to the dominant limb's direction
	limb_list.get(dominant_limb)->direction_planner.update_anim_mode(anim1);
	limb_list.get(dominant_limb)->direction_planner.update_anim_mode(anim2);

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
	limb_list.get(dominant_limb)->blend_anim(limb_list.get(dominant_limb)->space_time, 1, 2, dom_ratio, &(limb_list.get(dominant_limb)->limb_joint_info.joint_index));

	// update acceleration based on current timing space and space time.
	speed_accelerator.update_acceleration(limb_list.get(dominant_limb), blended_anim->get_timing_space());

	for(int i = 0; i < limb_list.size(); ++i)
	{
		if(i != dominant_limb)
		{
			limb_list.get(i)->direction_planner.update_direction((curr_t - last_t), &limb_list.get(i)->space_time);
			anim1 = limb_list.get(i)->get_walking_list()->get(1);
			anim2 = limb_list.get(i)->get_walking_list()->get(2);
			blended_anim = &limb_list.get(i)->blended_anim;

			//the following code computes the space value in accordance with that of the dominant limb:
			//get a subordinate timing space which has the same scale as the dominant timing space,
			//then map the frame number into the space value of this timing space.
			//the reason for this is that provided the freedom of all the limbs, the animation can be unsmooth if frame number is not mapped into the dominant limb's timing space
			limb_list.get(dominant_limb)->direction_planner.update_anim_mode(anim1);
			limb_list.get(dominant_limb)->direction_planner.update_anim_mode(anim2);
			get_blended_timing_space(blended_anim->get_timing_space(), anim1->get_timing_space(), anim2->get_timing_space(), dom_ratio);
			if(navigator.standing_factor != 0.0f) 
				limb_list.get(i)->space_time = blended_anim->get_timing_space()->get_space_value(frame_num);

			//compute the direction and orientation based on the real timing space
			limb_list.get(i)->direction_planner.update_anim_mode(anim1);
			limb_list.get(i)->direction_planner.update_anim_mode(anim2);
			ratio = limb_list.get(i)->direction_planner.get_ratio(anim1, anim2);
			get_blended_timing_space(blended_anim->get_timing_space(), anim1->get_timing_space(), anim2->get_timing_space(), ratio);
			navigator.update_facing(limb_list.get(i), false);
			limb_list.get(i)->blend_anim(limb_list.get(i)->space_time, 1, 2, ratio, &(limb_list.get(i)->limb_joint_info.joint_index));
		}
		//blend the animation with standing if acceleration < 1.0f
		//limb_list.get(i)->blend_standing(limb_list.get(i)->walking_list.get(0), navigator.standing_factor);
		//limb_list.get(i)->manipulate_turning();
	}

	get_frame(locomotion_anims.get(0), walking_skeleton, anim1->get_timing_space()->get_virtual_frame(limb_list.get(1-dominant_limb)->space_time), nonlimb_blending_base_name, &joint_quats1, &t_joint_quats1, &t_joint_quats2, &(nonlimb_joint_info.joint_index));
	get_frame(locomotion_anims.get(1), walking_skeleton, anim2->get_timing_space()->get_virtual_frame(limb_list.get(1-dominant_limb)->space_time), nonlimb_blending_base_name, &joint_quats2, &t_joint_quats1, &t_joint_quats2, &(nonlimb_joint_info.joint_index));
	get_blended_quat_buffer(&(nonlimb_joint_info.quat), &joint_quats1, &joint_quats2, dom_ratio);
 
	//recompute the dominant limb
	get_dominant_limb();
	/*if(navigator.standing_factor == 0.0f && navigator.reached_destination && navigator.has_destination) 
	{
		LOG("\n dom_limb: %d, 1:%f, 2:%f", dominant_limb, limb_list.get(0)->space_time, limb_list.get(1)->space_time);
	}*/

	for(int i = 0; i < limb_list.size(); ++i)
	{
		//blend the animation with standing if acceleration < 1.0f
		limb_list.get(i)->blend_standing(limb_list.get(i)->walking_list.get(0), navigator.standing_factor);
		limb_list.get(i)->manipulate_turning();
	}

	last_time = limb_list.get(dominant_limb)->space_time;
	update_pos();

	//SkJoint* base_joint1 = walking_skeleton->search_joint(limb_list.get(0)->limb_base_name);
	//SkJoint* base_joint2 = walking_skeleton->search_joint(limb_list.get(1)->limb_base_name);

	//if(ik_enabled) get_IK();
}

/*void MeCtLocomotion::blend_standing(MeCtLocomotionLimbAnim* anim, float weight)
{

}*/

void MeCtLocomotion::get_IK()
{
	MeCtIKScenario* ik_scenario = NULL;
	MeCtIKScenarioJointInfo* info = NULL;
	SkJoint* tjoint = walking_skeleton->search_joint(base_name);
	SrMat pmat = get_lmat(tjoint, &navigator.base_rot);
	float* ppos = pmat.pt(12);

	ppos[0] = navigator.base_offset.x;
	ppos[1] = navigator.base_offset.y;
	ppos[2] = navigator.base_offset.z;

	for(int i = 0; i < limb_list.size(); ++i)
	{
		ik_scenario = &(limb_list.get(i)->ik);

		//temp...............
		ik_scenario->joint_info_list.get(2).constraint.ball.max = 3.14159265f/4.0f;
		//temp...............

		ik_scenario->mat = pmat;
		ik_scenario->start_joint = &(ik_scenario->joint_info_list.get(0));
		ik_scenario->end_joint = &(ik_scenario->joint_info_list.get(ik_scenario->joint_info_list.size()-1));
		ik_scenario->set_plane_normal(SrVec(0.0f, 1.0f, 0.0f));
		if(i == 0) ik_scenario->plane_point = SrVec(0.0f, 20.0f, 0.0f);
		else ik_scenario->plane_point = SrVec(0.0f, 40.0f, 00.0f);
		ik_scenario->quat_list = limb_list.get(i)->limb_joint_info.quat;
		for(int j = 0; j < ik_scenario->joint_info_list.size(); ++j)
		{
			info = &(ik_scenario->joint_info_list.get(j));
			//if(j == 2) LOG("\n(%f)", limb_list.get(i)->pos_buffer.get(j).y);
			info->support_joint_comp = limb_list.get(i)->pos_buffer.get(j).y - info->support_joint_height;
			//if(j == 2) LOG("\n height: %f", info->support_joint_comp);
		}
		ik.update(ik_scenario);
		limb_list.get(i)->limb_joint_info.quat = ik_scenario->quat_list;
		//ik_scenario->quat_list. = limb_list.get(i)->quat_buffer
	}

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
	float* ppos;
	SkSkeleton* skeleton = limb->walking_skeleton;

	tjoint = skeleton->search_joint(base_name);
	pmat = get_lmat(tjoint, &navigator.base_rot);
	ppos = pmat.pt(12);

	ppos[0] = navigator.base_offset.x;
	ppos[1] = navigator.base_offset.y;
	ppos[2] = navigator.base_offset.z;

	gmat = pmat;

	// add transformation code later for joints between the limb base joint and base joint.......................

	// add transformation code later for joints between the limb base joint and base joint.......................

	//gmat.identity();

	tjoint = skeleton->search_joint(limb->get_limb_base_name());
	
	for(int j  = 0;j < limb->limb_joint_info.quat.size()-1;++j)
	{
		pmat = gmat;
		lmat = get_lmat(tjoint, &(limb->limb_joint_info.quat.get(j)));
		gmat.mult ( lmat, pmat );
		pos.set(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14));
		limb->pos_buffer.set(j, pos);
		if(tjoint->num_children()>0)
		{ 
			tjoint = tjoint->child(0);
		}
		else break;
	}
	gmat = pmat;

	pos.set(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14));

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
	for(int i = 0; i < 2; ++i)
	{
		//if(i != dominant_limb) continue;
		MeCtLocomotionLimb* limb = limb_list.get(i);
		currpos = get_limb_pos(limb);
		ratio[i] = 0;
		dis[i].set(0,0,0);
		if(limb->space_time >= 2.0f || limb->space_time <= 1.0f)
		{
			if(limb->space_time >= 2.0f) ratio[i] = limb->space_time - 2.0f;
			else if(limb->space_time <= 1.0f) ratio[i] = 1.0f - limb->space_time;
			ratio[i] *= ratio[i];
			SrMat mat;

			mat.roty(navigator.get_pre_facing_angle());

			SrVec v1 = limb->pos * mat;

			mat.roty(navigator.get_facing_angle());

			SrVec v2 = currpos * mat;

			dis[i] = v1 - v2;

			dis[i].y = 0.0f;
			sum += ratio[i];
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
				//displacement.set(0.0f, 0.0f, 0.0f);
			}
		}
		else
		{
			//LOG("\nsum = 0");
		}
	}
	else dis_initialized = true;
	navigator.update_displacement(&displacement);

	//LOG("\ndisplacement: (%f, %f, %f)", displacement.x, displacement.y, displacement.z);
	//LOG("\nratio1: %f, ratio2: %f", ratio[0], ratio[1]);
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