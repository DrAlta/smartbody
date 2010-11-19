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

class srLinearCurve	{

	private:

	class Key	{
		
		public:
		
			Key( double p, double v ) {
				param = p;
				value = v;
				dp = 0.0; inv_dp = 0.0;
				dv = 0.0;
				next_p = NULL;
			}
			~Key(void) {}

			void print( int i )	{
				printf( " key[ %d ]: ( %f, %f )\n", i, param, value );
			}

			void next( Key *set_p ) { next_p = set_p; }

			Key *next( void ) { return( next_p ); }

			void update( void ) {
				if( next_p )	{
					dp = next_p->param - param; inv_dp = 1.0 / dp;
					dv = next_p->value - value;
				}
			}

			double lerp( double t )	{
				return( value + dv * ( t - param ) * inv_dp );
			}

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
		}

		~srLinearCurve( void )	{
			Key *key_p = head_p;
			while( key_p ) {
				Key *tmp_p = key_p;
				key_p = key_p->next();
				delete tmp_p;
			}
			null();
		}

		void print( void )	{
			
			printf( "srLinearCurve: KEYS:\n" );
			int c = 0;
			Key *key_p = head_p;
			while( key_p ) {
				key_p->print( c++ );
				key_p = key_p->next();
			}
		}

		int get_num_keys( void ) { return( key_count ); }
		
		int insert( double p, double v )	{ /* sort by key.time, add after same time */
			return( insert_key( new Key( p, v ) ) );
		}
		
		double evaluate( double t )	{
			
			Key *floor_p = find_floor_key( t );
			if( floor_p )	{
			
				curr_p = floor_p;
				if( t > curr_p->param )	{
				
					if( curr_p->next() ) {

						if( dirty ) {
							update_intervals();
						}
						return( curr_p->lerp( t ) );
					}
				}
				return( curr_p->value );
			}
			if( head_p )	{
			
				curr_p = head_p;
				return( curr_p->value );
			}
			curr_p = NULL;
			return( 0.0 );
		}

	protected:

		Key* find_floor_key( double t )	{
			
			Key *key_p = curr_p;
			if( key_p )	{
			
				if( t < key_p->param ) {
					key_p = head_p;
				}
			}
			else	{
				key_p = head_p;
			}
			if( key_p )	{
			
				if( t < key_p->param )	{
					return( NULL );
				}
			}
			while( key_p )	{
			
				Key *next_p = key_p->next();
				if( next_p ) {
				
					if( t < next_p->param )  {
						return( key_p );
					}
					else	{
						key_p = next_p;
					}
				}
				else	{
					return( key_p );
				}
			}
			return( NULL );
		}
			
		int insert_key( Key *key_p ) {
			
			if( key_p )	{

				Key *floor_p = find_floor_key( key_p->param );
				if( floor_p )	{
					
					insert_after( floor_p, key_p );
					curr_p = floor_p;
					return( CMD_SUCCESS );
				}
				insert_head( key_p );
				curr_p = key_p;
				return( CMD_SUCCESS );
			}
			curr_p = NULL;
			return( CMD_FAILURE );
		}

		void update_intervals( void )	{
		
			int c = 0;
			Key *key_p = head_p;
			while( key_p ) {
			
				Key *prev_p = key_p;
				key_p = key_p->next();
				if( key_p ) {
					
					prev_p->update();
				}
				c++;
			}
			dirty = false;
			if( c != key_count )	{
				printf( "srLinearCurve::update_intervals ERR: corruption.\n" );
			}
		}

		void increment( void )	{
			key_count++;
			dirty = true;
		}
		void insert_head( Key *key_p )	{
			
			key_p->next( head_p );
			head_p = key_p;
			increment();
		}
		void insert_after( Key *prev_p, Key *key_p )	{
			
			Key *next_p = prev_p->next();
			prev_p->next( key_p );
			key_p->next( next_p );
			increment();
		}

	private:
	
		void null( void )	{
			key_count = 0;
			dirty = false;
			head_p = NULL;
			curr_p = NULL;
		}
		
		int 	key_count;
		bool	dirty;
		
		Key		*head_p;
		Key		*curr_p;
};
#endif
