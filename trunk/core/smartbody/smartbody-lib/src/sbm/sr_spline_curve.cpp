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

#include <math.h>
//#include <sbm/sr_spline_curve.h>
#include "sr_spline_curve.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void srSplineCurve::Key::print( int i ) {
	printf( " key[ %d ]: ( %f, %f )\n", i, p(), v() );
}

void srSplineCurve::Node::print( int i ) {
	printf( " node[ %d ]: ( %f, %f ) m( %f, %f ) d( %f, %f )\n", 
		i, p(), v(),
		ml(), mr(),
		dl(), dr()
	);
}

void srSplineCurve::print( void )	{

	if( dirty ) update();
	printf( "srSplineCurve: KEYS: %d[%d]\n", key_count, node_count );
	int c = 0;
	Key *key_p = head_key_p;
	while( key_p )	{
		key_p->print( c++ );
		key_p = key_p->next();
	}
	c = 0;
	Node *node_p = head_node_p;
	while( node_p )	{
		node_p->print( c++ );
		node_p = node_p->next();
	}
}

//////////////////////////////////////////////////////////////////

void srSplineCurve::clear( void )	{

	Key *key_p = head_key_p;
	while( key_p )	{
		Key *tmp_p = key_p;
		key_p = key_p->next();
		delete tmp_p;
		decrement_key();
	}
	if( key_count != 0 )	{
		printf( "key_count ERR: %d\n", key_count );
	}
	clear_nodes();
	init();
}

void srSplineCurve::clear_nodes( void )	{

	Node *node_p = head_node_p;
	while( node_p )	{
		Node *tmp_p = node_p;
		node_p = node_p->next();
		delete tmp_p;
		decrement_node();
	}
	if( node_count != 0 )	{
		printf( "node_count ERR: %d\n", node_count );
	}
	head_node_p = NULL;
}

double srSplineCurve::evaluate( double t, bool *cropped_p ) {

	if( dirty ) update();
	if( cropped_p ) { *cropped_p = false; }
	Node *node_p = find_floor_node( t );
	if( node_p )	{
		Node *next_p = node_p->next();
		if( next_p )	{
			return( GWIZ::hermite( t, *node_p, *next_p ) );
		}
		if( t < ( node_p->p() + GWIZ::epsilon8() ) ) {
			// capture end node
			return( node_p->v() );
		}
	}
	if( cropped_p ) { *cropped_p = true; }
	return( 0.0 );
}

//////////////////////////////////////////////////////////////////

bool srSplineCurve::insert_key( Key *key_p )	{

	if( key_p ) {
		Key *floor_p = find_floor_key( key_p->p() );
		if( floor_p )	{
			insert_after_key( floor_p, key_p );
			return( false );
		}
		insert_head_key( key_p );
		return( false );
	}
	return( true ); // error
}

void srSplineCurve::insert_head_key( Key *key_p ) {

	key_p->next( head_key_p );
	head_key_p = key_p;
	increment_key();
}

void srSplineCurve::insert_after_key( Key *prev_p, Key *key_p ) {

	Key *next_p = prev_p->next();
	prev_p->next( key_p );
	key_p->next( next_p );
	increment_key();
}

//////////////////////////////////////////////////////////////////

void srSplineCurve::update( void ) {

	// build cardinal keys from scratch...
	clear_nodes();
	if( key_count >= 3 ) { // at least 2 nodes to register an interval, with default esp8()

		Key *key0_p = head_key_p;
		Key *key1_p = head_key_p->next();
		Key *key2_p = key1_p->next();

		head_node_p = new Node;
		increment_node();
		Node *node_p = head_node_p;

		while( node_p ) {
			node_p->catmullrom( (*key0_p), (*key1_p), (*key2_p) );

			key0_p = key1_p;
			key1_p = key2_p;
			key2_p = key2_p->next();

			if( key2_p )	{
				Node *next_p = new Node;
				increment_node();
				node_p->next( next_p );
				node_p = next_p;
			}
			else	{
				node_p = NULL;
			}
		}
		if( node_count != ( key_count - 2 ) )	{
			printf( "update ERR: %d[%d]\n", key_count, node_count );
		}
	}
	dirty = false;
}

srSplineCurve::Key *srSplineCurve::find_floor_key( double t )	{

	// if( dirty ) update(); /* only if it affects search */
	Key *key_p = head_key_p;
	if( key_p ) {
		if( t < key_p->p() )	{
			return( NULL );
		}
	}
	while( key_p )	{
		Key *next_p = key_p->next();
		if( next_p )	{
			if( t < next_p->p() ) {
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

srSplineCurve::Node *srSplineCurve::find_floor_node( double t )	{

	Node *node_p = head_node_p;
	if( node_p ) {
		if( t < node_p->p() )	{
			return( NULL );
		}
	}
	while( node_p )	{
		Node *next_p = node_p->next();
		if( next_p )	{
			if( t < next_p->p() ) {
				return( node_p );
			}
			else	{
				node_p = next_p;
			}
		}
		else	{
			return( node_p );
		}
	}
	return( NULL );
}

//////////////////////////////////////////////////////////////////

bool srSplineCurve::probe_bbox_key( double t, double v, double radius, bool set_qu, bool set_ed ) {

	Key *key_p = head_key_p;
	while( key_p )	{
		if( key_p->bound_box( t, v, radius ) )	{
			if( set_qu )	{
				curr_query_key_p = key_p;
			}
			if( set_ed )	{
				curr_edit_key_p = key_p;
			}
			return( true );
		}
		key_p = key_p->next();
	}
	return( false );
}

bool srSplineCurve::edit_next( double t, double v, bool increment )	{

	if( curr_edit_key_p )	{
		curr_edit_key_p->set( t, v );
		dirty = true;
		if( increment ) {
			curr_edit_key_p = curr_edit_key_p->next();
		}
		return( true );
	}
	return( false );
}

bool srSplineCurve::query_next_key( 
	double *t_p, double *v_p, 
	bool increment 
	)	{

	if( curr_query_key_p )	{
		if( t_p ) { *t_p = curr_query_key_p->p(); }
		if( v_p ) { *v_p = curr_query_key_p->v(); }
		if( increment ) {
			curr_query_key_p = curr_query_key_p->next();
		}
		return( true );
	}
	return( false );
}

bool srSplineCurve::query_next_node( 
	double *t_p, double *v_p, 
	double *ml_p, double *mr_p, 
	double *dl_p, double *dr_p, 
	bool increment 
	)	{

	if( curr_query_node_p )	{
		if( t_p ) { *t_p = curr_query_node_p->p(); }
		if( v_p ) { *v_p = curr_query_node_p->v(); }
		if( ml_p ) { *ml_p = curr_query_node_p->ml(); }
		if( mr_p ) { *mr_p = curr_query_node_p->mr(); }
		if( dl_p ) { *dl_p = curr_query_node_p->dl(); }
		if( dr_p ) { *dr_p = curr_query_node_p->dr(); }
		if( increment ) {
			curr_query_node_p = curr_query_node_p->next();
		}
		return( true );
	}
	return( false );
}

//////////////////////////////////////////////////////////////////
