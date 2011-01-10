/*
 *  sr_synch_points.h - part of SmartBody-lib
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

#ifndef SR_SYNCH_POINTS_H
#define SR_SYNCH_POINTS_H

#include <stdio.h>
#include <vhcl_log.h>
#include <sbm/sr_linear_curve.h>

class srSynchPoints	{

	public:
	
		enum synch_point_enum_set	{
			START,
			READY,
			STROKE_START,
			STROKE,
			STROKE_STOP,
			RELAX,
			STOP,
			NUM_SYNCH_TAGS
		};

		srSynchPoints( void )	{
			init();
		}
		srSynchPoints( double start, double stop )	{
			set_synch( start, stop );
		}
		srSynchPoints( double start, double stop, double ramp )	{
			set_synch( start, stop, ramp );
		}
		srSynchPoints( double start, double ready, double relax, double stop )	{
			set_synch( start, ready, relax, stop );
		}
		srSynchPoints(
			double start, double ready, double stroke, double relax, double stop )	{
			set_synch( start, ready, stroke, relax, stop );
		}
		srSynchPoints(
			double start, double ready, 
			double st_start, double stroke, double st_stop, 
			double relax, double stop 
		)	{
			set_synch( start, ready, st_start, stroke, st_stop, relax, stop );
		}
		~srSynchPoints( void ) {}

		void init( void )	{
			for( int i=0; i<NUM_SYNCH_TAGS; i++ )	{
				synch_arr[ i ] = -1.0;
			}
		}

		bool valid_tag( int tag )	{

			if( tag < 0 ) return( false );
			if( tag < NUM_SYNCH_TAGS ) return( true );
			return( false );
		}

		bool valid_time( int tag ) {

			if( valid_tag( tag ) )	{
				if( synch_arr[ tag ] < 0.0 )	{
					return( false );
				}
				return( true );
			}
			return( false );
		}

		bool set_synch( int tag, double t )	{

			if( valid_tag( tag ) )	{
			
				for( int i = 0; i < tag; i++ )	{

					if( valid_time( i ) )	{
						if( t < get_rel_time( i ) ) {
//							return( false );
							set_synch( i, t );
						}
					}
				}
			
				synch_arr[ tag ] = t;

				for( int i = tag + 1; i < NUM_SYNCH_TAGS; i++ )	{

					if( valid_time( i ) )	{
						if( t > get_rel_time( i ) ) {
							set_synch( i, t );
						}
					}
				}
			
				return( true );
			}
			return( false );
		}

		void set_synch( double start, double stop ) {
			init();
			set_synch( START, start );
			set_synch( STOP, stop );
		}

		void set_synch( double start, double stop, double ramp ) {
			init();
			set_synch( START, start );
			set_synch( READY, start + ramp );
			set_synch( RELAX, stop - ramp );
			set_synch( STOP, stop );
		}

		void set_synch( double start, double ready, double relax, double stop ) {
			init();
			set_synch( START, start );
			set_synch( READY, ready );
			set_synch( RELAX, relax );
			set_synch( STOP, stop );
		}

		void set_synch( double start, double ready, double stroke, double relax, double stop ) {
			init();
			set_synch( START, start );
			set_synch( READY, ready );
			set_synch( STROKE, stroke );
			set_synch( RELAX, relax );
			set_synch( STOP, stop );
		}

		void set_synch( 
			double start, double ready, 
			double st_start, double stroke, double st_stop, 
			double relax, double stop 
		) {
			init();
			set_synch( START, start );
			set_synch( READY, ready );
			set_synch( STROKE_START, st_start );
			set_synch( STROKE, stroke );
			set_synch( STROKE_STOP, st_stop );
			set_synch( RELAX, relax );
			set_synch( STOP, stop );
		}

		double get_rel_time( int tag )	{

			if( valid_tag( tag ) )	{
				double t = synch_arr[ tag ];
				if( t < 0.0 )	{
					return( -1.0 );
				}
				return( t );
			}
			return( -1.0 );
		}

		double get_abs_time( int tag, double from ) {

			double t = get_rel_time( tag );
			if( t < 0.0 )	{
				return( -1.0 );
			}
			return( from + t );
		}

		double get_interval( int fr, int to )   {

			double f = get_rel_time( fr );
			if( f < 0.0 )	{
				return( -1.0 );
			}
			double t = get_rel_time( to );
			if( t < 0.0 )	{
				return( -1.0 );
			}
			return( abs( to - fr ) );
		}

		double get_duration( void ) {
			return( get_interval( START, STOP ) );
		}

		srLinearCurve *new_trapezoid( void ) {
			
			srLinearCurve *curve_p = new srLinearCurve();
			
			double t0 = get_rel_time( START );
			double t1 = get_rel_time( READY );
			double t2 = get_rel_time( RELAX );
			double t3 = get_rel_time( STOP );
			
			if( t0 < 0.0 ) t0 = 0.0;
			if( t1 < 0.0 ) t1 = 0.0;
			if( t2 < 0.0 ) t2 = 0.0;
			if( t3 < 0.0 ) t3 = 0.0;
			
			curve_p->insert( t0, 0.0 );
			curve_p->insert( t1, 1.0 );
			curve_p->insert( t2, 1.0 );
			curve_p->insert( t3, 0.0 );
			
			return( curve_p );
		}

	private:
		
		double synch_arr[ NUM_SYNCH_TAGS ];
};
#endif
