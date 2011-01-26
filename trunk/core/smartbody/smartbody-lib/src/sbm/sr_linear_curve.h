/*
 *  sr_linear_curve.h - part of SmartBody-lib
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

#ifndef SR_LINEAR_CURVE_H
#define SR_LINEAR_CURVE_H

#include <stdio.h>
#include <vhcl_log.h>
#include "gwiz_math.h"

#define ENABLE_OBJ_KEY_CT	0

//////////////////////////////////////////////////////////////////

class srLinearCurve	{

	private:

#if ENABLE_OBJ_KEY_CT
	static int objective_key_count;
	static int objective_key_count_max;
#endif

	class Key	{
		
		public:
		
			Key( double p, double v );
			~Key(void) {
#if ENABLE_OBJ_KEY_CT
				objective_key_count--;
#endif
			}

			void print( int i );

			void next( Key *set_p ) { next_p = set_p; }
			Key *next( void ) { return( next_p ); }

			void update( void );
			void copy_delta( Key *key_p );

			double slope( void );
			double lerp( double t );

			double	param; // ANIMATION: time
			double	value; // template<> ?
		
		private:
			
			double	dp, inv_dp;
			double	dv;

			Key	*next_p;
	};

	public:
		static const double MAX_SLOPE;
		static const double MAX_VALUE;

		enum boundary_mode_enum_set	{
			CROP,			// do not write
			CLAMP, 			// write boundary value
			REPEAT, 		// loop curve, skip tail
			EXTRAPOLATE,	// extrapolate boundary slope, min/max_value if undefined
			NUM_BOUNDARY_MODES
		};

		srLinearCurve( int bound_mode = CLAMP )	{
			null();
			set_boundary_mode( bound_mode, bound_mode );
		}
		srLinearCurve( int head_mode, int tail_mode )	{
			null();
			set_boundary_mode( head_mode, tail_mode );
		}
		~srLinearCurve( void )	{
			clear();
		}

		void print( void );

		void set_boundary_mode( int head_mode, int tail_mode )	{
			head_bound_mode = head_mode;
			tail_bound_mode = tail_mode;
		}
		void set_output_clamp( double min, double max )	{
			min_value = min;
			max_value = max;
		}

		int get_num_keys( void ) { return( key_count ); }
		
		int insert( double p, double v )	{ /* sort by key.time, add after same time */
			return( insert_key( new Key( p, v ) ) );
		}
		
		void clear( void );
		void clear_after( double t );

	// Preferably these should be subsumed by 
	//	boundary conditions, crop { PRE, POST }, and simple param-range queries.

		double get_head_param( void );
		double get_head_value( void );
		double get_head_slope( void );
		
		double get_tail_param( void );
		double get_tail_value( void );
		double get_tail_slope( void );

		double get_next_nonzero_value( double after );
		double get_next_nonzero_slope( double after );
//		double get_last_nonzero_param( double after );

		double evaluate( double t, bool *cropped_p = NULL );

	protected:

		int insert_key( Key *key_p );
		void insert_head( Key *key_p );
		void insert_after( Key *prev_p, Key *key_p );
		void decrement( void );
		void increment( void );
		void update( void );
		
		Key* find_floor_key( double t );
		double head_boundary( double t, bool *cropped_p );
		double tail_boundary( double t, bool *cropped_p );

	private:
	
		void null( void )	{
			head_bound_mode = 0;
			tail_bound_mode = 0;
			min_value = -MAX_VALUE;
			max_value = MAX_VALUE;
			init();
		}
		void init( void )	{
			key_count = 0;
			dirty = false;
			head_p = NULL;
			curr_p = NULL;
			tail_p = NULL;
		}
		
		int 	head_bound_mode;
		int 	tail_bound_mode;
		
		double	min_value;
		double	max_value;

		int 	key_count;
		bool	dirty;

		Key		*head_p;
		Key		*curr_p;
		Key		*tail_p;
};

//////////////////////////////////////////////////////////////////

