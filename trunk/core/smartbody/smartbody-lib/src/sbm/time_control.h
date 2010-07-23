#ifndef TIME_CONTROL_H
#define TIME_CONTROL_H

#include <cstdio>

class TimeControl	{

	private:
		void defaults( void )	{
			
			verbose = false;
			started = false;
			extern_src = false;
			
			speed = 1.0;
			sleep_dt = 0.0;
			eval_dt = 0.0;
			sim_dt = 0.0;
		
			start_time = 0.0;
			clock_time = 0.0;
			real_time = 0.0;
			prev_real_time = 0.0;
			prev_loop_time = 0.0;
			eval_wait = 0.0;
			
			do_pause = false;
			do_resume = false;
			do_steps = 0;
			paused = false;

			out_time = 0.0;
			out_dt = 0.0;
		}

	public:
	
		TimeControl( void ) { defaults(); }
		~TimeControl( void ) {}

		void set_verbose( bool v = true ) { verbose = v; }
		void set_speed( double s ) {
			speed = s; 
			sim_dt = 0.0;
		}
		
		void set_sleep_dt( double dt )	{ sleep_dt = dt; }
		void set_eval_dt( double dt )	{ eval_dt = dt; }
		void set_sim_dt( double dt )	{ sim_dt = dt; }

		void set_sleep_fps( double fps ) { 
			if( fps > 0.0 ) set_sleep_dt( 1.0 / fps );
			else set_sleep_dt( 0.0 );
		}
		void set_eval_fps( double fps ) { 
			if( fps > 0.0 ) set_eval_dt( 1.0 / fps );
			else set_eval_dt( 0.0 );
		}
		void set_sim_fps( double fps ) { 
			if( fps > 0.0 ) set_sim_dt( 1.0 / fps );
			else set_sim_dt( 0.0 );
		}
		
		void start( double in_time = -1.0 );
		bool update( double in_time = -1.0 );

		void pause( void )	{
			do_pause = true;
		}
		void step( int num_steps = 1 )	{
			do_steps = num_steps;
		}
		void resume( void )	{
			do_resume = true;
		}
		
		double get_time( void ) { return( out_time ); }
		double get_dt( void ) { return( out_dt ); }

		void print( void )	{
			printf( "TimeControl(%.3f): \n", real_time );
			printf( " speed:     %.3f\n", speed );
			printf( " sleep fps: %.3f\n", 1.0 / sleep_dt );
			printf( " eval fps:  %.3f\n", 1.0 / eval_dt );
			printf( " sim fps:   %.3f\n", 1.0 / sim_dt );
			printf( " time:      %.3f\n", out_time );
			printf( " status: %s\n", paused ? "PAUSED" : ( do_steps ? "STEPPING" : "RUNNING" ) );
		}
	
	private:
		bool	verbose;
		bool	started;
		bool	extern_src;
		
		double	speed;		
		double	sleep_dt;
		double	eval_dt;
		double	sim_dt;
		
		double	start_time;
		double	clock_time;
		double	real_time;
		double	prev_real_time;
		double	prev_loop_time;
		double	eval_wait;
		
		bool	do_pause;
		bool	do_resume;
		int 	do_steps;
		bool	paused;

		double	out_time;
		double	out_dt;
};

#endif
