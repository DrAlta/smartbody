#ifndef SR_SPLINE_CURVE_H
#define SR_SPLINE_CURVE_H

#include "gwiz_math.h"

class srSplineCurve {

	public:
		enum algorithm_enum_set    {
			ALG_SIMPLE, 
			ALG_HALTING, 
			ALG_CATMULL, 
			ALG_CARDINAL_C, 
			ALG_CARDINAL_ALT_C, 
			ALG_KBARTELS_TBC, 
			NUM_ALG_MODES
		};
		enum extend_enum_set    {
			EXTEND_REPEAT, 
			EXTEND_MIRROR, 
			EXTEND_DECEL, // DECELERATE: CONVERGE
			EXTEND_ACCEL, // ACCELERATE: EXTRAPOLATE
			NUM_EXTEND_MODES
		};

		static const char * algorithm_label( const int index );
		static const char * extend_label( const int index );
		static int algorithm_index( const char *label );
		static int extend_index( const char *label );
		
	private:

		int key_count;
		int node_count;
		bool dirty;
		int algorithm;

		class Key : public gwiz::ctrl_key	{

			public:
				Key( double p, double v ) 
				:	ctrl_key( p, v ) {
					next_p = NULL;
				}
				~Key( void )	{}
				void print( int i ) ;
				void next( Key *set_p ) { next_p = set_p; }
				Key *next( void ) { return( next_p ); }

			private:
				Key *next_p;
				// Node *node_p; for link editing... maybe not here
		};
		class Node : public gwiz::tempordinal_key	{

			public:
				Node( void ) {
					next_p = NULL;
				}
				~Node( void )	{}
				void print( int i );
				void next( Node *set_p ) { next_p = set_p; }
				Node *next( void ) { return( next_p ); }
				// void keyref( Key *k_p ) { ref_key_p = k_p; }
				// Key *keyref( void ) { return( ref_key_p ); }

			private:
				Node *next_p;
				// Key *ref_key_p; // if necessary.
		};

		Key *head_key_p;
		Key *tail_key_p;
//		Key *curr_key_p; // rapid local repeat access: to investigate
		Key *curr_edit_key_p;
		Key *curr_query_key_p;
		Node *curr_query_node_p;
		
		Node *head_node_p;
		Node *tail_node_p;

		void null( void )	{
			init();
		}
		void init( void )	{
			key_count = 0;
			node_count = 0;
			dirty = false;
			algorithm = ALG_CATMULL;
			head_key_p = NULL;
			tail_key_p = NULL;
//			curr_key_p = NULL;
			head_node_p = NULL;
			tail_node_p = NULL;
			curr_edit_key_p = NULL;
			curr_query_key_p = NULL;
			curr_query_node_p = NULL;
		}

	public:
		srSplineCurve( void )	{
			null();
		}
		~srSplineCurve( void )	{
			clear();
		}
		void print( void );

		int get_num_keys( void ) { return( key_count ); }
		int get_num_nodes( void ) { return( node_count ); }

		bool insert( double p, double v )	{
			return( insert_key( new Key( p, v ) ) );
		}
		void clear( void );

		void set_alg( int alg ) { algorithm = alg; }
		double evaluate( double t, bool *cropped_p = NULL );

	protected:

		void clear_nodes( void );

		bool insert_key( Key *key_p );
		void insert_head_key( Key *key_p ) ;
		void insert_after_key( Key *prev_p, Key *key_p );
		
		void decrement_key( void )	{ key_count--; dirty = true; }
		void increment_key( void )	{ key_count++; dirty = true; }
		void decrement_node( void )	{ node_count--; dirty = true; }
		void increment_node( void )	{ node_count++; dirty = true; }

		void update( void );

		Key *find_floor_key( double t );
		Node *find_floor_node( double t );

	public:
	// utilities for selecting, editing and display

		bool query_span( double *t_fr_p, double *t_to_p );
		
		bool edit_head( double t, double v );
		bool edit_tail( double t, double v );

		bool extend_head( int method = 0 );
		bool extend_tail( int method = 0 );

	// probe: -1 if nothing, otherwise index (compare to get_num_keys/nodes)
	// optionally set edit and query pointers
	
		int probe_bbox_key( 
			double t, double v, 
			double radius, 
			bool set_qu = true, bool set_ed = false,
			bool skip_head = false
			);
		int probe_bbox_node( 
			double t, double v, 
			double radius, 
			bool set_qu = true, 
			bool set_key_qu = false, bool set_key_ed = false
			);
	
	// iterate/access
		void edit_reset( void ) { curr_edit_key_p = head_key_p; }
		bool edit( double t, double v, bool increment = false );

		void query_reset( void )	{ 
			if( dirty ) update();
			curr_query_key_p = head_key_p; 
			curr_query_node_p = head_node_p; 
		}
		bool query_key( 
			double *t_p, double *v_p, 
			bool increment = false
			);
		bool query_node( 
			double *t_p, double *v_p, 
			double *ml_p, double *mr_p, 
			double *dl_p, double *dr_p, 
			bool increment = false
			);
};
#endif
