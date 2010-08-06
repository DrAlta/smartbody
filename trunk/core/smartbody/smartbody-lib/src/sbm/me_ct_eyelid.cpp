/*
 *  me_ct_tether.cpp - part of SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
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
 *      Marcus Thiebaux, USC
 *      Andrew n marshall, USC
 */

#include "gwiz_math.h"
#include "me_ct_eyelid.h"
#include <vhcl_log.h>

/*
	JOINTS:
	
		eyeball_left
		eyeball_right

		lower_eyelid_left
		upper_eyelid_left
		
		lower_eyelid_right
		upper_eyelid_right
*/

/*
	upper_eyelid_left
		offx = 3.15
		offy = 3.038
		offz = 11.75
	
	lookup
		posy = 0.372
		pitch = 0.0
	
	neutral
		posy = 0.0
		pitch = 0.0
	
	flat
		posy = -0.538
		pitch = 0.0

	lookdown
		posy = -0.788
		pitch = 0.0
	
	blink
		posy = -0.87
		pitch = 12.8
*/

//static int G_debug = 0;

//////////////////////////////////////////////////////////////////////////////////

const char* MeCtEyeLid::type_name = "EyeLid";

MeCtEyeLid::MeCtEyeLid( void )	{

   _skeleton_ref_p = NULL;
}

MeCtEyeLid::~MeCtEyeLid( void )	{
	
	clear();
}

void MeCtEyeLid::clear( void )	{
	
   _skeleton_ref_p = NULL;
}

#define EYEBALL_ROT_LIMIT_UP	-35.0f
#define EYEBALL_ROT_LIMIT_DN	30.0f

#define EYELID_Y_LIMIT_UP	0.372f
#define EYELID_Y_LIMIT_DN	-0.788f

float MeCtEyeLid::calc_upper_correction( 
	float in_eye_p,
	float in_lid_y
	)	{
	float adj_lid_y = 0.0;

	// adjust for eye pitch
	if( in_eye_p < 0.0 ) { // looking up
		float eye_norm = in_eye_p / EYEBALL_ROT_LIMIT_UP;
		adj_lid_y = eye_norm * EYELID_Y_LIMIT_UP;
	}
	else
	if( in_eye_p > 0.0 )	{ // looking down
		float eye_norm = in_eye_p / EYEBALL_ROT_LIMIT_DN;
		adj_lid_y = eye_norm * EYELID_Y_LIMIT_DN;
	}

	// adjust for blink/lift
	float out_lid_y = adj_lid_y;
	if( in_lid_y < 0.0 )	{ // eye is effectively closing/blinking
		float blink_norm = in_lid_y / EYELID_Y_LIMIT_DN;
		out_lid_y = adj_lid_y - blink_norm * ( adj_lid_y - EYELID_Y_LIMIT_DN );
	}
	else
	if( in_lid_y > 0.0 )	{ // eye is effectively lifting
		float blink_norm = in_lid_y / EYELID_Y_LIMIT_UP;
		out_lid_y = adj_lid_y + blink_norm * ( EYELID_Y_LIMIT_UP - adj_lid_y );
	}

//if( G_debug ) LOG( "eye: %f  lid: %f  --> %f\n", in_eye_p, in_lid_y, out_lid_y );

	return( out_lid_y );
}

void MeCtEyeLid::init( void ) {
	
	clear();
	
	_channels.add( "eyeball_left", SkChannel::Quat );
	_channels.add( "eyeball_right", SkChannel::Quat );
	
	_channels.add( "upper_eyelid_left", SkChannel::YPos );
	_channels.add( "upper_eyelid_right", SkChannel::YPos );

	MeController::init();
}

///////////////////////////////////////////////////////////////////////////////////

void MeCtEyeLid::context_updated( void ) {
}

void MeCtEyeLid::controller_map_updated( void ) {

}

void MeCtEyeLid::controller_start( void )	{

	if( _context->channels().size() > 0 )	{
		_skeleton_ref_p = _context->channels().skeleton();
	}
}

