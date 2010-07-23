#include "sbm/time_control.h"

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
#if 0
	Sleep( msec );
#else
	static int once = 1;
	if( once )	{
		once = 0;
		timeBeginPeriod( 1 ); // millisecond resolution
	}
	Sleep( msec );
#endif	
#else
	printf( "SBM_go_sleep ERR: not implemented\n" );
#endif
}

#if 0
double SBM_sleep_wait( double target_dt )	{ // sleep to reach target loop rate
//	static double prev_time = 0.0;
	static double prev_time = SBM_get_real_time();

	if( target_dt > 0.0 )	{
		double curr_time = SBM_get_real_time();
		double dt = curr_time - prev_time;

		if( dt < target_dt )	{
			double diff = target_dt - dt;
			int wait_msec = (int)( diff * 1000.0 + 1.0 );
//			int wait_msec = (int)( diff * 1000.0 );
			if( wait_msec > 0 ) {
				printf( "dt:%f diff:%f dt+dif:%f msec: %d\n", dt, diff, diff + dt, wait_msec );
				SBM_sleep_msec( wait_msec );
			}
		}
	}
	prev_time = SBM_get_real_time();
	return( prev_time );
}
#else

double SBM_sleep_wait( double prev_time, double target_dt )	{ // sleep to reach target loop rate

	if( target_dt > 0.0 )	{
		double curr_time = SBM_get_real_time();
		double dt = curr_time - prev_time;

		if( dt < target_dt )	{
			double diff = target_dt - dt;
			int wait_msec = (int)( diff * 1000.0 + 1.0 );
			if( wait_msec > 0 ) {
//				printf( "dt:%f diff:%f dt+dif:%f msec: %d\n", dt, diff, diff + dt, wait_msec );
				SBM_sleep_msec( wait_msec );
			}
		}
	}
	return( SBM_get_real_time() );
}
#endif

///////////////////////////////////////////////////////////////////////////////////

void TimeControl::start( double in_time ) {

	if( in_time < 0.0 )	{
		start_time = SBM_get_real_time();
//printf( "A start_time: %f\n", start_time );
		extern_src = false;
	}
	else	{
		start_time = in_time;
		extern_src = true;
	}
	
	clock_time = start_time;
	started = true;
}

bool TimeControl::update( double in_time ) {
	bool abort = false;
	
	if( !started ) start( in_time );
	
	if( in_time < 0.0 )	{
		if( extern_src )	{
			if( verbose ) 
				printf( "TimeControl::update NOTICE: switch to internal\n" );
			start( in_time );
			abort = true;
		}
		clock_time = SBM_sleep_wait( clock_time, sleep_dt );
//printf( "B clock_time: %f\n", clock_time );
//printf( "C dt: %f\n", clock_time - start_time );
	}
	else	{
		if( !extern_src )	{
			if( verbose ) 
				printf( "TimeControl::update NOTICE: switch to external\n" );
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
				if( verbose )
					printf( "TimeControl::update ERR: negative external increment: %f\n", loop_dt );
			}
			else
				printf( "TimeControl::update ERR: negative internal increment!!!!: %f\n", loop_dt );
			abort = true;
		}
		else
		if( loop_dt == 0.0 )	{
			if( extern_src )	{
				if( verbose )
					printf( "TimeControl::update NOTICE: zero external increment\n" );
			}
			else
				printf( "TimeControl::update ERR: zero internal increment!!!!\n" );
			abort = true;
		}
	}
	if( abort ) {
		out_dt = 0.0;
		if( verbose ) printf( "TimeControl::update ABORT\n" );
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

		double real_dt = real_time - prev_real_time;
		prev_real_time = real_time;

		if( do_pause )	{
			if( verbose ) printf( "TimeControl::update PAUSE\n" );
			do_pause = false;
			paused = true;
		}
		if( do_steps )	{
			do_resume = true;
		}
		if( do_resume )	{
			if( verbose ) printf( "TimeControl::update RESUME\n" );
			do_resume = false;
			paused = false;
		}
		if( do_steps )	{
			if( verbose ) printf( "TimeControl::update STEP\n" );
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
// perf.update( real_time, out_time );
			return( true );
		}
	}
	
	out_dt = 0.0;
	if( verbose ) printf( "TimeControl::update SKIP\n" );
	return( false );
}

///////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>

void test_time_control( void )	{
	TimeControl tc;
	bool res;
	
	printf( "=====================================================\n" );

#if 0
	res = tc.update();
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update();
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update();
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update();
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update();
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	printf( "=====================================================\n" );
	
	res = tc.update( 1.0 );
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update( 1.1 );
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update( 2.0 );
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update( 2.2 );
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update( 2.2 );
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	printf( "=====================================================\n" );

	res = tc.update();
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update();
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update();
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update();
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update();
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	printf( "=====================================================\n" );
	
	res = tc.update( 1.0 );
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update( 2.0 );
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update( 3.0 );
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update( 4.0 );
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

	res = tc.update( 0.0 );
	printf( "%f : %f : %d\n", tc.get_time(), tc.get_dt(), (int)res );

#endif
	
	tc.start();
//	tc.set_verbose();
	tc.set_speed( 10.0 );
	tc.set_sleep_fps( 10.0 );
	tc.set_eval_fps( 10.0 );
//	tc.set_sim_fps( 10.0 );

#if 0
#if 1
	for( int i = 0; i<100; i++ )	{
		double f = (double)rand() / (double)RAND_MAX;
		res = tc.update( f );
		if( res )	{
			printf( "%f : %f : %f\n", f, tc.get_time(), tc.get_dt() );
		}
		else printf( "-\n" );
	}
#else
	for( int i = 0; i<4; i++ )	{
		double f = -1.0;
		res = tc.update( f );
		printf( "[%d]: %f : %f : %f\n", res, f, tc.get_time(), tc.get_dt() );
	}
#endif
#endif

#if 1
	for( int i = 0; i<100; i++ )	{
		if( i == 10 ) tc.pause();
		if( i == 20 ) tc.step();
		if( i == 30 ) tc.resume();
//		if( i == 40 ) tc.pause();
		if( i == 50 ) tc.step( 10 );
		if( i == 70 ) tc.resume();

		res = tc.update();
		printf( "[%d]:(%d) %f : %f\n", i, res, tc.get_time(), tc.get_dt() );
	}
#endif


	printf( "=====================================================\n" );
}
















