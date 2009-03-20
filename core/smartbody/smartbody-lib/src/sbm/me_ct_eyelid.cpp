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

/*
	JOINTS:
	
		eyeball_left
		eyeball_right

		lower_eyelid_left
		upper_eyelid_left
		
		lower_eyelid_right
		upper_eyelid_right
		
	REF MOTIONS:
		face_neutral.skm			: eyes open
		fac_45_blink.skm			: eyes closed
		fac_5_upper_lid_raiser.skm	: eyes wide open
*/

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

float MeCtEyeLid::get_motion_joint_pitch( SkMotion* mot_p, const char *chan_name ) {

	SkChannelArray& channels = mot_p->channels();
	float *f_arr = mot_p->posture( 0 );
	int f_index = channels.float_position( channels.search( chan_name, SkChannel::Quat ) );
	euler_t e = quat_t( 
		f_arr[ f_index ], 
		f_arr[ f_index + 1 ], 
		f_arr[ f_index + 2 ], 
		f_arr[ f_index + 3 ] 
	);
	return( (float)( e.p() ) );
}

float MeCtEyeLid::calculate_upper_correction( 
	float in_lid, 
	float in_eye
	)	{
	// NOTE: 
	//  neutral eyelid is open, and typically 0.
	//  positive rotation closes eyelid.
	float adj_lid = 0.0;
	float out_lid = 0.0;
	float threshhold = neut_lid_deg + 0.5f * ( blink_lid_deg - neut_lid_deg );

//	printf( "lid: %f eye: %f thresh: %f\n", in_lid, in_eye, threshhold );

	adj_lid = in_eye + in_lid;
	if( in_lid <= threshhold )	{ // eye is effectively open
		out_lid = adj_lid;
	}
	else	{ // eye is effectively closing
		float norm = ( in_lid - threshhold ) / ( blink_lid_deg - threshhold );
//		printf( " norm: %f\n", norm );
		out_lid = adj_lid + ( blink_lid_deg - adj_lid ) * norm;
	}

//	printf( " out_lid: %f\n\n", out_lid );
	return( out_lid );
}

void MeCtEyeLid::print_motion_channel( SkMotion* mot_p, const char *chan_name ) {
		
	SkChannelArray& channels = mot_p->channels();
	float *f_arr = mot_p->posture( 0 );
	int f_index;
	
	float x, y, z;

	f_index = channels.float_position( channels.search( chan_name, SkChannel::XPos ) );
	x = f_arr[ f_index ];

	f_index = channels.float_position( channels.search( chan_name, SkChannel::YPos ) );
	y = f_arr[ f_index ];

	f_index = channels.float_position( channels.search( chan_name, SkChannel::ZPos ) );
	z = f_arr[ f_index ];

	f_index = channels.float_position( channels.search( chan_name, SkChannel::Quat ) );
	euler_t e = quat_t( 
		f_arr[ f_index ], 
		f_arr[ f_index + 1 ], 
		f_arr[ f_index + 2 ], 
		f_arr[ f_index + 3 ] 
	);
	
	printf( "MeCtEyeLid:: '%s'::'%s'\n",
		mot_p->name(), chan_name
	);
	printf( " euler-HPR = %.2f %.2f %.2f\n",
		e.h(), e.p(), e.r()
	);
	printf( " pos-XYZ = %.3f %.3f %.3f\n",
		x, y, z
	);
}