bool MeCtEyeLid::controller_evaluate( double t, MeFrameData& frame ) {

#if 0
	static int once = 1;
	if( once )	{
		once = 0;
		G_debug = 1;
		
		LOG( "look fwd:\n" );   // ( eye, lid )
		calc_upper_correction( 0.0, 0.4 ); // over-wide
		calc_upper_correction( 0.0, 0.1 ); // wide
		calc_upper_correction( 0.0, 0.05 );
		calc_upper_correction( 0.0, 0.0 ); // neutral
		calc_upper_correction( 0.0, -0.05 );
		calc_upper_correction( 0.0, -0.1 );
		calc_upper_correction( 0.0, -0.2 ); // sleepy
		calc_upper_correction( 0.0, -0.788 ); // closed
		calc_upper_correction( 0.0, -0.8 ); // over-closed

		LOG( "look up:\n" );
		calc_upper_correction( -20.0, 0.1 ); // wide
		calc_upper_correction( -20.0, 0.05 );
		calc_upper_correction( -20.0, 0.0 ); // neutral
		calc_upper_correction( -20.0, -0.05 );
		calc_upper_correction( -20.0, -0.1 );
		calc_upper_correction( -20.0, -0.2 ); // sleepy
		calc_upper_correction( -20.0, -0.788 ); // closed

		LOG( "look down:\n" );
		calc_upper_correction( 20.0, 0.1 ); // wide
		calc_upper_correction( 20.0, 0.05 );
		calc_upper_correction( 20.0, 0.0 ); // neutral
		calc_upper_correction( 20.0, -0.05 );
		calc_upper_correction( 20.0, -0.1 );
		calc_upper_correction( 20.0, -0.2 ); // sleepy
		calc_upper_correction( 20.0, -0.788 ); // closed
		G_debug = 0;
	}
#endif

	if( t < 0.0 )	{
		return( true );
	}

	float *fbuffer = &( frame.buffer()[0] );
	int n_chan = _channels.size();

	int L_eye_quat_chan_index = _context->channels().search( SkJointName( "eyeball_left" ), SkChannel::Quat );
	int R_eye_quat_chan_index =  _context->channels().search( SkJointName( "eyeball_right" ), SkChannel::Quat );

	int UL_lid_posy_chan_index =  _context->channels().search( SkJointName( "upper_eyelid_left" ), SkChannel::YPos );
	int UR_lid_posy_chan_index =  _context->channels().search( SkJointName( "upper_eyelid_right" ), SkChannel::YPos );

	int i_map;
	
	i_map = _context->toBufferIndex( L_eye_quat_chan_index );
	euler_t L_eye_e = quat_t(
		fbuffer[ i_map ],
		fbuffer[ i_map + 1 ],
		fbuffer[ i_map + 2 ],
		fbuffer[ i_map + 3 ]
	);

	int UL_lid_y_map = _context->toBufferIndex( UL_lid_posy_chan_index );
	float UL_lid_y = fbuffer[ UL_lid_y_map ];

	int UR_lid_y_map = _context->toBufferIndex( UR_lid_posy_chan_index );
	float UR_lid_y = fbuffer[ UR_lid_y_map ];

	float UL_correct_posy = calc_upper_correction( (float)( L_eye_e.p() ), UL_lid_y );
	fbuffer[ UL_lid_y_map ] = UL_correct_posy;
	fbuffer[ UR_lid_y_map ] = UL_correct_posy;

#if 0
	if( fabs( UL_lid_y ) > 0.0 ) LOG( "UL_lid_y: %f -> %f\n", UL_lid_y, UL_correct_posy );
#endif

	return true;
}

SkChannelArray& MeCtEyeLid::controller_channels( void )	{
	return( _channels );
}

double MeCtEyeLid::controller_duration( void ) {
	return( -1.0 );
}

const char* MeCtEyeLid::controller_type( void )	const {
	return( type_name );
}

void MeCtEyeLid::print_state( int tabCount ) {

	LOG("MeCtEyeLid" );

	const char* str = name();
	if( str )
		LOG(" \"%s\"", str );

	LOG("\n" );
}

