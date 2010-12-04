/*
 *  me_ct_eyelid.cpp - part of SmartBody-lib
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

//////////////////////////////////////////////////////////////////////////////////

const char* MeCtEyeLidRegulator::type_name = "EyeLidRegulator";

//////////////////////////////////////////////////////////////////////////////////

void MeCtEyeLidRegulator::LidSet::print( void )	{

	printf( "dirty      : %d\n", dirty_bit );
	printf( "base_angle : %f\n", base_angle );
	printf( "full_angle : %f\n", full_angle );
	printf( "blink_angle: %f\n", blink_angle );
	printf( "diff       : %f\n", diff );
	printf( "lid_tight  : %f\n", lid_tight );
	printf( "open_angle : %f\n", open_angle );
	printf( "tight_sweep: %f\n", tight_sweep );
	printf( "close_sweep: %f\n", close_sweep );
}

void MeCtEyeLidRegulator::LidSet::set_range( float fr, float to )	{
		
	base_angle = fr;
	full_angle = to;
	diff = full_angle - base_angle;
	if( fabs( diff ) > 0.0f )	{
		inv_diff = 1.0f / diff;
	}
	else	{
		inv_diff = 0.0f;
	}
	dirty_bit = true;
}

void MeCtEyeLidRegulator::LidSet::set_blink( float angle )	{
		
	blink_angle = angle;
	dirty_bit = true;
}

void MeCtEyeLidRegulator::LidSet::set_tighten( float tighten )	{
	
	lid_tight = tighten;
	if( lid_tight > 1.0f ) lid_tight = 1.0f;
	if( lid_tight < 0.0f ) lid_tight = 0.0f;
	dirty_bit = true;
}

void MeCtEyeLidRegulator::LidSet::set_pitch( float pitch )	{

	eye_pitch = pitch;
	dirty_bit = true;
}

float MeCtEyeLidRegulator::LidSet::get_mapped_weight( float in_weight )	{

	if( dirty_bit ) {

		open_angle = base_angle * ( 1.0f - lid_tight ) + eye_pitch;
		tight_sweep = open_angle - base_angle;
		close_sweep = blink_angle - open_angle;
		dirty_bit = false;
	}

	float weight = ( tight_sweep + in_weight * close_sweep ) * inv_diff;
	if( weight < 0.0f ) return( 0.0f );
	if( weight > 1.0f ) return( 1.0f );
	return( weight );
}

void MeCtEyeLidRegulator::test( void )	{

	LidSet lid;

	printf( "LidSet TEST:\n" );

//	lid.set_range( -30.0f, 30.0f );
//	lid.set_blink( 30.0f );
	lid.set_range( 30.0f, 0.0f );
	lid.set_blink( 0.0f );
	lid.set_pitch( -15.0f );
	lid.print();

	int i;
	for( i=0; i<=10; i++ )	{
		float f = (float)i/10.0f;
		float w = lid.get_mapped_weight( f );
		printf( "[%d] f:%f w:%f\n", i, f, w );
	}

	lid.set_tighten( 0.5 );
	for( i=0; i<=10; i++ )	{
		float f = (float)i/10.0f;
		float w = lid.get_mapped_weight( f );
		printf( "[%d] f:%f w:%f\n", i, f, w );
	}

	lid.set_tighten( 1.0 );
	for( i=0; i<=10; i++ )	{
		float f = (float)i/10.0f;
		float w = lid.get_mapped_weight( f );
		printf( "[%d] f:%f w:%f\n", i, f, w );
	}

//	lid.set_range( -50.0f, 0.0f );
	lid.set_blink( 0.0f );
	lid.set_tighten( 0.0f );
	lid.print();

	for( i=0; i<=10; i++ )	{
		float f = (float)i/10.0f;
		float w = lid.get_mapped_weight( f );
		printf( "[%d] f:%f w:%f\n", i, f, w );
	}

	lid.set_tighten( 0.5 );
	for( i=0; i<=10; i++ )	{
		float f = (float)i/10.0f;
		float w = lid.get_mapped_weight( f );
		printf( "[%d] f:%f w:%f\n", i, f, w );
	}

	lid.set_tighten( 1.0 );
	for( i=0; i<=10; i++ )	{
		float f = (float)i/10.0f;
		float w = lid.get_mapped_weight( f );
		printf( "[%d] f:%f w:%f\n", i, f, w );
	}
}

//////////////////////////////////////////////////////////////////////////////////

MeCtEyeLidRegulator::MeCtEyeLidRegulator( void )	{

}

MeCtEyeLidRegulator::~MeCtEyeLidRegulator( void )	{

}

void MeCtEyeLidRegulator::init( bool tracking_pitch )	{

	_channels.add( "eyeball_left", SkChannel::Quat );
	_channels.add( "eyeball_right", SkChannel::Quat );
	
	_channels.add( "au_45_left", SkChannel::XPos );
	_channels.add( "au_45_right", SkChannel::XPos );
	
	MeController::init();
	
	set_upper_range( -30.0f, 20.0f );
	set_lower_range( 20.0f, 20.0f ); // non existent...
	
	curve.insert( 0.0, 0.0 );
	curve.insert( 0.05, 1.0 );
	curve.insert( 0.2, 0.33 );
	curve.insert( 0.25, 0.0 );
	
	pitch_tracking = tracking_pitch;
	new_blink = false;
	
	blink_period_min = 4.0;
	blink_period_max = 8.0;
	
	blink_period = blink_period_min;
	prev_blink = 0.0;
	
	prev_UL_value = 0.0f;
	prev_LL_value = 0.0f;
	prev_UR_value = 0.0f;
	prev_LR_value = 0.0f;
	
	UL_value = 0.0f;
	LL_value = 0.0f;
	UR_value = 0.0f;
	LR_value = 0.0f;
	
//						test();
}

void MeCtEyeLidRegulator::context_updated( void ) {
}

void MeCtEyeLidRegulator::controller_map_updated( void ) {
}

void MeCtEyeLidRegulator::controller_start( void )	{
}

bool MeCtEyeLidRegulator::controller_evaluate( double t, MeFrameData& frame ) {

	if( t < 0.0 )	{
		return( true );
	}
	
	if( new_blink ) {
		prev_blink = 0.0;
		new_blink = false;
	}
	
	double blink_elapsed = t - prev_blink;
	if( blink_elapsed >= blink_period )	{
//LOG( "blink @ %f", blink_elapsed );
		blink_elapsed = 0.0;
		prev_blink = t;
		float r = (float)rand() / (float)RAND_MAX;
		blink_period = blink_period_min + r * ( blink_period_max - blink_period_min );
	}

	float *fbuffer = &( frame.buffer()[0] );
	int n_chan = _channels.size();

	if( pitch_tracking )	{
		int L_eye_quat_idx = _context->channels().search( SkJointName( "eyeball_left" ), SkChannel::Quat );
		int R_eye_quat_idx =  _context->channels().search( SkJointName( "eyeball_right" ), SkChannel::Quat );

		int buff_idx = _context->toBufferIndex( L_eye_quat_idx );
		euler_t L_eye_e = quat_t(
			fbuffer[ buff_idx ],
			fbuffer[ buff_idx + 1 ],
			fbuffer[ buff_idx + 2 ],
			fbuffer[ buff_idx + 3 ]
		);

		buff_idx = _context->toBufferIndex( R_eye_quat_idx );
		euler_t R_eye_e = quat_t(
			fbuffer[ buff_idx ],
			fbuffer[ buff_idx + 1 ],
			fbuffer[ buff_idx + 2 ],
			fbuffer[ buff_idx + 3 ]
		);

		UL_set.set_pitch( (float)( L_eye_e.p() ) );
		UR_set.set_pitch( (float)( R_eye_e.p() ) );
#if 0
		LL_set.set_pitch( (float)( L_eye_e.p() ) );
		LR_set.set_pitch( (float)( R_eye_e.p() ) );
#endif
	}

	prev_UL_value = UL_value; // for change detection
	prev_LL_value = LL_value;
	prev_UR_value = UR_value;
	prev_LR_value = LR_value;

	float raw_lid_val = (float)( curve.evaluate( blink_elapsed ) );
	
	UL_value = UL_set.get_mapped_weight( raw_lid_val );
	UR_value = UR_set.get_mapped_weight( raw_lid_val );
#if 0
	LL_value = LL_set.get_mapped_weight( raw_lid_val );
	LR_value = LR_set.get_mapped_weight( raw_lid_val );
#endif

	int UL_au_blink_idx =  _context->channels().search( SkJointName( "au_45_left" ), SkChannel::XPos );
	int UR_au_blink_idx =  _context->channels().search( SkJointName( "au_45_right" ), SkChannel::XPos );

	int UL_au_blink_buff_idx = _context->toBufferIndex( UL_au_blink_idx );
	int UR_au_blink_buff_idx = _context->toBufferIndex( UR_au_blink_idx );

	if( UL_au_blink_buff_idx >= 0 )	{
		fbuffer[ UL_au_blink_buff_idx ] = UL_value;
	}
	if( UR_au_blink_buff_idx >= 0 )	{
		fbuffer[ UR_au_blink_buff_idx ] = UR_value;
	}

#if 0
	int LL_au_blink_idx =  _context->channels().search( SkJointName( "xxxx" ), SkChannel::XPos );
	int LR_au_blink_idx =  _context->channels().search( SkJointName( "xxxx" ), SkChannel::XPos );

	int LL_au_blink_buff_idx = _context->toBufferIndex( LL_au_blink_idx );
	int LR_au_blink_buff_idx = _context->toBufferIndex( LR_au_blink_idx );

	if( LL_au_blink_buff_idx >= 0 )	{
		fbuffer[ LL_au_blink_buff_idx ] = UL_value;
	}
	if( LR_au_blink_buff_idx >= 0 )	{
		fbuffer[ LR_au_blink_buff_idx ] = UR_value;
	}
#endif

	return( true );
}

SkChannelArray& MeCtEyeLidRegulator::controller_channels( void )	{
	return( _channels );
}

double MeCtEyeLidRegulator::controller_duration( void ) {
	return( -1.0 );
}

const char* MeCtEyeLidRegulator::controller_type( void )	const {
	return( type_name );
}

void MeCtEyeLidRegulator::print_state( int tabCount ) {

	LOG("MeCtEyeLidRegulator" );

	const char* str = name();
	if( str )
		LOG(" \"%s\"", str );

	LOG("\n" );
}
//////////////////////////////////////////////////////////////////////////////////

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

	precision = 0.0001f; // ought to be enough, as long as you are not modeling in miles...
	
	set_weight( DFL_EYELID_LOWER_WEIGHT, DFL_EYELID_UPPER_WEIGHT );
	set_lower_lid_range( DFL_EYELID_LOWER_Y_LIMIT_DN, DFL_EYELID_LOWER_Y_LIMIT_UP );
	set_upper_lid_range( DFL_EYELID_UPPER_Y_LIMIT_DN, DFL_EYELID_UPPER_Y_LIMIT_UP );
	set_eye_pitch_range( DFL_EYEBALL_ROT_LIMIT_DN, DFL_EYEBALL_ROT_LIMIT_UP );
}

MeCtEyeLid::~MeCtEyeLid( void )	{
}

float MeCtEyeLid::calc_lid_correction( 
	float in_eye_p, float eye_range[ 2 ],
	float in_lid_y, float lid_range[ 2 ]
	)	{
	float adj_lid_y = 0.0;

	// first clamp eye-pitch to range...
	if( in_eye_p > eye_range[ 0 ] ) in_eye_p = eye_range[ 0 ]; // down is positive...
	if( in_eye_p < eye_range[ 1 ] ) in_eye_p = eye_range[ 1 ];

	// adjust for eye pitch
	if( in_eye_p > precision )	{ // looking down
		float eye_norm = in_eye_p / eye_range[ 0 ];
		adj_lid_y = eye_norm * lid_range[ 0 ];
	}
	else
	if( in_eye_p < -precision ) { // looking up
		float eye_norm = in_eye_p / eye_range[ 1 ];
		adj_lid_y = eye_norm * lid_range[ 1 ];
	}

	// then clamp lid to range...
	//  prevents normalization against 0.0
	if( in_lid_y < lid_range[ 0 ] ) in_lid_y = lid_range[ 0 ];
	if( in_lid_y > lid_range[ 1 ] ) in_lid_y = lid_range[ 1 ];

	// adjust for blink/lift
	float out_lid_y = adj_lid_y;
	if( in_lid_y < -precision )	{ // lid is lowering
		float blink_norm = in_lid_y / lid_range[ 0 ];
		out_lid_y = adj_lid_y - blink_norm * ( adj_lid_y - lid_range[ 0 ] );
	}
	else
	if( in_lid_y > precision )	{ // lid is lifting
		float blink_norm = in_lid_y / lid_range[ 1 ];
		out_lid_y = adj_lid_y + blink_norm * ( lid_range[ 1 ] - adj_lid_y );
	}

	if( G_debug ) LOG( "eye: %f  lid: %f  --> %f", in_eye_p, in_lid_y, out_lid_y );

	return( out_lid_y );
}

void MeCtEyeLid::init( void ) {
	
	_channels.add( "eyeball_left", SkChannel::Quat );
	_channels.add( "eyeball_right", SkChannel::Quat );
	
	_channels.add( "upper_eyelid_left", SkChannel::YPos );
	_channels.add( "upper_eyelid_right", SkChannel::YPos );

	_channels.add( "lower_eyelid_left", SkChannel::YPos );
	_channels.add( "lower_eyelid_right", SkChannel::YPos );

	MeController::init();
}

///////////////////////////////////////////////////////////////////////////////////

void MeCtEyeLid::context_updated( void ) {
}

void MeCtEyeLid::controller_map_updated( void ) {

}

void MeCtEyeLid::controller_start( void )	{

//	if( _context->channels().size() > 0 )	{
//		_skeleton_ref_p = _context->channels().skeleton();
//	}
}

bool MeCtEyeLid::controller_evaluate( double t, MeFrameData& frame ) {

#if 0
	static int once = 1;
	if( once )	{
		once = 0;
		G_debug = 1;
		
		LOG( "eye: %f %f", eye_pitch_range[ 0 ], eye_pitch_range[ 1 ] );
		int i;
#if 1

		LOG( "UPPER:" );
		LOG( "lid: %f %f", upper_lid_range[ 0 ], upper_lid_range[ 1 ] );

		float up_lid_samps[ 8 ] = { -1.0f, -0.5f, -0.2f, -0.1f, 0.0f, 0.001f, 0.1f, 0.4f };

		LOG( "look up:" );
		for( i=0; i<8; i++ )	{
			calc_lid_correction( -30.0f, eye_pitch_range, up_lid_samps[ i ], upper_lid_range );
		}
		for( i=0; i<8; i++ )	{
			calc_lid_correction( -10.0f, eye_pitch_range, up_lid_samps[ i ], upper_lid_range );
		}
		LOG( "look fwd:" ); 
		for( i=0; i<8; i++ )	{
			calc_lid_correction( 0.0f, eye_pitch_range, up_lid_samps[ i ], upper_lid_range );
		}
		LOG( "look down:" );
		for( i=0; i<8; i++ )	{
			calc_lid_correction( 10.0f, eye_pitch_range, up_lid_samps[ i ], upper_lid_range );
		}
		for( i=0; i<8; i++ )	{
			calc_lid_correction( 30.0f, eye_pitch_range, up_lid_samps[ i ], upper_lid_range );
		}
#endif
#if 1
		LOG( "LOWER:" );
		LOG( "lid: %f %f", lower_lid_range[ 0 ], lower_lid_range[ 1 ] );

		float lo_lid_samps[ 8 ] = { -0.3f, -0.2f, -0.1f, 0.0f, 0.001f, 0.1f, 0.2f, 0.3f };

		LOG( "look up:" );
		for( i=0; i<8; i++ )	{
			calc_lid_correction( -30.0f, eye_pitch_range, lo_lid_samps[ i ], lower_lid_range );
		}
		for( i=0; i<8; i++ )	{
			calc_lid_correction( -10.0f, eye_pitch_range, lo_lid_samps[ i ], lower_lid_range );
		}
		LOG( "look fwd:" ); 
		for( i=0; i<8; i++ )	{
			calc_lid_correction( 0.0f, eye_pitch_range, lo_lid_samps[ i ], lower_lid_range );
		}
		LOG( "look down:" );
		for( i=0; i<8; i++ )	{
			calc_lid_correction( 10.0f, eye_pitch_range, lo_lid_samps[ i ], lower_lid_range );
		}
		for( i=0; i<8; i++ )	{
			calc_lid_correction( 30.0f, eye_pitch_range, lo_lid_samps[ i ], lower_lid_range );
		}
#endif
		G_debug = 0;
	}
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
	int UR_lid_y_map = _context->toBufferIndex( UR_lid_posy_chan_index );
#if 1
	
	float UL_lid_y = fbuffer[ UL_lid_y_map ];

	float UL_correct_posy = lid_weight[ 1 ] * calc_lid_correction( 
		(float)( L_eye_e.p() ), 
		eye_pitch_range,
		UL_lid_y, 
		upper_lid_range
	);
	fbuffer[ UL_lid_y_map ] = UL_correct_posy;


	float UR_lid_y = fbuffer[ UR_lid_y_map ];

	float UR_correct_posy = lid_weight[ 1 ] * calc_lid_correction( 
		(float)( L_eye_e.p() ), 
		eye_pitch_range,
		UR_lid_y, 
		upper_lid_range
	);
	fbuffer[ UR_lid_y_map ] = UR_correct_posy;
#else
	fbuffer[ UL_lid_y_map ] = 0.0;
	fbuffer[ UR_lid_y_map ] = 0.0;
#endif

	int LL_lid_y_map = _context->toBufferIndex( LL_lid_posy_chan_index );
	int LR_lid_y_map = _context->toBufferIndex( LR_lid_posy_chan_index );
#if 1
	float LL_lid_y = fbuffer[ LL_lid_y_map ];

	float LL_correct_posy = lid_weight[ 0 ] * calc_lid_correction( 
		(float)( L_eye_e.p() ), 
		eye_pitch_range,
		LL_lid_y, 
		lower_lid_range
	);
	fbuffer[ LL_lid_y_map ] = LL_correct_posy;

	float LR_lid_y = fbuffer[ LR_lid_y_map ];

	float LR_correct_posy = lid_weight[ 0 ] * calc_lid_correction( 
		(float)( L_eye_e.p() ), 
		eye_pitch_range,
		LL_lid_y, 
		lower_lid_range
	);
	fbuffer[ LR_lid_y_map ] = LR_correct_posy;
#else
	fbuffer[ LL_lid_y_map ] = 0.0;
	fbuffer[ LR_lid_y_map ] = 0.0;
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

