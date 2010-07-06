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

#include "me_ct_locomotion_navigator.hpp"

#include "sbm_character.hpp"
#include "gwiz_math.h"
#include "limits.h"


const char* MeCtLocomotionNavigator::TYPE = "MeCtLocomotionNavigator";


/** Constructor */
MeCtLocomotionNavigator::MeCtLocomotionNavigator() 
{
	is_valid = false;
	displacement.set(0.0f, 0.0f, 0.0f);
	facing_angle = 0;
	pre_facing_angle = facing_angle;
	standing_factor = 1.0f;
	has_destination = false;
	reached_destination = false;
	destination_list.capacity(20);
	speed_list.capacity(20);
}

/** Destructor */
MeCtLocomotionNavigator::~MeCtLocomotionNavigator() 
{
	// Nothing allocated to the heap

}

void MeCtLocomotionNavigator::update_framerate_accelerator(float accelerator, SrArray<MeCtLocomotionLimb*>* limb_list)
{
	framerate_accelerator = accelerator;
	if(accelerator <= 1.0f)
	{
		if(accelerator == 0.0f)
		{
			standing_factor = 0.0f;
			//limb_list.get(dominating_limb)->space_time = 0.0f;
			for(int i = 0; i < limb_list->size(); ++i)
			{
				limb_list->get(i)->space_time = 0.0f;
				limb_list->get(i)->direction_planner.reset();
			}
		}
		else
		{
			standing_factor = accelerator;
			framerate_accelerator = 1.0f;
		}
	}
	else standing_factor = 1.0;
}

void MeCtLocomotionNavigator::update(SrBuffer<float>* buffer)
{
	
}

const char* MeCtLocomotionNavigator::controller_type( void )	
{
	return TYPE;
}

void MeCtLocomotionNavigator::CheckNewRoutine(MeFrameData& frame)
{
	SrBuffer<float>& buffer = frame.buffer(); // convenience reference
	float x = buffer[bi_id];
	if(buffer[bi_id] == 0.0f || buffer[bi_id] <= -2.0f) return;
	if(buffer[bi_id] == -1.0f)
	{
		routine_stack.size(0);
	}
	MeCtLocomotionRoutine routine;
	routine.global_rps = buffer[bi_loco_rot_global_y];
	routine.local_rps = buffer[bi_loco_rot_local_y];
	if(routine.global_rps == 0.0f) routine.type = ME_CT_LOCOMOTION_ROUTINE_TYPE_STRAIGHT;
	else routine.type = ME_CT_LOCOMOTION_ROUTINE_TYPE_CIRCULAR;
	routine.direction.set(buffer[bi_loco_vel_x], buffer[bi_loco_vel_y], buffer[bi_loco_vel_z]);
	routine.speed = routine.direction.len();
	routine.direction.normalize();
	if(buffer[bi_id] == -1.0f)
	{
		int max_id = 0;
		for(int i = 0; i < routine_stack.size(); ++i)
		{
			if(routine_stack.get(i).id > max_id)
			{
				max_id = routine_stack.get(i).id;
			}
		}
		routine.id = max_id + 1;
		routine_stack.push() = routine;
	}
	else 
	{
		routine.id = (int)buffer[bi_id];
		int i;
		for(i = 0; i < routine_stack.size(); ++i)
		{
			if(routine_stack.get(i).id == buffer[bi_id])
			{
				routine_stack.set(i, routine);
				break;
			}
		}
		if(i == routine_stack.size())
		{
			routine.id = (int)buffer[bi_id];
			routine_stack.push() = routine;
		}
	}
}

