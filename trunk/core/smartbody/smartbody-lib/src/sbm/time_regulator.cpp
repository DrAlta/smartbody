/*
 *  mcontrol_util.cpp - part of SmartBody-lib
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
#include "sbm/time_regulator.h"

#define WIN32_LEAN_AND_MEAN

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX        /* Don't defined min() and max() */
#endif
#include <windows.h>
#include <mmsystem.h>
#if SBM_REPORT_MEMORY_LEAKS
#include <malloc.h>
#include <crtdbg.h>
#endif
#endif

#ifdef WIN32_LEAN_AND_MEAN

#else
#include <sys/time.h>
#endif

#define ENABLE_QPF_TIME 	(1)

///////////////////////////////////////////////////////////////////////////////////

double SBM_get_real_time(void) {
#ifdef WIN32
#if ENABLE_QPF_TIME
	static int once = 1;
	static double inv_freq;
	static LONGLONG ref_quad;
	LARGE_INTEGER c;
	
	if( once )	{
		once = 0;
		LARGE_INTEGER f;
		QueryPerformanceFrequency( &f );
		inv_freq = 1.0 / (double)f.QuadPart;
		QueryPerformanceCounter( &c );
		ref_quad = c.QuadPart;
	}
	QueryPerformanceCounter( &c );
	LONGLONG diff_quad = c.QuadPart - ref_quad;
	if( diff_quad < 0 ) {
		diff_quad = 0;
	}
	return( (double)diff_quad * inv_freq );
#else
	return( (double)timeGetTime() / 1000.0 );
#endif
#else
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return( tv.tv_sec + ( tv.tv_usec / 1000000.0 ) );
#endif
}

void SBM_sleep_msec( int msec )	{
#ifdef WIN32
	static int once = 1;
	if( once )	{
		once = 0;
		timeBeginPeriod( 1 ); // millisecond resolution
	}
	Sleep( msec );
#else
	printf( "SBM_sleep_msec ERR: not implemented\n" );
#endif
}

double SBM_sleep_wait( double prev_time, double target_dt, bool verbose = false )	{ // sleep to reach target loop rate

	if( target_dt > 0.0 )	{
		
		int passes = 0;
		while( 1 )	{
		
			double curr_time = SBM_get_real_time();
			double elapse_dt = curr_time - prev_time;

			if( elapse_dt < target_dt )	{

				double diff = target_dt - elapse_dt;
				int wait_msec = (int)( diff * 1000.0 + 1.0 );
				
				if( wait_msec > 0 ) {
					if( verbose && passes )	{
						printf( "SBM_sleep_wait NOTICE: slipped %f seconds == %d msec\n", 
							diff, wait_msec 
						);
					}
					SBM_sleep_msec( wait_msec );
				}
				passes++;
			}
			else	{
				return( curr_time );
			}
		}
	}
	return( SBM_get_real_time() );
}

///////////////////////////////////////////////////////////////////////////////////

void TimeRegulator::start( double in_time ) {

	if( in_time < 0.0 )	{
		start_time = SBM_get_real_time();
		extern_src = false;
	}
	else	{
		start_time = in_time;
		extern_src = true;
	}
	
	clock_time = start_time;
	started = true;
}

bool TimeRegulator::update( double in_time ) {
	bool abort = false;
	
	if( !started ) start( in_time );
	
	if( in_time < 0.0 )	{
		if( extern_src )	{
			if( verbose ) printf( "TimeRegulator::update NOTICE: switch to internal\n" );
			start( in_time );
			abort = true;
		}
		clock_time = SBM_sleep_wait( clock_time, sleep_dt, verbose );
	}
	else	{
		if( !extern_src )	{
			if( verbose ) printf( "TimeRegulator::update NOTICE: switch to external\n" );
			start( in_time );
			abort = true;
		}
		clock_time = in_time;
	}

	double loop_time = clock_time - start_time;
	double loop_dt = loop_time - prev_loop_time;
	prev_loop_time = loop_time;

	if( !abort ) {
		if( loop_dt < 0.0 ) {
			if( extern_src )	{
				if( verbose ) printf( "TimeRegulator::update ERR: negative external increment: %f\n", loop_dt );
			}
			else
				printf( "TimeRegulator::update ERR: negative internal increment!!!!: %f\n", loop_dt );
			abort = true;
		}
		else
		if( loop_dt == 0.0 )	{
			if( extern_src )	{
				if( verbose ) printf( "TimeRegulator::update NOTICE: zero external increment\n" );
			}
			else
				printf( "TimeRegulator::update ERR: zero internal increment!!!!\n" );
			abort = true;
		}
	}
	if( abort ) {
		out_dt = 0.0;
		if( verbose ) printf( "TimeRegulator::update ABORT\n" );
		return( false );
	}
	
	bool eval_sim = false;
	eval_wait += loop_dt;

	if( eval_dt > 0.0 )	{
		if( eval_wait >= eval_dt )	{
			eval_sim = true;
			eval_wait = 0.0;
		}
	}
	else	{
		eval_sim = true;
	}

	real_time += loop_dt;

	if( eval_sim )	{

		real_dt = real_time - prev_real_time;
		prev_real_time = real_time;

		if( do_pause )	{
			if( verbose ) printf( "TimeRegulator::update PAUSE\n" );
			do_pause = false;
			paused = true;
		}
		if( do_steps )	{
			do_resume = true;
		}
		if( do_resume )	{
			if( verbose ) printf( "TimeRegulator::update RESUME\n" );
			do_resume = false;
			paused = false;
		}
		if( do_steps )	{
			if( verbose ) printf( "TimeRegulator::update STEP\n" );
			do_steps--;
			do_pause = true;
		}

		if( !paused )	{
			if( sim_dt > 0.0 )	{
				out_dt = sim_dt;
			}
			else	{
				out_dt = real_dt * speed;
			}
			out_time += out_dt;
			perf_update();
			return( true );
		}
	}
	
	out_dt = 0.0;
	if( verbose ) printf( "TimeRegulator::update SKIP\n" );
	return( false );
}

