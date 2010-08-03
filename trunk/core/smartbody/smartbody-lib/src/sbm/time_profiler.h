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

#include "sbm/time_regulator.h"

#define DEFAULT_REQ_ENABLED		false
#define DEFAULT_GROUP_ENABLED	true
#define DEFAULT_THRESHOLD		10.0
#define DEFAULT_SMOOTHING		0.99

////////////////////////////////////////////////////////////////////////////////////////

class TimeIntervalProfiler { // T.I.P.

		enum time_profiler_enum_set	{
		
			LABEL_SIZE =	8192,
			MAX_GROUPS =	256,
			MAX_PROFILES =	256,
			MAX_ROLLING = 	1024
		};

		typedef struct profile_entry_s {

			int 	level;
			char	label[ LABEL_SIZE ];

			double	prev_frame_dt;
			double  frame_dt;
			int 	frame_count;
			double	sub_frame_dt;
			double	max_dt;

			double	decay_dt;

			double	accum_dt;
			int 	accum_count;
			double	rolling_dt;
			double	rolling_dt_arr[ MAX_ROLLING ];
			int 	rolling_count;

		} profile_entry_t;

		typedef struct group_entry_s {

			bool	req_enable;
			bool	enabled;
			char	name[ LABEL_SIZE ];
			bool 	open;

			srHashMap <profile_entry_t> profile_map;
			profile_entry_t* profile_p_arr[ MAX_PROFILES ];
			int 	profile_count;

			profile_entry_t* curr_profile_p;
			double	curr_profile_time;
			int 	profile_event_count;

		} group_entry_t;


		srHashMap <group_entry_t> group_map;
		group_entry_t* group_p_arr[ MAX_GROUPS ];
		int 	group_count;

		bool	req_erase;
		bool	req_clobber;
		bool	req_enable;
		bool	enabled;
//		bool	req_report
		bool	reporting;

//		bool	dyn_threshold;
		double	threshold;
		double	smooth_factor;
		int 	suppression;
		int 	selection;

	public:
		TimeIntervalProfiler( void ) {
			group_count = 0;
			req_erase = false;
			req_clobber = false;
			req_enable = DEFAULT_REQ_ENABLED;
			enabled = false;
//			req_report = false;
			reporting = false;
			threshold = DEFAULT_THRESHOLD;
			smooth_factor = DEFAULT_SMOOTHING;
			suppression = -1;
			selection = -1;
		}
		~TimeIntervalProfiler( void ) {
		}
		
