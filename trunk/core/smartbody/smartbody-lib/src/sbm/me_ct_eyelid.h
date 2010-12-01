/*
 *  me_ct_eyelid.h - part of SmartBody-lib
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

#ifndef ME_CT_EYELID_H
#define ME_CT_EYELID_H

#include <SK/sk_skeleton.h>
#include <ME/me_controller.h>
#include "sr_linear_curve.h"

//////////////////////////////////////////////////////////////////////////////////

class MeCtEyeLidRegulator : public MeController	{ 

	private:

		SkChannelArray		_channels;

	public:

		static const char* type_name;

		MeCtEyeLidRegulator( void );
		~MeCtEyeLidRegulator( void );
		
		void init( void );

		void blink_now( void ) { new_blink = true; }

		float get_left( bool *changed_p = NULL ) { 
			if( changed_p ) {
				*changed_p = ( left_value != prev_left_value );
			}
			return( left_value ); 
		}
		
		float get_right( bool *changed_p = NULL ) { 
			if( changed_p ) {
				*changed_p = ( right_value != prev_right_value );
			}
			return( right_value ); 
		}

	private:
		srLinearCurve	curve;
		
		bool	new_blink;
		
		double	blink_period_min;
		double	blink_period_max;
		double	blink_period;
		double	prev_blink; // time at last blink

		float	prev_left_value;
		float	prev_right_value;
		float	left_value;
		float	right_value;
		
		virtual void context_updated( void );
		virtual void controller_map_updated();
		virtual void controller_start();
		virtual bool controller_evaluate ( double t, MeFrameData& frame );
		virtual SkChannelArray& controller_channels ();
		virtual double controller_duration ();
		virtual const char* controller_type () const;
		virtual void print_state( int tabCount );
};

//////////////////////////////////////////////////////////////////////////////////

/*
	char <> softeyes [on|off]
	char <> softeyes weight <upper> <lower>
	char <> softeyes eyepitch|upperlid|lowerlid <upper> <lower>
*/

//////////////////////////////////////////////////////////////////////////////////

class MeCtEyeLid : public MeController	{ 

	private:

		SkChannelArray		_channels; // override motion channels, to include world_offset
		
		float	precision;
		float	lid_weight[ 2 ];
		float	upper_lid_range[ 2 ];
		float	lower_lid_range[ 2 ];
		float	eye_pitch_range[ 2 ];
		
		float min( float a, float b )	{
			if( a < b ) return( a );
			return( b );
		}
		float max( float a, float b )	{
			if( a > b ) return( a );
			return( b );
		}

	public:
		static const char* type_name;

		/*! Constructor */
		MeCtEyeLid( void );

		/*! Destructor is public but pay attention to the use of ref()/unref() */
		virtual ~MeCtEyeLid( void );
		void init( void );
		
		void set_weight( float lo, float up )	{
			lid_weight[ 0 ] = lo;
			lid_weight[ 1 ] = up;
		}
		void get_weight( float &lo, float &up )	{
			lo = lid_weight[ 0 ];
			up = lid_weight[ 1 ];
		}
		
		void set_lower_lid_range( float a, float b ) {
			lower_lid_range[ 0 ] = min( a, b );
			lower_lid_range[ 1 ] = max( a, b );
		}
		void get_lower_lid_range( float &lo, float &up ) {
			lo = lower_lid_range[ 0 ];
			up = lower_lid_range[ 1 ];
		}
		
		void set_upper_lid_range( float a, float b ) {
			upper_lid_range[ 0 ] = min( a, b );
			upper_lid_range[ 1 ] = max( a, b );
		}
		void get_upper_lid_range( float &lo, float &up ) {
			lo = upper_lid_range[ 0 ];
			up = upper_lid_range[ 1 ];
		}
		
		void set_eye_pitch_range( float a, float b )	{
			eye_pitch_range[ 0 ] = max( a, b ); // down is positive...
			eye_pitch_range[ 1 ] = min( a, b );
		}
		void get_eye_pitch_range( float &lo, float &up )	{
			lo = eye_pitch_range[ 0 ];
			up = eye_pitch_range[ 1 ];
		}
		
	private:
		float	calc_lid_correction( 
			float in_eye_p, 
			float eye_range[ 2 ], 
			float in_lid_y,
			float lid_range[ 2 ]
		);
			
		virtual void context_updated( void );
		virtual void controller_map_updated();
		virtual void controller_start();
		virtual bool controller_evaluate ( double t, MeFrameData& frame );
		virtual SkChannelArray& controller_channels ();
		virtual double controller_duration ();
		virtual const char* controller_type () const;
		virtual void print_state( int tabCount );
};

//////////////////////////////////////////////////////////////////////////////////
#endif // ME_CT_EYELID_H
