#ifndef GWIZ_GUI_H
#define GWIZ_GUI_H

#include "gwiz_math.h"

namespace gwiz	{

enum gwiz_gui_enumeration_set    {

	GWIZ_GUI_NULL = -1, 
	
	GWIZ_GUI_NO_NUMBER = 0, 
	GWIZ_GUI_ROUND_FLOAT, 
	GWIZ_GUI_ROUND_INT, 
	GWIZ_GUI_ROUND_FLOOR, 
	GWIZ_GUI_ROUND_CIEL
};

extern int gwiz_gui_debug_tmp;

/////////////////////////////////////////////////

class gwizRectWidget {
	public:
		gwizRectWidget(void) { 
			instance_id = generate_id();
			set_rect( 0, 0, 10, 100 ); 
		}
		gwizRectWidget( int ox, int oy, int sx, int sy = -1 ) { 
			instance_id = generate_id();
			set_rect( ox, oy, sx, sy ); 
		}

		int get_id( void ) { return( instance_id ); }
		
		void set_origin( int ox, int oy )	 { 
			origin_x = ox; 
			origin_y = oy; 
		}
		void set_size( int sx, int sy = -1 )	 { 
			size_x = sx; 
			if( sy < 0 )
				size_y = size_x;
			else
				size_y = sy; 
		}
		void set_rect( int ox, int oy, int sx, int sy = -1 )	 { 
			origin_x = ox; 
			origin_y = oy; 
			size_x = sx; 
			if( sy < 0 )
				size_y = size_x;
			else
				size_y = sy; 
		}
		
	protected:
		static int generate_id( void )	{
			static int id = GWIZ_GUI_NULL;
			return( ++id );
		}
		
		float get_normal_x( float px );
		float get_normal_y( float py );

		int bound( float px, float py );
		int bound_sloppy_x( float px, float py );
		int bound_corner_bl( float px, float py );
		int bound_corner_tr( float px, float py );
		
		int origin_x, origin_y, size_x, size_y;
		int instance_id;
};

/////////////////////////////////////////////////

class gwizPlacementWidget : public gwizRectWidget {

	public:
		gwizPlacementWidget(void):gwizRectWidget() { 
			mini = 0;
			minifiable = 0;
			resizeable = 0;
			busy_p = 0;
			busy_m = 0;
			busy_r = 0;
		}
		gwizPlacementWidget(
			int ox, int oy, int sx, int sy
		):gwizRectWidget( ox, oy, sx, sy ) { 
			mini = 0;
			minifiable = 0;
			resizeable = 0;
			busy_p = 0;
			busy_m = 0;
			busy_r = 0;
		}
		void init( int ox, int oy, int sx, int sy, int minify = 0, int resize = 0 )	{ 
			set_rect( ox, oy, sx, sy ); 
			minifiable = minify;
			resizeable = resize;
		}
		void init( int minify, int resize )	{ 
			minifiable = minify;
			resizeable = resize;
		}
		
		int get_x( void )			{ return( origin_x ); }
		int get_y( void )			{ return( origin_y ); }
		int get_x2( void )			{ return( origin_x + size_x ); }
		int get_y2( void )			{ return( origin_y + size_y ); }
		int get_dx( void )			{ return( size_x ); }
		int get_dy( void )			{ return( size_y ); }
		int minified( void )		{ return( mini ); }
		
		int update( int action, int px, int py );
		void draw( void );

	private:
		int mini;
		int minifiable, resizeable;
		int dx, dy;
		int busy_p, busy_m, busy_r;
};

class gwizTriggerButton : public gwizRectWidget {

	public:
		gwizTriggerButton(void):gwizRectWidget() { 
			value = 0;
			busy = 0;
		}
		gwizTriggerButton(
			int ox, int oy, int sx, int sy
		):gwizRectWidget( ox, oy, sx, sy ) { 
			value = 0;
			busy = 0;
		}
		void init( int ox, int oy, int sx, int sy )	{ 
			set_rect( ox, oy, sx, sy ); 
		}
		
		int get( void )			{ return( value ); }

		int update( int action, int px, int py );
		void draw( char *label = 0x0 );

	private:
		int value, busy;
};

class gwizToggleButton : public gwizRectWidget {

	public:
		gwizToggleButton(void):gwizRectWidget() { 
			value = 0;
			busy = 0;
		}
		gwizToggleButton(
			int ox, int oy, int sx, int sy
		):gwizRectWidget( ox, oy, sx, sy ) { 
			value = 0;
			busy = 0;
		}
		void init( int ox, int oy, int sx, int sy )	{ 
			set_rect( ox, oy, sx, sy ); 
		}
		
		void set( int val )		{ value = val; }
		int get( void )			{ return( value ); }

