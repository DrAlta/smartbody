/*
 *  me_ct_examples.cpp - part of SmartBody-lib
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

#include "me_ct_examples.h"
#include "gwiz_math.h"

#define MAX_JOINT_LABEL_LEN	32

///////////////////////////////////////////////////////////////////////////

/*
	HEAD ORIENT JOINTS: { "spine4", "spine5", "skullbase" }
*/

const char* MeCtHeadOrient::_type_name = "HeadOrient";

MeCtHeadOrient::MeCtHeadOrient( void )	{

	_duration = -1.0;
	_pitch_deg = 0.0;
	_heading_deg = 0.0;
	_roll_deg = 0.0;
}

MeCtHeadOrient::~MeCtHeadOrient( void )	{}

void MeCtHeadOrient::init( void )	{
	char joint_labels[ 3 ][ MAX_JOINT_LABEL_LEN ] = {
		"spine4",
		"spine5",
		"skullbase"
	};
	int i;
	
	for( i = 0; i < 3; i++ )	{
		_channels.add( SkJointName( joint_labels[ i ] ), SkChannel::Quat );
	}

	MeController::init();
}

void MeCtHeadOrient::set_orient( float dur, float p, float h, float r )	{
	
	_duration = dur;
	_pitch_deg = p;
	_heading_deg = h;
	_roll_deg = r;
}

void MeCtHeadOrient::controller_start()	{}

bool MeCtHeadOrient::controller_evaluate( double t, MeFrameData& frame )	{

	if( _duration > 0.0 )	{
		if( t > (double)_duration )	{
			return( FALSE );
		}
	}
	
	SrBuffer<float>& buff = frame.buffer();
	int channels_size = _channels.size();
	float p_deg_per_joint = _pitch_deg / (float)channels_size;
	float h_deg_per_joint = _heading_deg / (float)channels_size;
	float r_deg_per_joint = _roll_deg / (float)channels_size;

	for( int i=0; i<channels_size; ++i ) {

		int index = frame.toBufferIndex( _toContextCh[ i ] );

		euler_t E_in = quat_t(
			buff[ index + 0 ],
			buff[ index + 1 ],
			buff[ index + 2 ],
			buff[ index + 3 ]
		);
		
		quat_t Q_out;
		if( frame.isChannelUpdated( i ) )	{
			// If channel has been touched, preserve components and add delta
			Q_out = euler_t(
				E_in.p() + p_deg_per_joint,
				E_in.h() + h_deg_per_joint,
				E_in.r() + r_deg_per_joint
			);
		}
		else	{
			// If channel has NOT been touched, set absolute values
			Q_out = euler_t(
				p_deg_per_joint,
				h_deg_per_joint,
				r_deg_per_joint
			);
		}
		
		buff[ index + 0 ] = (float) Q_out.w();
		buff[ index + 1 ] = (float) Q_out.x();
		buff[ index + 2 ] = (float) Q_out.y();
		buff[ index + 3 ] = (float) Q_out.z();

		// Mark channel changed
		frame.channelUpdated( i );
	}

	return( TRUE );
}

