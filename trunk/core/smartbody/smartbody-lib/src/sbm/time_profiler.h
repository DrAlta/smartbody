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

#define DEFAULT_REQ_ENABLED			false
#define DEFAULT_REQ_GROUP_ENABLED	true

#define DEFAULT_THRESHOLD		10.0
#define DEFAULT_SMOOTHING		0.99

////////////////////////////////////////////////////////////////////////////////////////

class TimeIntervalProfiler { // T.I.P.

		enum time_profiler_enum_set	{
		
			LABEL_SIZE =	8192,
			MAX_GROUPS =	256,
			MAX_PROFILES =	256,
			MAX_ROLLING = 	256
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

			bool	reset;
		} profile_entry_t;

		typedef struct group_entry_s {

			bool	req_enable;
			bool	enabled;
			char	name[ LABEL_SIZE ];
			bool 	open;

			double  frame_dt;
			double	decay_dt;
			double	rolling_dt;

			srHashMap <profile_entry_t> profile_map;
			profile_entry_t* profile_p_arr[ MAX_PROFILES ];
			
			int 	profile_arr_count;
			int 	profile_count;

			profile_entry_t* curr_profile_p;
			double	curr_profile_time;
			int 	profile_event_count;

		} group_entry_t;


		srHashMap <group_entry_t> group_map;
		group_entry_t* group_p_arr[ MAX_GROUPS ];
		
		int 	group_arr_count;
		int 	group_count;

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
		
		void null( void )	{
			group_arr_count = 0;
			group_count = 0;
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
			profile_p->reset = false;
		}

		void reset_profile( profile_entry_t* profile_p ) {
		
			profile_p->level = -1;
			profile_p->prev_frame_dt = profile_p->frame_dt;
			profile_p->frame_dt = 0.0;
			profile_p->frame_count = 0;
			profile_p->max_dt = 0.0;
			profile_p->reset = false;
		}

		void null_group( group_entry_t* group_p, const char* group_name = "" ) {
		
			group_p->req_enable = DEFAULT_REQ_GROUP_ENABLED;
			group_p->enabled = false;
			_snprintf( group_p->name, LABEL_SIZE, "%s", group_name );
			group_p->open = false;
			group_p->frame_dt = 0.0;
			group_p->decay_dt = 0.0;
			group_p->rolling_dt = 0.0;
			group_p->profile_map.expunge();
			group_p->profile_arr_count = 0;
			group_p->profile_count = 0;
			group_p->curr_profile_p = NULL;
			group_p->curr_profile_time = 0.0;
			group_p->profile_event_count = 0;
		}

///////////////////////////////////////////////////

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

		void print_group( group_entry_t *group_p )	{

			if( group_p )	{
				printf( "  group: %s %s\n", group_p->name, group_p->enabled ? "ENABLED" : "DISABLED" );
				for( int i=0; i< group_p->profile_arr_count; i++ )	{
					profile_entry_t* profile_p = group_p->profile_p_arr[ i ];
					if( profile_p ) {
						printf( "    prof <%d>: %s\n", profile_p->level, profile_p->label );
					}
				}
			}
		}
		void print_data( void )	{
		
			printf( "TIP <>: %s\n", enabled ? "ENABLED" : "DISABLED" );
//			printf( "   dynamic: %s\n", dyn_threshold ? "true" : "false" );
			printf( "  suppress: %d\n", suppression );
			printf( "    select: %d\n", selection );
			printf( "    smooth: %f\n", smooth_factor );
			printf( " threshold: %f\n", threshold );
			printf( "    groups: %d\n", group_arr_count );
			printf( "--\n" );
			for( int i=0; i< group_arr_count; i++ )	{
				group_entry_t *group_p = group_p_arr[ i ];
				if( group_p )	{
					print_group( group_p );
				}
			}
		}
		void print_profile_alert( double reset_dt, group_entry_t* group_p, profile_entry_t *profile_p )	{
			
			printf( 
				"TIP <%d>: [F:%.2f%%, G:%.2f%%] ",
				profile_p->level,
				100.0 * profile_p->frame_dt / reset_dt,
				100.0 * profile_p->frame_dt / group_p->frame_dt
			);
			if( profile_p->frame_count > 1 )	{
				printf( 
					"[Max(%d):%.2f%%] ",
					profile_p->frame_count,
					100.0 * profile_p->max_dt / profile_p->frame_dt
				);
			}
			printf( "Dt:%.5f %s:%s\n", 
				profile_p->frame_dt,
				group_p->name, 
				profile_p->label 
			);
		}
		void print_group_alert( double reset_dt, group_entry_t* group_p )	{
		
			printf( 
				"TIP <>: %s [F:%.2f%%] (Ra:%.5f, Da:%.5f) Dt:%.5f\n",
				group_p->name,
				100.0 * group_p->frame_dt / reset_dt,
				group_p->rolling_dt,
				group_p->decay_dt,
				group_p->frame_dt
			);
		}
		void print_profile_report( char *prefix, profile_entry_t *profile_p )	{
		
			printf( 
				"%s   <%d>: (Ra:%.5f, Da:%.5f) ",
				prefix,
				profile_p->level,
				profile_p->rolling_dt,
				profile_p->decay_dt,
				profile_p->frame_count
			);
			if( profile_p->frame_count > 1 )	{
				printf( 
					"[Mx(%d):%.2f%%] ",
					100.0 * profile_p->max_dt / profile_p->frame_dt
				);
			}
			printf( 
				"Dt:%.5f %s\n",
				profile_p->frame_dt,
				profile_p->label
			);
		}
		void print_group_report( const char *prefix, group_entry_t* group_p )	{
		
			printf( 
				"%s SUM: (Ra:%.5f, Da:%.5f) Dt:%.5f\n",
				prefix,
				group_p->rolling_dt,
				group_p->decay_dt,
				group_p->frame_dt
			);
		}
		
///////////////////////////////////////////////////

