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

#ifndef TIME_PROFILER_H
#define TIME_PROFILER_H

#include "sbm/time_regulator.h"

#define PROFILER_LABEL_SIZE 			8192
#define PROFILER_MAX_GROUP_ENTRIES		256
#define PROFILER_MAX_GROUPS				256

////////////////////////////////////////////////////////////////////////////////////////

class IntervalProfiler	{

	private:
		
		typedef struct profile_entry_s	{

			int 	level;
			char	label[ PROFILER_LABEL_SIZE ];
			double	time, dt;
			double	decay_dt;
			double	accum_dt;
			int 	count;

		} profile_entry_t;
		
		typedef struct group_entry_s	{

			char	name[ PROFILER_LABEL_SIZE ];
			bool 	open;
			double	prev_profile_time;
			double	total_dt;
			double	total_accum_dt;
			double	total_decay_dt;
			
			srHashMap <profile_entry_t> profile_map;
			profile_entry_t* profile_p_arr[ PROFILER_MAX_GROUP_ENTRIES ];
			int 	profile_count;
			
		} group_entry_t;

		srHashMap <group_entry_t> group_map;
		group_entry_t* group_p_arr[ PROFILER_MAX_GROUPS ];
		int group_count;
		
		bool req_erase;
		bool req_clobber;
		
		double smooth_factor;
		int suppression;
		int selection;
		
	public:
		IntervalProfiler( void ) {
			group_count = 0;
			req_erase = false;
			req_clobber = false;
			smooth_factor = 0.9;
			suppression = -1;
			selection = -1;
		}
		~IntervalProfiler( void ) {
		}
		
		void erase( void )	{	
			req_erase = true;
		}
		void clobber( void )	{	
			req_clobber = true;
		}
		
		void reset( void )	{
		
			printf( "IntervalProfiler:\n" );

			group_entry_t *group_p = NULL;
			for( int i=0; i< group_count; i++ )	{
				group_p = group_p_arr[ i ];

				if( group_p->open ) {
					std::cout << "IntervalProfiler::reset ERR: group '" << group_p->name << " found open" << std::endl;
				}
				
				printf( "  Group: \"%s\"\n", group_p->name );
				group_p->total_dt = 0.0;
				group_p->total_accum_dt = 0.0;
				group_p->total_decay_dt = 0.0;
				
				profile_entry_t *profile_p = NULL;
				for( int j=0; j< group_p->profile_count; j++ )	{

					profile_p = group_p->profile_p_arr[ j ];
					double avg_dt = profile_p->accum_dt / (double) profile_p->count;
					
					if( 
						( ( selection < 0 )&&( profile_p->level > suppression ) )||
						( ( suppression < 0 )&&( selection > -1 )&&( profile_p->level == selection ) )
					)	{
						printf( "    REPORT[ %d ]( dt:%f )( da:%f )( av:%f ) \"%s\"\n", 
							 profile_p->level,
							 profile_p->dt,
							 profile_p->decay_dt,
							 avg_dt,
							 profile_p->label
						);
						group_p->total_dt += profile_p->dt;
						group_p->total_decay_dt += profile_p->decay_dt;
						group_p->total_accum_dt += avg_dt;
					}
				}

				printf( "  Total: ( dt:%f )( da:%f )( av:%f )\n", 
					group_p->total_dt,
					group_p->total_decay_dt,
					group_p->total_accum_dt
				);
				group_p->profile_count = 0;
			}

			if( req_erase ) {
				req_erase = false;
				group_map.reset();
				while( ( group_p = group_map.next() ) != NULL ) {
					group_p->profile_map.expunge();
				}
			}
			if( req_clobber ) {
				req_clobber = false;
				group_map.expunge();
			}
			group_count = 0;
		}
		