		int update( int action, int px, int py );
		void draw( char *label = 0x0, char *op_0 = 0x0, char *op_1 = 0x0 );

	private:
		int value, busy;
};

#if 0
class gwizCycleButton : public gwizRectWidget {

	public:
		gwizCycleButton(void):gwizRectWidget() { 
			value = 0;
			busy = 0;
		}
		gwizCycleButton(
			int ox, int oy, int sx, int sy, 
			int steps
		):gwizRectWidget( ox, oy, sx, sy ) { 
			num_steps = steps;
			value = 0;
			busy = 0;
		}
		void init( int ox, int oy, int sx, int sy, int steps )	{ 
			set_rect( ox, oy, sx, sy ); 
			num_steps = steps;
		}
		
		void set( int val )		{ value = val; }
		int get( void )			{ return( value ); }

		int update( int action, int px, int py );
		void draw( char *label = 0x0, char *op_0 = 0x0, char *op_1 = 0x0 );

	private:
		int num_steps;
		int value, busy;
};
#endif

class gwizStepButton : public gwizRectWidget {

	public:
		gwizStepButton(void):gwizRectWidget() { 
			init( 1.0 ); 
		}
		gwizStepButton(
			int ox, int oy, int sx, int sy, 
			float inc, 
			float min = -HUGE, 
			float max = HUGE
		):gwizRectWidget( ox, oy, sx, sy ) { 
			init( inc, min, max ); 
		}
		void init( 
			int ox, int oy, int sx, int sy, 
			float inc, 
			float min = -HUGE, 
			float max = HUGE
		)	{ 
			set_rect( ox, oy, sx, sy ); 
			init( inc, min, max ); 
		}
		void init( float inc, float min = -HUGE, float max = HUGE )	{
			increment = inc;
			range_min = min;
			range_max = max;
			value = 0.0;
			busy_mode = 0;
			busy = 0;
		}
		void set_range( float min = -HUGE, float max = HUGE )	{
			range_min = min;
			range_max = max;
		}

		void set( float val );
		float get( void )		{ return( value ); }
		int get_int( void )		{ 
			if( value < 0.0 )	{
				return( (int)( value - 0.5 ) ); 
			}
			return( (int)( value + 0.5 ) ); 
		}

		int update( int action, int px, int py );
		void draw( char *label = 0x0, int round_mode = GWIZ_GUI_ROUND_FLOAT, int ensigned = 1 );
		
	private:
		float range_min, range_max;
		float increment, value;
		int busy_mode, busy;
};

class gwizDiscreteMapper : public gwizRectWidget {

	public:
		gwizDiscreteMapper(void):gwizRectWidget() { 
			init( 2 ); 
		}
		gwizDiscreteMapper( 
			int ox, int oy, int sx, int sy, 
			int steps, 
			int click = 0
		):gwizRectWidget( ox, oy, sx, sy ) { 
			init( steps, click ); 
		}
		void init( 
			int ox, int oy, int sx, int sy, 
			int steps, 
			int click = 0
		)	{ 
			set_rect( ox, oy, sx, sy ); 
			init( steps, click ); 
		}
		void init( int steps, int click = 0 )	{
			num_steps = steps;
			num_slots = steps;
			slots = new int[ num_slots ];
			for( int i=0;i<num_slots;i++ ) slots[i] = i;
			normal = 0.0;
			value = 0;
			busy = 0;
			on_click = click;
			clicked = 0;
		}

		void set_groups( int num_groups, int *groups );
		void set_click( int c )	{ on_click = c; }
		void set( int val );
		int set_slot( int s );
		int get( void )			{ return( value ); }
		int get_click( void )	{ return( clicked ); }

		int update( int action, int px, int py );
		void draw( char *label = 0x0, char **names = 0x0 );
		
	private:
		int num_steps;
		int num_slots;
		int *slots;
		float normal;
		int value;
		int busy;
		int on_click, clicked;
};

class gwizScalarMapper : public gwizRectWidget {
	
	public:
		gwizScalarMapper(void):gwizRectWidget() { 
			init( 0.0, 1.0, 1.0 ); 
		}
		gwizScalarMapper( 
			int ox, int oy, int sx, int sy, 
			float min, 
			float max, 
			float power = 1.0 
		):gwizRectWidget( ox, oy, sx, sy ) { 
			init( min, max, power ); 
		}
		void init( 
			int ox, int oy, int sx, int sy, 
			float min, 
			float max, 
			float power = 1.0 
		)	{ 
			set_rect( ox, oy, sx, sy ); 
			init( min, max, power ); 
		}
		void init( 
			float min, 
			float max, 
			float power = 1.0 
		)	 { 
			range_min = min; 
			range_max = max; 
			range_power = power;
			normal = 0.0;
			value = range_min;
			busy = 0;
		}
		
