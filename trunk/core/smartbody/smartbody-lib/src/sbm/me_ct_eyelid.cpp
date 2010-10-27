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


#define DFL_EYEBALL_ROT_LIMIT_UP	-30.0f
#define DFL_EYEBALL_ROT_LIMIT_DN	35.0f

#define DFL_EYELID_UPPER_Y_LIMIT_UP	0.372f
#define DFL_EYELID_UPPER_Y_LIMIT_DN	-0.788f

#define DFL_EYELID_LOWER_Y_LIMIT_UP	0.2f
#define DFL_EYELID_LOWER_Y_LIMIT_DN	-0.2f

#define DFL_EYELID_UPPER_WEIGHT		1.0f
#define DFL_EYELID_LOWER_WEIGHT		0.2f

//////////////////////////////////////////////////////////////////////////////////

static bool G_debug = false;

const char* MeCtEyeLid::type_name = "EyeLid";

MeCtEyeLid::MeCtEyeLid( void )	{

   _skeleton_ref_p = NULL;

	setEyeballPitchRange( DFL_EYEBALL_ROT_LIMIT_UP, DFL_EYEBALL_ROT_LIMIT_DN );
	setEyelidUpperTransRange( DFL_EYELID_UPPER_Y_LIMIT_UP, DFL_EYELID_UPPER_Y_LIMIT_DN );
	setEyelidLowerTransRange( DFL_EYELID_LOWER_Y_LIMIT_UP, DFL_EYELID_LOWER_Y_LIMIT_DN );
	setEyelidWeight( DFL_EYELID_UPPER_WEIGHT, DFL_EYELID_LOWER_WEIGHT );
}

MeCtEyeLid::~MeCtEyeLid( void )	{
	
	clear();
}

void MeCtEyeLid::clear( void )	{
	
   _skeleton_ref_p = NULL;
}

#define LID_CORRECTION_EPSILON	0.0001f