void TimeRegulator::perf_update( void )	{

	if( !perf_enabled ) return;

	perf_real_sum += real_dt;
	perf_sim_sum += sim_dt;
	perf_count++;

	if( perf_real_sum >= perf_interval )	{
		double avg = perf_real_sum / perf_count;

		if( perf_sim_sum > 0.0 ) {
			double sim_avg = perf_sim_sum / perf_count;
			printf( "PERF: REAL dt:%.3f fps:%.1f ~ SIM dt:%.3f fps:%.1f\n", 
//			printf( "PERF: REAL dt:%f fps:%f ~ SIM dt:%f fps:%f\n", 
				avg, 1.0 / avg,
				sim_avg, 1.0 / sim_avg
			);
		}
		else	{
			printf( "PERF: dt:%.3f fps:%.1f\n", avg, 1.0 / avg );
//			printf( "PERF: dt:%f fps:%f\n", avg, 1.0 / avg );
		}
		perf_real_sum = 0.0;
		perf_sim_sum = 0.0;
		perf_count = 0;
	}
}

void TimeRegulator::print( void )	{
	printf( "TimeRegulator( %.3f ): \n", real_time );
	printf( "   status: %s\n", paused ? "PAUSED" : ( do_steps ? "STEPPING" : "RUNNING" ) );
	printf( "    speed: %.3f\n", speed );
	printf( "    sleep: %.4f : %.2f fps\n", sleep_dt, ( sleep_dt > 0.0 )? ( 1.0 / sleep_dt ): 0.0 );
	printf( "     eval: %.4f : %.2f fps\n", eval_dt, ( eval_dt > 0.0 )? ( 1.0 / eval_dt ): 0.0 );
	printf( "      sim: %.4f : %.2f fps\n", sim_dt, ( sim_dt > 0.0 )? ( 1.0 / sim_dt ): 0.0 );
	printf( "   out dt: %.4f : %.2f fps\n", out_dt, ( out_dt > 0.0 )? ( 1.0 / out_dt ): 0.0 );
	printf( " out time: %.3f\n", out_time );
}
		
///////////////////////////////////////////////////////////////////////////////////

//#include <stdlib.h>

void test_time_regulator( void )	{
	TimeRegulator tc;
	bool res;
	
	tc.start();
	tc.set_verbose();
	tc.print();
	
	printf( "=====================================================\n" );

#if 0
	res = tc.update();
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update();
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update();
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update();
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update();
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	printf( "=====================================================\n" );
	
	res = tc.update( 1.0 );
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update( 1.1 );
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update( 2.0 );
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update( 2.2 );
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update( 2.2 );
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	printf( "=====================================================\n" );

	res = tc.update();
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update();
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update();
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update();
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update();
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	printf( "=====================================================\n" );
	
	res = tc.update( 1.0 );
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update( 2.0 );
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update( 3.0 );
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update( 0.0 );
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	res = tc.update( 4.0 );
	printf( "[%d]: %f : %f\n", (int)res, tc.get_time(), tc.get_dt() );

	printf( "=====================================================\n" );
	tc.print();
#endif
	
	tc.start();
//	tc.set_speed( 10.0 );
	tc.set_sleep_fps( 10.0 );
	tc.set_eval_fps( 10.0 );
	tc.set_sim_fps( 10.0 );

#if 0
	for( int i = 0; i<100; i++ )	{
		double f = (double)rand() / (double)RAND_MAX;
		res = tc.update( f );
		if( res )	{
			printf( "%f : %f : %f\n", f, tc.get_time(), tc.get_dt() );
		}
		else printf( "-\n" );
	}
	printf( "=====================================================\n" );
#endif

#if 0
	tc.set_perf( 1.0 );
	for( int i = 0; i<100; i++ )	{
		res = tc.update();
	}
	printf( "=====================================================\n" );
	tc.set_perf( 0.0 );
#endif

#if 1
	for( int i = 0; i<100; i++ )	{
		if( i == 10 ) tc.pause();
		if( i == 20 ) tc.step();
		if( i == 30 ) tc.resume();
//		if( i == 40 ) tc.pause();
		if( i == 50 ) tc.step( 10 );
		if( i == 70 ) tc.resume();

		tc.print_update( i );
	}
	printf( "=====================================================\n" );
#endif
}
















