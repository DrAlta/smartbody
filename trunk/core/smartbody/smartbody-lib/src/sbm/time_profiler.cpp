/*
 *  time_profiler.cpp - part of SmartBody-lib
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

#include <math.h>
#include "sbm/time_profiler.h"

////////////////////////////////////////////////////////////////////////////////////////

void TimeIntervalProfiler::null( void )	{

	group_map.expunge();
	for( int i=0; i<MAX_GROUPS; i++ ) group_p_arr[ i ] = NULL;
	full_err = false;
	group_arr_count = 0;
	active_group_count = 0;
	group_event_count = 0;
	sys_bypass = false;
	dis_sys_bypass = false;
	enabled = false;
	preloading = false;
	update_time = 0.0;
	update_dt = 0.0;
	req_print = false;
	req_report = false;
	req_erase = false;
	req_clobber = false;
	req_enable = false;
	req_disable = false;
	req_preload = false;
	pending_request = false;
	abs_threshold = 0.0;
	rel_threshold = 0.0;
	dyn_abs_thr = false;
	dyn_rel_thr = false;
	dyn_sniff = 1.0;
	dyn_avoid = 1.0;
	decaying_factor = 1.0;
//	rolling_length = 1;
	suppression = -1;
	selection = -1;
}

void TimeIntervalProfiler::null_group( group_entry_t* group_p, const char* group_name ) {

	group_p->req_enable = false;
	group_p->req_disable = false;
	group_p->enabled = false;
	_snprintf( group_p->name, LABEL_SIZE, "%s", group_name );
	group_p->open = false;
	group_p->open_err = false;
	group_p->req_preload = false;
	group_p->preloading = false;
	group_p->spike = false;
	group_p->req_reset = false;		
	group_p->interval_dt = 0.0;
	group_p->decay_dt = 0.0;
	group_p->roll_dt = 0.0;
	group_p->profile_map.expunge();
	for( int i=0; i<MAX_PROFILES; i++ ) group_p->profile_p_arr[ i ] = NULL;
	group_p->full_err = false;
	group_p->profile_arr_count = 0;
	group_p->active_profile_count = 0;
	group_p->profile_event_count = 0;
	group_p->curr_profile_p = NULL;
}

void TimeIntervalProfiler::null_profile( profile_entry_t* profile_p, const char* label ) {

	profile_p->level = -1;
	_snprintf( profile_p->label, LABEL_SIZE, "%s", label );
	profile_p->spike = false;
	profile_p->req_reset = false;
	profile_p->event_time = 0.0;
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
}

void TimeIntervalProfiler::reset_group( group_entry_t* group_p ) {

	group_p->spike = false;
	group_p->req_reset = false;
	group_p->interval_dt = 0.0;
	group_p->decay_dt = 0.0;
	group_p->roll_dt = 0.0;
	group_p->active_profile_count = 0;
	group_p->profile_event_count = 0;
}

void TimeIntervalProfiler::reset_profile( profile_entry_t* profile_p ) {

	profile_p->level = -1;
	profile_p->event_time = 0.0;
	profile_p->spike = false;
	profile_p->req_reset = false;
	profile_p->prev_dt = profile_p->interval_dt;
	profile_p->interval_dt = 0.0;
	profile_p->intra_count = 0;
	profile_p->max_intra_dt = 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////

void TimeIntervalProfiler::print_legend( void )	{
	
	printf( "TIP <> legend:" );								printf( "\n" );
	printf( "@	- profile Active" );						printf( "\n" );
	printf( "*	- *spike reported in active profile" );		printf( "\n" );
	printf( "<>	- priority level" );						printf( "\n" );
	printf( "()	- (2)==(number of items, or items)" );		printf( "\n" );
	printf( "[]	- percent" );					printf( "\n" );
	printf( "e	- Event count" );				printf( "\n" );
	printf( "a	- Active count" );				printf( "\n" );
	printf( "c	- array Count" );				printf( "\n" );
	printf( "h	- Hash count" );				printf( "\n" );
	printf( "T	- event Time" );				printf( "\n" );
	printf( "i	- raw Interval" );				printf( "\n" );
	printf( "R	- Rolling average" );			printf( "\n" );
	printf( "D	- Decaying average" );			printf( "\n" );
	printf( "F	- %% of whole Frame taken up by interval or group" );	printf( "\n" );
	printf( "G	- %% of whole Group time taken up by interval" );		printf( "\n" );
	printf( "M(rep)	- %% of interval taken up by Maximum repetition" );	printf( "\n" );
	printf( "--tip\n" );
}

void TimeIntervalProfiler::print_group( group_entry_t *group_p )	{

	if( group_p )	{

		printf( "  GRP: %s (e:%d, a:%d, c:%d, h:%d) :%s\n",
			group_p->name,
			group_p->profile_event_count,
			group_p->active_profile_count,
			group_p->profile_arr_count,
			group_p->profile_map.get_num_entries(),
			group_p->enabled ? "ENABLED" : "DISABLED"
		);
		for( int i=0; i< group_p->profile_arr_count; i++ )	{

			profile_entry_t* profile_p = group_p->profile_p_arr[ i ];
			if( profile_p ) {

				printf( "%s   <%d>: T:%.4f %s",
					( profile_p->req_reset == false ) ? "@" : " ",
					profile_p->level,
					profile_p->event_time,
					profile_p->label
				);
				if( profile_p->intra_count > 1 )	{
					printf( " (e:%d)", profile_p->intra_count );
				}
				printf( "\n" );
			}
		}
	}
}

void TimeIntervalProfiler::print_data( void )	{

	printf( "TIP <>: %s\n", enabled ? "ENABLED" : "DISABLED" );
	printf( "  suppress: %d\n", suppression );
	printf( "    select: %d\n", selection );
	printf( "  averages:\n" );
	printf( "   decaying: %f\n", decaying_factor );
	printf( "    rolling: %f\n", MAX_ROLLING );
	printf( " threshold:\n" );
	printf( "        abs: %f :%s\n", abs_threshold, dyn_abs_thr ? "dyn" : "fix" );
	printf( "        rel: %f :%s\n", rel_threshold, dyn_rel_thr ? "dyn" : "fix" );
	printf( "   dynamic:\n" );
	printf( "      sniff: %f\n", dyn_sniff );
	printf( "      avoid: %f\n", dyn_avoid );
	printf( "--\n" );
	printf( "TIP groups (e:%d, a:%d, c:%d, h:%d):\n", 
		group_event_count,
		active_group_count,
		group_arr_count,
		group_map.get_num_entries()
	);
	for( int i=0; i< group_arr_count; i++ )	{
		group_entry_t *group_p = group_p_arr[ i ];
		if( group_p )	{
			print_group( group_p );
		}
	}
	printf( "--tip\n" );
}

void TimeIntervalProfiler::print_profile_report( char *prefix, profile_entry_t *profile_p )	{

	printf( 
		"%s   <%d>: i:%.5f (R:%.5f, D:%.5f) ",
		prefix,
		profile_p->level,
		profile_p->interval_dt,
		profile_p->roll_dt,
		profile_p->decay_dt
	);
	if( profile_p->intra_count > 1 )	{
		printf( 
			"[M(%d):%.1f] ",
			profile_p->intra_count,
			100.0 * profile_p->max_intra_dt / profile_p->interval_dt
		);
	}
	printf( "%s ", profile_p->label );
	if( profile_p->spike ) {
			printf( "T:%.4f", profile_p->event_time );
	}
	printf( "\n" );
}

void TimeIntervalProfiler::print_group_report( const char *prefix, group_entry_t* group_p )	{

	printf( 
		"%s   SUM: i:%.5f (R:%.5f, D:%.5f)\n",
		prefix,
		group_p->interval_dt,
		group_p->roll_dt,
		group_p->decay_dt
	);
}

void TimeIntervalProfiler::print_profile_alert( double dt, group_entry_t* group_p, profile_entry_t *profile_p )	{
	
	printf( 
		"TIP <%d>: i:%.5f [F:%.3f, G:%.1f",
		profile_p->level,
		profile_p->interval_dt,
		100.0 * profile_p->interval_dt / dt,
		100.0 * profile_p->interval_dt / group_p->interval_dt
	);
	if( profile_p->intra_count > 1 )	{
		printf( 
			", M(%d):%.1f",
			profile_p->intra_count,
			100.0 * profile_p->max_intra_dt / profile_p->interval_dt
		);
	}
	printf( "] %s:%s\n", 
		group_p->name, 
		profile_p->label 
	);
}
void TimeIntervalProfiler::print_group_alert( const char *prefix, double dt, group_entry_t* group_p )	{

	printf( 
		"TIP %s: i:%.5f [F:%.3f] (R:%.5f, D:%.5f) %s (a:%d)\n",
		prefix,
		group_p->interval_dt,
		100.0 * group_p->interval_dt / dt,
		group_p->roll_dt,
		group_p->decay_dt,
		group_p->name,
		group_p->active_profile_count
	);
}

///////////////////////////////////////////////////

void TimeIntervalProfiler::accum_profile( profile_entry_t *profile_p )	{

	profile_p->avg_intra_dt = profile_p->interval_dt / (double) profile_p->intra_count;
	
	if( profile_p->decay_dt == 0.0 )	{
		profile_p->decay_dt = profile_p->interval_dt;
	}
	else	{
		profile_p->decay_dt =
			decaying_factor * profile_p->decay_dt +
			( 1.0 - decaying_factor ) * profile_p->interval_dt;
	}
	
	profile_p->accum_roll_dt += profile_p->interval_dt;
	if( profile_p->accum_count >= MAX_ROLLING )	{
		profile_p->accum_roll_dt -= profile_p->roll_dt_arr[ profile_p->roll_index ];
	}
	else	{
		profile_p->accum_count++;
	}
	
	profile_p->roll_dt_arr[ profile_p->roll_index ] = profile_p->interval_dt;
	profile_p->roll_index++;
	if( profile_p->roll_index >= MAX_ROLLING )	{
		profile_p->roll_index = 0;
	}
	
	profile_p->roll_dt = profile_p->accum_roll_dt / (double)profile_p->accum_count;
}

bool TimeIntervalProfiler::check_profile_spike( profile_entry_t *profile_p )	{

	// absolute 
	if( abs_threshold > 0.0 )	{
		if( profile_p->interval_dt > abs_threshold )	{
			return( true );
		}
	}
	// harsh
#if 0
	if(
		( profile_p->intra_count > 1 )&&
		( profile_p->max_intra_dt > ( profile_p->prev_dt * rel_threshold ) )
	)	{
		return( true );
	}
#endif
	// smooth harsh
	if( profile_p->interval_dt > ( profile_p->decay_dt * rel_threshold ) )	{
		return( true );
	}
	// fair
	if( profile_p->interval_dt > ( profile_p->roll_dt * rel_threshold ) )	{
		return( true );
	}
	return( false );
}

bool TimeIntervalProfiler::check_group_spike( group_entry_t* group_p )  {

	if( abs_threshold > 0.0 )	{
		if( group_p->interval_dt > abs_threshold )	{
			return( true );
		}
	}
	if( group_p->interval_dt > ( group_p->decay_dt * rel_threshold ) )	{
			return( true );
	}
	if( group_p->interval_dt > ( group_p->roll_dt * rel_threshold ) )	{
			return( true );
	}
	return( false );
}

bool TimeIntervalProfiler::check_profile_show( int level )	{

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

void TimeIntervalProfiler::sys_update( double time ) {

	double prev = update_time;
	update_dt = time - prev;
	if( update_dt > 0.0 )	{
		update_time += update_dt;
	}
	else	{
		printf( "TimeIntervalProfiler::sys_update WARN: negative input dt: %f\n", update_dt );
		return;
	}

	if( enabled )	{

	// Check arrays:
		for( int i=0; i< group_arr_count; i++ ) {

			group_entry_t *group_p = group_p_arr[ i ];
			if( group_p == NULL ) {
				printf( "TimeIntervalProfiler::sys_update ERR: NULL group_p in group_p_arr[ %d ]\n", i );
				return;
			}
			for( int j=0; j< group_p->profile_arr_count; j++ ) {

				profile_entry_t *profile_p = group_p->profile_p_arr[ j ];
				if( profile_p == NULL ) {
					printf( "TimeIntervalProfiler::sys_update ERR: NULL profile_p in group_p_arr[ %d ][ %d ]\n", i, j );
					return;
				}
			}
		}
		
	// Close hanging intervals:
		group_entry_t *group_p;
		group_map.reset();
		while( ( group_p = group_map.next() ) != NULL ) {
			if( group_p->enabled )	{
				if( group_p->open ) {  // close...
					if( group_p->open_err == false ) {
						printf( "TIP WARN: dangling label: %s:%s\n", group_p->name, group_p->curr_profile_p->label );
						group_p->open_err = true;
					}
					mark_time( group_p->name, time );
				}
			}
		}
	}

	if( pending_request )	{
		if( req_print ) {
			print_data();
			req_print = false;
		}

	// Report Previous
		if( req_report )  {

			printf( "TIP <> report:\n" );
			printf( "RECENT GROUPS:\n" );
			for( int i=0; i< group_arr_count; i++ ) {

				group_entry_t *group_p = group_p_arr[ i ];
				if( group_p->req_reset == true ) {

					printf( "  GRP: %s (e:%d, a:%d) :%s\n",
						group_p->name,
						group_p->profile_event_count,
						group_p->active_profile_count,
						group_p->enabled ? "ENABLED" : "DISABLED"
					);
					for( int j=0; j< group_p->profile_arr_count; j++ ) {

						profile_entry_t *profile_p = group_p->profile_p_arr[ j ];
						if( profile_p->intra_count > 0 )	{

							print_profile_report( " ", profile_p );
						}
					}
					print_group_report( " ", group_p );
				}
			}

			printf( "RECENT PROFILES:\n" );
			for( int i=0; i< group_arr_count; i++ ) {

				group_entry_t *group_p = group_p_arr[ i ];
				if( group_p->req_reset == false ) {

					bool found_prev = false;
					for( int j=0; !found_prev &&( j< group_p->profile_arr_count ); j++ ) {
						if( group_p->profile_p_arr[ j ]->req_reset == true ) found_prev = true;
					}
					if( found_prev )	{

						printf( "  GRP: %s (e:%d, a:%d) :%s\n",
							group_p->name,
							group_p->profile_event_count,
							group_p->active_profile_count,
							group_p->enabled ? "ENABLED" : "DISABLED"
						);
						for( int j=0; j< group_p->profile_arr_count; j++ ) {

							profile_entry_t *profile_p = group_p->profile_p_arr[ j ];
							if( profile_p->req_reset == true )	{

								if( profile_p->intra_count > 0 )	{
									print_profile_report( " ", profile_p );
								}
							}
						}
						print_group_report( " ", group_p );
					}
				}
			}
		}
	}
	
	if( enabled )	{

	// Accumulate:
		group_entry_t *group_p;
		group_map.reset();
		while( ( group_p = group_map.next() ) != NULL ) {

			if( group_p->enabled )	{
				if( group_p->req_reset == false ) {

					profile_entry_t *profile_p;
					group_p->profile_map.reset();
					while( ( profile_p = group_p->profile_map.next() ) != NULL ) {

						if( profile_p->req_reset == false )	{
							if( profile_p->intra_count > 0 )	{

								if( profile_p->interval_dt < 0.0 )    {
									printf( "TimeIntervalProfiler::sys_update WARN: negative dt: %f\n", profile_p->interval_dt );
								}
								accum_profile( profile_p );
								group_p->interval_dt += profile_p->interval_dt;
								group_p->decay_dt += profile_p->decay_dt;
								group_p->roll_dt += profile_p->roll_dt;
							}
						}
					}
				}
			}
		}

	// Report Current:
		int total_spike_count = 0;
		if( req_report )	{
			printf( "ACTIVE: (e:%d, a:%d)\n",
				group_event_count,
				active_group_count
			);
		}
		for( int i=0; i< group_arr_count; i++ ) {

			int group_spike_count = 0;
			group_entry_t *group_p = group_p_arr[ i ];
			if( group_p->enabled )	{
				if( group_p->req_reset == false ) {

					if( req_report )	{
						printf( "  GRP: %s (e:%d, a:%d)\n",
							group_p->name,
							group_p->profile_event_count,
							group_p->active_profile_count
						);
					}
					for( int j=0; j< group_p->profile_arr_count; j++ ) {

						profile_entry_t *profile_p = group_p->profile_p_arr[ j ];
						if( profile_p->req_reset == false )	{
							if( profile_p->intra_count > 0 )	{

								profile_p->spike = check_profile_spike( profile_p );
								if( req_report )	{
									if( profile_p->spike )	{
										group_spike_count++;
										print_profile_report( "*", profile_p );
									}
									else	{
										print_profile_report( " ", profile_p );
									}
								}
								else
								if( profile_p->spike )	{
									if( check_profile_show( profile_p->level  ) )	{
										group_spike_count++;
										print_profile_alert( update_dt, group_p, profile_p );
									}
								}
							}
						}
					}
					group_p->spike = check_group_spike( group_p );
					if( req_report )	{
						if( group_p->spike ) {
							if( group_spike_count == 0 )	{
								total_spike_count++;
							}
							print_group_report( "*", group_p );
						}
						else	{
							print_group_report( " ", group_p );
						}
					}
					else
					if( group_spike_count > 0 )	{
						print_group_alert( "SUM", update_dt, group_p );
					}
					else
					if( group_p->spike )	{
						print_group_alert( "GRP", update_dt, group_p );
						total_spike_count++;
					}
				}
			}
			if( group_p->req_enable )	{
				group_p->enabled = true;
				group_p->req_enable = false;
			}
			if( group_p->req_disable )	{
				group_p->enabled = false;
				group_p->req_disable = false;
			}
			group_p->preloading = false;
			if( group_p->req_preload )	{
				group_p->preloading = true;
			}
			total_spike_count += group_spike_count;
		}
		if( req_report )	{
			printf( "--tip\n" );
		}
		if( dyn_abs_thr || dyn_rel_thr )	{
		
			double avoid = 1.0;
			if( total_spike_count > 0 ) {
				avoid = dyn_avoid;
			}
			double clamp_dt = ( update_dt < MAX_SNIFF_DT ) ? update_dt : MAX_SNIFF_DT;
			double sniff = 1.0 - clamp_dt * ( 1.0 - dyn_sniff );

			if( dyn_abs_thr &&( abs_threshold > 0.0 ) ) {
				abs_threshold *= avoid * sniff;
			}
			if( dyn_rel_thr )	{
				rel_threshold *= avoid * sniff;
			}
		}

	// Reset:
		group_map.reset();
		while( ( group_p = group_map.next() ) != NULL ) {

			if( group_p->enabled )	{

				group_p->profile_map.reset();
				profile_entry_t *profile_p;
				while( ( profile_p = group_p->profile_map.next() ) != NULL ) {

					profile_p->req_reset = true;
				}
				group_p->req_reset = true;
			}
		}
		active_group_count = 0;
		group_event_count = 0;
	}

	if( pending_request )	{
	// Wrapup:
		req_report = false;
		if( req_enable )	{
			enabled = true;
			req_enable = false;
		}
		if( req_disable )	{
			enabled = false;
			req_disable = false;
		}
		if( req_erase ) {
			group_map.reset();
			group_entry_t *group_p;
			while( ( group_p = group_map.next() ) != NULL ) {
				null_group( group_p, group_p->name );
			}
			req_erase = false;
		}
		preloading = false;
		if( req_preload )	{
			preloading = true;
		}
		pending_request = false;
	}
}

///////////////////////////////////////////////////

TimeIntervalProfiler::group_entry_t* 
TimeIntervalProfiler::get_group( const char* group_name ) {

	group_entry_t *group_p = group_map.lookup( group_name );
	if( group_p == NULL ) {

		group_p = new group_entry_t;
		null_group( group_p, group_name );
		group_p->enabled = true;
		group_map.insert( group_name, group_p, true );

		if( group_arr_count < MAX_GROUPS )	{
			group_p_arr[ group_arr_count ] = group_p;
			group_arr_count++;
		}
		else
		if( full_err == false )	{
			full_err = true;
			printf( "TimeIntervalProfiler::get_group ERR: MAX_GROUPS: %d", MAX_GROUPS );
			printf( "\n" );
		}
	}
	else
	if( group_p->req_reset ) {
		reset_group( group_p );
	}
	return( group_p );
}

TimeIntervalProfiler::profile_entry_t* 
TimeIntervalProfiler::get_profile( group_entry_t *group_p, const char* label ) {

	profile_entry_t *profile_p = group_p->profile_map.lookup( label );
	if( profile_p == NULL ) {

		profile_p = new profile_entry_t;
		null_profile( profile_p, label );
		group_p->profile_map.insert( label, profile_p, true );

		if( group_p->profile_arr_count < MAX_PROFILES ) {
			group_p->profile_p_arr[ group_p->profile_arr_count ] = profile_p;
			group_p->profile_arr_count++;
		}
		else
		if( group_p->full_err == false )	{
			group_p->full_err = true;
			printf( "TimeIntervalProfiler::get_profile ERR: MAX_PROFILES: %d", MAX_PROFILES );
			printf( "\n" );
		}
	}
	else
	if( profile_p->req_reset ) {
		reset_profile( profile_p );
	}
	return( profile_p );
}

void TimeIntervalProfiler::accum_mark( group_entry_t *group_p, double curr_time )	{

	profile_entry_t *profile_p = group_p->curr_profile_p;
	if( profile_p )	{

		if( group_p->active_profile_count == 0 ) {
			active_group_count++;
		}
		if( profile_p->intra_count == 0 )   {
			group_p->active_profile_count++;
		}

		double dt = curr_time - profile_p->event_time;
		if( dt < 0.0 )    { 
			printf( "TimeIntervalProfiler::accum_mark WARN: negative dt: %f\n", dt );
		}
		if( dt > profile_p->max_intra_dt )	{
			profile_p->max_intra_dt = dt;
		}

		profile_p->interval_dt += dt;
		profile_p->intra_count++;

		group_p->profile_event_count++;
		group_event_count++;
		return;
	}
	printf( "TimeIntervalProfiler::accum_mark ERR: no current profile" );
}

///////////////////////////////////////////////////

double TimeIntervalProfiler::test_clock( int reps )	{

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