class srCurveBuilder	{

	private:
		int num_keys;
		double in_range_fr, in_range_to;
		double out_range_fr, out_range_to;
		
	public:

		void set_resolution( int num_segs ) {
			num_keys = num_segs + 1;
		}
		void set_input_range( double fr, double to )	{
			in_range_fr = fr;
			in_range_to = to;
		}
		void set_output_range( double fr, double to )	{
			out_range_fr = fr;
			out_range_to = to;
		}

		srLinearCurve *get_std_hump_curve( srLinearCurve *curve_p, int num_segs )	{
		
			set_resolution( num_segs );
			set_input_range( 0.0, 1.0 );
			set_output_range( -1.0, 1.0 );
			return( get_sin_curve( curve_p, 0.0, M_PI ) );
		}
		srLinearCurve *get_std_bell_curve( srLinearCurve *curve_p, int num_segs )	{
		
			set_resolution( num_segs );
			set_input_range( 0.0, 1.0 );
			set_output_range( 0.0, 1.0 );
			return( get_cos_curve( curve_p, -M_PI, M_PI ) );
		}

		srLinearCurve *get_sin_curve( srLinearCurve *curve_p, double alpha, double beta )	{
			if( curve_p )	{
				curve_p->clear();

				double in_span = in_range_to - in_range_fr;
				double out_span = out_range_to - out_range_fr;
				double trig_span = beta - alpha;
				double denom = 1.0 / (float)( num_keys - 1 );

				for( int i = 0; i<num_keys; i++ )	{

					double tn = (float)i * denom;
					double t = in_range_fr + tn * in_span;
					double th = alpha + tn * trig_span;
//					double s = sin( RAD( th ) );
					double s = sin( th );
					double sn = ( s + 1.0 ) * 0.5;
					double v = out_range_fr + sn * out_span;
					curve_p->insert( t, v );
/*
					double tn = (float)i * denom;
					curve_p->insert( 
						in_range_fr + tn * in_span,
						out_range_fr + ( sin( RAD( alpha + tn * trig_span ) ) + 1.0 ) * 0.5 * out_span
					);
*/
				}
			}
			return( curve_p );
		}
		srLinearCurve *get_cos_curve( srLinearCurve *curve_p, double alpha, double beta )	{
			
			if( curve_p )	{
				curve_p->clear();

				double in_span = in_range_to - in_range_fr;
				double out_span = out_range_to - out_range_fr;
				double trig_span = beta - alpha;
				double denom = 1.0 / (float)( num_keys - 1 );

				for( int i = 0; i<num_keys; i++ )	{

					double tn = (float)i * denom;
					double t = in_range_fr + tn * in_span;
					double th = alpha + tn * trig_span;
//					double s = cos( RAD( th ) );
					double s = cos( th );
					double sn = ( s + 1.0 ) * 0.5;
					double v = out_range_fr + sn * out_span;
					curve_p->insert( t, v );
/*
					double tn = (float)i * denom;
					curve_p->insert( 
						in_range_fr + tn * in_span,
						out_range_fr + ( cos( RAD( alpha + tn * trig_span ) ) + 1.0 ) * 0.5 * out_span
					);
*/
				}
			}
			return( curve_p );
		}

		srLinearCurve *new_std_hump_curve( int num_segs )	{
			return( get_std_hump_curve( new srLinearCurve, num_segs ) );
		}
		srLinearCurve *new_std_bell_curve( int num_segs )	{
			return( get_std_bell_curve( new srLinearCurve, num_segs ) );
		}
		srLinearCurve *new_sin_curve( double alpha, double beta )	{
			return( get_sin_curve( new srLinearCurve, alpha, beta ) );
		}
		srLinearCurve *new_cos_curve( double alpha, double beta )	{
			return( get_cos_curve( new srLinearCurve, alpha, beta ) );
		}

};

//////////////////////////////////////////////////////////////////
#endif

#if 1
void test_linear_curve( void );
#endif