		void set_range( float min, float max )	{
			range_min = min;
			range_max = max;
			set( value );
		}
		void set( float val );
		void wrap( float val );
		float get( void )		{ return( value ); }
		int get_int( void )		{ 
			if( value < 0.0 )	{
				return( (int)( value - 0.5 ) ); 
			}
			return( (int)( value + 0.5 ) ); 
		}

		int update( int action, int px, int py );
		void draw( char *label = 0x0, int round_mode = GWIZ_GUI_ROUND_FLOAT );
		void draw_ghost( char *label = 0x0, int round_mode = GWIZ_GUI_ROUND_FLOAT );
		
	private:
		float range_min, range_max, range_power;
		float normal, value;
		int busy;
};

class gwizRangeMapper : public gwizRectWidget {
	
	public:
		gwizRangeMapper(void):gwizRectWidget() { 
			init( 0.0, 1.0, 1.0 ); 
		}

		gwizRangeMapper( 
			int ox, int oy, int sx, int sy, 
			float min, 
			float max, 
			float power = 1.0, 
			float diff_min = -HUGE, 
			float diff_max = HUGE
		):gwizRectWidget( ox, oy, sx, sy ) { 
			init( min, max, power, diff_min, diff_max ); 
		}

		void init( 
			int ox, int oy, int sx, int sy, 
			float min, 
			float max, 
			float power = 1.0, 
			float diff_min = -HUGE, 
			float diff_max = HUGE
		)	{ 
			set_rect( ox, oy, sx, sy ); 
			init( min, max, power, diff_min, diff_max ); 
		}

		void init( 
			float min, 
			float max, 
			float power = 1.0, 
			float diff_min = -HUGE, 
			float diff_max = HUGE
		)	 { 
			range_min = min; 
			range_max = max; 
			range_power = power;
			normal_a = 0.0;
			value_a = range_min;
			normal_b = 0.0;
			value_b = range_min;
			diff_lock_min = diff_min;
			diff_lock_max = diff_max;
			if( diff_lock_max < diff_lock_min )
				diff_lock_max = diff_lock_min;
			busy_a = 0;
			busy_b = 0;
			busy = 0;
		}
		
		void set_range( float min, float max )	{
			range_min = min;
			range_max = max;
			set( value_a, value_b );
		}
		void set_diff( float min, float max )	{
			diff_lock_min = min;
			diff_lock_max = max;
			if( diff_lock_max < diff_lock_min )
				diff_lock_max = diff_lock_min;
			set( value_a, value_b );
		}
		void set_a( float a );
		void set_b( float b );
		void set( float a, float b )	{ set_b( b ); set_a( a ); }
		void wrap_a( float a );
		void wrap_b( float b );
		
		float get_a( void )				{ return( value_a ); }
		float get_b( void )				{ return( value_b ); }
		int get_int_a( void )		{ 
			if( value_a < 0.0 )	{
				return( (int)( value_a - 0.5 ) ); 
			}
			return( (int)( value_a + 0.5 ) ); 
		}
		int get_int_b( void )		{ 
			if( value_b < 0.0 )	{
				return( (int)( value_b - 0.5 ) ); 
			}
			return( (int)( value_b + 0.5 ) ); 
		}

		int update( int action, int px, int py );
		int update( int action1, int action2, int px, int py );
		void draw( char *label = 0x0, int round_mode = GWIZ_GUI_ROUND_FLOAT );
//		void draw_ghost( char *label = 0x0, int round_mode = GWIZ_GUI_ROUND_FLOAT );
		
	private:
		float range_min, range_max, range_power;
		float normal_a, value_a;
		float normal_b, value_b;
		float diff_lock_min, diff_lock_max;
		int busy_a, busy_b;
		int busy;
};

/////////////////////////////////////////////////

class gwizCrystalBall : public gwizRectWidget	{
	
	public:
		gwizCrystalBall( void ):gwizRectWidget()	{ 
			curr_q = quat_t(); 
			delta_q = quat_t(); 
			r_from = vector_t();
			curr_p = vector_t();
			p_from = vector_t();
			busy_rot = 0;
			busy_pos = 0;
		}
		gwizCrystalBall( int ox, int oy, int diameter )
			:gwizRectWidget( ox, oy, diameter ) { 
			curr_q = quat_t(); 
			delta_q = quat_t(); 
			r_from = vector_t();
			curr_p = vector_t();
			p_from = vector_t();
			busy_rot = 0;
			busy_pos = 0;
		}

		void		set( quat_t q )			{ curr_q = q; delta_q = quat_t(); }
		void		set_delta( quat_t dq )	{ delta_q = dq; }
		void		set( vector_t v )		{ curr_p = v; }
		quat_t		get_quat(void)			{ return( curr_q ); }
		quat_t		get_delta_quat(void)	{ return( delta_q ); }
		vector_t	get_vector(void)		{ return( curr_p ); }

