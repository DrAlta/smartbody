/*
 *  sr_linear_curve.cpp - part of SmartBody-lib
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

//#include <sr_linear_curve.h>
#include <sbm/sr_linear_curve.h>

#define SR_CURVE_INFINITE_SLOPE		(1000000.0)

//////////////////////////////////////////////////////////////////

srLinearCurve::Key::Key( double p, double v ) {

	param = p;
	value = v;
	dp = 0.0; inv_dp = 0.0;
	dv = 0.0;
	next_p = NULL;
//		objective_key_count++;
//		if( objective_key_count > objective_key_count_max ) objective_key_count_max = objective_key_count;
}

void srLinearCurve::Key::print( int i )	{

#if 1
	printf( " key[ %d ]: ( %f, %f )\n", 
		i, param, value
	);
#elif 0
	printf( " key[ %d ]: ( %f, %f ):{ %f, %f, %f }:[ 0x%x ]\n", 
		i, param, value,
		dp, inv_dp, dv,
		next_p
	);
#else
	printf( " key[ %d ]: ( %f, %f ):{ %f }\n", 
		i, param, value,
		slope()
	);
#endif
}

void srLinearCurve::Key::update( void ) {

	if( next_p )	{
		dp = next_p->param - param; 
		if( dp > 0.0 )	{
			inv_dp = 1.0 / dp;
		}
		else	{
			inv_dp = SR_CURVE_INFINITE_SLOPE;
		}
		dv = next_p->value - value;
	}
}

void srLinearCurve::Key::copy_delta( Key *key_p ) {

	if( key_p )	{
		dp = key_p->dp; inv_dp = key_p->inv_dp;
		dv = key_p->dv;
	}
}

double srLinearCurve::Key::slope( void )	{

	if( dp > 0.0 )	{
		return( dv * inv_dp );
	}
	return( SR_CURVE_INFINITE_SLOPE );
}

double srLinearCurve::Key::lerp( double t )	{

	return( value + dv * ( t - param ) * inv_dp );
}

//////////////////////////////////////////////////////////////////

void srLinearCurve::print( void )	{

	if( dirty ) {
		update_intervals();
	}
//			printf( "srLinearCurve: KEYS: %d of %d (%d)\n", key_count, objective_key_count, objective_key_count_max );
	printf( "srLinearCurve: KEYS: %d\n", key_count );
	int c = 0;
	Key *key_p = head_p;
	while( key_p ) {
		key_p->print( c++ );
		key_p = key_p->next();
	}
}

void srLinearCurve::clear( void )	{
	Key *key_p = head_p;
	while( key_p ) {
		Key *tmp_p = key_p;
		key_p = key_p->next();
		delete tmp_p;
		decrement();
	}
	null();
}

void srLinearCurve::clear_after( double t )	{

	if( head_p == NULL )	{
		return;
	}
	Key *key_p = NULL;
	Key *floor_p = find_floor_key( t );
	if( floor_p )	{
		key_p = floor_p->next();
		floor_p->next( NULL );
	}
	else	{
		key_p = head_p;
		head_p = NULL;
	}
	while( key_p ) {
		if( key_p == curr_p )	{
			curr_p = NULL;
		}
		Key *tmp_p = key_p;
		key_p = key_p->next();
		delete tmp_p;
		decrement();
	}
	tail_p = NULL;
}

double srLinearCurve::get_head_param( void )	{
	if( head_p )	{
		return( head_p->param );
	}
	return( 0.0 );
}

double srLinearCurve::get_head_value( void )	{
	if( head_p )	{
		return( head_p->value );
	}
	return( 0.0 );
}

double srLinearCurve::get_head_slope( void )	{
	if( head_p )	{
		return( head_p->slope() );
	}
	return( 0.0 );
}

double srLinearCurve::get_tail_param( void )	{
	if( dirty ) {
		update_intervals();
	}
	if( tail_p )	{
		return( tail_p->param );
	}
	return( 0.0 );
}

double srLinearCurve::get_tail_value( void )	{
	if( dirty ) {
		update_intervals();
	}
	if( tail_p )	{
		return( tail_p->value );
	}
	return( 0.0 );
}

double srLinearCurve::get_tail_slope( void )	{
	if( dirty ) {
		update_intervals();
	}
	if( tail_p )	{
		return( tail_p->slope() );
	}
	return( 0.0 );
}

double srLinearCurve::get_next_nonzero_value( double after )	{

	if( head_p == NULL )	{
		return( -1.0 );
	}
	Key *key_p = NULL;
	Key *floor_p = find_floor_key( after );
	if( floor_p )	{
		key_p = floor_p->next();
	}
	else	{
		key_p = head_p;
	}
	while( key_p )	{
		if( key_p->value != 0.0 )	{
			return( key_p->param );
		}
		key_p = key_p->next();
	}
	return( -1.0 );
}

double srLinearCurve::get_next_nonzero_slope( double after )	{

	if( head_p == NULL )	{
		return( -1.0 );
	}
	Key *key_p = NULL;
	Key *floor_p = find_floor_key( after );
	if( floor_p )	{
		key_p = floor_p->next();
	}
	else	{
		key_p = head_p;
	}
	if( dirty ) {
		update_intervals();
	}
	while( key_p )	{
		if( key_p->slope() != 0.0 )	{
			return( key_p->param );
		}
		key_p = key_p->next();
	}
	return( -1.0 );
}

#if 0
double srLinearCurve::get_last_nonzero_param( double after )	{

	if( head_p == NULL )	{
		return( -1.0 );
	}
	Key *key_p = NULL;
	Key *floor_p = find_floor_key( after );
	if( floor_p )	{
		key_p = floor_p->next();
	}
	else	{
		key_p = head_p;
	}
	double t = -1.0;
	while( key_p )	{
		if( key_p->value != 0.0 )	{
			t = key_p->param;
		}
		key_p = key_p->next();
	}
	return( t );
}
#endif

double srLinearCurve::evaluate( double t )	{

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

srLinearCurve::Key* srLinearCurve::find_floor_key( double t )	{

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

int srLinearCurve::insert_key( Key *key_p ) {

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

void srLinearCurve::update_intervals( void )	{

	int c = 0;
	Key *prev_p = NULL;
	Key *key_p = head_p;
	while( key_p ) {

		Key *tmp_p = prev_p;
		prev_p = key_p;
		key_p = key_p->next();
		if( key_p ) {
			prev_p->update();
		}
		else	{
			tail_p = prev_p;
			tail_p->copy_delta( tmp_p );
		}
		c++;
	}
	dirty = false;
	if( c != key_count )	{
		LOG( "srLinearCurve::update_intervals ERR: corruption: counted %d of %d keys", c, key_count );
	}
}

void srLinearCurve::decrement( void )	{

	key_count--;
	dirty = true;
}

void srLinearCurve::increment( void )	{

	key_count++;
	dirty = true;
}

void srLinearCurve::insert_head( Key *key_p )	{

	key_p->next( head_p );
	head_p = key_p;
	increment();
}

void srLinearCurve::insert_after( Key *prev_p, Key *key_p )	{

	Key *next_p = prev_p->next();
	prev_p->next( key_p );
	key_p->next( next_p );
	increment();
}

//////////////////////////////////////////////////////////////////

#if 0
void test_linear_curve( int reps = 1000000 )	{

	srLinearCurve C;
	for( int i=0; i<reps; i++ )	{
		int r = rand() % 10;
		switch( r )	{
			case 0:
				C.insert( (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX );
				break;
			case 1:
//				C.clear();
				C.clear_after( (float)rand() / (float)RAND_MAX );
				break;
			case 2:
				C.clear_after( (float)rand() / (float)RAND_MAX );
				break;
			case 3:
				C.get_next_nonzero_slope( (float)rand() / (float)RAND_MAX );
				break;
			case 4:
				C.get_next_nonzero_value( (float)rand() / (float)RAND_MAX );
				break;
			case 5:
				C.get_last_nonzero( (float)rand() / (float)RAND_MAX );
				break;
			case 6:
				C.get_tail_param();
				break;
			case 7:
				C.get_tail_value();
				break;
			case 8:
				C.get_tail_slope();
				break;
			default:	
				C.evaluate( (float)rand() / (float)RAND_MAX );
				break;
		}
	}

	C.print();
}
#endif

#if 0
void test_linear_curve( void )	{
	srLinearCurve curve;
	int i;
#if 1
	curve.insert( 2.0, 10.0 ); curve.print();
	curve.insert( 3.0, 20.0 ); curve.print();
	curve.insert( 1.0, 0.9 ); curve.print();
	curve.insert( 0.1, 0.5 ); curve.print();
	curve.insert( 2.5, 11.0 ); curve.print();
#endif
#if 0
	for( i=0; i<50; i++ )	{
		curve.insert( (double)( rand()%100 ), (double)i );
	}
#endif
	curve.print();
#if 0
	for( i=0; i<100; i++ )	{
		double s = (double)i * 0.1;
		double v = curve.evaluate( s );
		printf( "%f: %f\n", s, v );
	}
#endif
}
#endif
