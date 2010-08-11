/*
 *  time_profiler.h - part of SmartBody-lib
 *  Copyright (C) 2010  University of Southern California
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

#ifndef TIME_INTERVAL_PROFILER_H
#define TIME_INTERVAL_PROFILER_H

#include <iostream>
#include "sbm/sr_hash_map.h"
#include "sbm/time_regulator.h"

#define DEFAULT_ENABLED			false
#define DEFAULT_SNIFF			0.95
#define DEFAULT_AVOID			1.5
#define DEFAULT_THRESHOLD		10.0
#define DEFAULT_DECAYING		0.99

////////////////////////////////////////////////////////////////////////////////////////

class TimeIntervalProfiler { // T.I.P.
	
	private:
		enum time_profiler_enum_set	{
		
			LABEL_SIZE =	8192,
			MAX_GROUPS =	256,
			MAX_PROFILES =	256,
			MAX_ROLLING = 	256
		};

		typedef struct profile_entry_s {

			int 	level;
			char	label[ LABEL_SIZE ];

			double	prev_dt;
			double  interval_dt;

			int 	intra_count;
			double	avg_intra_dt;
			double	max_intra_dt;

			double	decay_dt;

			double	accum_roll_dt;
			int 	accum_count;
			double	roll_dt;
			double	roll_dt_arr[ MAX_ROLLING ];
			int 	roll_index;
			
			double	event_time;
			bool	spike;
			bool	reset;

		} profile_entry_t;

		typedef struct group_entry_s {

			bool	req_enable;
			bool	req_disable;
			bool	enabled;
			char	name[ LABEL_SIZE ];
			bool 	open;

			double  interval_dt;
			double	decay_dt;
			double	roll_dt;

			srHashMap <profile_entry_t> profile_map;
			profile_entry_t* profile_p_arr[ MAX_PROFILES ];
			
			int 	profile_arr_count;
			int 	active_profile_count;

			profile_entry_t* curr_profile_p;
			int 	profile_event_count;
			bool	spike;
			bool	req_preload;
			bool	preloading;
			bool	reset;

		} group_entry_t;


		srHashMap <group_entry_t> group_map;
		group_entry_t* group_p_arr[ MAX_GROUPS ];
		
		int 	group_arr_count;
		int 	active_group_count; // UNUSED!!!

		double	reset_time;
		double	reset_dt;

		bool	req_print;
		bool	req_erase;
		bool	req_clobber;
		bool	req_enable;
		bool	req_disable;
		bool	req_preload;
		bool	enabled;
		bool	reporting;
		bool	preloading;

		bool	dyn_threshold;
		double	dyn_sniff; // decaying: positive, less than 1
		double	dyn_avoid; // bumping: greater than 1: small hiccup, x2... large hiccup, x1.1
		double	fix_threshold;
		double	threshold;

		double	decaying_factor;
		int 	suppression;
		int 	selection;

	public:
		TimeIntervalProfiler( void ) { null(); }
		~TimeIntervalProfiler( void ) {}

	private:

		void null( void )	{
			group_arr_count = 0;
			active_group_count = 0;
			reset_time = 0.0;
			reset_dt = 0.0;
			req_print = false;
			req_erase = false;
			req_clobber = false;
			req_enable = DEFAULT_ENABLED;
			req_disable = false;
			req_preload = false;
			enabled = false;
			reporting = false;
			preloading = false;
			dyn_threshold = false;
			dyn_sniff = DEFAULT_SNIFF;
			dyn_avoid = DEFAULT_AVOID;
			fix_threshold = 0.0;
			threshold = DEFAULT_THRESHOLD;
			decaying_factor = DEFAULT_DECAYING;
			suppression = -1;
			selection = -1;
		}

		void null_group( group_entry_t* group_p, const char* group_name = "" ) {
		
			group_p->req_enable = false;
			group_p->req_disable = false;
			group_p->enabled = true;
			_snprintf( group_p->name, LABEL_SIZE, "%s", group_name );
			group_p->open = false;
			group_p->interval_dt = 0.0;
			group_p->decay_dt = 0.0;
			group_p->roll_dt = 0.0;
			group_p->profile_map.expunge();
			group_p->profile_arr_count = 0;
			group_p->active_profile_count = 0;
			group_p->curr_profile_p = NULL;
			group_p->profile_event_count = 0;
			group_p->spike = false;
			group_p->req_preload = false;
			group_p->preloading = false;
			group_p->reset = false;		
		}

		void null_profile( profile_entry_t* profile_p, const char* label = "" ) {
		
			profile_p->level = -1;
			_snprintf( profile_p->label, LABEL_SIZE, "%s", label );
			profile_p->prev_dt = 0.0;
			profile_p->interval_dt = 0.0;
			profile_p->intra_count = 0;
			profile_p->avg_intra_dt = 0.0;
			profile_p->max_intra_dt = 0.0;
			profile_p->decay_dt = 0.0;
			profile_p->accum_roll_dt = 0.0;
			profile_p->accum_count = 0;
			profile_p->roll_dt = 0.0;
			profile_p->roll_index = 0;
			profile_p->event_time = 0.0;
			profile_p->spike = false;
			profile_p->reset = false;
		}

		void reset_group( group_entry_t* group_p ) {
		
			group_p->interval_dt = 0.0;
			group_p->decay_dt = 0.0;
			group_p->roll_dt = 0.0;
			group_p->active_profile_count = 0;
			group_p->profile_event_count = 0;
			group_p->spike = false;
			group_p->reset = false;
		}

		void reset_profile( profile_entry_t* profile_p ) {
		
			profile_p->level = -1;
			profile_p->prev_dt = profile_p->interval_dt;
			profile_p->interval_dt = 0.0;
			profile_p->intra_count = 0;
			profile_p->max_intra_dt = 0.0;
			profile_p->event_time = 0.0;
			profile_p->spike = false;
			profile_p->reset = false;
		}

///////////////////////////////////////////////////

	public:

		void print( void )	{	
			req_print = true;
		}
		void erase( void )	{	
			req_erase = true;
		}
		void clobber( void )	{	
			req_clobber = true;
		}
		void enable( bool en )	{
			if( en )
				req_enable = true;
			else
				req_disable = true;
		}
		int enable( const char* group_name, bool en )	{
			group_entry_t* group_p = get_group( group_name );
			if( group_p )	{
				if( en )
					group_p->req_enable = true;
				else
					group_p->req_disable = true;
				return( false );
			}
			return( true );
		}
		void preload( void )	{
			req_preload = true;
		}
		int preload( const char* group_name )	{
			group_entry_t* group_p = get_group( group_name );
			if( group_p )	{
				group_p->req_preload = true;
				return( false );
			}
			return( true );
		}
		void report( void )	{
			if( enabled )	{
				reporting = true;
			}
			else	{
				printf( "TimeIntervalProfiler::report ERR: not enabled \n" );
			}
		}

		void set_suppression( int sup )	{
			suppression = sup;
			selection = -1;
		}
		void set_selection( int sel )	{
			selection = sel;
			suppression = -1;
		}
		void set_threshold( double factor )	{
			threshold = factor;
			if( threshold > 0.0 )	{
				dyn_threshold = false;
			}
			else	{
				dyn_threshold = true;
			}
			fix_threshold = 0.0;
		}
		void set_override( double value )	{
			fix_threshold = value;
			if( fix_threshold > 0.0 )	{
				dyn_threshold = false;
			}
		}
		void set_dynamic( double sniff, double avoid )	{
			if( sniff > 0.0 ) dyn_sniff = sniff;
			if( avoid > 1.0 ) dyn_avoid = avoid;
			if( dyn_sniff >= 1.0 ) dyn_sniff = 0.99999;
			if( dyn_avoid <= 1.0 ) dyn_avoid = 1.1;
			dyn_threshold = true; 
		}
		void set_sniff( double sniff )	{
			if( sniff > 0.0 ) dyn_sniff = sniff;
			if( dyn_sniff >= 1.0 ) dyn_sniff = 0.99999;
		}
		void set_avoid( double avoid )	{
			if( avoid > 1.0 ) dyn_avoid = avoid;
			if( dyn_avoid <= 1.0 ) dyn_avoid = 1.1;
		}
		void set_decay( double s )	{
			decaying_factor = s;
			if( decaying_factor < 0.0 ) decaying_factor = 0.0;
			if( decaying_factor > 0.999 ) decaying_factor = 0.999;
		}

///////////////////////////////////////////////////

		void print_legend( void );
		void print_group( group_entry_t *group_p );
		void print_data( void );

	private:

		void print_profile_report( char *prefix, profile_entry_t *profile_p );
		void print_group_report( const char *prefix, group_entry_t* group_p );

		void print_profile_alert( double reset_dt, group_entry_t* group_p, profile_entry_t *profile_p );
		void print_group_alert( const char *prefix, double reset_dt, group_entry_t* group_p );
		
		void accum_profile( profile_entry_t *profile_p );
		bool check_profile_spike( profile_entry_t *profile_p );
		bool check_profile_show( int level );

		group_entry_t* get_group( const char* group_name );
		profile_entry_t* get_profile( group_entry_t *group_p, const char* label );

		void make_mark( group_entry_t *group_p, double curr_time );

	public:

		void update( double time );
		void update( void ) {
			update( SBM_get_real_time() );
		}

		void mark_time( const char* group_name, int level, const char* label, double curr_time )	{

			if( enabled )	{

				group_entry_t *group_p = get_group( group_name );
				if( group_p ) {

					if( group_p->enabled )	{

						profile_entry_t *profile_p = get_profile( group_p, label );
						if( group_p->open ) {  // continuation
							make_mark( group_p, curr_time );
						}
						else	{  // new segment
							group_p->open = true;
						}
						if( level > profile_p->level )	{
							profile_p->level = level;
						}
						profile_p->event_time = curr_time;
						group_p->curr_profile_p = profile_p;
					}
					else
					if( group_p->preloading )	{
						profile_entry_t *profile_p = get_profile( group_p, label ); // already done?
						if( level > profile_p->level )	{
							profile_p->level = level;
						}
					}
				}
			}
			else
			if( preloading )	{
				group_entry_t *group_p = get_group( group_name );
				profile_entry_t *profile_p = get_profile( group_p, label );
				if( level > profile_p->level )	{
					profile_p->level = level;
				}
			}
		}

		int mark_time( const char* group_name, double curr_time )	{

			if( enabled )	{
				group_entry_t *group_p = group_map.lookup( group_name );
				if( group_p ) {
					if( group_p->enabled )	{
						if( group_p->open ) {  // close...
							make_mark( group_p, curr_time );
							group_p->open = false;
							group_p->curr_profile_p = NULL;
							return( group_p->profile_event_count );
						}
					}
				}
			}
			return( 0 );
		}

		void mark( const char* group_name, int level, const char* label )	{
			mark_time( group_name, level, label, SBM_get_real_time() );
		}
		int mark( const char* group_name )	{
			return( mark_time( group_name, SBM_get_real_time() ) );
		}

		static double test_clock( int reps = 0 );
};

////////////////////////////////////////////////////////////////////////////////////////
#endif
