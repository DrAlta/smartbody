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

/*
	srSynchPoints:

	ASSUMPTIONS:
		Time is a 'double' precision float.
		Any negative time values are interpreted as undefined.
		This code uses -1.0, but all negative inputs are equivalent.
		A negative tag value, or -1, is interpreted as uninitialized.
		Undefined and uninitialied values are not consdiered errors.
		If tag >= NUM_SYNCH_TAGS, this is considered an error.
*/

//////////////////////////////////////////////////////////////////

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
		enum error_enum_set	{
			NO_ERROR_DETECTED,
			BAD_TAG,
			BUMPED_BACK_TAG,
			BUMPED_FWD_TAG,
			NEGATIVE_RAMP,
			UNDEFINED_INTERVAL,
			OVERLAPPED_RAMPS,
			NUM_ERROR_CODES
		};

		int tag_index( const char *label )	{
			if( label )	{
				if( _stricmp( label, "start" ) == 0 )			return( START );
				if( _stricmp( label, "ready" ) == 0 )			return( READY );
				if( _stricmp( label, "stroke_start" ) == 0 )	return( STROKE_START );
				if( _stricmp( label, "stroke" ) == 0 )			return( STROKE );
				if( _stricmp( label, "stroke_stop" ) == 0 ) 	return( STROKE_STOP );
				if( _stricmp( label, "relax" ) == 0 )			return( RELAX );
				if( _stricmp( label, "stop" ) == 0 )			return( STOP );
			}
			return( -1 ); // default err
		}

		const char * tag_label( const int index ) {
			switch( index )	{
				case START: 		return( "start" );
				case READY: 		return( "ready" );
				case STROKE_START:	return( "stroke_start" );
				case STROKE:		return( "stroke" );
				case STROKE_STOP:	return( "stroke_stop" );
				case RELAX: 		return( "relax" );
				case STOP:			return( "stop" );
			}
			return( "UNKNOWN" ); // default err
		}

	// CPP cruft:
		srSynchPoints( void )	{
			init();
		}
		srSynchPoints( double start, double stop )	{
			set_time( start, stop );
		}
		srSynchPoints( double start, double stop, double ramp )	{
			set_time( start, stop, ramp );
		}
		srSynchPoints( double start, double ready, double relax, double stop )	{
			set_time( start, ready, relax, stop );
		}
		srSynchPoints(
			double start, double ready, double stroke, double relax, double stop )	{
			set_time( start, ready, stroke, relax, stop );
		}
		srSynchPoints(
			double start, double ready, 
			double st_start, double stroke, double st_stop, 
			double relax, double stop 
		)	{
			set_time( start, ready, st_start, stroke, st_stop, relax, stop );
		}
		~srSynchPoints( void ) {}

	protected:

	// internal maintenance:

		void init( void )	{
			for( int i=0; i<NUM_SYNCH_TAGS; i++ )	{
				synch_time_arr[ i ] = -1.0;
			}
			global_offset = 0.0;
			implied_ramp_out = 0.0;
			default_ramp_out = -1.0;
			duration_defined = false;
			err_code = NO_ERROR_DETECTED;
		}

		bool valid_tag( int tag )	{

			if( tag < 0 ) return( false );
			if( tag < NUM_SYNCH_TAGS ) return( true );
			err_code = BAD_TAG;
			return( false );
		}
		bool valid_time( int tag ) {

			if( valid_tag( tag ) )	{
				if( synch_time_arr[ tag ] < 0.0 )	{
					return( false );
				}
				return( true );
			}
			return( false );
		}

		bool set_time( int tag, double t )	{

			if( valid_tag( tag ) )	{
			
				synch_time_arr[ tag ] = t;
				if( t < 0.0 )	{
					return( true );
				}
			// check and bump back order of preceding:
				for( int i = 0; i < tag; i++ )	{

					if( valid_time( i ) )	{
						if( get_time( i ) > t ) {
							set_time( i, t );
							err_code = BUMPED_BACK_TAG;
						}
					}
				}
			// check and bump forward order of succeeding:
				for( int i = tag + 1; i < NUM_SYNCH_TAGS; i++ )	{

					if( valid_time( i ) )	{
						if( get_time( i ) < t ) {
							set_time( i, t );
							err_code = BUMPED_FWD_TAG;
						}
					}
				}
				return( true );
			}
			return( false );
		}

		bool set_interval( int tag, double t )	{

			if( valid_tag( tag ) )	{
				for( int i = tag-1; i>=0; i-- )	{

					if( valid_time( i ) )	{
						return( set_time( tag, synch_time_arr[ i ] + t ) ); // force bump
					}
				}
				err_code = UNDEFINED_INTERVAL;
				return( false );
			}
			return( false );
		}

	public:

		void set_global_offset( double offset )	{
			global_offset = offset;
		}
		double get_global_offset( void )	{
			return( global_offset );
		}
		
	// set by explicit time:

		void set_time( double start, double stop = -1.0 ) {
			init();
			set_time( START, start );
			set_time( STOP, stop );
		}
		void set_time( double start, double stop, double ramp ) {
			init();
			set_time( START, start );
			set_time( READY, start + ramp );
			if( stop >= 0.0 )	{
				set_time( RELAX, stop - ramp );
				set_time( STOP, stop );
			}
			implied_ramp_out = ramp;
		}
		void set_time( double start, double ready, double relax, double stop ) {
			init();
			set_time( START, start );
			set_time( READY, ready );
			set_time( RELAX, relax );
			set_time( STOP, stop );
			if( ( relax >= 0.0 )&&( stop >= 0.0 ) ) {
				implied_ramp_out = stop - relax;
			}
		}
		void set_time( double start, double ready, double stroke, double relax, double stop ) {
			init();
			set_time( START, start );
			set_time( READY, ready );
			set_time( STROKE, stroke );
			set_time( RELAX, relax );
			set_time( STOP, stop );
			if( ( relax >= 0.0 )&&( stop >= 0.0 ) ) {
				implied_ramp_out = stop - relax;
			}
		}
		void set_time( 
			double start, double ready, 
			double st_start, double stroke, double st_stop, 
			double relax, double stop 
		) {
			init();
			set_time( START, start );
			set_time( READY, ready );
			set_time( STROKE_START, st_start );
			set_time( STROKE, stroke );
			set_time( STROKE_STOP, st_stop );
			set_time( RELAX, relax );
			set_time( STOP, stop );
			if( ( relax >= 0.0 )&&( stop >= 0.0 ) ) {
				implied_ramp_out = stop - relax;
			}
		}

	// set by START time and intervals:

		void set_interval( double start_at, double start_to_stop )	{
			set_time( START, start_at );
			set_interval( STOP, start_to_stop );
		}
		void set_interval( double start_at, double start_to_stop, double ramp )	{
			if( ramp < 0.0 )	{
				ramp = 0.0;
				err_code = NEGATIVE_RAMP;
			}
			set_time( START, start_at );
			set_interval( READY, ramp );
			set_interval( RELAX, start_to_stop - 2 * ramp );
			set_interval( STOP, ramp );
			implied_ramp_out = ramp;
		}
		void set_interval( double start_at, double start_to_ready, double ready_to_relax, double relax_to_stop )	{
			set_time( START, start_at );
			set_interval( READY, start_to_ready );
			set_interval( RELAX, ready_to_relax );
			set_interval( STOP, relax_to_stop );
			if( relax_to_stop > 0.0 )	{
				implied_ramp_out = relax_to_stop;
			}
		}
		void set_interval( 
			double start_at, 
			double start_to_ready, 
			double ready_to_stroke, 
			double stroke_to_relax, 
			double relax_to_stop
		)	{
			set_time( START, start_at );
			set_interval( READY, start_to_ready );
			set_interval( STROKE, ready_to_stroke );
			set_interval( RELAX, stroke_to_relax );
			set_interval( STOP, relax_to_stop );
			if( relax_to_stop > 0.0 )	{
				implied_ramp_out = relax_to_stop;
			}
		}
		void set_interval( 
			double start_at, 
			double start_to_ready, 
			double ready_to_S_start, // S: stroke_
			double S_start_to_S, 
			double S_to_S_stop, 
#if 0
			int repetitions,
			double S_stop_return_to_S_start,
#endif
			double S_stop_to_relax, 
			double relax_to_stop
		)	{
			set_time( START, start_at );
			set_interval( READY, start_to_ready );
			set_interval( STROKE_START, ready_to_S_start );
			set_interval( STROKE, S_start_to_S );
#if 0
			if( repetitions > 1 ) set_interval( STROKE_STOP, S_stop_return_to_start ); // what a mess
			double rep_offset = repetitions * ( S_start_to_S + S_to_S_stop );
			double rep_offset = ( 0.0 ); // definition of rep?
			set_interval( RELAX, rep_offset + S_stop_to_relax ); // what if reps == 0?
#else
			set_interval( STROKE_STOP, S_to_S_stop );
			set_interval( RELAX, S_stop_to_relax );
#endif
			set_interval( STOP, relax_to_stop );
			if( relax_to_stop > 0.0 )	{
				implied_ramp_out = relax_to_stop;
			}
		}

	// termination options:
		void set_stop_interval( double ramp )	{
			default_ramp_out = ramp;
		}
		void set_stop( double at_time, double ramp_out ) {
			set_time( RELAX, at_time );
			set_time( STOP, at_time + ramp_out );
		}
		void set_stop( double at_time ) {
			double ramp = default_ramp_out;
			if( ramp < 0.0 )	{
				ramp = implied_ramp_out;
			}
			set_stop( at_time, ramp );
		}

	// queries:
		double get_time( int tag, bool global = false )	{

			if( valid_tag( tag ) )	{
				double t = synch_time_arr[ tag ];
				if( t < 0.0 )	{
					return( -1.0 );
				}
				if( global )	{
					return( global_offset + t );
				}
				return( t );
			}
			return( -1.0 );
		}
		double get_interval( int fr_tag, int to_tag )   {

			double f = get_time( fr_tag );
			if( f < 0.0 )	{
				return( -1.0 );
			}
			double t = get_time( to_tag );
			if( t < 0.0 )	{
				return( -1.0 );
			}
			return( abs( t - f ) );
		}
		double get_length( void ) {
			return( get_interval( START, STOP ) );
		}
		double get_duration( void ) {
			return( get_time( STOP ) );
		}
		double get_next( int *tag_p, bool global = false )	{

			if( tag_p == NULL )	{
				return( -1.0 );
			}
			int tag = *tag_p;
			if( tag < 0 )	{
				tag = START;
			}
			else	{
				tag++;
			}
			double t = -1.0;
			bool done = false;
			while( !done )	{
				if( tag >= NUM_SYNCH_TAGS )	{
					done = true;
					tag = -1;
				}
				else	{
					t = get_time( tag, global );
					if( t >= 0.0 )	{
						done = true;
					}
					else	{
						tag++;
					}
				}
			}
			*tag_p = tag;
			return( t );
		}

		srLinearCurve *get_trapezoid( srLinearCurve *curve_p, double dfl_dur, double dfl_in, double dfl_out ) {
			
		// extract ordered data, check and apply defaults:

			double t0 = get_time( START );
			if( t0 < 0.0 )	{
				t0 = 0.0;
				err_code = UNDEFINED_INTERVAL;
			}
			double t3 = get_time( STOP );
			duration_defined = true;
			if( t3 < 0.0 )	{
				if( dfl_dur < 0.0 ) {
					duration_defined = false;
				}
				else	{
					t3 = t0 + dfl_dur;
				}
			}

			double t1 = get_time( READY );
			if( t1 < 0.0 )	{
				if( dfl_in < 0.0 )	{
					dfl_in = 0.0;
				}
				t1 = t0 + dfl_in;
			}
			double t2 = get_time( RELAX );
			if( t2 < 0.0 )	{
				if( duration_defined )	{
					if( dfl_out < 0.0 )	{
						dfl_out = 0.0;
					}
					t2 = t3 - dfl_out;
				}
			}

		// adjust ramps as needed:
			if( duration_defined )	{
				double len = t3 - t0;
				double ramp_in = t1 - t0;
				double ramp_out = t3 - t2;
				if( ( ramp_in + ramp_out ) > len ) {
					double ratio = ramp_in / ( ramp_in + ramp_out );
					ramp_in = ratio * len;
					ramp_out = ( 1.0 - ratio ) * len;
					t1 = t0 + ramp_in;
					t2 = t3 - ramp_out;
					err_code = OVERLAPPED_RAMPS;
				}
			}

			curve_p->clear();
			curve_p->insert( t0, 0.0 );
			curve_p->insert( t1, 1.0 );
			if( duration_defined )	{
				curve_p->insert( t2, 1.0 );
				curve_p->insert( t3, 0.0 );
			}
			return( curve_p );
		}
		srLinearCurve *get_trapezoid( srLinearCurve *curve_p, double dfl_dur = 1.0, double dfl_ramp = 0.0 ) {
			return( get_trapezoid( curve_p, dfl_dur, dfl_ramp, dfl_ramp ) );
		}
		srLinearCurve *new_trapezoid( double dfl_dur, double dfl_in, double dfl_out ) {
			return( get_trapezoid( new srLinearCurve(), dfl_dur, dfl_in, dfl_out ) );
		}
		srLinearCurve *new_trapezoid( double dfl_dur = 1.0, double dfl_ramp = 0.0 ) {
			return( new_trapezoid( dfl_dur, dfl_ramp, dfl_ramp ) );
		}
			
		int get_error( bool print_out )	{
			if( err_code > 0 )	{
				if( print_out )	{
					switch( err_code )	{
						case BAD_TAG: 				LOG( "srSynchPoints ERR: BAD_TAG" ); break;
						case BUMPED_BACK_TAG:		LOG( "srSynchPoints ERR: BUMPED_BACK_TAG" ); break;
						case BUMPED_FWD_TAG:		LOG( "srSynchPoints ERR: BUMPED_FWD_TAG" ); break;
						case NEGATIVE_RAMP:			LOG( "srSynchPoints ERR: NEGATIVE_RAMP" ); break;
						case UNDEFINED_INTERVAL:	LOG( "srSynchPoints ERR: UNDEFINED_INTERVAL" ); break;
						case OVERLAPPED_RAMPS: 		LOG( "srSynchPoints ERR: OVERLAPPED_RAMPS" ); break;
						default: 					LOG( "srSynchPoints ERR: BAD ERR CODE: %d", err_code ); break;
					}
				}
			}
			return( err_code );
		}
	private:
		
		int err_code;

		bool duration_defined;
		double default_ramp_out;
		double implied_ramp_out;
		double global_offset;

		double synch_time_arr[ NUM_SYNCH_TAGS ];
};
#endif
