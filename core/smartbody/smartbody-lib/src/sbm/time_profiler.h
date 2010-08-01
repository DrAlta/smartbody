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

#define PROFILER_LABEL_SIZE 8192

#if 0
class IntervalProfiler	{

	private:
		
		typedef struct profile_entry_s	{

			char	label[ PROFILER_LABEL_SIZE ];
			double	time, dt;

			profile_entry_s	*next;

		} profile_entry_t;
		
		typedef struct group_entry_s	{

			char	name[ PROFILER_LABEL_SIZE ];
			bool 	open;
			
			profile_entry_t *profiles;
			srHashMap <profile_entry_t> profile_map;

		} group_entry_t;

		srHashMap <profile_entry_t> profile_map;
		srHashMap <group_entry_t> group_map;
		srHashMap <int> interval_map;
		
	public:
		IntervalProfiler( void ) {
		}
		~IntervalProfiler( void ) {
		}
		
		void reset( void )	{
			
			group_entry_t *group_p = NULL;
			while( ( group_p = group_map.next() ) != NULL ) {
				if( group_p->open ) {
					std::cout << "IntervalProfiler::reset ERR: group '" << group_name << " found open" << std::endl;
				}
				
				profile_entry_t *profiles_p = NULL;
				while( ( profiles_p = group_p->profile_map.next() ) != NULL ) {
					
					
				}
				
				
			}
		}
		
		void begin( const char* group_name ) {
			
			group_entry_t *group_p = group_map.lookup( group_name );
			if( group_p == NULL )	{
				group_p = new group_entry_t;
				_snprintf( group_p->name, PROFILER_LABEL_SIZE, "%s", group_name );
				group_p->open = false;
				group_p->profiles = NULL;
				group_map.insert( group_name, group_p, true ); // delete upon destructor...
			}
			
			if( group_p->open ) {
				std::cout << "IntervalProfiler::begin ERR: group '" << group_name << " already open" << std::endl;
			}

			group_p->open = true;
			mark( -1, group_name, "IntervalProfiler-mark-BEGIN" );
		}
		
		void mark( int level, const char* group_name, const char* label )	{
		
			group_entry_t *group_p = group_map.lookup( group_name );
			if( group_p == NULL )	{
				std::cout << "IntervalProfiler::mark ERR: group '" << group_name << " not available" << std::endl;
				return;
			}
			if( !group_p->open ) {
				std::cout << "IntervalProfiler::mark ERR: group '" << group_name << " not open" << std::endl;
				return;
			}
			
//			char profile_key[ PROFILER_LABEL_SIZE ];
//			_snprintf( profile_key, PROFILER_LABEL_SIZE, "%s:%s", group_name, label );

//			profile_entry_t *profile_p = group_p->profile_map.lookup( profile_key );
			profile_entry_t *profile_p = group_p->profile_map.lookup( label );
			if( profile_p == NULL )	{
				profile_p = new profile_entry_t;
//				_snprintf( profile_p->label, PROFILER_LABEL_SIZE, "%s", profile_key );
				_snprintf( profile_p->label, PROFILER_LABEL_SIZE, "%s", label );
				group_p->profile_map.insert( profile_key, profile_p, true ); // delete upon destructor...
			}
			
			profile_p->time = SBM_get_real_time();
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
		
	private:
	
};
#endif

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
						std::cout << "  REPORT[ " << count << " ][ " << level << " ]( " << dt << " ): \"" << label << "\" " << std::endl;
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

#endif
