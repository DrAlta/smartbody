/*
 *  sbm_perf.h - part of SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
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

#ifndef SBM_PERF_H
#define SBM_PERF_H

#include <stdio.h>

class SbmPerfReport	{
	
	public:
		SbmPerfReport( double real_interval = 10.0 ) {
			interval = real_interval;
			prev_real = 0.0;
			prev_sim = 0.0;
			reset();
			enabled = false; // keep updating reference time, but don't report
		}
		~SbmPerfReport( void ) {}
		
		void reset( void )	{
			real_sum = 0.0;
			sim_sum = 0.0;
			count = 0;
		}
		
		void on( void ) { enabled = true; }
		void off( void ) { enabled = false; }
		void enable( bool e ) { enabled = e; }
		void toggle( void ) { enabled = !enabled; }
		void set_interval( double i ) { interval = i; }
		
		void update( double real_time, double sim_time )	{
			
			if( enabled == false )	{
				prev_real = real_time;
				prev_sim = sim_time;
				return;
			}
			
			double dt = real_time - prev_real;
			prev_real = real_time;
			real_sum += dt;

			dt = sim_time - prev_sim;
			prev_sim = sim_time;
			sim_sum += dt;

			count ++;
			
			if( real_sum >= interval )	{
				double avg = real_sum / count;
//				if( real_time == sim_time ) {
				if( fabs( real_sum - sim_sum ) < 0.001 ) {
					printf( "PERF: dt:%.3f fps:%.1f\n", avg, 1.0 / avg );
				}
				else	{
					double sim_avg = sim_sum / count;
					printf( "PERF: REAL dt:%.3f fps:%.1f SIM dt:%.3f fps:%.1f\n", 
						avg, 1.0 / avg,
						sim_avg, 1.0 / sim_avg
					);
				}
				reset();
			}
		}
		
	private:
		double interval;
		double prev_real, prev_sim;
		double real_sum, sim_sum;
		int count;
		bool enabled;
};

#endif SBM_PERF_H