//void MeCtEyeLid::init( SkMotion* neutral_p, SkMotion* blink_p, SkMotion* raise_p ) {
void MeCtEyeLid::init( SkMotion* neutral_p, SkMotion* blink_p ) {
	
	clear();
	
	_neutral_pose_p = neutral_p;
	_blink_pose_p = blink_p;
//	_raise_pose_p = raise_p;
	
	_neutral_pose_p->ref();
	_blink_pose_p->ref();
//	_raise_pose_p->ref();
	
	_neutral_pose_p->move_keytimes( 0 );
	_blink_pose_p->move_keytimes( 0 );
//	_raise_pose_p->move_keytimes( 0 );

//	SkChannelArray& mChannels = _neutral_pose_p->channels();
//	float *f_arr = _neutral_pose_p->posture( 0 );
//	int f_pos = mChannels.float_position( mChannels.search( "lower_eyelid_left", SkChannel::YPos ) );
#if 0
	print_motion_channel( _neutral_pose_p, "lower_eyelid_left" ); 
	print_motion_channel( _neutral_pose_p, "upper_eyelid_left" ); 

	print_motion_channel( _blink_pose_p, "lower_eyelid_left" ); 
	print_motion_channel( _blink_pose_p, "upper_eyelid_left" ); 

	print_motion_channel( _raise_pose_p, "lower_eyelid_left" ); 
	print_motion_channel( _raise_pose_p, "upper_eyelid_left" ); 
#endif

	neut_lid_deg = get_motion_joint_pitch( _neutral_pose_p, "upper_eyelid_left" ); // 0.0
	blink_lid_deg = get_motion_joint_pitch( _blink_pose_p, "upper_eyelid_left" );  // 12.81
//	raise_lid_deg = get_motion_joint_pitch( _raise_pose_p, "upper_eyelid_left" );  // -1.84

#if 0
printf( "neutral: %f\n", neut_lid_deg );
printf( "blink:  %f\n", blink_lid_deg );
//printf( "raise:  %f\n", raise_lid_deg );

	float p;
	printf( "look fwd:\n" );   // ( lid, eye )
	p = calculate_upper_correction( -2.0, 0.0 ); // wide
	p = calculate_upper_correction( 0.0, 0.0 ); // neutral
	p = calculate_upper_correction( 4.0, 0.0 );
	p = calculate_upper_correction( 6.3, 0.0 );
	p = calculate_upper_correction( 6.4, 0.0 ); // threshhold
	p = calculate_upper_correction( 6.5, 0.0 );
	p = calculate_upper_correction( 12.8, 0.0 ); // closed

	printf( "look up:\n" );
	p = calculate_upper_correction( -2.0, -10.0 );
	p = calculate_upper_correction( 0.0, -10.0 );
	p = calculate_upper_correction( 4.0, -10.0 );
	p = calculate_upper_correction( 6.3, -10.0 );
	p = calculate_upper_correction( 6.4, -10.0 );
	p = calculate_upper_correction( 6.5, -10.0 );
	p = calculate_upper_correction( 12.8, -10.0 );

	printf( "look down:\n" );
	p = calculate_upper_correction( -2.0, 10.0 );
	p = calculate_upper_correction( 0.0, 10.0 );
	p = calculate_upper_correction( 4.0, 10.0 );
	p = calculate_upper_correction( 6.3, 10.0 );
	p = calculate_upper_correction( 6.4, 10.0 );
	p = calculate_upper_correction( 6.5, 10.0 );
	p = calculate_upper_correction( 12.8, 10.0 );
#endif


	_channels.add( "eyeball_left", SkChannel::Quat );
	_channels.add( "eyeball_right", SkChannel::Quat );

//	_channels.add( "lower_eyelid_left", SkChannel::XPos );
//	_channels.add( "lower_eyelid_left", SkChannel::YPos );
//	_channels.add( "lower_eyelid_left", SkChannel::ZPos );
//	_channels.add( "lower_eyelid_left", SkChannel::Quat );
	
//	_channels.add( "upper_eyelid_left", SkChannel::XPos );
//	_channels.add( "upper_eyelid_left", SkChannel::YPos );
//	_channels.add( "upper_eyelid_left", SkChannel::ZPos );
	_channels.add( "upper_eyelid_left", SkChannel::Quat );
	
//	_channels.add( "lower_eyelid_right", SkChannel::XPos );
//	_channels.add( "lower_eyelid_right", SkChannel::YPos );
//	_channels.add( "lower_eyelid_right", SkChannel::ZPos );
//	_channels.add( "lower_eyelid_right", SkChannel::Quat );
	
//	_channels.add( "upper_eyelid_right", SkChannel::XPos );
//	_channels.add( "upper_eyelid_right", SkChannel::YPos );
//	_channels.add( "upper_eyelid_right", SkChannel::ZPos );
	_channels.add( "upper_eyelid_right", SkChannel::Quat );
	
//	UL_lid_quat_chan_index = _channels.search( SkJointName( "upper_eyelid_left" ), SkChannel::Quat );

	MeController::init();
}

///////////////////////////////////////////////////////////////////////////////////

