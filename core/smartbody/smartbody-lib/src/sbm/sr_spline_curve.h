#ifndef SR_SPLINE_CURVE_H
#define SR_SPLINE_CURVE_H

#include "gwiz_math.h"

// class srRapidCurve { srLinearCurve, srSplineCurve }

class srSplineCurve {

	private:

	class Key : public GWIZ::ctrl_key	{
		
		public:
			Key( double p, double v ) 
			:	ctrl_key( p, v ) {
				next_p = NULL;
			}
			~Key( void )	{
			}
			
			void print( int i ) {
				printf( " key[ %d ]: ( %f, %f )\n", i, t, v );
			}

			void next( Key *set_p ) { next_p = set_p; }
			Key *next( void ) { return( next_p ); }
			
		private:
			Key *next_p;
//			Node *node_p; for link editing
	};

	class Node : public GWIZ::cardinal_key	{
		
		public:
			Node( void ) {
				next_p = NULL;
			}
			void print( int i ) {
				printf( " node[ %d ]: ( %f, %f ) m( %f, %f ) d( %f, %f )\n", 
					i, t, v,
					m_in, m_out,
					dt_in, dt_out
				);
			}
			void next( Node *set_p ) { next_p = set_p; }
			Node *next( void ) { return( next_p ); }
			
		private:
			Node *next_p;
	};

	public:
		srSplineCurve( void )	{
			null();
		}
		~srSplineCurve( void )	{
			clear();
		}
		void print( void )	{
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

		int get_num_keys( void ) { return( key_count ); }
		int get_num_nodes( void ) { return( node_count ); }

		bool insert( double p, double v )	{
			return( insert_key( new Key( p, v ) ) );
		}
		void clear( void )	{
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
		void clear_nodes( void )	{
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

		double evaluate( double t, bool *cropped_p = NULL ) {
			if( dirty ) update();
			if( cropped_p ) {
				*cropped_p = false;
			}
			Node *node_p = find_floor_node( t );
			if( node_p )	{
				Node *next_p = node_p->next();
				if( next_p )	{
					return( GWIZ::hermite( t, *node_p, *next_p ) );
				}
				if( t < ( node_p->t + GWIZ::epsilon6() ) ) {
				// capture end node
					return( node_p->v );
				}
			}
			if( cropped_p ) {
				*cropped_p = true;
			}
			return( 0.0 );
		}

	protected:

		bool insert_key( Key *key_p )	{

			if( key_p ) {
				Key *floor_p = find_floor_key( key_p->t );
				if( floor_p )	{
					insert_after_key( floor_p, key_p );
					return( false );
				}
				insert_head_key( key_p );
				return( false );
			}
			return( true ); // error
		}

		void insert_head_key( Key *key_p ) {
			key_p->next( head_key_p );
			head_key_p = key_p;
			increment_key();
		}
		void insert_after_key( Key *prev_p, Key *key_p ) {
			Key *next_p = prev_p->next();
			prev_p->next( key_p );
			key_p->next( next_p );
			increment_key();
		}
		void decrement_key( void )	{
			key_count--;
			dirty = true;
		}
		void increment_key( void )	{
			key_count++;
			dirty = true;
		}
		void decrement_node( void )	{
			node_count--;
			dirty = true;
		}
		void increment_node( void )	{
			node_count++;
			dirty = true;
		}
		
		void update( void ) {

			// build cardinal keys from scratch...
			clear_nodes();
			if( key_count > 3 ) {

				Key *key0_p = head_key_p;
				Key *key1_p = head_key_p->next();
				Key *key2_p = key1_p->next();

				head_node_p = new Node;
				increment_node();
				Node *node_p = head_node_p;
				
				while( node_p ) {
//					node_p->catmullrom( key0_p, key1_p, key2_p );
					node_p->catmullrom( *key0_p, *key1_p, *key2_p );
					
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
		
		Key *find_floor_key( double t )	{
			// if( dirty ) update(); /* only if it affects search */
			Key *key_p = head_key_p;
			if( key_p ) {
				if( t < key_p->t )	{
					return( NULL );
				}
			}
			while( key_p )	{
				Key *next_p = key_p->next();
				if( next_p )	{
					if( t < next_p->t ) {
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
		Node *find_floor_node( double t )	{
			Node *node_p = head_node_p;
			if( node_p ) {
				if( t < node_p->t )	{
					return( NULL );
				}
			}
			while( node_p )	{
				Node *next_p = node_p->next();
				if( next_p )	{
					if( t < next_p->t ) {
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

	public:

	// utilities for selecting, editing and display
	
		bool probe_bound_box_key( double t, double v, double radius, bool set_edit ) {
			Key *key_p = head_key_p;
			while( key_p )	{
				if( key_p->bound_box( t, v, radius ) )	{
					if( set_edit )	{
						curr_edit_key_p = key_p;
					}
					return( true );
				}
				key_p = key_p->next();
			}
			return( false );
		}
		
		void edit_reset( void ) { curr_edit_key_p = head_key_p; }
		bool edit_next( double t, double v, bool increment )	{
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
		bool edit( double t, double v ) 
			{ return( edit_next( t, v, false ) ); }

		void query_reset( void ) { curr_query_key_p = head_key_p; }
		bool query_next( double *t_p, double *v_p, bool increment )	{
			if( curr_query_key_p )	{
				if( t_p ) { *t_p = curr_query_key_p->t; }
				if( v_p ) { *v_p = curr_query_key_p->v; }
				if( increment ) {
					curr_query_key_p = curr_query_key_p->next();
				}
				return( true );
			}
			return( false );
		}
		bool query( double *t_p, double *v_p ) 
			{ return( query_next( t_p, v_p, false ) ); }

	private:
		void null( void )	{
			init();
		}
		void init( void )	{
			key_count = 0;
			node_count = 0;
			dirty = false;
			head_key_p = NULL;
			head_node_p = NULL;
		}
		
		int key_count;
		int node_count;
		bool dirty;

		Key *head_key_p;
//		Key *curr_key_p; // rapid access
		Key *curr_edit_key_p;
		Key *curr_query_key_p;
		
		Node *head_node_p;
};

#endif