		void begin( const char* group_name ) {
			
			group_entry_t *group_p = group_map.lookup( group_name );
			if( group_p == NULL )	{
				group_p = new group_entry_t;
				_snprintf( group_p->name, PROFILER_LABEL_SIZE, "%s", group_name );
				group_p->open = false;
				group_p->profile_count = 0;
				group_map.insert( group_name, group_p, true ); // delete upon destructor...
			}
			else
			
			if( group_p->open ) {
				std::cout << "IntervalProfiler::begin ERR: group '" << group_name << " already open" << std::endl;
			}
			if( group_p->profile_count == 0 )	{
				group_p_arr[ group_count ] = group_p;
				group_count++;
			}
			
			double curr_time = SBM_get_real_time();
			group_p->open = true;
			group_p->prev_profile_time = curr_time;
			mark( -1, group_name, "IntervalProfiler-mark-BEGIN", curr_time );
		}
		
		void mark( int level, const char* group_name, const char* label, double curr_time )	{
		
			group_entry_t *group_p = group_map.lookup( group_name );
			if( group_p == NULL )	{
				std::cout << "IntervalProfiler::mark ERR: group '" << group_name << " not available" << std::endl;
				return;
			}
			if( !group_p->open ) {
				group_p->open = true;
				group_p->prev_profile_time = curr_time;
				level = -1;
			}
			
			profile_entry_t *profile_p = group_p->profile_map.lookup( label );
			if( profile_p == NULL )	{
				profile_p = new profile_entry_t;
				_snprintf( profile_p->label, PROFILER_LABEL_SIZE, "%s", label );
				profile_p->accum_dt = 0.0;
				profile_p->decay_dt = 0.0;
				profile_p->count = 0;
				group_p->profile_map.insert( label, profile_p, true ); // delete upon destructor...
			}
			
			profile_p->level = level;
			profile_p->time = curr_time;
			profile_p->dt = curr_time - group_p->prev_profile_time;
			profile_p->accum_dt += profile_p->dt;
			profile_p->decay_dt = smooth_factor * profile_p->decay_dt + ( 1.0 - smooth_factor ) * profile_p->dt;
			profile_p->count++;
			group_p->prev_profile_time = curr_time;
			group_p->profile_p_arr[ group_p->profile_count++ ] = profile_p;
		}
		
		void mark( int level, const char* group_name, const char* label )	{

			mark( level, group_name, label, SBM_get_real_time() );
		}
		
		void end( const char* group_name ) {
		
			group_entry_t *group_p = group_map.lookup( group_name );
			if( group_p == NULL )	{
				std::cout << "IntervalProfiler::end ERR: group '" << group_name << " not available" << std::endl;
				return;
			}
			if( !group_p->open ) {
				std::cout << "IntervalProfiler::end ERR: group '" << group_name << " not open" << std::endl;
			}
			group_p->open = false;
		}
		
		void end( int level, const char* group_name, const char* label ) {

			mark( level, group_name, label );
			end( group_name );
		}
};

////////////////////////////////////////////////////////////////////////////////////////

class TimeProfiler	{

	public:
	
		TimeProfiler( void ) {
			req_enable = false;
			enabled = false;
			req_report = false;
			reporting = false;
			dyn_threshold = true;
			smooth_dt = 0.7;
			prev_dt = 0.0;
			report_time = 0.0;
			report_accum_time = 0.0;
			threshold = 0.0;
			suppression = -1;
			selection = -1;
			count = 0;
			prev_count = 0;
			_snprintf( prev_label, PROFILER_LABEL_SIZE, "START" );
			reset( 0.0 );
		}
		~TimeProfiler( void ) {
		}
		
		void enable( bool en )	{
			if( en )
				req_enable = true;
			else
				enabled = false;
		}
		void report( void )	{
			req_report = true;
		}
		
		void set_suppression( int sup )	{
			suppression = sup;
			selection = -1;
		}
		void set_selection( int sel )	{
			selection = sel;
			suppression = -1;
		}
		void set_threshold( double min )	{
			threshold = min;
			if( min > 0.0 )
				dyn_threshold = false;
			else
				dyn_threshold = true;
		}
		void set_smooth( double s )	{
			smooth_dt = s;
			if( smooth_dt < 0.0 ) smooth_dt = 0.0;
			if( smooth_dt > 0.999 ) smooth_dt = 0.999;
		}