bool MeCtLocomotionNavigator::controller_evaluate(double delta_time, MeCtLocomotionLimbDirectionPlanner* direction_planner, MeCtLocomotionSpeedAccelerator* acc, MeFrameData& frame ) 
{
	//if(reached_destination) return true;
	SrMat mat;
	SrQuat q;
	SrBuffer<float>& buffer = frame.buffer(); // convenience reference
	if(has_destination && curr_dest_index == -1 && destination_list.size() > 0) next_destination(frame);
	this->delta_time = delta_time;
	CheckNewRoutine(frame);
	world_pos.set( buffer[ bi_world_x ], buffer[ bi_world_y ], buffer[ bi_world_z ] );
	world_rot.set( buffer[ bi_world_rot ], buffer[ bi_world_rot+1 ], buffer[ bi_world_rot+2 ], buffer[ bi_world_rot+3 ] );
	base_offset.set ( buffer[ bi_base_offset_x ], buffer[ bi_base_offset_y ], buffer[ bi_base_offset_z ] );
	base_rot.set( buffer[ bi_base_rot ], buffer[ bi_base_rot+1 ], buffer[ bi_base_rot+2 ], buffer[ bi_base_rot+3 ] );


	world_pos.y = 0.0f;
	SrQuat t_world_rot;
	mat.roty(pre_facing_angle);
	t_world_rot.set(mat);
	
	if(t_world_rot.w != world_rot.w
		|| t_world_rot.x != world_rot.x
		|| t_world_rot.y != world_rot.y
		|| t_world_rot.z != world_rot.z
		)// if the orientation has been changed manually 
	{
		pre_facing_angle = world_rot.angle();
		mat.roty(pre_facing_angle);
		t_world_rot.set(mat);
		if(dot(t_world_rot.axis(), world_rot.axis())< 0.0f) 
			pre_facing_angle = -pre_facing_angle;

		facing_angle = pre_facing_angle;
	}

	//SrVec d = world_rot.axis();
	//float x = world_rot.angle();
	//if(d.y < 0.0f) x = -x;
	//facing_angle = x;

	global_vel.set(0,0,0);
	local_rps = 0.0f;
	MeCtLocomotionRoutine routine;
	int i;
	for(i = 0; i < routine_stack.size(); ++i)
	{
		routine = routine_stack.get(i);
		//routine.elapsed_time += delta_time;
		switch(routine.type)
		{
		case ME_CT_LOCOMOTION_ROUTINE_TYPE_STRAIGHT:
			break;

		case ME_CT_LOCOMOTION_ROUTINE_TYPE_CIRCULAR:
			break;

		default:
			break;
		}
		local_rps += routine.local_rps;
		global_vel += routine.direction * routine.speed;
	}

	mat.roty(-pre_facing_angle);
	local_vel = global_vel*mat;

	calc_target_velocity();
	//local_vel.set(0,0,routine.speed);
	//printf("\n%f", pre_facing_angle);
	//printf("\nvel:(%f, %f, %f)", global_vel.x, global_vel.y, global_vel.z);

	return true;
}

inline void MeCtLocomotionNavigator::calc_target_velocity()
{
	if(has_destination)
	{
		if(destination_list.size() > curr_dest_index && curr_dest_index >= 0)
		{
			SrMat mat;
			mat.roty(-pre_facing_angle);
			dis_to_dest = destination_list.get(curr_dest_index) - world_pos;
			dis_to_dest.y = 0.0f;
			if(!reached_destination)
			{
				target_global_vel = dis_to_dest;
				target_global_vel.normalize();
				target_local_vel = target_global_vel*mat;
				dis_to_dest_local = dis_to_dest*mat;
				target_local_vel *= global_vel.len();
			}
			else
			{
				target_local_vel.set(0,0,0);
				target_global_vel.set(0,0,0);
			}
		}
	}
	else
	{
		target_global_vel = global_vel;
		target_local_vel = local_vel;
	}
}

SrVec MeCtLocomotionNavigator::get_dis_to_dest_local()
{
	return dis_to_dest_local;
}

SrVec MeCtLocomotionNavigator::get_target_local_velocity()
{
	//if(destination_list.size() > curr_dest_index && curr_dest_index >= 0) return local_vel;
	return target_local_vel;
}

SrVec MeCtLocomotionNavigator::get_dis_to_dest()
{
	return dis_to_dest;
}

SrVec MeCtLocomotionNavigator::get_local_velocity()
{
	return local_vel;
}

void MeCtLocomotionNavigator::set_reached_destination(MeFrameData& frame)
{
	SrBuffer<float>& buffer = frame.buffer();
	buffer[ bi_loco_vel_x ] = 0.0f;
	buffer[ bi_loco_vel_y ] = 0.0f;
	buffer[ bi_loco_vel_z ] = 0.0f;
	reached_destination = true;
}

