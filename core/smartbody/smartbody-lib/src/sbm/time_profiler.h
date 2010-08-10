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

#define DEFAULT_REQ_ENABLED			false
#define DEFAULT_REQ_GROUP_ENABLED	true

#define DEFAULT_THRESHOLD		10.0
#define DEFAULT_SMOOTHING		0.99

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

			bool	reset;
		} profile_entry_t;

		typedef struct group_entry_s {

			bool	req_enable;
			bool	enabled;
			char	name[ LABEL_SIZE ];
			bool 	open;

			double  interval_dt;
			double	decay_dt;
			double	roll_dt;

			srHashMap <profile_entry_t> profile_map;
			profile_entry_t* profile_p_arr[ MAX_PROFILES ];
			
			int 	profile_arr_count;
//			int 	profile_count;
			int 	active_profile_count;

			profile_entry_t* curr_profile_p;
			double	curr_profile_time;
			int 	profile_event_count;
		} group_entry_t;


		srHashMap <group_entry_t> group_map;
		group_entry_t* group_p_arr[ MAX_GROUPS ];
		
		int 	group_arr_count;
//		int 	group_count;
		int 	active_group_count;

		double	reset_time;
		double	reset_dt;

		bool	req_print;
		bool	req_erase;
		bool	req_clobber;
		bool	req_enable;
		bool	enabled;
		bool	reporting;

//		bool	dyn_threshold;
//		bool	fix_threshold;
		double	threshold;
		double	smooth_factor;
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
			req_enable = DEFAULT_REQ_ENABLED;
			enabled = false;
			reporting = false;
			threshold = DEFAULT_THRESHOLD;
			smooth_factor = DEFAULT_SMOOTHING;
			suppression = -1;
			selection = -1;
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
			profile_p->reset = false;
		}

		void reset_profile( profile_entry_t* profile_p ) {
		
			profile_p->level = -1;
			profile_p->prev_dt = profile_p->interval_dt;
			profile_p->interval_dt = 0.0;
			profile_p->intra_count = 0;
			profile_p->max_intra_dt = 0.0;
			profile_p->reset = false;
		}

		void null_group( group_entry_t* group_p, const char* group_name = "" ) {
		
			group_p->req_enable = DEFAULT_REQ_GROUP_ENABLED;
			group_p->enabled = false;
			_snprintf( group_p->name, LABEL_SIZE, "%s", group_name );
			group_p->open = false;
			group_p->interval_dt = 0.0;
			group_p->decay_dt = 0.0;
			group_p->roll_dt = 0.0;
			group_p->profile_map.expunge();
			group_p->profile_arr_count = 0;
			group_p->active_profile_count = 0;
			group_p->curr_profile_p = NULL;
			group_p->curr_profile_time = 0.0;
			group_p->profile_event_count = 0;
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
				enabled = false;
		}
		int enable( const char* group_name, bool en )	{
			group_entry_t* group_p = get_group( group_name );
			if( group_p )	{
				if( en )
					group_p->req_enable = true;
				else
					group_p->enabled = false;
				return( false );
			}
			return( true );
		}
		void report( void )	{
			if( enabled )	{
				reporting = true;
			}
			else	{
				std::cout << "TimeIntervalProfiler::report ERR: not enabled" << std::endl;
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
#if 0
			if( factor > 0.0 )
				dyn_threshold = false;
			else
				dyn_threshold = true;
#endif
		}
		void set_smooth( double s )	{
			smooth_factor = s;
			if( smooth_factor < 0.0 ) smooth_factor = 0.0;
			if( smooth_factor > 0.999 ) smooth_factor = 0.999;
		}

///////////////////////////////////////////////////

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

		void reset( double time );
		void reset( void ) {
			reset( SBM_get_real_time() );
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
						group_p->curr_profile_p = profile_p;
						group_p->curr_profile_time = curr_time;
					}
#if 0
					else
					if( group_p->preload )	{
						profile_entry_t *profile_p = get_profile( group_p, label ); // already done?
						if( level > profile_p->level )	{
							profile_p->level = level;
						}
					}
#endif
				}
			}
#if 0
			else
			if( preload )	{
				group_entry_t *group_p = get_group( group_name );
				profile_entry_t *profile_p = get_profile( group_p, label );
				if( level > profile_p->level )	{
					profile_p->level = level;
				}
			}
#endif
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
							group_p->curr_profile_time = 0.0; // make a spike...
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