void MeCtHeadOrient::print_state( int tabs )	{
	fprintf( stdout, "MeCtSimpleTilt" );

	const char* str = name();
	if( str )
		fprintf( stdout, " \"%s\"", str );

    fprintf( stdout, " p:%.3g h:%.3g r:%.3g degs for %.3g sec\n", 
		_pitch_deg, _heading_deg, _roll_deg, _duration );
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/*
	TILTING JOINTS: { "spine4", "spine5", "skullbase" }
*/

const char* MeCtSimpleTilt::_type_name = "SimpleTilt";

MeCtSimpleTilt::MeCtSimpleTilt( void )	{

	_duration = -1.0;
	_angle_deg = 0.0;
}

MeCtSimpleTilt::~MeCtSimpleTilt( void )	{}

void MeCtSimpleTilt::init( void )	{
	char joint_labels[ 3 ][ MAX_JOINT_LABEL_LEN ] = {
		"spine4",
		"spine5",
		"skullbase"
	};
	int i;

	for( i = 0; i < 3; i++ )	{
		_channels.add( SkJointName( joint_labels[ i ] ), SkChannel::Quat );
	}

	MeController::init();
}

void MeCtSimpleTilt::set_tilt( float dur, float angle_deg )	{
	
	_duration = dur;
	_angle_deg = angle_deg;
}

void MeCtSimpleTilt::controller_start()	{}

bool MeCtSimpleTilt::controller_evaluate( double t, MeFrameData& frame )	{

	if( _duration > 0.0 )	{
		if( t > (double)_duration )	{
			return( FALSE );
		}
	}
	
	SrBuffer<float>& buff = frame.buffer();
	int channels_size = _channels.size();
	float angle_deg_per_joint = _angle_deg / (float)channels_size;

	for( int i=0; i<channels_size; ++i ) {

		int index = frame.toBufferIndex( _toContextCh[ i ] );

		euler_t E_in = quat_t(
			buff[ index + 0 ],
			buff[ index + 1 ],
			buff[ index + 2 ],
			buff[ index + 3 ]
		);
		
		quat_t Q_out;
		if( frame.isChannelUpdated( i ) )	{
			// If channel has been touched, preserve components and add delta
			Q_out = euler_t(
				E_in.p(),
				E_in.h(),
				E_in.r() + angle_deg_per_joint
			);
		}
		else	{
			// If channel has NOT been touched, set absolute values
			Q_out = euler_t(
				0.0,
				0.0,
				angle_deg_per_joint
			);
		}
		
		buff[ index + 0 ] = (float) Q_out.w();
		buff[ index + 1 ] = (float) Q_out.x();
		buff[ index + 2 ] = (float) Q_out.y();
		buff[ index + 3 ] = (float) Q_out.z();

		// Mark channel changed
		frame.channelUpdated( i );
	}

	return( TRUE );
}

void MeCtSimpleTilt::print_state( int tabs )	{
	fprintf( stdout, "MeCtSimpleTilt" );

	const char* str = name();
	if( str )
		fprintf( stdout, " \"%s\"", str );

    fprintf( stdout, " %.3g degs for %.3g sec\n", _angle_deg, _duration );
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/*
	NODDING JOINTS: { "spine4", "spine5", "skullbase" }
*/

const char* MeCtSimpleNod::_type_name = "SimpleNod";

MeCtSimpleNod::MeCtSimpleNod( void )	{

	_duration = -1.0;
	_magnitude = 0.0;
	_repetitions = 0.0;
	_affirmative = TRUE;
}

MeCtSimpleNod::~MeCtSimpleNod( void )	{}

void MeCtSimpleNod::init( void )	{
	char joint_labels[ 3 ][ MAX_JOINT_LABEL_LEN ] = {
		"spine4",
		"spine5",
		"skullbase"
	};
	int i;
	
	for( i = 0; i < 3; i++ )	{
		_channels.add( SkJointName( joint_labels[ i ] ), SkChannel::Quat );
	}

	_first_eval = true;

	MeController::init();
}

void MeCtSimpleNod::set_nod( float dur, float mag, float rep, int aff, float smooth )	{
	
	_duration = dur;
	_magnitude = mag;
	_repetitions = rep;
	_affirmative = aff;
	_smooth = smooth;
}

void MeCtSimpleNod::controller_start()	{}

bool MeCtSimpleNod::controller_evaluate( double t, MeFrameData& frame )	{
	
	if( _duration > 0.0 )	{
		if( t > (double)_duration * 2.0) {
			return( FALSE );
		}
	}

	float dt;
	if ( _first_eval ) {
		_first_eval = false;
		dt = 0.001f;
	}
	else	{
		dt = (float)(t - _prev_time);
	}
	_prev_time = t;

	float angle_deg = 0;
	if (t <= (double) _duration)
	{
		float x = (float)( t / (double)_duration );
		angle_deg = (float)( -_magnitude * sin( x * 2.0 * M_PI * _repetitions ) );
	}
	
	SrBuffer<float>& buff = frame.buffer();

	// All of our channels are quats and recieve the same values
	int channels_size = _channels.size(); // Will be zero if init() errored
	float angle_deg_per_joint = angle_deg / (float)channels_size;

	float smooth_lerp = (float)(0.01 + ( 1.0 - powf( _smooth, dt * 30.0f /*SMOOTH_RATE_REF*/ ) ) * 0.99);

	for( int local_channel_index=0;
			local_channel_index<channels_size;
			++local_channel_index )
	{

		// get buffer index
		int context_channel_index = _toContextCh[ local_channel_index ];
		int index = frame.toBufferIndex( context_channel_index );

		quat_t Q_in = quat_t(
			buff[ index + 0 ],
			buff[ index + 1 ],
			buff[ index + 2 ],
			buff[ index + 3 ]
		);
		euler_t E_in = Q_in;

		quat_t Q_out;
		if (frame.isChannelUpdated( context_channel_index ) )	{
			// If channel has been touched, preserve components and add delta
			if( _affirmative )	{
				Q_out = euler_t(
					E_in.p() + angle_deg_per_joint,
					E_in.h(),
					E_in.r()
				);
			}
			else	{
				Q_out = euler_t(
					E_in.p(),
					E_in.h() + angle_deg_per_joint,
					E_in.r()
				);
			}
		}
		else	{
			// If channel has NOT been touched, set absolute values
			if( _affirmative )	{
				Q_out = euler_t(
					angle_deg_per_joint,
					0.0,
					0.0
				);
			}
			else	{
				Q_out = euler_t(
					0.0,
					angle_deg_per_joint,
					0.0
				);
			}
		}
		
		Q_out.lerp( 
				smooth_lerp, 
				Q_in,
				Q_out
			);

		buff[ index + 0 ] = (float) Q_out.w();
		buff[ index + 1 ] = (float) Q_out.x();
		buff[ index + 2 ] = (float) Q_out.y();
		buff[ index + 3 ] = (float) Q_out.z();

		// Mark channel changed
		frame.channelUpdated( context_channel_index );
	}
	
	return( TRUE );
}
		
void MeCtSimpleNod::print_state( int tabs )	{
	fprintf( stdout, "MeCtSimpleNod" );

	const char* str = name();
	if( str )
		fprintf( stdout, " \"%s\"", str );

    if( _affirmative )
		fprintf( stdout, " affirmative" );
    else
		fprintf( stdout, " negative" );

    fprintf( stdout, " %.3g reps @ %.3g degs for %.3g sec\n", _repetitions, _magnitude, _duration );
}

///////////////////////////////////////////////////////////////////////////