SkJoint* MeCtEyeLid::get_joint( char *joint_str, SkJoint *joint_p )	{

	if( joint_str )	{
		if( joint_p == NULL )	{
			if( _skeleton_ref_p )	{
				joint_p = _skeleton_ref_p->search_joint( joint_str );
				if( joint_p == NULL )	{
					fprintf( stderr, "MeCtEyeLid::get_joint ERR: joint '%s' NOT FOUND in skeleton\n", joint_str );
					free( joint_str );
					joint_str = NULL;
				}
			}
			else	{
				fprintf( stderr, "MeCtEyeLid::get_joint ERR: skeleton NOT FOUND\n" );
			}
		}
	}
	return( joint_p );
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

	if( t < 0.0 )	{
		return( true );
	}

	float *fbuffer = &( frame.buffer()[0] );
	int n_chan = _channels.size();
// _channels.name( i )
// _channels.type( i )

	int UL_eye_quat_chan_index = _channels.search( SkJointName( "eyeball_left" ), SkChannel::Quat );
	int UR_eye_quat_chan_index = _channels.search( SkJointName( "eyeball_right" ), SkChannel::Quat );
	int UL_lid_quat_chan_index = _channels.search( SkJointName( "upper_eyelid_left" ), SkChannel::Quat );
	int UR_lid_quat_chan_index = _channels.search( SkJointName( "upper_eyelid_right" ), SkChannel::Quat );
	int i_map;
	
	i_map = _context->toBufferIndex( UL_eye_quat_chan_index );
	euler_t left_eye_e = quat_t(
		fbuffer[ i_map ],
		fbuffer[ i_map + 1 ],
		fbuffer[ i_map + 2 ],
		fbuffer[ i_map + 3 ]
	);

	i_map = _context->toBufferIndex( UR_eye_quat_chan_index );
	euler_t right_eye_e = quat_t(
		fbuffer[ i_map ],
		fbuffer[ i_map + 1 ],
		fbuffer[ i_map + 2 ],
		fbuffer[ i_map + 3 ]
	);

	int left_lid_map = _context->toBufferIndex( UL_lid_quat_chan_index );
	euler_t left_lid_e = quat_t(
		fbuffer[ left_lid_map ],
		fbuffer[ left_lid_map + 1 ],
		fbuffer[ left_lid_map + 2 ],
		fbuffer[ left_lid_map + 3 ]
	);

	int right_lid_map = _context->toBufferIndex( UR_lid_quat_chan_index );
	euler_t right_lid_e = quat_t(
		fbuffer[ right_lid_map ],
		fbuffer[ right_lid_map + 1 ],
		fbuffer[ right_lid_map + 2 ],
		fbuffer[ right_lid_map + 3 ]
	);
	
	float correct_pitch;
	
	correct_pitch = calculate_upper_correction( (float)left_lid_e.p(), (float)left_eye_e.p() );
	left_lid_e.p( correct_pitch );
	quat_t left_lid_q = left_lid_e;
	
	fbuffer[ left_lid_map ] = (float)left_lid_q.w();
	fbuffer[ left_lid_map + 1 ] = (float)left_lid_q.x();
	fbuffer[ left_lid_map + 2 ] = (float)left_lid_q.y();
	fbuffer[ left_lid_map + 3 ] = (float)left_lid_q.z();

	correct_pitch = calculate_upper_correction( (float)right_lid_e.p(), (float)right_eye_e.p() );
	right_lid_e.p( correct_pitch );
	quat_t right_lid_q = right_lid_e;

	fbuffer[ right_lid_map ] = (float)right_lid_q.w();
	fbuffer[ right_lid_map + 1 ] = (float)right_lid_q.x();
	fbuffer[ right_lid_map + 2 ] = (float)right_lid_q.y();
	fbuffer[ right_lid_map + 3 ] = (float)right_lid_q.z();

	
#if 0
	SkJoint* joint_p = source_ref_joint();
	joint_state_t state_in = capture_joint_state( joint_p );
	joint_state_t state_out = calc_channel_state( state_in );
	
	for( int i=0; i<n_chan; i++ ) {

		int index = frame.toBufferIndex( _toContextCh[ i ] );

		if( _channels.type( i ) == SkChannel::XPos )	{
			fbuffer[ index ] = (float)state_out.local_pos.x();
		}
		else
		if( _channels.type( i ) == SkChannel::YPos )	{
			fbuffer[ index ] = (float)state_out.local_pos.y();
		}
		else
		if( _channels.type( i ) == SkChannel::ZPos )	{
			fbuffer[ index ] = (float)state_out.local_pos.z();
		}
		else
		if( _channels.type( i ) == SkChannel::Quat )	{
			fbuffer[ index + 0 ] = (float) state_out.local_rot.w();
			fbuffer[ index + 1 ] = (float) state_out.local_rot.x();
			fbuffer[ index + 2 ] = (float) state_out.local_rot.y();
			fbuffer[ index + 3 ] = (float) state_out.local_rot.z();
		}

		// Mark channel changed
		frame.channelUpdated( i );
	}
#endif

	return true;
}

SkChannelArray& MeCtEyeLid::controller_channels( void )	{
	return( _channels );
}

double MeCtEyeLid::controller_duration( void ) {
	return( -1.0 );
}

const char* MeCtEyeLid::controller_type( void )	{
	return( type_name );
}

void MeCtEyeLid::print_state( int tabCount ) {

	fprintf( stdout, "MeCtEyeLid" );

	const char* str = name();
	if( str )
		fprintf( stdout, " \"%s\"", str );

	fprintf( stdout, "\n" );
}