		void accum_profile( profile_entry_t *profile_p )	{

			profile_p->sub_frame_dt = profile_p->frame_dt / (double) profile_p->frame_count;
			
			if( profile_p->decay_dt == 0.0 )	{
				profile_p->decay_dt = profile_p->frame_dt;
			}
			else	{
				profile_p->decay_dt =
					smooth_factor * profile_p->decay_dt +
					( 1.0 - smooth_factor ) * profile_p->frame_dt;
			}
			
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

		bool check_profile_spike( profile_entry_t *profile_p )	{

#if 0
			// harsh
			if(
				( profile_p->frame_count > 1 )&&
				( profile_p->max_dt > ( profile_p->prev_frame_dt * threshold ) )
			)	{
				return( true );
			}
#endif
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

///////////////////////////////////////////////////

		void reset( double time ) {

			double prev_time = reset_time;
			reset_time = time;
			reset_dt = time - prev_time;

			if( req_print ) {
				print_data();
				req_print = false;
			}

		// Report Previous
			if( reporting )  {

				printf( "TIP <> report:\n" );
				printf( "PREVIOUS:\n" );
				for( int i=group_count; i< group_arr_count; i++ ) {

					group_entry_t *group_p = group_p_arr[ i ];
					printf( "  GRP: %s: %s\n", group_p->enabled ? "ENABLED" : "DISABLED", group_p->name );

					for( int j=group_p->profile_count; j< group_p->profile_arr_count; j++ ) {

						profile_entry_t *profile_p = group_p->profile_p_arr[ j ];
						if( profile_p->frame_count )	{
							print_profile_report( " ", profile_p );
						}
					}
					print_group_report( " ", group_p );
				}
			}

			if( enabled )	{

			// Accumulate:
				for( int i=0; i< group_count; i++ ) {

					group_entry_t *group_p = group_p_arr[ i ];
					if( group_p == NULL ) {
						std::cout 
							<< "TimeIntervalProfiler::reset ERR: NULL group_p in group_p_arr[ " 
							<< i << "]" 
							<< std::endl;
						return;
					}

					if( group_p->enabled )	{

						group_p->frame_dt = 0.0;
						group_p->decay_dt = 0.0;
						group_p->rolling_dt = 0.0;

						for( int j=0; j< group_p->profile_count; j++ ) {

							profile_entry_t *profile_p = group_p->profile_p_arr[ j ];
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

								if( profile_p->frame_dt < 0.0 )    { 
									std::cout 
										<< "TimeIntervalProfiler::reset WARN: negative dt: %f" 
										<< profile_p->frame_dt
										<< std::endl;
								}
								accum_profile( profile_p );
								group_p->frame_dt += profile_p->frame_dt;
								group_p->decay_dt += profile_p->decay_dt;
								group_p->rolling_dt += profile_p->rolling_dt;
							}
						}
					}
				}

			// Report Current:
				if( reporting )	{
					printf( "CURRENT:\n" );
				}
				for( int i=0; i< group_count; i++ ) {

					group_entry_t *group_p = group_p_arr[ i ];
					if( group_p->enabled )	{

						if( reporting )	{
							printf( "  GRP: %s\n", group_p->name );
						}
						bool spike = false;
						int spike_count = 0;

						for( int j=0; j< group_p->profile_count; j++ ) {

							profile_entry_t *profile_p = group_p->profile_p_arr[ j ];
							if( profile_p->frame_count )	{

								spike = check_profile_spike( profile_p );
								if( spike ) spike_count++;
								if( reporting )	{
									if( spike )	{
										print_profile_report( "*", profile_p );
									}
									else	{
										print_profile_report( " ", profile_p );
									}
								}
								else
								if( spike )	{
									if( check_profile_show( profile_p->level  ) )	{
										print_profile_alert( reset_dt, group_p, profile_p );
									}
								}
							}
						}
						spike = false;
						if( group_p->frame_dt > ( group_p->decay_dt * threshold ) )	{
							spike = true;
						}
						else
						if( group_p->frame_dt > ( group_p->rolling_dt * threshold ) )	{
							spike = true;
						}
						if( reporting )	{
							if( spike ) {
								print_group_report( "*", group_p );
							}
							else	{
								print_group_report( " ", group_p );
							}
						}
						else
						if( spike || ( spike_count > 0 ) )	{
							print_group_alert( reset_dt, group_p );
						}
					}
					
					group_p->profile_count = 0;
					group_p->profile_event_count = 0;
				}
				for( int i=0; i< group_arr_count; i++ ) {
					group_entry_t *group_p = group_p_arr[ i ];
					if( group_p->req_enable )	{
						group_p->enabled = true;
						group_p->req_enable = false;
					}
					for( int j=0; j< group_p->profile_arr_count; j++ ) {
						profile_entry_t *profile_p = group_p->profile_p_arr[ j ];
						profile_p->reset = true;
					}
				}
				group_count = 0;
			}

		// Wrapup:
			if( reporting )	{
				reporting = false;
			}
			if( req_enable )	{
				enabled = true;
				req_enable = false;
			}
			if( req_erase ) {
				group_map.reset();
				group_entry_t *group_p;
				while( ( group_p = group_map.next() ) != NULL ) {
					null_group( group_p, group_p->name );
				}
				req_erase = false;
			}
			if( req_clobber ) {
				group_map.expunge();
				group_arr_count = 0;
				group_count = 0;
				req_enable = false;
				reporting = false;
				req_clobber = false;
			}
		}
		void reset( void ) {
			reset( SBM_get_real_time() );
		}

///////////////////////////////////////////////////

		group_entry_t* get_group( const char* group_name ) {

			group_entry_t *group_p = group_map.lookup( group_name );
			if( group_p == NULL ) {

				group_p = new group_entry_t;
				null_group( group_p, group_name );
				group_map.insert( group_name, group_p, true ); // delete upon destructor...

				if( group_arr_count < MAX_GROUPS )	{
					group_p_arr[ group_arr_count ] = group_p;
					group_arr_count++;
				}
				else	{
					std::cout << "TimeIntervalProfiler::get_group ERR: MAX_GROUPS: " << MAX_GROUPS << std::endl;
				}
			}
			return( group_p );
		}

		profile_entry_t* get_profile( group_entry_t *group_p, const char* label ) {

			profile_entry_t *profile_p = group_p->profile_map.lookup( label );
			if( profile_p == NULL ) {

				profile_p = new profile_entry_t;
				null_profile( profile_p, label );
				group_p->profile_map.insert( label, profile_p, true ); // delete upon destructor...

				if( group_p->profile_arr_count < MAX_PROFILES ) {
					group_p->profile_p_arr[ group_p->profile_arr_count ] = profile_p;
					group_p->profile_arr_count++;
				}
				else	{
					std::cout << "TimeIntervalProfiler::get_profile ERR: MAX_PROFILES: " << MAX_PROFILES << std::endl;
				}
			}
			if( profile_p->reset ) {
				reset_profile( profile_p );
			}
			return( profile_p );
		}

		void make_mark( group_entry_t *group_p, double curr_time )	{

			profile_entry_t *profile_p = group_p->curr_profile_p;
			if( profile_p )	{

				if( group_p->profile_event_count == 0 ) {
					if( group_p->profile_count < MAX_PROFILES ) {
						group_p->profile_count++;
					}
					if( group_count < MAX_GROUPS )	{
						group_count++;
					}
				}

				double dt = curr_time - group_p->curr_profile_time;
				if( dt < 0.0 )    { 
					std::cout 
						<< "TimeIntervalProfiler::make_mark WARN: negative dt: %f" 
						<< profile_p->frame_dt
						<< std::endl;
				}
				
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
		
///////////////////////////////////////////////////

		static double test_clock( int reps = 0 )	{

			if( reps < 1 ) reps = 1000000;

			double *time_hist_arr = new double[ reps ];
    		double *time_hist_end_p = time_hist_arr + reps;
			
    		int countdown = reps;
    		double start = SBM_get_real_time();
    		while( countdown )    {
        		*( time_hist_end_p - ( countdown-- ) ) = SBM_get_real_time();
    		}
			
			double *dt_hist_arr = new double[ reps ];
			double t = start;
			double prev = 0.0;
			for( int i=0; i<reps; i++ )	{
				prev = t;
				t = time_hist_arr[ i ];
				dt_hist_arr[ i ] = t - prev;
			}
			delete [] time_hist_arr;
			double total = t - start;
			double avg = total / (double)reps;

			int negs = 0;
			int hits = 0;
			int run = 0;
			int run_min = 999999999;
			int run_max = 0;
			int accum_run = 0;

			int halt = 0;
			int halt_min = 999999999;
			int halt_max = 0;
			int accum_halt = 0;

			double min = 999999999.0;
			double max = 0.0;
			double accum_dev = 0.0;
			for( int i=0; i<reps; i++ )	{

				double dt = dt_hist_arr[ i ];

#if 0
				if( i % 10 == 0 ) dt = 0.0;
#endif
#if 0
				if( ( (double)rand() / (double)RAND_MAX ) > 0.5 ) dt = 0.0;
#endif

				if( min > dt ) min = dt;
				if( max < dt ) max = dt;
				accum_dev += abs( dt - avg );

				if( dt > 0.0 )	{
					
					if( halt )	{
						if( halt_min > halt ) halt_min = halt;
						if( halt_max < halt ) halt_max = halt;
						accum_halt += halt;
						halt = 0;
					}
					run++;
					hits++;
				}
				else	{

					if( dt < 0.0 )	{
						negs++;
					}
					if( run )	{
						if( run_min > run ) run_min = run;
						if( run_max < run ) run_max = run;
						accum_run += run;
						run = 0;
					}
					halt++;
				}
			}

			if( run )	{
				if( run_min > run ) run_min = run;
				if( run_max < run ) run_max = run;
				accum_run += run;
			}
			if( halt )	{
				if( halt_min > halt ) halt_min = halt;
				if( halt_max < halt ) halt_max = halt;
				accum_halt += halt;
			}
			double dev = accum_dev / (double)reps;
			double hit_ratio = (double)hits / (double)reps;
			double run_avg = (double)accum_run / (double)reps;
			double halt_avg = (double)accum_halt / (double)reps;

#if 0
			for( int i=0; i<reps; i++ )	{

				double dt = dt_hist_arr[ i ];

				// deviation in run/halt length
			}
#endif
			delete [] dt_hist_arr;

			printf( "test_clock: %d reps\n", reps );
			printf( " Tot: %.6f\n", total );
			printf( "--\n" );

			printf( " avg: %.12f\n", avg );
			if( min < max ) {
				if( min == 0.0 )	{
					printf( " min: <zero>\n" );
				}
				else	{
					printf( " min: %.12f\n", min );
				}
				printf( " max: %.12f\n", max );
			}
			printf( " dev: %.12f\n", dev );
			if( negs )	{
				printf( " neg: %d\n", negs );
			}
			printf( "--\n" );

			printf( " fps: %.2f\n", 1.0 / avg );
			if( max > 0.0 )
				printf( " min: %.2f\n", 1.0 / max );
			else
				printf( " min: <inf>\n" );
			if( min > 0.0 )
				printf( " max: %.2f\n", 1.0 / min );
			else
				printf( " max: <inf>\n" );
			printf( "--\n" );

			printf( " hit: %.6f\n", hit_ratio );
			printf( " avg: %.2f\n", run_avg );
			if( run_min < run_max ) {
				printf( " min: %d\n", run_min );
				printf( " max: %d\n", run_max );
			}
			printf( "--\n" );

			printf( " mis: %d\n", accum_halt );
			printf( " avg: %.2f\n", halt_avg );
			if( halt_min < halt_max ) {
				printf( " min: %d\n", halt_min );
				printf( " max: %d\n", halt_max );
			}
			printf( "--\n" );

			double resolution = avg / hit_ratio;
			printf( " Res: %.12f == %.2f fps\n", resolution, 1.0 / resolution );
			printf( "--\n" );

			return( resolution );
		}
};

////////////////////////////////////////////////////////////////////////////////////////
#endif