		void print_main( void )	{
			printf( "TimeIntervalProfiler:\n" );
			printf( "    status: %s\n", enabled ? "ENABLED" : "DISABLED" );
//			printf( "   dynamic: %s\n", dyn_threshold ? "true" : "false" );
			printf( "  suppress: %d\n", suppression );
			printf( "    select: %d\n", selection );
			printf( "    smooth: %f\n", smooth_factor );
			printf( " threshold: %f\n", threshold );
		}
		void print_group( group_entry_t *group_p )	{

			if( group_p )	{

				printf( "TimeIntervalProfiler: Group: \"%s\"\n", group_p->name );
				printf( "    status: %s\n", group_p->enabled ? "ENABLED" : "DISABLED" );
				printf( "  profiles: %d\n", group_p->profile_count );
				
				for( int i=0; i< group_p->profile_count; i++ )	{
					profile_entry_t* profile_p = group_p->profile_p_arr[ i ];
					if( profile_p ) {
					
						printf( "  profile[ %d ]: \"%s\"\n", i, profile_p->label );
						printf( "          level: %d\n", profile_p->level );
						printf( "          accum: %d\n", profile_p->accum_count );
					}
					else	{
					
					}
				}
			}
		}
		void print_groups( void )	{
			for( int i=0; i< group_count; i++ )	{
				group_entry_t *group_p = group_p_arr[ i ];
				if( group_p )	{
					print_group( group_p );
				}
				else	{
				
				}
			}
		}
		void print( void )	{
			print_main();
			print_groups();
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
					group_p->req_enable = false;
				return( false );
			}
			return( true );
		}
		void report( void )	{
			reporting = true;
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

		void accum_profile( profile_entry_t *profile_p )	{

// if( profile_p->frame_dt == 0.0 ) {.... !!???}
			profile_p->sub_frame_dt = profile_p->frame_dt / (double) profile_p->frame_count;

			profile_p->decay_dt =
				smooth_factor * profile_p->decay_dt +
				( 1.0 - smooth_factor ) * profile_p->frame_dt;

			profile_p->accum_dt += profile_p->frame_dt;
			if( profile_p->accum_count >= MAX_ROLLING )	{
				profile_p->accum_dt -= profile_p->rolling_dt_arr[ profile_p->rolling_count ];
			}
			else	{
				profile_p->accum_count++;
			}
			profile_p->rolling_dt_arr[ profile_p->rolling_count ] = profile_p->frame_dt;
			profile_p->rolling_count++;
			if( profile_p->rolling_count >= MAX_ROLLING )	{
				profile_p->rolling_count = 0;
			}
			profile_p->rolling_dt = profile_p->accum_dt / (double)profile_p->accum_count;
		}

		void print_profile( const char *prefix, const char *group_name, profile_entry_t *profile_p )	{

			printf( 
				"%s%s [%d]< m[%d]:%.6f | f:%.6f | d:%.6f | r[%d]:%.6f >:\"%s\"\n",
				prefix,
				group_name,
				profile_p->level,
				profile_p->frame_count,
				profile_p->max_dt,
				profile_p->frame_dt,
				profile_p->decay_dt,
				profile_p->accum_count,
				profile_p->rolling_dt,
				profile_p->label
			);
		}

		bool check_profile_spike( profile_entry_t *profile_p )	{

			// harsh
			if(
				(  profile_p->frame_count > 1 )&&
				( profile_p->max_dt > ( profile_p->prev_frame_dt * threshold ) )
			)	{
				return( true );
			}

			// smooth harsh
			if( profile_p->frame_dt > ( profile_p->decay_dt * threshold ) )	{
				return( true );
			}

			// fair
			if( profile_p->frame_dt > ( profile_p->rolling_dt * threshold ) )	{
				return( true );
			}
			return( false );
		}

		bool check_profile_show( int level )	{

			return (
				( selection < 0 )&&
				( level > suppression )
			)||(
				( suppression < 0 )&&
				( selection > -1 )&&
				( level == selection )
			);
		}

		void reset( void ) {

			if( enabled )	{
				if( reporting )	{
					printf( "TimeIntervalProfiler:\n" );
				}

				group_entry_t *group_p = NULL;
				for( int i=0; i< group_count; i++ ) {

					group_p = group_p_arr[ i ];
					if( group_p == NULL ) {
						std::cout 
							<< "TimeIntervalProfiler::reset ERR: NULL group_p in group_p_arr[ " 
							<< i << "]" 
							<< std::endl;
						return;
					}

					if( group_p->enabled )	{
						if( reporting )	{
							printf( " Group: \"%s\"\n", group_p->name );
						}
						double total_frame_dt = 0.0;
						double total_decay_dt = 0.0;
						double total_rolling_dt = 0.0;
						bool spike = false;

						profile_entry_t *profile_p = NULL;
						for( int j=0; j< group_p->profile_count; j++ ) {

							profile_p = group_p->profile_p_arr[ j ];
							if( profile_p == NULL ) {
								std::cout 
									<< "TimeIntervalProfiler::reset ERR: NULL profile_p in group_p_arr[ " 
									<< i 
									<< "] profile_p_arr[ " 
									<< j 
									<< " ]" 
									<< std::endl;
								return;
							}

							if( profile_p->frame_count )	{
								accum_profile( profile_p );

								total_frame_dt += profile_p->frame_dt;
								total_decay_dt += profile_p->decay_dt;
								total_rolling_dt += profile_p->rolling_dt;

								spike = check_profile_spike( profile_p );
								if( reporting )	{
									if( spike )	{
										print_profile( " X", "", profile_p );
									}
									else	{
										print_profile( "  ", "", profile_p );
									}
								}
								else
								if( spike )	{
									if( check_profile_show( profile_p->level  ) )	{
										print_profile( "X ", group_p->name, profile_p );
									}
								}

								profile_p->level = -1;
								profile_p->max_dt = 0.0;
								profile_p->prev_frame_dt = profile_p->frame_dt;
								profile_p->frame_dt = 0.0;
								profile_p->frame_count = 0;
							}
						}

						spike = false;
						if( total_frame_dt > ( total_decay_dt * threshold ) )	{
							spike = true;
						}
						else
						if( total_frame_dt > ( total_rolling_dt * threshold ) )	{
							spike = true;
						}
						if( reporting )	{
							printf( " %s Total:< f:%f | d:%f | r:%f >\n",
								spike ? "X" : "",
								total_frame_dt,
								total_decay_dt,
								total_rolling_dt
							);
						}
						else
						if( spike )	{
							printf( "X %s:< f:%f | d:%f | r:%f >\n",
								group_p->name,
								total_frame_dt,
								total_decay_dt,
								total_rolling_dt
							);
						}
						group_p->profile_event_count = 0;
					}
					
					if( group_p->req_enable )	{
						group_p->enabled = true;
						group_p->req_enable = false;
						
// reset values for next run?
					}
				}
				if( reporting )	{
					reporting = false;
				}
			}
			if( req_enable )	{
				enabled = true;
				req_enable = false;
			}
			if( req_erase ) {
#if 0
				group_map.reset();
				group_entry_t *erase_group_p;
				while( ( erase_group_p = group_map.next() ) != NULL ) {
					erase_group_p->profile_map.expunge();
				}
#endif
				req_erase = false;
			}
			if( req_clobber ) {
#if 0
				group_map.expunge();
#endif
				req_clobber = false;
			}
		}

		void make_mark( group_entry_t *group_p, double curr_time )	{

			profile_entry_t *profile_p = group_p->curr_profile_p;
			if( profile_p )	{

				double dt = curr_time - group_p->curr_profile_time;
				if( dt > profile_p->max_dt )	{
					profile_p->max_dt = dt;
				}
				profile_p->frame_dt += dt;
				profile_p->frame_count++;
				group_p->profile_event_count++;
				return;
			}
			std::cout << "TimeIntervalProfiler::make_mark ERR: no current profile" << std::endl;
		}

		group_entry_t* get_group( const char* group_name ) {

			group_entry_t *group_p = group_map.lookup( group_name );
			if( group_p == NULL ) {

				group_p = new group_entry_t;
				group_p->req_enable = DEFAULT_GROUP_ENABLED;
				group_p->enabled = false;
				_snprintf( group_p->name, LABEL_SIZE, "%s", group_name );
				group_p->open = false;
				group_p->profile_count = 0;
				group_p->curr_profile_p = NULL;
				group_p->curr_profile_time = 0.0;
				group_p->profile_event_count = 0;
				
				group_map.insert( group_name, group_p, true ); // delete upon destructor...
				if( group_count < MAX_GROUPS )	{
					group_p_arr[ group_count ] = group_p;
					group_count++;
				}
				else	{
				}
			}
			return( group_p );
		}

		profile_entry_t* get_profile( group_entry_t *group_p, const char* label ) {

			profile_entry_t *profile_p = group_p->profile_map.lookup( label );
			if( profile_p == NULL ) {

				profile_p = new profile_entry_t;
				profile_p->level = -1;
				_snprintf( profile_p->label, LABEL_SIZE, "%s", label );
				profile_p->prev_frame_dt = 0.0;
				profile_p->frame_dt = 0.0;
				profile_p->frame_count = 0;
				profile_p->sub_frame_dt = 0.0;
				profile_p->max_dt = 0.0;
				profile_p->decay_dt = 0.0;
				profile_p->accum_dt = 0.0;
				profile_p->accum_count = 0;
				profile_p->rolling_dt = 0.0;
				profile_p->rolling_count = 0;
				
				group_p->profile_map.insert( label, profile_p, true ); // delete upon destructor...
				if( group_p->profile_count < MAX_PROFILES ) {
					group_p->profile_p_arr[ group_p->profile_count ] = profile_p;
					group_p->profile_count++;
				}
				else	{
					std::cout << "TimeIntervalProfiler::get_profile ERR: MAX_PROFILES: " << MAX_PROFILES << std::endl;
				}
			}
			return( profile_p );
		}

		void mark( const char* group_name, int level, const char* label, double curr_time )	{

			if( enabled )	{
				group_entry_t *group_p = get_group( group_name );
				if( group_p ) {
					if( group_p->enabled )	{
						profile_entry_t *profile_p = get_profile( group_p, label );

						if( level > profile_p->level )	{
							profile_p->level = level;
						}
						if( group_p->open ) {  // continuation
							make_mark( group_p, curr_time );
						}
						else	{  // new segment
							group_p->open = true;
						}

						group_p->curr_profile_p = profile_p;
						group_p->curr_profile_time = curr_time;
					}
				}
			}
		}

		int mark( const char* group_name, double curr_time )	{

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
			mark( group_name, level, label, SBM_get_real_time() );
		}
		int mark( const char* group_name )	{
			return( mark( group_name, SBM_get_real_time() ) );
		}

};

////////////////////////////////////////////////////////////////////////////////////////
#endif
