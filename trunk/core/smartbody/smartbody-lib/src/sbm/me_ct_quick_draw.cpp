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

#define TMP_NUM_ARM_JOINTS  	4
#define MAX_JOINT_LABEL_LEN 	64

//////////////////////////////////////////////////////////////////////////////////

const char* MeCtQuickDraw::type_name = "QuickDraw";

MeCtQuickDraw::MeCtQuickDraw( void )	{

   _motion = NULL;
   _play_mode = SkMotion::Linear;
   _duration = -1.0;
   _last_apply_frame = 0;

   skeleton_ref_p = NULL;
   interim_pose_buff_p = NULL;

	aim_mode = AIM_LOCAL;
}

MeCtQuickDraw::~MeCtQuickDraw( void )	{

	if( interim_pose_buff_p )	{
		delete [] interim_pose_buff_p;
		interim_pose_buff_p = NULL;
	}
}

void MeCtQuickDraw::init( SkMotion* mot_p ) {
//	static char l_arm_labels[ TMP_NUM_ARM_JOINTS ][ MAX_JOINT_LABEL_LEN ] = {
//		"l_shoulder", "l_elbow", "l_forearm", "l_wrist"
//	};
	static char r_arm_labels[ TMP_NUM_ARM_JOINTS ][ MAX_JOINT_LABEL_LEN ] = {
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

	_arm_chan_indices.size( TMP_NUM_ARM_JOINTS );
	for( int i = 0; i < TMP_NUM_ARM_JOINTS; i++ )	{

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

void MeCtQuickDraw::set_target_joint( float x, float y, float z, SkJoint* ref_joint_p ) {

	// TODO (added by Andrew)
	printf( "WARNING: MeCtQuickDraw::set_target_joint(..) not implemented.\n" );
}

void MeCtQuickDraw::set_aim_local( float p, float h, float r )    {

	aim_mode = AIM_LOCAL;
	// TODO
}

void MeCtQuickDraw::set_aim_world( float p, float h, float r )    {
	
	aim_mode = AIM_WORLD;
	// TODO
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
}

bool MeCtQuickDraw::controller_evaluate( double t, MeFrameData& frame ) {

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


	int index = 0;
	for( int i=0; i<n_chan; i++ )	{

		int ch_size = motion_channels[ i ].size();
		SkChannel::Type ch_type = motion_channels[ i ].type;
		
		float tmp_f[ 4 ];
		for( int j=0; j<ch_size; j++ ) {
			tmp_f[ j ] = interim_pose_buff_p[ index + j ];
		}

		bool arm_chan = false;
#if 0
		for( int j = 0; j < TMP_NUM_ARM_JOINTS; j++ )	{
			if( i == _arm_chan_indices[ j ] )	{
				arm_chan = true;
			}
		}
#endif
		
		if( arm_chan == true )	{
			int i_map = _motion_chan_to_buff[ i ];
			fbuffer[ i_map + 0 ] = 1.0;
			fbuffer[ i_map + 1 ] = 0.0;
			fbuffer[ i_map + 2 ] = 0.0;
			fbuffer[ i_map + 3 ] = 0.0;
		}
		else	{
			for( int j=0; j<ch_size; j++ ) {
				fbuffer[ _motion_chan_to_buff[ i ] + j ] = tmp_f[ j ];
			}
		}
		
		index += ch_size;
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