		int	update( 
			int action1, int action2, 
			int xpos, int ypos, 
			float dt = 1.0, 
			float rot_factor = 1.0, 
			float pos_factor = 1.0 
		);
		
		void		draw( void );
		
	protected:
		int bound_ortho_circle( float_t normal_xpos, float_t normal_ypos );
		vector_t intersect_ortho_sphere( float_t normal_xpos, float_t normal_ypos );

	private:
		quat_t curr_q, delta_q;
		vector_t r_from;
		vector_t curr_p, p_from;
		int busy_rot, busy_pos;
};

/////////////////////////////////////////////////

class gwizWidgetAction	{
	
	public:
		gwizWidgetAction( void )	{
			busy_gui = GWIZ_GUI_NULL;
		}
		
		int update( gwizPlacementWidget & widget, int action, int px, int py )	{
			int gui_id = widget.get_id();
			busy_gui = get_busy(
				widget.update( 
					get_action( 
						action, gui_id 
					), 
					px, py 
				), 
				gui_id
			);
			return( busy_gui );
		}
		int update( gwizTriggerButton & widget, int action, int px, int py )	{
			int gui_id = widget.get_id();
			busy_gui = get_busy(
				widget.update( 
					get_action( 
						action, gui_id 
					), 
					px, py 
				), 
				gui_id
			);
			return( busy_gui );
		}
		int update( gwizToggleButton & widget, int action, int px, int py )	{
			int gui_id = widget.get_id();
			busy_gui = get_busy(
				widget.update( 
					get_action( 
						action, gui_id 
					), 
					px, py 
				), 
				gui_id
			);
			return( busy_gui );
		}
#if 0
		int update( gwizCycleButton & widget, int action, int px, int py )	{
			int gui_id = widget.get_id();
			busy_gui = get_busy(
				widget.update( 
					get_action( 
						action, gui_id 
					), 
					px, py 
				), 
				gui_id
			);
			return( busy_gui );
		}
#endif
		int update( gwizStepButton & widget, int action, int px, int py )	{
			int gui_id = widget.get_id();
			busy_gui = get_busy(
				widget.update( 
					get_action( 
						action, gui_id 
					), 
					px, py 
				), 
				gui_id
			);
			return( busy_gui );
		}
		int update( gwizDiscreteMapper & widget, int action, int px, int py )	{
			int gui_id = widget.get_id();
			busy_gui = get_busy(
				widget.update( 
					get_action( 
						action, gui_id 
					), 
					px, py 
				), 
				gui_id
			);
			return( busy_gui );
		}
		int update( gwizScalarMapper & widget, int action, int px, int py )	{
			int gui_id = widget.get_id();
			busy_gui = get_busy(
				widget.update( 
					get_action( 
						action, gui_id 
					), 
					px, py 
				), 
				gui_id
			);
			return( busy_gui );
		}
		int update( gwizRangeMapper & widget, int action, int px, int py )	{
			int gui_id = widget.get_id();
			busy_gui = get_busy(
				widget.update( 
					get_action( 
						action, gui_id 
					), 
					px, py 
				), 
				gui_id
			);
			return( busy_gui );
		}
		int update( gwizRangeMapper & widget, int action1, int action2, int px, int py )	{
			int gui_id = widget.get_id();
			busy_gui = get_busy(
				widget.update( 
					get_action( 
						action1, gui_id 
					), 
					get_action( 
						action2, gui_id 
					), 
					px, py 
				), 
				gui_id
			);
			return( busy_gui );
		}
		int update( 
			gwizCrystalBall & widget, int action1, int action2, 
			int px, int py, float dt, float rot_factor, float pos_factor 
		)	{
			int gui_id = widget.get_id();
			busy_gui = get_busy(
				widget.update( 
					get_action( 
						action1, gui_id 
					), 
					get_action( 
						action2, gui_id 
					), 
					px, py, 
					dt, 
					rot_factor, pos_factor
				), 
				gui_id
			);
			return( busy_gui );
		}
		
	protected:
		int get_action( int button, int gui_id )	{
			return( button && ( ( gui_id == busy_gui )||( busy_gui == GWIZ_GUI_NULL ) ) );
		}
		int get_busy( int gui_busy, int gui_id )	{
			if( gui_busy ) 
				return( gui_id );
			if( gui_id == busy_gui ) 
				return( GWIZ_GUI_NULL );
			return( busy_gui );
		}

	private:
		int busy_gui;
};

/////////////////////////////////////////////////

} // namespace gwiz

char *gwizPrintValue( char *label, float f, int round_mode, int ensigned );

/////////////////////////////////////////////////
#endif
