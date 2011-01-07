/*
 *  me_ct_curve_writer.cpp - part of SmartBody-lib's Motion Engine
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
 *      
 */

#include <vhcl.h>
#include <me/me_ct_curve_writer.hpp>

const char* MeCtCurveWriter::TYPE = "MeCtCurveWriter";

///////////////////////////////////////////////////////////////////////////////

void MeCtCurveWriter::init( SkChannelArray& channels )	{
	
	_channels.init();
	_channels.merge( channels );
	_channels.compress();
	_channels.rebuild_hash_table();
	
	num_curves = _channels.floats();
	curve_arr = new srLinearCurve[ num_curves ];
	write_once_arr = new bool[ num_curves ];
	for( int i=0; i<num_curves; i++ )	{
		write_once_arr[ i ] = write_once;
	}
	
	const int size = _channels.size();
	_local_ch_to_buffer.size( size );	// ends up being a 1 to 1 mapping...
	int index = 0;
	for( int i=0; i<size; ++ i ) {
		_local_ch_to_buffer[i] = index;
		int chan_size = SkChannel::size( _channels.type( i ) );
		if( chan_size > 1 ) {
			// ERROR !!!
		}
		index += chan_size;
	}

	MeController::init ();
}

bool MeCtCurveWriter::controller_evaluate( double time, MeFrameData& frame )	{

	if( time < 0.0 )	{
		return( true );
	}
	if( num_curves < 1 )	{
		return( true );
	}
	
	SkChannelArray& channels = controller_channels();
	SrBuffer<float>& frame_buffer = frame.buffer();

	for( int i=0; i<num_curves; i++ )	{

		int context_ch = _toContextCh[i];
		if( context_ch != -1 ) {

			int buff_index = _local_ch_to_buffer[i];
			int frame_buffer_index = frame.toBufferIndex( context_ch );

			int num_keys = curve_arr[ i ].get_num_keys();
			if( num_keys > 0 ) {

				double head_time = curve_arr[ i ].get_head_param();
				double tail_time = curve_arr[ i ].get_tail_param();

				if( time < head_time )	 {

					switch( left_bound_mode )	{

						case BOUNDARY_CROP: // no write
							break;

						case BOUNDARY_CLAMP:
							frame_buffer[ frame_buffer_index ] = (float)curve_arr[ buff_index ].evaluate( time );
							break;

						case BOUNDARY_EXTRAPOLATE:
							{
								double head_value = curve_arr[ buff_index ].get_head_value();
								double slope = curve_arr[ buff_index ].get_head_slope();
								double diff = head_time - time;
								double extrap_value = head_value - slope * diff;
								frame_buffer[ frame_buffer_index ] = (float)extrap_value;
							}
							break;

						case BOUNDARY_REPEAT:
							{
								double diff = head_time - time;
								double dur = tail_time - head_time;
								double rep_time = head_time + fmod( dur, diff );
								frame_buffer[ frame_buffer_index ] = (float)curve_arr[ buff_index ].evaluate( rep_time );
							}
							break;
					}
				}
				else
				if( time > tail_time )	{

					switch( right_bound_mode )	{

						case BOUNDARY_CROP:
							if( write_once_arr[ i ] )	{
								frame_buffer[ frame_buffer_index ] = (float)curve_arr[ buff_index ].evaluate( time );
								write_once_arr[ i ] = false;
							}
							break;

						case BOUNDARY_CLAMP:
							frame_buffer[ frame_buffer_index ] = (float)curve_arr[ buff_index ].evaluate( time );
							break;

						case BOUNDARY_EXTRAPOLATE:
							{
								double tail_value = curve_arr[ buff_index ].get_tail_value();
								double slope = curve_arr[ buff_index ].get_tail_slope();
								double diff = time - tail_time;
								double extrap_value = tail_value + slope * diff;
								frame_buffer[ frame_buffer_index ] = (float)extrap_value;
							}
							break;

						case BOUNDARY_REPEAT:
							{
								double diff = time - tail_time;
								double dur = tail_time - head_time;
								double rep_time = tail_time + fmod( dur, diff );
								frame_buffer[ frame_buffer_index ] = (float)curve_arr[ buff_index ].evaluate( rep_time );
							}
							break;
					}
				}
				else	{ // within bounds
					frame_buffer[ frame_buffer_index ] = (float)curve_arr[ buff_index ].evaluate( time );
					write_once_arr[ i ] = false;
				}
			}
		}
	}
	return( true );
}

double MeCtCurveWriter::controller_duration()	{

	if( right_bound_mode == BOUNDARY_CROP ) {
		double max_dur = 0.0;
		for( int i=0; i<num_curves; i++ )	{
			double dur = curve_arr[ i ].get_tail_param();
			if( dur > max_dur ) {
				max_dur = dur;
			}
		}
		return( max_dur );
	}
	return( -1.0 );
}

///////////////////////////////////////////////////////////////////////////////

void MeCtCurveWriter::print_state( int tab_count )	{
#if 0
	using namespace std;
	string indent( tab_count, '\t' );

	const char* name = this->name();
	SkChannelArray& channels = controller_channels();

	std::stringstream strstr;
	strstr << controller_type();
	if( name!=NULL && name[0]!='\0' )
		strstr << " \"" << name << "\"";
	strstr << " @0x" << this;
	LOG("%s", strstr.str().c_str());
#endif
}
