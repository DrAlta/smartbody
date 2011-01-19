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

		enum boundary_mode_enum_set	{
			BOUNDARY_CROP,				// do not write
			BOUNDARY_CLAMP, 			// write boundary value
			BOUNDARY_REPEAT, 			// loop curve
			BOUNDARY_EXTRAPOLATE,		// extrapolate boundary slope
			CROP = BOUNDARY_CROP,
			CLAMP = BOUNDARY_CLAMP,
			REPEAT = BOUNDARY_REPEAT,
			EXTRAPOLATE = BOUNDARY_EXTRAPOLATE
		};

		MeCtCurveWriter( 
			int left_bound = BOUNDARY_CROP, 
			int right_bound = BOUNDARY_CROP, 
			bool at_least_once = true 
		)	{
			curve_arr = NULL;
			num_curves = 0;
			left_bound_mode = left_bound;
			right_bound_mode = right_bound;
			write_once = at_least_once;
			write_once_arr = NULL;
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
	
		void init( SkChannelArray& channels );
		
		void insert_key( int curve_index, double t, double v )	{
			if( curve_index < 0 ) return;
			if( curve_index >= num_curves ) return;
			curve_arr[ curve_index ].insert( t, v );
		}

	private:
	
		SkChannelArray  _channels;
		SrBuffer<float> _data;
		SrBuffer<int>   _local_ch_to_buffer;
		
		srLinearCurve	*curve_arr;
		int 	num_curves;
		
		int 	left_bound_mode;
		int 	right_bound_mode;
		
		bool	write_once;
		bool	*write_once_arr;
		
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