void MeCtLocomotionNavigator::post_controller_evaluate(MeFrameData& frame, MeCtLocomotionLimb* limb, bool reset) 
{
	//if(reached_destination) return;
	pre_facing_angle = facing_angle;
	SrBuffer<float>& buffer = frame.buffer();
	if(reset)
	{
		buffer[ bi_world_x ] = 0.0f;
		//buffer[ bi_world_y ] = 0.0f;
		buffer[ bi_world_z ] = 0.0f;
	}
	buffer[ bi_world_x ] = displacement.x + buffer[ bi_world_x ];
	buffer[ bi_world_y ] = displacement.y + buffer[ bi_world_y ];
	buffer[ bi_world_z ] = displacement.z + buffer[ bi_world_z ];

	//buffer[ bi_world_x ] = displacement.x;
	//buffer[ bi_world_y ] = displacement.y;
	//buffer[ bi_world_z ] = displacement.z;

	//x = buffer[ bi_world_x ];
	//y = buffer[ bi_world_y ];
	//z = buffer[ bi_world_z ];

	//printf("\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>  base world_offset   (%f, %f, %f)", x, y, z);

	//if(facing_angle != 0.0f) printf("\n\npos:[%f, %f, %f]", limb->pos.x+buffer[ bi_world_x ], limb->pos.y+buffer[ bi_world_y ], limb->pos.z+buffer[ bi_world_z ]);
	//if(facing_angle != 0.0f) printf("\ncurr_pos:[%f, %f, %f]", limb->pos.x, limb->pos.y, limb->pos.z);
	//if(facing_angle != 0.0f) printf("\ndisplacement:[%f, %f, %f]", displacement.x, displacement.y, displacement.z);

	SrMat mat;
	mat.roty(facing_angle);
	world_rot.set(mat);

	buffer[ bi_world_rot+0 ] = world_rot.w;
	buffer[ bi_world_rot+1 ] = world_rot.x;
	buffer[ bi_world_rot+2 ] = world_rot.y;
	buffer[ bi_world_rot+3 ] = world_rot.z;

	MeCtLocomotionRoutine routine;
	SrVec di;
	float delta_angle;
	for(int i = 0; i < routine_stack.size(); ++i)
	{
		routine = routine_stack.get(i);
		//routine.elapsed_time += delta_time;
		switch(routine.type)
		{
		case ME_CT_LOCOMOTION_ROUTINE_TYPE_STRAIGHT:
			target_world_pos += routine.direction * (routine.speed * (float)delta_time);
			break;

		case ME_CT_LOCOMOTION_ROUTINE_TYPE_CIRCULAR:
			delta_angle = routine.global_rps * (float)delta_time;

			//get the displacement of rotation
			mat.roty(0.5f * delta_angle);
			di = routine.direction * mat;
			di *= 2.0f * sin(0.5f * delta_angle) * routine.speed / routine.global_rps;
			target_world_pos += di; 

			//mat.roty(facing_angle-routine.start_facing_angle);
			//routine.direction = SrVec(0,0,1)* mat;
			mat.roty(-delta_angle);

			//mat.roty(0);

			routine.direction = routine.direction* mat;
			break;

		default:
			break;
		}
		//mat.roty(routine.local_rps * (float)delta_time);
		//target_world_rot = target_world_rot * mat;
		routine_stack.set(i, routine);
	}
	//printf("\ntarget world pos: (%f, %f, %f)", target_world_pos.x, target_world_pos.y, target_world_pos.z);
}

SkChannelArray& MeCtLocomotionNavigator::controller_channels() {
	
	return request_channels;
}

// Implements MeController::context_updated(..).
void MeCtLocomotionNavigator::context_updated(MeControllerContext* _context) 
{
	if( _context == NULL )
		is_valid = false;
}

// Look up the context indices, and check to make sure it isn't -1
#define LOOKUP_BUFFER_INDEX( var_name, index ) \
	var_name = _context->toBufferIndex( _toContextCh->get( index ));  \
	is_valid &= ( var_name != -1 );

