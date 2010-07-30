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

class TimeProfiler	{

	public:
	
		TimeProfiler( void ) {
			req_enable = false;
			enabled = false;
			dyn_threshold = false;
			report_time = 0.0;
			report_accum_time = 0.0;
			threshold = 0.0;
			suppression = -1;
			count = 0;
			prev_count = 0;
			prev_label = _strdup( "START" );
			reset( 0.0 );
		}
		~TimeProfiler( void ) {
			if( prev_label )	{
				free( prev_label );
			}
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
		}
		void set_threshold( double min )	{
			threshold = min;
			if( min > 0.0 )
				dyn_threshold = false;
			else
				dyn_threshold = true;
		}
		
		void reset( double curr_dt )	{
			set_mark( "RESET", 0 );
			fr = SBM_get_real_time();
			to = fr;
			if( dyn_threshold )
				threshold = curr_dt;
			if( req_enable )	{
				if( threshold > 0.0 )	{
					enabled = true;
					req_enable = false;
				}
			}
			if( reporting ) {
				reporting = false;
				double report_interval = fr - report_time;
				double loss = report_interval - report_accum_time;
				double percent = 100.0 * loss / report_interval;
				std::cout << "Loss: " << loss << " ( " << percent << " % )" << std::endl;
			}
			if( req_report )	{
				reporting = true;
				req_report = false;
				std::cout << "TimeProfiler:" << std::endl;
				report_time = fr;
				report_accum_time = 0.0;
			}
			count = 0;
		}
		
		void set_mark( char *label, int level )	{
			if( enabled || reporting )	{
				to = SBM_get_real_time();
				double dt = to - fr;
				fr = to;
				if( reporting ) {
					if( label ) {
						std::cout << "  REPORT[ " << level << " ]( " << dt << " ): \"" << label << "\" " << std::endl;
					}
					else	{
						std::cout << "  REPORT[ " << level << " ]( " << dt << " ): # " << count << std::endl;
					}
					report_accum_time += dt;
				}
				if( enabled && ( level > suppression ) )	{
					if( dt > threshold )	{
						std::cout << "TimeProfiler[ " << level << " ]( " << dt << " )" << std::endl;
						if( prev_label ) {
							std::cout << "  FR: \"" << prev_label << "\"" << std::endl;
							free( prev_label );
							prev_label = NULL;
						}
						else	{
							std::cout << "  FR: # " << prev_count << std::endl;
						}
						if( label ) {
							std::cout << "  TO: \"" << label << "\"" << std::endl;
							prev_label = _strdup( label );
						}
						else	{
							std::cout << "  TO: # " << count << std::endl;
						}
						prev_count = count;
					}
				}
			}
			count++;
		}

		void mark( int level = 0 )	{
			set_mark( NULL, level );
		}
		void mark( char *label, int level = 0 )	{
			set_mark( label, level );
		}
		void mark( char *prefix, char *suffix, int level = 0 )	{
			char label[ 1024 ];
			sprintf( label, "%s: %s", prefix, suffix );
			set_mark( label, level );
		}
		void mark_line( char *file, int line, int level = 0 )	{ // ie: mark_line( __FILE__, __LINE__ )
			char label[ 1024 ];
			sprintf( label, "file:'%s' line:%d", file, line );
			set_mark( label, level );
		}

		void print( void )	{
			printf( "TimeProfiler:\n" );
			printf( "    status: %s\n", enabled ? "ENABLED" : "DISABLED" );
			printf( "   dynamic: %s\n", dyn_threshold ? "true" : "false" );
			printf( "  suppress: %d\n", suppression );
			printf( " threshold: %f\n", threshold );
		}
		
	private:
		bool req_enable, enabled;
		bool req_report, reporting;
		bool dyn_threshold;
		double report_time;
		double report_accum_time;
		double threshold;
		double fr, to;
		char *prev_label;
		int prev_count, count;
		int suppression;
};

#endif