float MeCtEyeLid::calc_lid_correction( 
	float in_eye_p, float eye_range[ 2 ],
	float in_lid_y, float lid_range[ 2 ]
	)	{
	float adj_lid_y = 0.0;

	// TODO: first clamp eye-pitch to range...
	// use EPSILON
	if( in_eye_p < eye_range[ 0 ] ) in_eye_p = eye_range[ 0 ];
	if( in_eye_p > eye_range[ 1 ] ) in_eye_p = eye_range[ 1 ];

	// adjust for eye pitch
	if( in_eye_p < -LID_CORRECTION_EPSILON ) { // looking up
		float eye_norm = in_eye_p / eye_range[ 0 ];
		adj_lid_y = eye_norm * lid_range[ 0 ];
	}
	else
	if( in_eye_p > LID_CORRECTION_EPSILON )	{ // looking down
		float eye_norm = in_eye_p / eye_range[ 1 ];
		adj_lid_y = eye_norm * lid_range[ 1 ];
	}

	// TODO: then clamp lid to range...
	// will this resolve normalization against 0.0?
	if( in_lid_y > lid_range[ 0 ] ) in_lid_y = lid_range[ 0 ];
	if( in_lid_y < lid_range[ 1 ] ) in_lid_y = lid_range[ 1 ];

	// adjust for blink/lift
	float out_lid_y = adj_lid_y;
	if( in_lid_y < -LID_CORRECTION_EPSILON )	{ // lid is lowering
		float blink_norm = in_lid_y / lid_range[ 1 ];
		out_lid_y = adj_lid_y - blink_norm * ( adj_lid_y - lid_range[ 1 ] );
	}
	else
	if( in_lid_y > LID_CORRECTION_EPSILON )	{ // lid is lifting
		float blink_norm = in_lid_y / lid_range[ 0 ];
		out_lid_y = adj_lid_y + blink_norm * ( lid_range[ 0 ] - adj_lid_y );
	}

	if( G_debug ) LOG( "eye: %f  lid: %f  --> %f", in_eye_p, in_lid_y, out_lid_y );

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
		
#if 1

		LOG( "eye: %f %f", _eyeballPitchRange[ 0 ], _eyeballPitchRange[ 1 ] );

		LOG( "UPPER:" );
		LOG( "lid: %f %f", _eyelidUpperTransRange[ 0 ], _eyelidUpperTransRange[ 1 ] );

		LOG( "look fwd:" ); 
		calc_lid_correction( 0.0, _eyeballPitchRange, 0.4, _eyelidUpperTransRange ); // over-wide
		calc_lid_correction( 0.0, _eyeballPitchRange, 0.1, _eyelidUpperTransRange ); // wide
		calc_lid_correction( 0.0, _eyeballPitchRange, 0.05, _eyelidUpperTransRange );
		calc_lid_correction( 0.0, _eyeballPitchRange, 0.0, _eyelidUpperTransRange ); // neutral
		calc_lid_correction( 0.0, _eyeballPitchRange, -0.05, _eyelidUpperTransRange );
		calc_lid_correction( 0.0, _eyeballPitchRange, -0.1, _eyelidUpperTransRange );
		calc_lid_correction( 0.0, _eyeballPitchRange, -0.2, _eyelidUpperTransRange ); // sleepy
		calc_lid_correction( 0.0, _eyeballPitchRange, -0.8, _eyelidUpperTransRange ); // closed
		calc_lid_correction( 0.0, _eyeballPitchRange, -1.0, _eyelidUpperTransRange ); // over-closed

		LOG( "look up:" );
		calc_lid_correction( -20.0, _eyeballPitchRange, 0.1, _eyelidUpperTransRange ); // wide
		calc_lid_correction( -20.0, _eyeballPitchRange, 0.05, _eyelidUpperTransRange );
		calc_lid_correction( -20.0, _eyeballPitchRange, 0.0, _eyelidUpperTransRange ); // neutral
		calc_lid_correction( -20.0, _eyeballPitchRange, -0.05, _eyelidUpperTransRange );
		calc_lid_correction( -20.0, _eyeballPitchRange, -0.1, _eyelidUpperTransRange );
		calc_lid_correction( -20.0, _eyeballPitchRange, -0.2, _eyelidUpperTransRange ); // sleepy
		calc_lid_correction( -20.0, _eyeballPitchRange, -0.8, _eyelidUpperTransRange ); // closed
		calc_lid_correction( -20.0, _eyeballPitchRange, -1.0, _eyelidUpperTransRange ); // closed

		LOG( "look down:" );
		calc_lid_correction( 20.0, _eyeballPitchRange, 0.1, _eyelidUpperTransRange ); // wide
		calc_lid_correction( 20.0, _eyeballPitchRange, 0.05, _eyelidUpperTransRange );
		calc_lid_correction( 20.0, _eyeballPitchRange, 0.0, _eyelidUpperTransRange ); // neutral
		calc_lid_correction( 20.0, _eyeballPitchRange, -0.05, _eyelidUpperTransRange );
		calc_lid_correction( 20.0, _eyeballPitchRange, -0.1, _eyelidUpperTransRange );
		calc_lid_correction( 20.0, _eyeballPitchRange, -0.2, _eyelidUpperTransRange ); // sleepy
		calc_lid_correction( 20.0, _eyeballPitchRange, -0.8, _eyelidUpperTransRange ); // closed
		calc_lid_correction( 20.0, _eyeballPitchRange, -1.0, _eyelidUpperTransRange ); // closed
#endif
#if 1
		LOG( "LOWER:" );
		LOG( "lid: %f %f", _eyelidLowerTransRange[ 0 ], _eyelidLowerTransRange[ 1 ] );

		LOG( "look fwd:" ); 
		calc_lid_correction( 0.0, _eyeballPitchRange, 0.3, _eyelidLowerTransRange ); // neutral
		calc_lid_correction( 0.0, _eyeballPitchRange, 0.2, _eyelidLowerTransRange ); // neutral
		calc_lid_correction( 0.0, _eyeballPitchRange, 0.1, _eyelidLowerTransRange ); // neutral
		calc_lid_correction( 0.0, _eyeballPitchRange, 0.0, _eyelidLowerTransRange ); // neutral
		calc_lid_correction( 0.0, _eyeballPitchRange, -0.05, _eyelidLowerTransRange );
		calc_lid_correction( 0.0, _eyeballPitchRange, -0.1, _eyelidLowerTransRange );
		calc_lid_correction( 0.0, _eyeballPitchRange, -0.2, _eyelidLowerTransRange ); // sleepy
		calc_lid_correction( 0.0, _eyeballPitchRange, -0.3, _eyelidLowerTransRange ); // closed

		LOG( "look up:" );
		calc_lid_correction( -20.0, _eyeballPitchRange, 0.3, _eyelidLowerTransRange ); // neutral
		calc_lid_correction( -20.0, _eyeballPitchRange, 0.2, _eyelidLowerTransRange ); // neutral
		calc_lid_correction( -20.0, _eyeballPitchRange, 0.1, _eyelidLowerTransRange ); // neutral
		calc_lid_correction( -20.0, _eyeballPitchRange, 0.0, _eyelidLowerTransRange ); // neutral
		calc_lid_correction( -20.0, _eyeballPitchRange, -0.05, _eyelidLowerTransRange );
		calc_lid_correction( -20.0, _eyeballPitchRange, -0.1, _eyelidLowerTransRange );
		calc_lid_correction( -20.0, _eyeballPitchRange, -0.2, _eyelidLowerTransRange ); // sleepy
		calc_lid_correction( -20.0, _eyeballPitchRange, -0.3, _eyelidLowerTransRange ); // closed

		LOG( "look down:" );
		calc_lid_correction( 20.0, _eyeballPitchRange, 0.3, _eyelidLowerTransRange ); // neutral
		calc_lid_correction( 20.0, _eyeballPitchRange, 0.2, _eyelidLowerTransRange ); // neutral
		calc_lid_correction( 20.0, _eyeballPitchRange, 0.1, _eyelidLowerTransRange ); // neutral
		calc_lid_correction( 20.0, _eyeballPitchRange, 0.0, _eyelidLowerTransRange ); // neutral
		calc_lid_correction( 20.0, _eyeballPitchRange, -0.05, _eyelidLowerTransRange );
		calc_lid_correction( 20.0, _eyeballPitchRange, -0.1, _eyelidLowerTransRange );
		calc_lid_correction( 20.0, _eyeballPitchRange, -0.2, _eyelidLowerTransRange ); // sleepy
		calc_lid_correction( 20.0, _eyeballPitchRange, -0.3, _eyelidLowerTransRange ); // closed
		G_debug = 0;
	}
