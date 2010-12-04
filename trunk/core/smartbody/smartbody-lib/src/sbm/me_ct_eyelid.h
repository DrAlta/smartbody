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

	class LidSet	{
	
		public:
		
			LidSet( void ) {
				dirty_bit = true;
				base_angle = 0.0f;
				full_angle = 0.0f;
				blink_angle = 0.0f;
				diff = 0.0f;
				inv_diff = 0.0f;
				lid_tight = 0.0f;
				open_angle = 0.0f;
				tight_sweep = 0.0f;
				close_sweep = 0.0f;
				eye_pitch = 0.0f;
			}
			~LidSet( void ) {}

			void print( void );

			// specify normalized range in degrees
			void set_range( float fr, float to );
			
			// specify blink angle in degrees
			void set_blink( float angle );
			
			// modifier: 0.0: wide; 0.5: neutral; 0.9: squinting
			void set_tighten( float tighten ); 

			// specify eyeball pitch in degrees
			void set_pitch( float pitch );

			// convert normalized weight (0..1) to output weight with tigthener/eye-pitch
			float get_mapped_weight( float in_weight );

		private:
			bool  dirty_bit;
			
			float base_angle;
			float full_angle;

			float blink_angle;

			float diff;
			float inv_diff;

			float lid_tight;
			float open_angle;

			float tight_sweep;
			float close_sweep;
			
			float eye_pitch;
	};

	public:
	
		void test( void );

		static const char* type_name;

		MeCtEyeLidRegulator( void );
		~MeCtEyeLidRegulator( void );
		
		void init( bool tracking_pitch = false );

		void set_upper_range( float fr, float to )	{
			UL_set.set_range( fr, to );
			UR_set.set_range( fr, to );
		}
		
		void set_lower_range( float fr, float to )	{
			LL_set.set_range( fr, to );
			LR_set.set_range( fr, to );
		}

		void set_blink_angle( float angle )	{
			LL_set.set_blink( angle );
			LR_set.set_blink( angle );
			UL_set.set_blink( angle );
			UR_set.set_blink( angle );
		}

		void set_upper_tighten( float tight )	{
			UL_set.set_tighten( tight );
			UR_set.set_tighten( tight );
		}

		void set_lower_tighten( float tight )	{
			LL_set.set_tighten( tight );
			LR_set.set_tighten( tight );
		}

		void blink_now( void ) { new_blink = true; }

		float get_upper_left( bool *changed_p = NULL ) { 
			if( changed_p ) {
				*changed_p = ( UL_value != prev_UL_value );
			}
			return( UL_value ); 
		}
		
		float get_upper_right( bool *changed_p = NULL ) { 
			if( changed_p ) {
				*changed_p = ( UR_value != prev_UR_value );
			}
			return( UR_value ); 
		}

	private:
		LidSet	UL_set;
		LidSet	LL_set;
		LidSet	UR_set;
		LidSet	LR_set;
		
		srLinearCurve	curve;
		
		bool	pitch_tracking;

		float	hard_upper_tighten;
		float	hard_lower_tighten;

		bool	new_blink;
		double	blink_period_min;
		double	blink_period_max;
		double	blink_period;
		double	prev_blink; // time at last blink

		float	prev_UL_value;
		float	prev_LL_value;
		float	prev_UR_value;
		float	prev_LR_value;
		
		float	UL_value;
		float	LL_value;
		float	UR_value;
		float	LR_value;
		
		SkChannelArray		_channels;

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
