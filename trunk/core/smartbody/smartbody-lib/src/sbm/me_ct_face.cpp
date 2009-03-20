/*
 *  me_ct_face.cpp - part of SmartBody-lib
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
#include "gwiz_math.h"
#include "me_ct_face.h"

//////////////////////////////////////////////////////////////////////////////////

const char* MeCtFace::type_name = "Face";

MeCtFace::MeCtFace( void )	{

   _duration = -1.0;
   _base_pose_p = NULL;
   _skeleton_ref_p = NULL;
}

MeCtFace::~MeCtFace( void )	{
	
	clear();
}

void MeCtFace::clear( void )	{
	
   _duration = -1.0;
   _skeleton_ref_p = NULL;

	if( _base_pose_p )	{
		_base_pose_p->unref();
		_base_pose_p = NULL;
	}

	SkMotion* key_pose_p;
	_key_pose_map.reset();
	while( key_pose_p = _key_pose_map.pull() )	{
		key_pose_p->unref();
	}
}

void MeCtFace::init( SkMotion* base_ref_p ) {
	
	clear();
	
	_base_pose_p = base_ref_p;
	_base_pose_p->ref();
	_base_pose_p->move_keytimes( 0.0 ); // make sure motion starts at 0
	
	SkChannelArray& mchan_arr = _base_pose_p->channels();
	int size = mchan_arr.size();
	for( int i = 0; i < size; i++ )	{
		_channels.add( mchan_arr.name( i ), mchan_arr.type( i ) );
	}
	
	MeController::init();
}

void MeCtFace::add_key( char *weight_key, SkMotion* key_pose_p ) {
	
	
	key_pose_p->ref();
	key_pose_p->move_keytimes( 0.0 );
	_key_pose_map.insert( weight_key, key_pose_p );
	
	_channels.add( weight_key, SkChannel::XPos );
}

void MeCtFace::finish_adding( void )	{
	if( _context ) {
		_context->child_channels_updated( this );
	}
}

///////////////////////////////////////////////////////////////////////////////////

void MeCtFace::context_updated( void ) {

#if 0
	if( _context ) {
		_skeleton_ref_p = _context->channels().skeleton(); // WHY HERE?
		if( _skeleton_ref_p == NULL )	{
			printf( "MeCtFace::context_updated ERR: _skeleton_ref_p is NULL\n" );
		}
	}
	else	{
		printf( "MeCtFace::context_updated ERR: context is NULL\n" );
	}
#endif
}

void MeCtFace::controller_map_updated( void ) {

	if( _base_pose_p ) {
		// Map motion channel index to context float buffer index
		SkChannelArray& base_channels = _base_pose_p->channels();
		
		const int num_base_chans = base_channels.size();
		int num_key_chans = _key_pose_map.get_num_entries();

		_bChan_to_buff.size( num_base_chans );
		_kChan_to_buff.size( num_key_chans );

		if( _context ) {

			SkChannelArray& cChannels = _context->channels();
			for( int i=0; i<num_base_chans; ++i ) {
				int chan_index = cChannels.search( base_channels.name( i ), base_channels.type( i ) );
				// NOTE: check for -1 : report not found? NOT NECESSARILY
				_bChan_to_buff[ i ] = _context->toBufferIndex( chan_index );
			}
			
			// The following assumes that there will be no new additions to the hash-map
			int c = 0;
			char* key_pose_name;
			_key_pose_map.reset();
			while( _key_pose_map.next( & key_pose_name ) )	{
				int chan_index = cChannels.search( key_pose_name, SkChannel::XPos );
				// NOTE: check for -1 : report not found?
				_kChan_to_buff[ c++ ] = _context->toBufferIndex( chan_index );
			}

		} 
		else {
			_bChan_to_buff.setall( -1 );
			_kChan_to_buff.setall( -1 );
		}
	}
}

void MeCtFace::controller_start( void )	{

}

bool MeCtFace::controller_evaluate( double t, MeFrameData& frame ) {

	bool continuing = true;
	continuing = t < _duration;
	if( t < 0.0 )	{
		return( continuing );
	}

	float *fbuffer = &( frame.buffer()[0] );
	SkChannelArray& base_channels = _base_pose_p->channels();
	int nchan = base_channels.size();
	float * base_pose_buff_p = _base_pose_p->posture( 0 );

	int pose_var_index = 0;
	for( int i=0; i<nchan; i++ )	{
		
		int ch_size = base_channels[ i ].size();
		SkChannel::Type ch_type = base_channels[ i ].type;

		int base_ch_index = _bChan_to_buff[ i ];
		if( base_ch_index >= 0 )	{
			if( 
				( ch_type == SkChannel::XPos ) ||
				( ch_type == SkChannel::YPos ) ||
				( ch_type == SkChannel::ZPos )
			)	{
				fbuffer[ base_ch_index ] = base_pose_buff_p[ pose_var_index ];
			}
			else
			if( ch_type == SkChannel::Quat )	{
				fbuffer[ base_ch_index ] = base_pose_buff_p[ pose_var_index ];
				fbuffer[ base_ch_index + 1 ] = base_pose_buff_p[ pose_var_index + 1 ];
				fbuffer[ base_ch_index + 2 ] = base_pose_buff_p[ pose_var_index + 2 ];
				fbuffer[ base_ch_index + 3 ] = base_pose_buff_p[ pose_var_index + 3 ];
			}
		}
		pose_var_index += ch_size;
	}

	int c = 0;
	SkMotion* key_pose_p;
	_key_pose_map.reset();
	while( key_pose_p = _key_pose_map.next() )	{

		int weight_index = _kChan_to_buff[ c++ ];
		if( weight_index >= 0 )	{
		
			float key_weight = fbuffer[ weight_index ];
			if( fabs( key_weight ) > 0.0 )	{
				float* key_pose_buff_p = key_pose_p->posture( 0 );

				pose_var_index = 0;
				for( int i=0; i<nchan; i++ )	{

					int ch_size = base_channels[ i ].size();
					SkChannel::Type ch_type = base_channels[ i ].type;
					
					int base_ch_index = _bChan_to_buff[ i ];
					if( base_ch_index >= 0 )	{
						if( 
							( ch_type == SkChannel::XPos ) ||
							( ch_type == SkChannel::YPos ) ||
							( ch_type == SkChannel::ZPos )
						)	{
							float key_diff = key_pose_buff_p[ pose_var_index ] - base_pose_buff_p[ pose_var_index ];
							if( fabs( key_diff ) > 0.0 )	{
								fbuffer[ base_ch_index ] += key_diff * key_weight;
							}
						}
						else
						if( ch_type == SkChannel::Quat )	{

							quat_t base_q( 
								base_pose_buff_p[ pose_var_index ], 
								base_pose_buff_p[ pose_var_index + 1 ], 
								base_pose_buff_p[ pose_var_index + 2 ], 
								base_pose_buff_p[ pose_var_index + 3 ] 
							);
							quat_t key_q( 
								key_pose_buff_p[ pose_var_index ], 
								key_pose_buff_p[ pose_var_index + 1 ], 
								key_pose_buff_p[ pose_var_index + 2 ], 
								key_pose_buff_p[ pose_var_index + 3 ] 
							);
							quat_t key_diff_q = key_q * -base_q;
							if( key_diff_q.non_identity() ) {
								quat_t accum_q(
									fbuffer[ base_ch_index ],
									fbuffer[ base_ch_index + 1 ],
									fbuffer[ base_ch_index + 2 ],
									fbuffer[ base_ch_index + 3 ]
								);
								quat_t result_q = ( ( key_q * -base_q ) * key_weight ) * accum_q;
								fbuffer[ base_ch_index ] = (float)result_q.w();
								fbuffer[ base_ch_index + 1 ] = (float)result_q.x();
								fbuffer[ base_ch_index + 2 ] = (float)result_q.y();
								fbuffer[ base_ch_index + 3 ] = (float)result_q.z();
							}
						}
					}
					pose_var_index += ch_size;
				}
			}
		}
	}

	return continuing;
}

SkChannelArray& MeCtFace::controller_channels( void )	{
	return( _channels );
}

double MeCtFace::controller_duration( void ) {
	return( _duration );
}

const char* MeCtFace::controller_type( void )	{
	return( type_name );
}

void MeCtFace::print_state( int tabCount ) {

	fprintf( stdout, "MeCtFace" );

	const char* str = name();
	if( str )
		fprintf( stdout, " \"%s\"", str );

	fprintf( stdout, "\n" );
}

//////////////////////////////////////////////////////////////////////////////////

#if 0
void check_quat_morph( void )	{
	
	// test formula: 
	//	quat_t result_q = ( ( key_q * -base_q ) * key_weight ) * accum_q;

#if 0
	euler_t base( 30.0, 40.0, 10.0 );
	euler_t key( 35.0, 40.0, 10.0 );
#elif 0
	euler_t base( 30.0, 40.0, 10.0 );
	euler_t key( 30.0, 45.0, 10.0 );
#else
	euler_t base( 30.0, 40.0, 10.0 );
	euler_t key( 30.0, 40.0, 15.0 );
#endif

	euler_t dif = key * -base;
	dif.print();

	euler_t sum = dif * base;
	sum.print();

	euler_t sum1 = ( dif * 0.0 ) * base;
	sum1.print();

	euler_t sum2 = ( dif * 0.5 ) * base;
	sum2.print();

	euler_t sum3 = ( dif * 2.0 ) * base;
	sum3.print();
}
#endif
