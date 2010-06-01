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
	joint_num = 0;
	last_time = std::numeric_limits<float>::quiet_NaN();
	bi_joint_quats.capacity(0);
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
}

/** Destructor */
MeCtLocomotion::~MeCtLocomotion() {
	// Nothing allocated to the heap

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

// Implements MeController::controller_channels().
SkChannelArray& MeCtLocomotion::controller_channels() {
	if( request_channels.size() == 0 ) {
		// Initialize Requested Channels                                                           // Indices
		request_channels.add( SkJointName( SbmPawn::WORLD_OFFSET_JOINT_NAME ), SkChannel::XPos );  // #0
		request_channels.add( SkJointName( SbmPawn::WORLD_OFFSET_JOINT_NAME ), SkChannel::YPos );  //  1
		request_channels.add( SkJointName( SbmPawn::WORLD_OFFSET_JOINT_NAME ), SkChannel::ZPos );  //  2
		request_channels.add( SkJointName( SbmPawn::WORLD_OFFSET_JOINT_NAME ), SkChannel::Quat );  //  3

		//request_channels.add( SkJointName("base"), SkChannel::XPos); //4
		//request_channels.add( SkJointName("base"), SkChannel::YPos); //5
		//request_channels.add( SkJointName("base"), SkChannel::ZPos); //6
		//request_channels.add( SkJointName("base"), SkChannel::Quat); //7

		joint_channel_start_ind = 4;
		joint_channel_start_ind += navigator.controller_channels(&request_channels);

		//init_limbs();
		joint_num = 0;
		for(int i = 0; i < limb_list.size(); ++i)
		{
			MeCtLocomotionLimb* limb = limb_list.get(i);
			SkJoint* base = standing_skeleton->search_joint(limb->get_limb_base_name());
			joint_num += iterate_children(base); // starting from joint_channel_start_ind
		}
	}

	return request_channels;
}

int MeCtLocomotion::iterate_children(SkJoint* base)
{
	const char* name = base->name().get_string();
	request_channels.add( SkJointName(name), SkChannel::Quat );
	int sum = 1;
	for(int i = 0; i < base->num_children(); ++i)
	{
		sum += iterate_children(base->child(i));
	}
	return sum;
}

// Implements MeController::context_updated(..).
void MeCtLocomotion::context_updated() {
	if( _context == NULL )
		is_valid = false;
}

// Look up the context indices, and check to make sure it isn't -1
#define LOOKUP_BUFFER_INDEX( var_name, index ) \
	var_name = _context->toBufferIndex( _toContextCh[ ( index ) ] );  \
	is_valid &= ( var_name != -1 );

void MeCtLocomotion::controller_map_updated() 
{
	int index = 0;
	is_valid = navigator.controller_map_updated(_context, &_toContextCh);
	for(int i = 0; i < joint_num; ++i)
	{
		LOOKUP_BUFFER_INDEX( index,  i+joint_channel_start_ind);
		bi_joint_quats.push() = index;
	}
}

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
		if(2.0f * dis_to_dest.len() * speed_accelerator.get_target_acceleration()/2.0f <= speed_accelerator.get_curr_speed()*speed_accelerator.get_curr_speed())
		{
			if(navigator.get_destination_count() == navigator.get_curr_destinatio_index()+1) navigator.set_reached_destination(frame);
			else navigator.next_destination(frame);
		}
		//printf("\n[%f, %f]", 2.0f * dis_to_dest.len() * speed_accelerator.get_target_acceleration(), speed_accelerator.get_curr_speed()*speed_accelerator.get_curr_speed());
	}

	if(navigator.get_local_velocity().len() != 0.0f)
	{
		for(int i = 0; i < limb_list.size(); ++i)
		{
			limb = limb_list.get(i);
			if(navigator.has_destination) limb->direction_planner.set_target_direction(&navigator.get_dis_to_dest());
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
	for(int i = 0; i < joint_num;)
	{
		for(int j = 0; j < limb_list.size(); ++j)
		{
			SrArray<SrQuat>* quat_buffer = &(limb_list.get(j)->quat_buffer);
			for(int k = 0; k < quat_buffer->size(); ++k)
			{
				index = bi_joint_quats.get(i);
				quat = quat_buffer->get(k);
				if(k != quat_buffer->size()-1)
				{
					buffer[index+0] = (float)quat.w;
					buffer[index+1] = (float)quat.x;
					buffer[index+2] = (float)quat.y;
					buffer[index+3] = (float)quat.z;
				}
				++i;
			}
		}
	}
	
	last_t = (float)time;
	//navigator.print_foot_pos(frame, limb_list.get(dominant_limb));

	//if(dominant_limb == 0) printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> yes    (%f, %f, %f)", limb_list.get(dominant_limb)->pos.x+navigator.x, limb_list.get(dominant_limb)->pos.y+navigator.y, limb_list.get(dominant_limb)->pos.z+navigator.z);
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

void MeCtLocomotion::init_skeleton(SkSkeleton* standing, SkSkeleton* walking)
{
	walking_skeleton = walking;
	standing_skeleton = standing;
	initialized = true;
}

const char* MeCtLocomotion::controller_type( void )	const {

	return TYPE;
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
	limb_list.get(dominant_limb)->space_time = blended_anim->get_timing_space()->get_space_value(frame_num);
	
	// update the current orientation of dominant limb
	navigator.update_facing(limb_list.get(dominant_limb), true);

	// blend the two animations
	limb_list.get(dominant_limb)->blend_anim(limb_list.get(dominant_limb)->space_time, 1, 2, dom_ratio);

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
			limb_list.get(i)->space_time = blended_anim->get_timing_space()->get_space_value(frame_num);

			//compute the direction and orientation based on the real timing space
			limb_list.get(i)->direction_planner.update_anim_mode(anim1);
			limb_list.get(i)->direction_planner.update_anim_mode(anim2);
			ratio = limb_list.get(i)->direction_planner.get_ratio(anim1, anim2);
			get_blended_timing_space(blended_anim->get_timing_space(), anim1->get_timing_space(), anim2->get_timing_space(), ratio);
			navigator.update_facing(limb_list.get(i), false);
			limb_list.get(i)->blend_anim(limb_list.get(i)->space_time, 1, 2, ratio);
		}
		//blend the animation with standing if acceleration < 1.0f
		limb_list.get(i)->blend_standing(limb_list.get(i)->walking_list.get(0), navigator.standing_factor);
		limb_list.get(i)->manipulate_turning();
	}

	//recompute the dominant limb
	get_dominant_limb();

	last_time = limb_list.get(dominant_limb)->space_time;
	update_pos();


	SkJoint* base_joint1 = walking_skeleton->search_joint(limb_list.get(0)->limb_base_name);
	SkJoint* base_joint2 = walking_skeleton->search_joint(limb_list.get(1)->limb_base_name);

	if(ik_enabled) get_IK();

}

