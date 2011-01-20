/*
 *  me_ct_curve_writer.hpp - part of SmartBody-lib's Motion Engine
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
 *     Marcus Thiebaux
 */

#ifndef ME_CT_CURVE_WRITER_HPP
#define ME_CT_CURVE_WRITER_HPP

#include <ME/me_controller.h>
#include <sbm/sr_linear_curve.h>

class MeCtCurveWriter : public MeController {

	public:

		static const char* TYPE;

		MeCtCurveWriter( void )	{
			curve_arr = NULL;
			num_curves = 0;
			write_once_arr = NULL;
			tail_bound_mode = 0;
		}
		~MeCtCurveWriter( void )	{
			if( curve_arr ) {
				delete [] curve_arr;
				curve_arr = NULL;
			}
			if( write_once_arr ) {
				delete [] write_once_arr;
				write_once_arr = NULL;
			}
		}
	
		void init( 
			SkChannelArray& channels,
			int left_bound = srLinearCurve::CROP, 
			int right_bound = srLinearCurve::CROP, 
			bool at_least_once = true 
		);
		
		void insert_key( int curve_index, double t, double v )	{
			if( curve_index < 0 ) return;
			if( curve_index >= num_curves ) return;
			curve_arr[ curve_index ].insert( t, v );
		}

		void insert_key( double t, double v )	{
			curve_arr[ 0 ].insert( t, v );
		}

	private:
	
		SkChannelArray  _channels;
		SrBuffer<float> _data;
		SrBuffer<int>   _local_ch_to_buffer;
		
		int 	num_curves;
		srLinearCurve	*curve_arr;
		bool	*write_once_arr;
		int 	tail_bound_mode;
		
	public:
		virtual const char* controller_type() const {
			return( MeCtCurveWriter::TYPE );
		}
	
		virtual SkChannelArray& controller_channels()	{
			return( _channels );
		}
		
		virtual double controller_duration();
		virtual bool controller_evaluate( double time, MeFrameData& frame );

		virtual void print_state( int tab_count );
};

#endif // ME_CT_CURVE_WRITER_HPP
