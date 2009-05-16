/*
 *  me_ct_locomotion_simple.hpp - part of SmartBody-lib's Motion Engine
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
 *      Andrew n marshall, USC
 */

#include "me_ct_locomotion_simple.hpp"

#include "sbm_character.hpp"
#include "gwiz_math.h"
#include "limits.h"


const char* MeCtLocomotionSimple::TYPE = "MeCtLocomotionSimple";


/** Constructor */
MeCtLocomotionSimple::MeCtLocomotionSimple() {
	is_valid = false;
	last_time = std::numeric_limits<float>::quiet_NaN();
}

/** Destructor */
MeCtLocomotionSimple::~MeCtLocomotionSimple() {
	// Nothing allocated to the heap
}

// Implements MeController::controller_channels().
SkChannelArray& MeCtLocomotionSimple::controller_channels() {
	if( request_channels.size() == 0 ) {
		// Initialize Requested Channels                                                           // Indices
		request_channels.add( SkJointName( SbmPawn::WORLD_OFFSET_JOINT_NAME ), SkChannel::XPos );  // #0
		request_channels.add( SkJointName( SbmPawn::WORLD_OFFSET_JOINT_NAME ), SkChannel::YPos );  //  1
		request_channels.add( SkJointName( SbmPawn::WORLD_OFFSET_JOINT_NAME ), SkChannel::ZPos );  //  2
		request_channels.add( SkJointName( SbmPawn::WORLD_OFFSET_JOINT_NAME ), SkChannel::Quat );  //  3

		request_channels.add( SkJointName( SbmCharacter::LOCOMOTION_VELOCITY ), SkChannel::XPos ); //  4
		request_channels.add( SkJointName( SbmCharacter::LOCOMOTION_VELOCITY ), SkChannel::YPos ); //  5
		request_channels.add( SkJointName( SbmCharacter::LOCOMOTION_VELOCITY ), SkChannel::ZPos ); //  6

		request_channels.add( SkJointName( SbmCharacter::ORIENTATION_TARGET ), SkChannel::XPos );  //  7
		request_channels.add( SkJointName( SbmCharacter::ORIENTATION_TARGET ), SkChannel::YPos );  //  8
		request_channels.add( SkJointName( SbmCharacter::ORIENTATION_TARGET ), SkChannel::ZPos );  //  9
	}

	return request_channels;
}

// Implements MeController::context_updated(..).
void MeCtLocomotionSimple::context_updated() {
	if( _context == NULL )
		is_valid = false;
}

// Look up the context indices, and check to make sure it isn't -1
#define LOOKUP_BUFFER_INDEX( var_name, index ) \
	var_name = _context->toBufferIndex( _toContextCh[ ( index ) ] );  \
	is_valid &= ( var_name != -1 );

void MeCtLocomotionSimple::controller_map_updated() {
	is_valid = true;

	if( _context != NULL ) {
		// request_channel indices (second param) come from the order of request_channels.add(..) calls in controller_channels()
		LOOKUP_BUFFER_INDEX( bi_world_x,    0 );
		LOOKUP_BUFFER_INDEX( bi_world_y,    1 );
		LOOKUP_BUFFER_INDEX( bi_world_z,    2 );
		LOOKUP_BUFFER_INDEX( bi_world_rot,  3 );

		LOOKUP_BUFFER_INDEX( bi_loco_vel_x, 4 );
		LOOKUP_BUFFER_INDEX( bi_loco_vel_y, 5 );
		LOOKUP_BUFFER_INDEX( bi_loco_vel_z, 6 );

		LOOKUP_BUFFER_INDEX( bi_orient_x,   7 );
		LOOKUP_BUFFER_INDEX( bi_orient_y,   8 );
		LOOKUP_BUFFER_INDEX( bi_orient_z,   9 );
	} else {
		// This shouldn't get here
		is_valid = false;
	}
}


bool MeCtLocomotionSimple::controller_evaluate( double time, MeFrameData& frame ) {
	// TODO: Update MeController to pass in delta time.
	// Until then, fake it or compute it ourself (but there are some gotchas)
	float time_delta = 0.033f;


	const vector3_t UP_VECTOR( 0, 1, 0 );

	if( is_valid ) {
		SrBuffer<float>& buffer = frame.buffer(); // convenience reference

		// Read inputs
		vector3_t world_pos( buffer[ bi_world_x ], buffer[ bi_world_y ], buffer[ bi_world_z ] );
		vector3_t loco_vel( buffer[ bi_loco_vel_x ], buffer[ bi_loco_vel_y ], buffer[ bi_loco_vel_z ] );
		vector3_t orient_pos( buffer[ bi_orient_x ], buffer[ bi_orient_y ], buffer[ bi_orient_z ] );

		// Position Calc
		world_pos += ( loco_vel * time_delta );

		// Orientation Calc (using the update world_pos)
		vector3_t orient_vec = orient_pos - world_pos;
		orient_vec.y( 0 ); // rotate around y axis by setting y-delta to 0

		quat_t orient_quat( euler_t( orient_vec, UP_VECTOR ) );

		// Write Results
		buffer[ bi_world_x ] = world_pos.x();
		buffer[ bi_world_y ] = world_pos.y();
		buffer[ bi_world_z ] = world_pos.z();

		buffer[ bi_world_rot+0 ] = orient_quat.w();
		buffer[ bi_world_rot+1 ] = orient_quat.x();
		buffer[ bi_world_rot+2 ] = orient_quat.y();
		buffer[ bi_world_rot+3 ] = orient_quat.z();
	}

	return true;
}