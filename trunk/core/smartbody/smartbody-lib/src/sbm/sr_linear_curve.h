/*
 *  sr_linear_curve.h - part of SmartBody-lib
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

#ifndef SR_LINEAR_CURVE_H
#define SR_LINEAR_CURVE_H

#include <stdio.h>
#include "sbm_constants.h"
#include <vhcl_log.h>

//////////////////////////////////////////////////////////////////

class srLinearCurve	{

	private:

//		int objective_key_count;
//		int objective_key_count_max;

	class Key	{
		
		public:
		
			Key( double p, double v );
			~Key(void) {
//				objective_key_count--;
			}

			void print( int i );

			void next( Key *set_p ) { next_p = set_p; }
			Key *next( void ) { return( next_p ); }

			void update( void );
			void copy_delta( Key *key_p );

			double slope( void );
			double lerp( double t );

			double	param; // ANIMATION: time
			double	value; // template<> ?
		
		private:
			
			double	dp, inv_dp;
			double	dv;

			Key	*next_p;
	};

	public:
		srLinearCurve( void )	{
			null();
//			objective_key_count = 0;
//			objective_key_count_max = 0;
		}
		~srLinearCurve( void )	{
			clear();
		}

		void print( void );

		int get_num_keys( void ) { return( key_count ); }
		
		int insert( double p, double v )	{ /* sort by key.time, add after same time */
			return( insert_key( new Key( p, v ) ) );
		}
		
		void clear( void );
		void clear_after( double t );

		double get_head_param( void );
		double get_head_value( void );
		double get_head_slope( void );
		
		double get_tail_param( void );
		double get_tail_value( void );
		double get_tail_slope( void );

		double get_next_nonzero_value( double after );
		double get_next_nonzero_slope( double after );
//		double get_last_nonzero_param( double after );

		double evaluate( double t );

	protected:

		Key* find_floor_key( double t );
		int insert_key( Key *key_p );
		void update_intervals( void );
		void decrement( void );
		void increment( void );
		void insert_head( Key *key_p );
		void insert_after( Key *prev_p, Key *key_p );

	private:
	
		void null( void )	{
			key_count = 0;
			dirty = false;
			head_p = NULL;
			curr_p = NULL;
			tail_p = NULL;
		}
		
		int 	key_count;
		bool	dirty;
		
		Key		*head_p;
		Key		*curr_p;
		Key		*tail_p;
};

//////////////////////////////////////////////////////////////////
#endif