void MeCtLocomotion::get_IK()
{
	MeCtIKScenario* ik_scenario = NULL;
	MeCtIKScenarioJointInfo* info = NULL;
	SkJoint* tjoint = walking_skeleton->search_joint("base");
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
		ik_scenario->quat_list = limb_list.get(i)->quat_buffer;
		for(int j = 0; j < ik_scenario->joint_info_list.size(); ++j)
		{
			info = &(ik_scenario->joint_info_list.get(j));
			//if(j == 2) printf("\n(%f)", limb_list.get(i)->pos_buffer.get(j).y);
			info->support_joint_comp = limb_list.get(i)->pos_buffer.get(j).y - info->support_joint_height;
			//if(j == 2) printf("\n height: %f", info->support_joint_comp);
		}
		ik.update(ik_scenario);
		limb_list.get(i)->quat_buffer = ik_scenario->quat_list;
		//ik_scenario->quat_list. = limb_list.get(i)->quat_buffer
	}

}

int MeCtLocomotion::get_dominant_limb()
{
	float remnant = 0.0f;
	float r;
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

	tjoint = skeleton->search_joint("base");
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
	
	for(int j  = 0;j < limb->quat_buffer.size()-1;++j)
	{
		pmat = gmat;
		lmat = get_lmat(tjoint, &(limb->quat_buffer.get(j)));
		gmat.mult ( lmat, pmat );
		pos.set(*gmat.pt(12), *gmat.pt(13), *gmat.pt(14));
		limb->pos_buffer.set(j, pos);
		if(tjoint->num_children()>0)
		{ 
			tjoint = tjoint->child(0);
		}
		else break;
	}
	//gmat = gmat * pmat;
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
			//printf("\nsum = 0");
		}
	}
	else dis_initialized = true;
	navigator.update_displacement(&displacement);

	//printf("\ndisplacement: (%f, %f, %f)", displacement.x, displacement.y, displacement.z);
	//printf("\nratio1: %f, ratio2: %f", ratio[0], ratio[1]);
}


MeCtLocomotionNavigator* MeCtLocomotion::get_navigator()
{
	return &navigator;
}