#endif
#endif

	if( t < 0.0 )	{
		return( true );
	}

	float *fbuffer = &( frame.buffer()[0] );
	int n_chan = _channels.size();

	int L_eye_quat_chan_index = _context->channels().search( SkJointName( "eyeball_left" ), SkChannel::Quat );
//	int R_eye_quat_chan_index =  _context->channels().search( SkJointName( "eyeball_right" ), SkChannel::Quat );

	int UL_lid_posy_chan_index =  _context->channels().search( SkJointName( "upper_eyelid_left" ), SkChannel::YPos );
	int UR_lid_posy_chan_index =  _context->channels().search( SkJointName( "upper_eyelid_right" ), SkChannel::YPos );

	int LL_lid_posy_chan_index =  _context->channels().search( SkJointName( "lower_eyelid_left" ), SkChannel::YPos );
	int LR_lid_posy_chan_index =  _context->channels().search( SkJointName( "lower_eyelid_right" ), SkChannel::YPos );

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
//	float UR_lid_y = fbuffer[ UR_lid_y_map ];

	int LL_lid_y_map = _context->toBufferIndex( LL_lid_posy_chan_index );
	float LL_lid_y = fbuffer[ LL_lid_y_map ];

	int LR_lid_y_map = _context->toBufferIndex( LR_lid_posy_chan_index );
//	float LR_lid_y = fbuffer[ LR_lid_y_map ];

	float UL_correct_posy = _eyelidWeight[ 0 ] * calc_lid_correction( 
		(float)( L_eye_e.p() ), 
		_eyeballPitchRange,
		UL_lid_y, 
		_eyelidUpperTransRange
	);
	fbuffer[ UL_lid_y_map ] = UL_correct_posy;
	fbuffer[ UR_lid_y_map ] = UL_correct_posy;

#if 0
	float LL_correct_posy = _eyelidWeight[ 1 ] * calc_lid_correction( 
		(float)( L_eye_e.p() ), 
		_eyeballPitchRange,
		LL_lid_y, 
		_eyelidLowerTransRange
	);
	fbuffer[ LL_lid_y_map ] = LL_correct_posy;
	fbuffer[ LR_lid_y_map ] = LL_correct_posy;
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

