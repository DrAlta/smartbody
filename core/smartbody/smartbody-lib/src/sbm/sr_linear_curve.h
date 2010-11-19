/*
 *  sr_cmd_seq.h - part of SmartBody-lib
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

	typedef struct sr_curve_key_s  {
		
		double			time;
		double			value;
		
		double			dt;
		double			inv_dt;
		double			dv;

		sr_curve_key_s	*prev;
		sr_curve_key_s	*next;
		
	} sr_curve_key_t;

	public:
		srLinearCurve( void )	{
			null();
		}

		~srLinearCurve( void )	{
			sr_curve_key_t *key_p = head_p;
			while( key_p ) {
				sr_curve_key_t *tmp_p = key_p;
				key_p = key_p->next;
				delete tmp_p;
			}
			null();
		}

		void null( void )	{
			key_count = 0;
			dirty = false;
			head_p = NULL;
			curr_p = NULL;
		}
		
		void print( void )	{
			
			printf( "srLinearCurve: KEYS:\n" );
			int c = 0;
			sr_curve_key_t *key_p = head_p;
			while( key_p ) {
				printf( " key[ %d ]: ( %f, %f )\n", c++, key_p->time, key_p->value );
				key_p = key_p->next;
			}
		}

		int get_num_keys( void ) { return( key_count ); }
		
		int insert( double time, double v )	{
			
			sr_curve_key_t *key_p = new sr_curve_key_t;
			key_p->time = time;
			key_p->value = v;
			key_p->prev = NULL;
			key_p->next = NULL;

			/* sort by key.time, add after same time */
			return( insert_key( key_p ) );
		}
		
		double evaluate( double t )	{
			
			sr_curve_key_t *floor_p = find_floor_key( t );
			if( floor_p )	{
			
				curr_p = floor_p;
				if( t > curr_p->time )	{
				
					if( curr_p->next ) {

						if( dirty ) {
							update_intervals();
						}
						return( lerp_keys( t, curr_p, curr_p->next ) );
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
		
		double lerp_keys( double t, sr_curve_key_t *fr_p, sr_curve_key_t *to_p )	{

			register double norm = ( t - fr_p->time ) * fr_p->inv_dt;
			return( fr_p->value + norm * fr_p->dv );
		}
		
		sr_curve_key_t* find_floor_key( double t )	{
			
			sr_curve_key_t *key_p = curr_p;
			if( key_p )	{
			
				if( t < key_p->time ) {
					key_p = head_p;
				}
			}
			else	{
				key_p = head_p;
			}
			if( key_p )	{
			
				if( t < key_p->time )	{
					return( NULL );
				}
			}
			while( key_p )	{
			
				if( key_p->next )	{
				
					if( t < key_p->next->time )	{
						return( key_p );
					}
					else	{
						key_p = key_p->next;
					}
				}
				else	{
					return( key_p );
				}
			}
			return( key_p );
		}
			
		int insert_key( sr_curve_key_t *key_p ) {
			
			if( key_p )	{

				sr_curve_key_t *floor_p = find_floor_key( key_p->time );
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
		
		void update_interval( sr_curve_key_t *key_p, sr_curve_key_t *next_p ) {
		
			key_p->dt = next_p->time - key_p->time;
			key_p->inv_dt = 1.0 / key_p->dt;
			key_p->dv = next_p->value - key_p->value;
		}
		
		void update_intervals( void )	{
		
			sr_curve_key_t *key_p = head_p;
			while( key_p ) {
			
				sr_curve_key_t *prev_p = key_p;
				key_p = key_p->next;
				if( key_p ) {
				
					update_interval( prev_p, key_p );
				}
			}
			dirty = false;
			printf( "UPDATE!\n" );
		}
		
		void insert_head( sr_curve_key_t *key_p )	{
			
			key_p->next = head_p;
			
//			if( head_p )	{
//				update_interval_cache( key_p, head_p );
//			}

			head_p = key_p;
			key_count++;
			dirty = true;
		}
		
		void insert_after( sr_curve_key_t *prev_p, sr_curve_key_t *key_p )	{
			
//			update_interval_cache( prev_p, key_p );

			sr_curve_key_t *next_p = prev_p->next;
//			if( next_p )	{
//				update_interval_cache( key_p, next_p );
//			}
			
			prev_p->next = key_p;
			key_p->next = next_p;
			key_count++;
			dirty = true;
		}
		
	private:
	
		int		key_count;
		bool	dirty;
		
		sr_curve_key_t	*head_p;
		sr_curve_key_t	*curr_p;
};
#endif