bool MeCtLocomotionNavigator::controller_map_updated(MeControllerContext* _context, SrBuffer<int>* _toContextCh) 
{
	int index = 0;
	is_valid = true;
	if( _context != NULL ) 
	{
		// request_channel indices (second param) come from the order of request_channels.add(..) calls in controller_channels()
		LOOKUP_BUFFER_INDEX( bi_world_x,    0 );
		LOOKUP_BUFFER_INDEX( bi_world_y,    1 );
		LOOKUP_BUFFER_INDEX( bi_world_z,    2 );
		LOOKUP_BUFFER_INDEX( bi_world_rot,  3 );

		LOOKUP_BUFFER_INDEX( bi_loco_vel_x, 4 );
		LOOKUP_BUFFER_INDEX( bi_loco_vel_y, 5 );
		LOOKUP_BUFFER_INDEX( bi_loco_vel_z, 6 );

		LOOKUP_BUFFER_INDEX( bi_loco_rot_global_y, 7 );
		LOOKUP_BUFFER_INDEX( bi_loco_rot_local_y, 8 );

		LOOKUP_BUFFER_INDEX( bi_id, 9 );

		//LOOKUP_BUFFER_INDEX( bi_has_destination, 10 );
		//LOOKUP_BUFFER_INDEX( bi_loco_dest_x, 10 );
		//LOOKUP_BUFFER_INDEX( bi_loco_dest_y, 11 );
		//LOOKUP_BUFFER_INDEX( bi_loco_dest_z, 12 );

		LOOKUP_BUFFER_INDEX( bi_base_offset_x,    10 );
		LOOKUP_BUFFER_INDEX( bi_base_offset_y,    11 );
		LOOKUP_BUFFER_INDEX( bi_base_offset_z,    12 );
		LOOKUP_BUFFER_INDEX( bi_base_rot,  13 );
	} 
	else 
	{
		// This shouldn't get here
		is_valid = false;
	}
	return is_valid;
}

int MeCtLocomotionNavigator::controller_channels(SkChannelArray* request_channels) 
{
	// Initialize Requested Channels                                                  // Indices
	routine_channel_num = 0;
	AddChannel(request_channels, SbmCharacter::LOCOMOTION_VELOCITY, SkChannel::XPos); 
	AddChannel(request_channels, SbmCharacter::LOCOMOTION_VELOCITY, SkChannel::YPos );
	AddChannel(request_channels, SbmCharacter::LOCOMOTION_VELOCITY, SkChannel::ZPos );
	AddChannel(request_channels, SbmCharacter::LOCOMOTION_GLOBAL_ROTATION, SkChannel::YPos );
	AddChannel(request_channels, SbmCharacter::LOCOMOTION_LOCAL_ROTATION, SkChannel::YPos );
	AddChannel(request_channels, SbmCharacter::LOCOMOTION_ID, SkChannel::YPos );

	//AddChannel(request_channels, SbmCharacter::LOCOMOTION_HAS_DESTINATION, SkChannel::YPos ); 
	//AddChannel(request_channels, SbmCharacter::LOCOMOTION_DESTINATION, SkChannel::XPos ); 
	//AddChannel(request_channels, SbmCharacter::LOCOMOTION_DESTINATION, SkChannel::YPos );
	//AddChannel(request_channels, SbmCharacter::LOCOMOTION_DESTINATION, SkChannel::ZPos );

	AddChannel(request_channels, "base", SkChannel::XPos ); 
	AddChannel(request_channels, "base", SkChannel::YPos );
	AddChannel(request_channels, "base", SkChannel::ZPos );
	AddChannel(request_channels, "base", SkChannel::Quat );

	return routine_channel_num;
}

void MeCtLocomotionNavigator::AddChannel(SkChannelArray* request_channels, const char* name, SkChannel::Type type)
{
	request_channels->add( SkJointName(name), type);
	++routine_channel_num;
}

void MeCtLocomotionNavigator::update_facing(MeCtLocomotionLimb* limb, bool dominant_limb)
{
	if(local_rps == 0.0f && local_vel.len() != 0.0f) return;
	SrMat mat;
	float time = 0.0f;
	limb->pre_rotation = limb->curr_rotation;

	if(limb->space_time > 1.0f && limb->space_time <= 1.5f) 
	{
		limb->curr_rotation = limb->rotation_record * (1.5f - limb->space_time)*2.0f;
	}

	else if(limb->space_time >= 2.0f || limb->space_time <= 1.0f)
	{
		limb->curr_rotation += (float)delta_time * local_rps;
		limb->rotation_record = limb->curr_rotation;
		if(dominant_limb) 
		{
			facing_angle += -(float)delta_time * local_rps;
			if(facing_angle > 0.0f) facing_angle -= (int)(0.5f*facing_angle/(float)M_PI)*(float)M_PI*2;
			else facing_angle += ((int)(-0.5f*facing_angle/(float)M_PI))*(float)M_PI*2;
		}
	}
	else if(limb->space_time > 1.5f && limb->space_time < 2.0f)
	{
		limb->curr_rotation -= (float)delta_time * local_rps;
		//limb->rotation_record = limb->curr_rotation;
	}
	//printf("\nrotation: %f; space time: %f", limb->curr_rotation, limb->space_time);
}