		void reset( double curr_dt )	{
			mark( 0, "RESET" );
			if( dyn_threshold )	{
				threshold = smooth_dt * threshold + ( 1.0 - smooth_dt ) * curr_dt;
			}
			if( req_enable )	{
				if( threshold > 0.0 )	{
					enabled = true;
					req_enable = false;
				}
			}
			if( enabled || reporting || req_report )	{
				fr = SBM_get_real_time();
				to = fr;
				if( reporting ) {
					reporting = false;
					double report_interval = fr - report_time;
					double loss = report_interval - report_accum_time;
					double percent = 100.0 * loss / report_interval;
					std::cout << "  Total: " << report_accum_time << " Loss: " << loss << " ( " << percent << " % )" << std::endl;
				}
				if( req_report )	{
					reporting = true;
					req_report = false;
					std::cout << "TimeProfiler:" << std::endl;
					report_time = fr;
					report_accum_time = 0.0;
				}
			}
			count = 0;
		}
		
		void mark( int level, const char *label )	{
			if( enabled || reporting )	{
				to = SBM_get_real_time();
				double dt = to - fr;
				if( reporting ) {
					if( label ) {
						std::cout << "  REPORT[ " << count << " ][ " << level << " ]( " << dt << " ): \"" << label << "\"" << std::endl;
					}
					else	{
						std::cout << "  REPORT[ " << count << " ][ " << level << " ]( " << dt << " ): # " << count << std::endl;
					}
					report_accum_time += dt;
				}
				if( enabled )	{
					if( 
						( ( selection < 0 )&&( level > suppression ) )||
						( ( suppression < 0 )&&( level == selection ) )
					)	{

						if( dt > threshold )	{
							std::cout << "TimeProfiler[ " << count << " ][ " << level << " ]( " << dt << " )" << std::endl;
							if( prev_label[ 0 ] != '\0' ) {
								std::cout << "  FR: \"" << prev_label << "\"" << std::endl;
							}
							else	{
								std::cout << "  FR: # " << prev_count << std::endl;
							}
							if( label ) {
								std::cout << "  TO: \"" << label << "\"" << std::endl;
							}
							else	{
								std::cout << "  TO: # " << count << std::endl;
							}
							if( dyn_threshold )	{
								threshold = dt;
							}
						}
					}
				}
				prev_label[ 0 ] = '\0';
				if( label ) {
					strncpy( prev_label, label, PROFILER_LABEL_SIZE );
				}
				fr = to;
				prev_count = count;
			}
			count++;
		}

		void mark( int level = 0 )	{
			mark( level, NULL );
		}
		void mark( int level, const char *prefix, const char *suffix )	{
			char label[ PROFILER_LABEL_SIZE ];
			_snprintf( label, PROFILER_LABEL_SIZE, "%s: %s", prefix, suffix );
			mark( level, label );
		}
		void mark( int level, const char *file, int line )	{ // ie: mark( __FILE__, __LINE__ )
			char label[ PROFILER_LABEL_SIZE ];
			_snprintf( label, PROFILER_LABEL_SIZE, "file:'%s' line:%d", file, line );
			mark( level, label );
		}

		void print( void )	{
			printf( "TimeProfiler:\n" );
			printf( "    status: %s\n", enabled ? "ENABLED" : "DISABLED" );
			printf( "   dynamic: %s\n", dyn_threshold ? "true" : "false" );
			printf( "  suppress: %d\n", suppression );
			printf( "    select: %d\n", selection );
			printf( "    smooth: %f\n", smooth_dt );
			printf( " threshold: %f\n", threshold );
		}
		
	private:
		bool req_enable, enabled;
		bool req_report, reporting;
		bool dyn_threshold;
		double smooth_dt;
		double prev_dt;
		double report_accum_time;
		double report_time;
		double threshold;
		double fr, to;
		char prev_label[ PROFILER_LABEL_SIZE ];
		int prev_count;
		int count;
		int suppression;
		int selection;
};

////////////////////////////////////////////////////////////////////////////////////////
#endif
