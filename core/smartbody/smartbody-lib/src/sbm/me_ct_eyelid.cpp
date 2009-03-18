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

void MeCtEyeLid::init( SkMotion* neutral_p, SkMotion* blink_p, SkMotion* wide_p ) {
	
	clear();
	
	_neutral_pose_p = neutral_p;
	_blink_pose_p = blink_p;
	_wide_eye_pose_p = wide_p;
	
	_neutral_pose_p->ref();
	_blink_pose_p->ref();
	_wide_eye_pose_p->ref();
	
	_neutral_pose_p->move_keytimes( 0 );
	_blink_pose_p->move_keytimes( 0 );
	_wide_eye_pose_p->move_keytimes( 0 );

//	SkChannelArray& mChannels = _neutral_pose_p->channels();
//	float *f_arr = _neutral_pose_p->posture( 0 );
//	int f_pos = mChannels.float_position( mChannels.search( "lower_eyelid_left", SkChannel::YPos ) );
	print_motion_channel( _neutral_pose_p, "lower_eyelid_left" ); 
	print_motion_channel( _neutral_pose_p, "upper_eyelid_left" ); 

	print_motion_channel( _blink_pose_p, "lower_eyelid_left" ); 
	print_motion_channel( _blink_pose_p, "upper_eyelid_left" ); 

	print_motion_channel( _wide_eye_pose_p, "lower_eyelid_left" ); 
	print_motion_channel( _wide_eye_pose_p, "upper_eyelid_left" ); 


	_channels.add( "eyeball_left", SkChannel::Quat );
	_channels.add( "eyeball_right", SkChannel::Quat );

	_channels.add( "lower_eyelid_left", SkChannel::XPos );
	_channels.add( "lower_eyelid_left", SkChannel::YPos );
	_channels.add( "lower_eyelid_left", SkChannel::ZPos );
	_channels.add( "lower_eyelid_left", SkChannel::Quat );
	
	_channels.add( "upper_eyelid_left", SkChannel::XPos );
	_channels.add( "upper_eyelid_left", SkChannel::YPos );
	_channels.add( "upper_eyelid_left", SkChannel::ZPos );
	_channels.add( "upper_eyelid_left", SkChannel::Quat );
	
	_channels.add( "lower_eyelid_right", SkChannel::XPos );
	_channels.add( "lower_eyelid_right", SkChannel::YPos );
	_channels.add( "lower_eyelid_right", SkChannel::ZPos );
	_channels.add( "lower_eyelid_right", SkChannel::Quat );
	
	_channels.add( "upper_eyelid_right", SkChannel::XPos );
	_channels.add( "upper_eyelid_right", SkChannel::YPos );
	_channels.add( "upper_eyelid_right", SkChannel::ZPos );
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

/*
SkJoint* MeCtEyeLid::source_ref_joint( void ) {

	return( source_ref_joint_p = get_joint( source_ref_joint_str, source_ref_joint_p ) );
}
*/

/*
MeCtEyeLid::joint_state_t MeCtEyeLid::capture_joint_state( SkJoint *joint_p ) {
	SrMat sr_M;
	matrix_t M;
	int i, j;
	joint_state_t state;

	if( joint_p )	{

		sr_M = joint_p->lmat();
		for( i=0; i<4; i++ )	{
			for( j=0; j<4; j++ )	{
				M.set( i, j, sr_M.get( i, j ) );
			}
		}
		state.local_pos = M.translation( GWIZ_M_TR );
		state.local_rot = M.quat( GWIZ_M_TR );

		sr_M = joint_p->gmat();
		for( i=0; i<4; i++ )	{
			for( j=0; j<4; j++ )	{
				M.set( i, j, sr_M.get( i, j ) );
			}
		}
		state.world_pos = M.translation( GWIZ_M_TR );
		state.world_rot = M.quat( GWIZ_M_TR );
	}
	return( state );
}

MeCtEyeLid::joint_state_t MeCtEyeLid::calc_channel_state( MeCtEyeLid::joint_state_t source )	{
	joint_state_t state;

	state.world_pos = source.world_pos + source.world_rot * offset_pos;
	state.world_rot = source.world_rot * offset_rot;
	
	// if channel has a skeletal parent: subtract parent from world
	//   state.local_pos = 
	//   state.local_rot = 
	// else assume same
	state.local_pos = state.world_pos;
	state.local_rot = state.world_rot;

	return( state );
}
*/

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

	int UL_lid_quat_chan_index = _channels.search( SkJointName( "upper_eyelid_left" ), SkChannel::YPos );
	int i_map = _context->toBufferIndex( UL_lid_quat_chan_index );

	float Y = fbuffer[ i_map ];

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