void MeCtLocomotionNavigator::clear_destination_list()
{
	destination_list.size(0);
	speed_list.size(0);
	curr_dest_index = -1;
}

void MeCtLocomotionNavigator::next_destination(MeFrameData& frame)
{
	++curr_dest_index;
	if(destination_list.size() <= curr_dest_index) return;

	SrVec dest = destination_list.get(curr_dest_index);
	SrVec v = dest-world_pos;
	v.normalize();
	v *= speed_list.get(curr_dest_index);
	SrBuffer<float>& buffer = frame.buffer();
	buffer[ bi_loco_vel_x ] = v.x;
	buffer[ bi_loco_vel_y ] = v.y;
	buffer[ bi_loco_vel_z ] = v.z;
}

int MeCtLocomotionNavigator::get_destination_count()
{
	return destination_list.size();
}

int MeCtLocomotionNavigator::get_curr_destinatio_index()
{
	return curr_dest_index;
}

void MeCtLocomotionNavigator::add_destination(SrVec* destination)
{
	destination_list.push() = *destination;
	reached_destination = false;
}

void MeCtLocomotionNavigator::add_speed(float speed)
{
	int num = destination_list.size()-speed_list.size();
	for(int i = 0; i < num; ++i)
	{
		speed_list.push() = speed;
	}
}

SrVec MeCtLocomotionNavigator::get_world_pos()
{
	return world_pos;
}

void MeCtLocomotionNavigator::update_displacement(SrVec* displacement)
{
	this->displacement = *displacement;
}

float MeCtLocomotionNavigator::get_facing_angle()
{
	return facing_angle;
}

float MeCtLocomotionNavigator::get_pre_facing_angle()
{
	return pre_facing_angle;
}

void MeCtLocomotionNavigator::AddRoutine(MeCtLocomotionRoutine& routine)
{
	routine_stack.push() = routine;
}

void MeCtLocomotionNavigator::DelRoutine(char* name)
{
	for(int i = 0; i < routine_stack.size(); ++i)
	{
		if(strcmp(routine_stack.get(i).name, name) == 0)
		{
			routine_stack.remove(i,1);
			break;
		}
	}
}

void MeCtLocomotionNavigator::print_foot_pos(MeFrameData& frame, MeCtLocomotionLimb* limb)
{
	SrMat gmat;
	SrMat pmat;
	SrMat lmat;
	SrQuat rotation;
	SrVec pos;
	//float* ppos;
	SkSkeleton* skeleton = limb->walking_skeleton;
	SkJoint* tjoint = skeleton->search_joint(limb->get_limb_base_name());
	gmat = tjoint->parent()->gmat();
	//ppos = gmat.pt(12);
	//ppos[0] = 0;
	//ppos[1] = 0;
	//ppos[2] = 0; 
	for(int j  = 0;j < limb->quat_buffer.size()-1;++j)
	{
		pmat = gmat;
		lmat = get_lmat(tjoint, &(limb->quat_buffer.get(j)));
		gmat.mult ( lmat, pmat );
		if(tjoint->num_children()>0)
		{ 
			tjoint = tjoint->child(0);
		}
		else break;
	}
	pos.set(gmat.get(12), gmat.get(13), gmat.get(14));

	SrBuffer<float>& buffer = frame.buffer(); // convenience reference
	rotation.w = buffer[ bi_world_rot+0 ];
	rotation.x = buffer[ bi_world_rot+1 ];
	rotation.y = buffer[ bi_world_rot+2 ];
	rotation.z = buffer[ bi_world_rot+3 ];
	gmat = rotation.get_mat(gmat);
	pos = pos * gmat;
	pos.x += buffer[ bi_world_x ];
	pos.y += buffer[ bi_world_y ];
	pos.z += buffer[ bi_world_z ];
	//if(facing_angle != 0.0f) printf("\npos: [%f, %f, %f]", pos.x, pos.y, pos.z);

}
