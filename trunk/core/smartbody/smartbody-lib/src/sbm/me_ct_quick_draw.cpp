/*
 *  me_ct_quick_draw.cpp - part of SmartBody-lib
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

//#include "sbm_pawn.hpp"
#include "me_ct_quick_draw.h"

#define NUM_QD_ARM_JOINTS  		4
#define MAX_JOINT_LABEL_LEN 	64

gw_float_t gwiz_safe_acos( gw_float_t c );

//////////////////////////////////////////////////////////////////////////////////

const char* MeCtQuickDraw::type_name = "QuickDraw";

MeCtQuickDraw::MeCtQuickDraw( void )	{

	_motion = NULL;
	_play_mode = SkMotion::Linear;
	_duration = -1.0;
	_last_apply_frame = 0;

	skeleton_ref_p = NULL;
	target_ref_joint_str = NULL;
	target_ref_joint_p = NULL;

	interim_pose_buff_p = NULL;
}

MeCtQuickDraw::~MeCtQuickDraw( void )	{

	if( interim_pose_buff_p )	{
		delete [] interim_pose_buff_p;
		interim_pose_buff_p = NULL;
	}
}

void MeCtQuickDraw::init( SkMotion* mot_p ) {
//	static char l_arm_labels[ NUM_QD_ARM_JOINTS ][ MAX_JOINT_LABEL_LEN ] = {
//		"l_shoulder", "l_elbow", "l_forearm", "l_wrist"
//	};
	static char r_arm_labels[ NUM_QD_ARM_JOINTS ][ MAX_JOINT_LABEL_LEN ] = {
		"r_shoulder", "r_elbow", "r_forearm", "r_wrist"
	};

	if( _motion ) {
		if( mot_p == _motion ) {
			// Minimal init()
			_last_apply_frame = 0;
			MeController::init ();
			return;
		}
	}

	_last_apply_frame = 0;
	
	_motion = mot_p;
	_motion->move_keytimes( 0 ); 
	raw_time = _motion->duration();

	set_time( raw_time );
	
//	const int n_chan = _motion->channels().size();
	const int n_float = _motion->channels().count_floats();

	_arm_chan_indices.size( NUM_QD_ARM_JOINTS );
	for( int i = 0; i < NUM_QD_ARM_JOINTS; i++ )	{

		// ASSUME right handed draw
		_arm_chan_indices[ i ] = 
			_motion->channels().search( SkJointName( r_arm_labels[ i ] ), SkChannel::Quat );
	}

	if( interim_pose_buff_p )	{
		delete [] interim_pose_buff_p;
	}
	interim_pose_buff_p = new float[ n_float ];
	
	MeController::init();

	if( _context ) {
		// Notify _context of channel change.
		_context->child_channels_updated( this );
	}
}

///////////////////////////////////////////////////////////////////////////////////

#if 0
void MeCtQuickDraw::capture_world_offset_state( void )	{
	
	if( _context )	{
		if( _context->channels().size() > 0 )	{

			if( skeleton_ref_p == NULL )	{
				skeleton_ref_p = _context->channels().skeleton(); // WHY HERE?
			}
			if( skeleton_ref_p )	{
				
				SkJoint* joint_p = skeleton_ref_p->search_joint( SbmPawn::WORLD_OFFSET_JOINT_NAME );
				if( joint_p )	{
					joint_p->update_gmat_up();

					SrMat sr_M;
					matrix_t M;
					int i, j;

					sr_M = joint_p->gmat();
					for( i=0; i<4; i++ )	{
						for( j=0; j<4; j++ )	{
							M.set( i, j, sr_M.get( i, j ) );
						}
					}
					
					world_offset_pos = M.translation( GWIZ_M_TR );
					world_offset_rot = M.quat( GWIZ_M_TR );
					return;
				}
				printf( "MeCtQuickDraw::capture_world_offset_state ERR: '%s' joint is NULL\n", SbmPawn::WORLD_OFFSET_JOINT_NAME );
				return;
			}
			printf( "MeCtQuickDraw::capture_world_offset_state ERR: skeleton reference is still NULL\n" );
			return;
		}
		printf( "MeCtQuickDraw::capture_world_offset_state ERR: context channels have no size\n" );
		return;
	}
	printf( "MeCtQuickDraw::capture_world_offset_state ERR: context is NULL\n" );
}
#endif

MeCtQuickDraw::joint_state_t MeCtQuickDraw::capture_joint_state( SkJoint *joint_p ) {
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

		SkJoint* parent_p = joint_p->parent();
		if( parent_p )	{

			sr_M = parent_p->gmat();
			for( i=0; i<4; i++ )	{
				for( j=0; j<4; j++ )	{
					M.set( i, j, sr_M.get( i, j ) );
				}
			}
			state.parent_pos = M.translation( GWIZ_M_TR );
			state.parent_rot = M.quat( GWIZ_M_TR );
		}
		else	{
			const char *name = joint_p->name();
			printf( "MeCtQuickDraw::capture_joint_state ERR: parent of joint '%s' not found\n", name );
		}
	}
	return( state );
}

quat_t MeCtQuickDraw::rotation_to_target( vector_t target_pos, joint_state_t* state_p )	{

	// parent coord
	vector_t l_target_pos = ( -state_p->parent_rot ) * ( target_pos - state_p->parent_pos );
	return( quat_t( euler_t( l_target_pos - state_p->local_pos, 0.0 ) ) );
}

SkJoint* MeCtQuickDraw::get_joint( char *joint_str, SkJoint **joint_pp )	{

	if( joint_str )	{
		if( *joint_pp == NULL )	{
			if( skeleton_ref_p )	{
				*joint_pp = skeleton_ref_p->search_joint( joint_str );
				if( *joint_pp == NULL )	{
					fprintf( stderr, "MeCtQuickDraw::get_joint ERR: joint '%s' NOT FOUND in skeleton\n", joint_str );
					free( joint_str );
					joint_str = NULL;
				}
			}
			else	{
				fprintf( stderr, "MeCtQuickDraw::get_joint ERR: skeleton NOT FOUND\n" );
			}
		}
	}
	return( *joint_pp );
}

SkJoint* MeCtQuickDraw::target_ref_joint( void ) {
//	return( target_ref_joint_p = get_joint( target_ref_joint_str, target_ref_joint_p ) );
	return( get_joint( target_ref_joint_str, &target_ref_joint_p ) );
}


vector_t MeCtQuickDraw::world_target_point( void )	{
	
	SkJoint* joint_p = target_ref_joint();
	if( joint_p )	{
		SrMat sr_M;
		matrix_t M;
		int i, j;
		
		joint_p->update_gmat_up();
		sr_M = joint_p->gmat();
		for( i=0; i<4; i++ )	{
			for( j=0; j<4; j++ )	{
				M.set( i, j, sr_M.get( i, j ) );
			}
		}
		vector_t pos = M.translation( GWIZ_M_TR );
		quat_t rot = M.quat( GWIZ_M_TR );
		return( pos + rot * point_target_pos );
	}
	return( point_target_pos );
}

void MeCtQuickDraw::set_target_coord_joint( SkJoint* joint_p )	{

	if( target_ref_joint_str ) free( target_ref_joint_str );
	target_ref_joint_str = NULL;
	target_ref_joint_p = joint_p;
}

void MeCtQuickDraw::set_target_point( float x, float y, float z )	{

	point_target_pos = vector_t( x, y, z );
}

void MeCtQuickDraw::set_target_joint( float x, float y, float z, SkJoint* ref_joint_p ) {

	set_target_point( x, y, z );
	set_target_coord_joint( ref_joint_p );
}

void MeCtQuickDraw::set_time( float sec )	{
	
	play_time = sec;
	play_time_scale = raw_time / play_time;
}

///////////////////////////////////////////////////////////////////////////////////

void MeCtQuickDraw::context_updated( void ) {

#if 0
	if( _context ) {
		skeleton_ref_p = _context->channels().skeleton(); // WHY HERE?
		if( skeleton_ref_p == NULL )	{
			printf( "MeCtQuickDraw::context_updated ERR: skeleton_ref_p is NULL\n" );
		}
	}
	else	{
		printf( "MeCtQuickDraw::context_updated ERR: context is NULL\n" );
	}
#endif
}

void MeCtQuickDraw::controller_map_updated( void ) {

	SkChannelArray& motion_channels = _motion->channels();
	const int n_chan = motion_channels.size();
	_motion_chan_to_buff.size( n_chan );


	if( _context ) {

		SkChannelArray& context_channels = _context->channels();
		for( int i=0; i<n_chan; ++i ) {
			int chan_index = context_channels.search( motion_channels.name( i ), motion_channels.type( i ) );
			_motion_chan_to_buff[ i ] = _context->toBufferIndex( chan_index );
		}

	} 
	else {
		printf( "MeCtQuickDraw::controller_map_updated: context NOT found\n" );
		_motion_chan_to_buff.setall( -1 );
	}
}

void MeCtQuickDraw::controller_start( void )	{

//	capture_world_offset_state();

	if( _context->channels().size() > 0 )	{
		skeleton_ref_p = _context->channels().skeleton();
	}

}

bool MeCtQuickDraw::controller_evaluate( double t, MeFrameData& frame ) {

#if 1
static float tmp_prev_t = 0.0;
static float tmp_accum = 0.0;
static int tmp_count = 0;
int tmp_event = 0;
tmp_accum += ( t - tmp_prev_t );
tmp_prev_t = t;
if( ( tmp_accum > 0.2 )&&( tmp_count < 10 ) )	{
	tmp_accum = 0.0;
	tmp_event = 1;
	tmp_count++;
}
else	{
	tmp_event = 0;
}
#endif

	bool continuing = true;
	continuing = t < _duration;

	if( t < 0.0 )	{
		return( continuing );
	}

#if 0 // RAW QUICKDRAW
	_motion->apply( 
		float( t * play_time_scale ),
		&( frame.buffer()[0] ),  // pointer to buffer's float array
		&_motion_chan_to_buff,
		_play_mode, 
		&_last_apply_frame 
	);

#else // ADAPTED QUICKDRAW

	_motion->apply( 
		float( t * play_time_scale ),
		interim_pose_buff_p,
		NULL, // same order in interim_buffer
		_play_mode, 
		&_last_apply_frame 
	);

	float *fbuffer = &( frame.buffer()[0] );
	SkChannelArray& motion_channels = _motion->channels();
	int n_chan = motion_channels.size();
	
	int interim_arm_chan_indices[ NUM_QD_ARM_JOINTS ];
	int i_interim = 0;
	int c_interim = 0;
	for( int i_chan=0; i_chan<n_chan; i_chan++ )	{

		int ch_size = motion_channels[ i_chan ].size();
//		SkChannel::Type ch_type = motion_channels[ i_chan ].type;

		bool is_arm_chan = false;
		for( int j = 0; j < NUM_QD_ARM_JOINTS; j++ )	{
			if( i_chan == _arm_chan_indices[ j ] )	{
				is_arm_chan = true;
			}
		}
		
		if( is_arm_chan == true )	{
			interim_arm_chan_indices[ c_interim ] = i_interim;
			c_interim++;
		}
		else	{
			for( int j=0; j<ch_size; j++ ) {
				fbuffer[ _motion_chan_to_buff[ i_chan ] + j ] = interim_pose_buff_p[ i_interim + j ];
			}
		}
		
		i_interim += ch_size;
	}

	if( skeleton_ref_p )	{
		SkJoint* r_wrist_joint_p = skeleton_ref_p->search_joint( "r_wrist" );
		r_wrist_joint_p->update_gmat_up();
	}
	else	{
		fprintf( stderr, "MeCtQuickDraw::controller_evaluate ERR: skeleton NOT FOUND\n" );
	}

	vector_t w_point = world_target_point();
	quat_t target_q[ NUM_QD_ARM_JOINTS ];

//if( tmp_event ) printf( "==\n" );

	for( int j = 0; j < NUM_QD_ARM_JOINTS; j++ )	{

		int i_map = _motion_chan_to_buff[ _arm_chan_indices[ j ] ];
		int i_interim = interim_arm_chan_indices[ j ];

		SkJoint* joint_p = _context->channels().joint( _toContextCh[ _arm_chan_indices[ j ] ] );
		if( joint_p == NULL ) printf( "joint is NULL\n" );
		joint_state_t state = capture_joint_state( joint_p );

		euler_t forward_rot = euler_t( 0.0, 0.0, 0.0 );
		vector_t forward_ref( -1.0, 0.0, 0.0 );
		vector_t forward_dir_n = forward_rot * forward_ref.normal();

		vector_t l_target_dir_n = ( -state.parent_rot ) * ( w_point - state.world_pos ).normal();
		gw_float_t angle = DEG( gwiz_safe_acos( forward_dir_n.dot( l_target_dir_n ) ) );

		vector_t axis = forward_dir_n.cross( l_target_dir_n ).normal();
		quat_t q( angle, axis );
		euler_t e = q;

		quat_t in_q(
			interim_pose_buff_p[ i_interim + 0 ],
			interim_pose_buff_p[ i_interim + 1 ],
			interim_pose_buff_p[ i_interim + 2 ],
			interim_pose_buff_p[ i_interim + 3 ]
		);
		euler_t in_e = in_q;

//if( tmp_event )	in_e.print();

		float s = t / play_time;
		if( s > 1.0 ) s = 1.0;

#if 1
		if( j == 0 )	{ // shoulder
//			set_q = in_q; 
//			set_q = in_q.lerp( s, quat_t() ); // null rot
//			set_q = in_q.lerp( s, euler_t( 0.0, 90.0, 0.0 ) );
//			set_q = in_q.lerp( s, q );
//			set_q = in_q.lerp( s * 0.33, q );
//			target_q[ j ] = q;
//			target_q[ j ] = euler_t( in_e.x(), 0.0, in_e.z() );
//			target_q[ j ] = euler_t( in_e.x(), e.y(), in_e.z() );
			target_q[ j ] = euler_t( in_e.x(), e.y(), e.z() );
		}
		else
		if( j == 1 )	{ // elbow
//			set_q = in_q; 
//			set_q = in_q.lerp( s, quat_t() ); // null rot
//			set_q = in_q.lerp( s, euler_t( 0.0, 90.0, 0.0 ) );
//			set_q = in_q.lerp( s, q );
//			target_q[ j ] = q;
//			target_q[ j ] = euler_t( in_e.x(), 0.0, 0.0 );
			target_q[ j ] = euler_t( in_e.x(), e.y(), e.z() );
		}
		else
		if( j == 2 )	{ // forearm
//			set_q = in_q; 
//			set_q = in_q.lerp( s, quat_t() ); // null rot
//			set_q = in_q.lerp( s, euler_t( -90.0, 0.0, 0.0 ) ); // null rot
//			target_q[ j ] = q;
//			target_q[ j ] = euler_t( -90.0, 0.0, 0.0 );
//			target_q[ j ] = euler_t( 0.0, 0.0, 0.0 );
			target_q[ j ] = euler_t( in_e.x(), 0.0, 0.0 );
		}
		else	{ // wrist
//			set_q = in_q; 
//			set_q = in_q.lerp( s, quat_t() ); // null rot
//			target_q[ j ] = q;
//			target_q[ j ] = euler_t( -90.0, 0.0, 0.0 );
//			target_q[ j ] = euler_t( 0.0, 0.0, 0.0 );
			target_q[ j ] = euler_t( in_e.x(), e.y(), e.z() );
		}
#else
		target_q[ j ] = euler_t( in_e.x(), e.y(), e.z() );
#endif
	}

	for( int j = 0; j < NUM_QD_ARM_JOINTS; j++ )	{

		int i_map = _motion_chan_to_buff[ _arm_chan_indices[ j ] ];
		int i_interim = interim_arm_chan_indices[ j ];
		
		quat_t in_q(
			interim_pose_buff_p[ i_interim + 0 ],
			interim_pose_buff_p[ i_interim + 1 ],
			interim_pose_buff_p[ i_interim + 2 ],
			interim_pose_buff_p[ i_interim + 3 ]
		);

		float s = t / play_time;
		if( s > 1.0 ) s = 1.0;
#if 1
		if( s < 0.5 ) s = 0.0;
		else s = ( s - 0.5 ) * 2.0;
#elif 0
		if( s < 0.667 ) s = 0.0;
		else s = ( s - 0.667 ) * 3.0;
#elif 0
		if( s < 0.75 ) s = 0.0;
		else s = ( s - 0.75 ) * 4.0;
#endif
//		quat_t blend_q = in_q.lerp( s, target_q[ j ] );
		quat_t blend_q = in_q.lerp( s * s, target_q[ j ] );
//		quat_t blend_q = in_q;
//		quat_t blend_q = target_q[ j ];
		
#if 1
		fbuffer[ i_map + 0 ] = blend_q.w();
		fbuffer[ i_map + 1 ] = blend_q.x();
		fbuffer[ i_map + 2 ] = blend_q.y();
		fbuffer[ i_map + 3 ] = blend_q.z();
#elif 0
		fbuffer[ i_map + 0 ] = in_q.w();
		fbuffer[ i_map + 1 ] = in_q.x();
		fbuffer[ i_map + 2 ] = in_q.y();
		fbuffer[ i_map + 3 ] = in_q.z();
#else
		fbuffer[ i_map + 0 ] = target_q[ j ].w();
		fbuffer[ i_map + 1 ] = target_q[ j ].x();
		fbuffer[ i_map + 2 ] = target_q[ j ].y();
		fbuffer[ i_map + 3 ] = target_q[ j ].z();
#endif
	}

#endif // ADAPTED QUICKDRAW

	return continuing;
}

SkChannelArray& MeCtQuickDraw::controller_channels( void )	{

	return( _motion->channels() );
}

double MeCtQuickDraw::controller_duration( void ) {

// THIS IS CALLED PRIOR TO controller_start().
	return( -1.0 );
}

const char* MeCtQuickDraw::controller_type( void )	{

	return type_name;
}

void MeCtQuickDraw::print_state( int tabCount ) {

	fprintf( stdout, "MeCtQuickDraw" );

	const char* str = name();
	if( str )
		fprintf( stdout, " \"%s\"", str );

	fprintf( stdout, ", motion" );
	if( _motion ) {

		// motion name
		str = _motion->name();
		if( str )
			fprintf( stdout, " \"%s\"", str );

		// motion filename
		str = _motion->filename();
		if( str )
			fprintf( stdout, " file \"%s\"", str );
	} 
	else {
		fprintf( stdout, "=NULL" );
	}
	fprintf( stdout, "\n" );
}

//////////////////////////////////////////////////////////////////////////////////

