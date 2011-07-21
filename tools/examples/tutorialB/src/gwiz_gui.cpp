#include <stdio.h>
#include <string.h>
#include  <GL/gl.h>
#include  <GL/glu.h>

#include "gwiz_gui.h"

using namespace gwiz;

#include "sgi_ogl_font.h"

#define RIGHT_LABEL_GAP		2
#define CORNER_SIZE			20

#define EPSILON3			0.0001
#define EPSILON4			0.00001
#define ANGLE_DELTA_FACTOR	10000.0

////////////////////////////////////////////////////////////////////////////

#define COL_BLACK			0

#define COL_LIGHT_GREY		1
#define COL_MID_GREY		2
#define COL_DARK_GREY		3
#define COL_BLACK_GREY		4

#define COL_ALPHA_GREY		5
#define COL_ALPHA2_GREY		6
#define COL_ALPHA3_GREY		7

#define COL_PALE_YELLOW		8
#define COL_DARK_YELLOW		9
#define COL_PALE_BLUE		10
#define COL_DARK_BLUE		11

int gwiz_gui_debug_tmp = 0;

////////////////////////////////////////////////////////////////////////////

int gwizGetGuiAction( int button, int gui_id, int busy_id )	{

		return( button && ( ( gui_id == busy_id )||( busy_id == gwiz::GWIZ_GUI_NULL ) ) );
}

int gwizGetGuiBusy( int gui_busy, int gui_id, int busy_id )	{

		if( gui_busy ) 
			return( gui_id );
		if( gui_id == busy_id ) 
			return( gwiz::GWIZ_GUI_NULL );
		return( busy_id );
}

////////////////////////////////////////////////////////////////////////////

float GWIZ_map_normal_to_power_range( float n, float min, float max, float power )	{

		if( n < 0.0 )
			return( min - powf( -n, power ) * ( max - min ) );
		return( min + powf( n, power ) * ( max - min ) );
}

float GWIZ_unmap_normal_from_power_range( float r, float min, float max, float power )	{
		
		if( ( max - min ) > 0.0 )	{
			return( powf( ( r - min )/( max - min ), 1.0/power ) );
		}
		return( 0.0 );
}

void GWIZ_color( int color_id )	{
	
	switch( color_id )	{
		case COL_BLACK:	
			glColor4ub( 0, 0, 0, 255 );
			break;
		case COL_LIGHT_GREY:	
			glColor4ub( 191, 191, 191, 255 );
			break;
		case COL_MID_GREY:	
			glColor4ub( 127, 127, 127, 255 );
			break;
		case COL_DARK_GREY:	
			glColor4ub( 63, 63, 63, 255 );
			break;
		case COL_BLACK_GREY:	
			glColor4ub( 31, 31, 31, 255 );
			break;
		case COL_ALPHA_GREY:	
			glColor4ub( 127, 127, 127, 127 );
			break;
		case COL_ALPHA2_GREY:	
			glColor4ub( 127, 127, 127, 63 );
			break;
		case COL_ALPHA3_GREY:	
			glColor4ub( 127, 127, 127, 31 );
			break;
		case COL_PALE_YELLOW:	
			glColor4ub( 255, 255, 127, 255 );
			break;
		case COL_DARK_YELLOW:	
			glColor4ub( 127, 127, 63, 255 );
			break;
		case COL_PALE_BLUE:	
			glColor4ub( 127, 127, 255, 255 );
			break;
		case COL_DARK_BLUE:	
			glColor4ub( 63, 63, 127, 255 );
			break;
	}
}

////////////////////////////////////////////////////////////////////////////

/*
  print_float_value:

	0.000001	-> 1.000u
	0.00001		-> 10.00u
	0.0001		-> 100.0u

	0.001		-> 0.001
	0.01		-> 0.010
	0.1			-> 0.100

	1.0			-> 1.000
	10.0		-> 10.00
	100.0		-> 100.0

	1000.0		-> 1.000K
	10000.0		-> 10.00K
	100000.0	-> 100.0K

	1000000.0	-> 1.000M
*/

char *print_float_value( char *label, float f )	{
	static char abs_label[256];
	float abs_f = fabs( f );
	
		if( abs_f < 1.0 )	{
			if( abs_f < 0.000000001 )	{
				sprintf( abs_label, "0.000 " );
			}
			else
			if( abs_f < 0.00001 )	{
				sprintf( abs_label, "%.3fu", abs_f * 1000000.0 );
			}
			else
			if( abs_f < 0.0001 )	{
				sprintf( abs_label, "%.2fu", abs_f * 1000000.0 );
			}
			else
			if( abs_f < 0.001 )	{
				sprintf( abs_label, "%.1fu", abs_f * 1000000.0 );
			}
			else	{
				sprintf( abs_label, "%.3f ", abs_f );
			}
		}
		else	{
			if( abs_f < 9.9995 )	{
				sprintf( abs_label, "%.3f ", abs_f );
			}
			else
			if( abs_f < 99.995 )	{
				sprintf( abs_label, "%.2f ", abs_f );
			}
			else
			if( abs_f < 999.95 )	{
				sprintf( abs_label, "%.1f ", abs_f );
			}
			else
			if( abs_f < 9999.5 )	{
				sprintf( abs_label, "%.3fK", abs_f * 0.001 );
			}
			else
			if( abs_f < 99995.0 )	{
				sprintf( abs_label, "%.2fK", abs_f * 0.001 );
			}
			else
			if( abs_f < 999950.0 )	{
				sprintf( abs_label, "%.1fK", abs_f * 0.001 );
			}
			else	{
				sprintf( abs_label, "%.3fM", abs_f * 0.000001 );
			}
		}
		
		if( f < 0.0 )
			sprintf( label, "-%s", abs_label );
		else
			sprintf( label, " %s", abs_label );
		return( label );
}

char *print_int_value( char *label, int i, int ensigned = 0 )	{
		
		if( ensigned )	{
			if( i > 0 )
				sprintf( label, "+%d", i );
			else
			if( i < 0 )
				sprintf( label, "%d", i );
			else
				sprintf( label, " 0" );
		}
		else	{
			if( i < 0 )
				sprintf( label, "%d", i );
			else
				sprintf( label, " %d", i );
		}
		return( label );
}

char *print_value( char *label, float f, int round_mode, int ensigned = 0 )	{
		
		if( round_mode == gwiz::GWIZ_GUI_NO_NUMBER )	{
			sprintf( label, "%s", "" );
			return( label );
		}
		if( round_mode == gwiz::GWIZ_GUI_ROUND_FLOAT )	{
			return( print_float_value( label, f ) );
		}
		if( round_mode == gwiz::GWIZ_GUI_ROUND_INT )	{
			if( f < 0.0 )
				return( print_int_value( label, (int)( f - 0.5 ), ensigned ) );
			return( print_int_value( label, (int)( f + 0.5 ), ensigned ) );
		}
		if( round_mode == gwiz::GWIZ_GUI_ROUND_FLOOR )	{
			return( print_int_value( label, (int)f, ensigned ) );
		}
		if( round_mode == gwiz::GWIZ_GUI_ROUND_CIEL )	{
			return( print_int_value( label, (int)( f + 1.0 ), ensigned ) );
		}
		return( label );
}

char *gwizPrintValue( char *label, float f, int round_mode, int ensigned )	{
	print_value( label, f, round_mode, ensigned );
	return( label );
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

float gwiz::gwizRectWidget::get_normal_x( float px )	{
	
	if( size_x > 0 )	{
		return( (float)( px - origin_x )/(float)size_x );
	}
	return( 0.0 );
}

float gwiz::gwizRectWidget::get_normal_y( float py )	{

	return( (float)( py - origin_y )/(float)size_y );
}

int gwiz::gwizRectWidget::bound( float px, float py )	{

	float normal_ypos = get_normal_y( py );
		if( normal_ypos > 1.0 ) return( 0 );
		if( normal_ypos < 0.0 ) return( 0 );

	float normal_xpos = get_normal_x( px );
		if( normal_xpos > 1.0 ) return( 0 );
		if( normal_xpos < 0.0 ) return( 0 );
		return( 1 );
}

int gwiz::gwizRectWidget::bound_sloppy_x( float px, float py )	{

		if( py < origin_y ) return( 0 );
		if( py > ( origin_y + size_y ) ) return( 0 );
		if( px < ( origin_x - SGI_GLFONT_SPACE_WID ) ) return( 0 );
		if( px > ( origin_x + size_x + SGI_GLFONT_SPACE_WID ) ) return( 0 );
		return( 1 );
}

int gwiz::gwizRectWidget::bound_corner_bl( float px, float py )	{
		
		if( px < origin_x ) return( 0 );
		if( py < origin_y ) return( 0 );
		float dx = px - origin_x;
		float dy = py - origin_y;
		return( ( dx + dy ) < CORNER_SIZE );
}

int gwiz::gwizRectWidget::bound_corner_tr( float px, float py )	{
		
		if( px > ( origin_x + size_x ) ) return( 0 );
		if( py > ( origin_y + size_y ) ) return( 0 );
		float dx = origin_x + size_x - px;
		float dy = origin_y + size_y - py;
		return( ( dx + dy ) < CORNER_SIZE );
}

////////////////////////////////////////////////////////////////////////////

int gwiz::gwizPlacementWidget::update( int action, int px, int py )	{

		if( action )	{
			if( mini )	{
				if( bound_corner_bl( px, py ) )	{
					if( !busy_m )	{
						busy_m = 1;
						mini = 0;
					}
				}
			}
			else
			if( bound( px, py ) )	{
				if( !busy_p )	{
					if( resizeable )	{
						if( bound_corner_tr( px, py ) )	{
							if( !busy_r )	{
								busy_r = 1;
								dx = px - ( origin_x + size_x );
								dy = py - ( origin_y + size_y );
							}
						}
					}
					if( !busy_r )	{
						if( minifiable )	{
							if( bound_corner_bl( px, py ) )	{
								if( !busy_m )	{
									busy_m = 1;
									mini = 1;
								}
							}
						}
						if( !busy_m )	{
							busy_p = 1;
							dx = px - origin_x;
							dy = py - origin_y;
						}
					}
				}
			}
		}
		else	{
			busy_p = 0;
			busy_m = 0;
			busy_r = 0;
		}

		if( busy_p )	{
			set_origin( px - dx, py - dy ); 
		}
		else
		if( busy_r )	{
			int sx = px - dx - origin_x;
			if( sx < CORNER_SIZE )
				sx = CORNER_SIZE;
			int sy = py - dy - origin_y;
			if( sy < CORNER_SIZE )
				sy = CORNER_SIZE;
			set_size( sx, sy ); 
		}

		return( busy_p || busy_m || busy_r );
}

void gwiz::gwizPlacementWidget::draw( void )	{
	
		if( minifiable || resizeable )	{
			if( mini )	{
				GWIZ_color( COL_PALE_BLUE );
				glBegin( GL_POLYGON );
					glVertex2i( origin_x, origin_y );
					glVertex2i( origin_x, origin_y + CORNER_SIZE - 1 );
					glVertex2i( origin_x + CORNER_SIZE - 1, origin_y );
				glEnd();
			}
			else	{
				if( resizeable )	{
					if( busy_r )	{
						GWIZ_color( COL_PALE_YELLOW );
						glBegin( GL_POLYGON );
							glVertex2i( origin_x + size_x, origin_y + size_y );
							glVertex2i( origin_x + size_x, origin_y + size_y - CORNER_SIZE );
							glVertex2i( origin_x + size_x - CORNER_SIZE, origin_y + size_y );
						glEnd();
					}
					else	{
						GWIZ_color( COL_DARK_YELLOW );
						glBegin( GL_LINE_STRIP );
							glVertex2i( origin_x + size_x - CORNER_SIZE + 1, origin_y + size_y - 1 );
							glVertex2i( origin_x + size_x - 1, origin_y + size_y - 1 );
							glVertex2i( origin_x + size_x - 1, origin_y + size_y - CORNER_SIZE );
						glEnd();
					}
				}
				if( minifiable )	{
					GWIZ_color( COL_DARK_BLUE );
					glBegin( GL_LINE_STRIP );
						glVertex2i( origin_x + CORNER_SIZE - 2, origin_y );
						glVertex2i( origin_x, origin_y );
						glVertex2i( origin_x, origin_y + CORNER_SIZE - 1 );
					glEnd();
				}
			}
		}
		else
		if( busy_p )	{
			glBegin( GL_POLYGON );
				GWIZ_color( COL_ALPHA3_GREY );
				glVertex2i( origin_x, origin_y + size_y );
				glVertex2i( origin_x, origin_y );
				GWIZ_color( COL_ALPHA2_GREY );
				glVertex2i( origin_x + size_x, origin_y );
				glVertex2i( origin_x + size_x, origin_y + size_y );
			glEnd();
		}
}

////////////////////////////////////////////////////////////////////////////

int gwiz::gwizTriggerButton::update( int action, int px, int py )	{
	
		int is_bound = bound( px, py );
		if( action )	{
			if( is_bound )	{
				if( !busy )	{
					busy = 1;
					value = 1;
				}
				else	{
					value = 0;
				}
			}
			else	{
				value = 0;
			}
		}				
		else	{
			busy = 0;
			value = 0;
		}
		return( busy );	
}

void gwiz::gwizTriggerButton::draw( char *label )	{

		GWIZ_color( COL_DARK_GREY );
		glBegin( GL_LINE_LOOP );
			glVertex2i( origin_x, origin_y );
			glVertex2i( origin_x + size_x, origin_y );
			glVertex2i( origin_x + size_x, origin_y + size_y );
			glVertex2i( origin_x, origin_y + size_y );
		glEnd();

	// lower
		if( busy )
			GWIZ_color( COL_DARK_YELLOW );
		else
			GWIZ_color( COL_BLACK_GREY );
		glBegin( GL_LINE_STRIP );
			glVertex2i( origin_x + 1, origin_y + 1 );
			glVertex2i( origin_x + size_x - 1, origin_y + 1 );
			glVertex2i( origin_x + size_x - 1, origin_y + size_y - 1 );
		glEnd();
	// upper
		if( busy )
			GWIZ_color( COL_BLACK_GREY );
		else
			GWIZ_color( COL_DARK_YELLOW );
		glBegin( GL_LINE_STRIP );
			glVertex2i( origin_x + size_x - 1, origin_y + size_y - 1 );
			glVertex2i( origin_x + 1, origin_y + size_y - 1 );
			glVertex2i( origin_x + 1, origin_y + 1 );
		glEnd();

		GWIZ_color( COL_MID_GREY );
		if( label )	{
			alphabet.print( label, origin_x - (int)strlen( label ) * SGI_GLFONT_SPACE_WID, origin_y + 2 );
		}
		if( value )	{
			char val_str[128] = " *";
			alphabet.print( val_str, origin_x + size_x + RIGHT_LABEL_GAP, origin_y + 2 );
		}
}

////////////////////////////////////////////////////////////////////////////

int gwiz::gwizToggleButton::update( int action, int px, int py )	{
	
		int is_bound = bound( px, py );
		if( action )	{
			if( is_bound )	{
				if( !busy )	{
					busy = 1;
					value = !value;
				}
			}
		}				
		else	{
			busy = 0;
		}
		return( busy );	
}

void gwiz::gwizToggleButton::draw( char *label, char *op_0, char *op_1 )	{

		GWIZ_color( COL_DARK_GREY );
		glBegin( GL_LINE_LOOP );
			glVertex2i( origin_x, origin_y );
			glVertex2i( origin_x + size_x, origin_y );
			glVertex2i( origin_x + size_x, origin_y + size_y );
			glVertex2i( origin_x, origin_y + size_y );
		glEnd();

	// lower
		if( value )
			GWIZ_color( COL_DARK_YELLOW );
		else
			GWIZ_color( COL_BLACK_GREY );
		glBegin( GL_LINE_STRIP );
			glVertex2i( origin_x + 1, origin_y + 1 );
			glVertex2i( origin_x + size_x - 1, origin_y + 1 );
			glVertex2i( origin_x + size_x - 1, origin_y + size_y - 1 );
		glEnd();
	// upper
		if( value )
			GWIZ_color( COL_BLACK_GREY );
		else
			GWIZ_color( COL_DARK_YELLOW );
		glBegin( GL_LINE_STRIP );
			glVertex2i( origin_x + size_x - 1, origin_y + size_y - 1 );
			glVertex2i( origin_x + 1, origin_y + size_y - 1 );
			glVertex2i( origin_x + 1, origin_y + 1 );
		glEnd();

		GWIZ_color( COL_MID_GREY );
		if( label )	{
			alphabet.print( label, origin_x - (int)strlen( label ) * SGI_GLFONT_SPACE_WID, origin_y + 2 );
		}
		char val_str[128];
		if( value )	{
			if( op_1 )
				sprintf( val_str, " %s", op_1 );
			else
				sprintf( val_str, " ON" );
		}
		else	{
			if( op_0 )
				sprintf( val_str, " %s", op_0 );
			else
				sprintf( val_str, " OFF" );
		}
		alphabet.print( val_str, origin_x + size_x + RIGHT_LABEL_GAP, origin_y + 2 );
}

////////////////////////////////////////////////////////////////////////////

#define GWIZ_STEP_LEFT_MODE		1
#define GWIZ_STEP_ZERO_MODE		2
#define GWIZ_STEP_RIGHT_MODE	3

void gwiz::gwizStepButton::set( float val )	{
		value = val; 
		if( value < range_min ) value = range_min;
		if( value > range_max ) value = range_max;
}

int gwiz::gwizStepButton::update( int action, int px, int py )	{
	
		int is_bound = bound( px, py );
		if( action )	{
			if( is_bound )	{
				if( !busy )	{
					busy = 1;
					int half_x = size_x/2;
					int half_y = size_y/2;
					int pos_x = (int)( get_normal_x( px ) * (float)size_x );
					if( pos_x < ( half_x - half_y ) )	{
						set( value - increment );
						busy_mode = GWIZ_STEP_LEFT_MODE;
					}
					else
					if( pos_x < ( half_x + half_y ) )	{
						set( 0.0 );
						busy_mode = GWIZ_STEP_ZERO_MODE;
					}
					else	{
						set( value + increment );
						busy_mode = GWIZ_STEP_RIGHT_MODE;
					}
				}
			}
		}
		else	{
			busy = 0;
			busy_mode = 0;
		}
		return( busy );
}

void gwiz::gwizStepButton::draw( char *label, int round_mode, int ensigned )	{
	
		int half_x = size_x/2;
		int half_y = size_y/2;
		int stop_l = half_x - half_y;
		int stop_r = half_x + half_y;
		int stop_l2 = stop_l - 5;
		int stop_r2 = stop_r + 5;
		int tip;
		if( stop_l2 > size_y )
			tip = 2;
		else
			tip = 1;
		
	// LEFT ARROW:
		GWIZ_color( COL_DARK_GREY );
		glBegin( GL_LINE_LOOP );
			glVertex2i( origin_x, origin_y + half_y );
			glVertex2i( origin_x + stop_l2 - 1, origin_y );
			glVertex2i( origin_x + stop_l2 - 1, origin_y + size_y );
		glEnd();

		if( busy_mode == GWIZ_STEP_LEFT_MODE )
			GWIZ_color( COL_DARK_YELLOW );
		else
			GWIZ_color( COL_BLACK_GREY );
		glBegin( GL_LINE_STRIP );
			glVertex2i( origin_x + tip, origin_y + half_y );
			glVertex2i( origin_x + stop_l2 - 2, origin_y + 1 );
			glVertex2i( origin_x + stop_l2 - 2, origin_y + size_y - 1 );
		glEnd();

		if( busy_mode == GWIZ_STEP_LEFT_MODE )
			GWIZ_color( COL_BLACK_GREY );
		else
			GWIZ_color( COL_DARK_YELLOW );
		glBegin( GL_LINE_STRIP );
			glVertex2i( origin_x + stop_l2 - 2, origin_y + size_y - 1 );
			glVertex2i( origin_x + tip, origin_y + half_y );
		glEnd();
		
	// CENTER BOX:
		GWIZ_color( COL_DARK_GREY );
		glBegin( GL_LINE_LOOP );
			glVertex2i( origin_x + stop_l, origin_y );
			glVertex2i( origin_x + stop_r, origin_y );
			glVertex2i( origin_x + stop_r, origin_y + size_y );
			glVertex2i( origin_x + stop_l, origin_y + size_y );
		glEnd();

		if( busy_mode == GWIZ_STEP_ZERO_MODE )
			GWIZ_color( COL_DARK_YELLOW );
		else
			GWIZ_color( COL_BLACK_GREY );
		glBegin( GL_LINE_STRIP );
			glVertex2i( origin_x + stop_l + 1, origin_y + 1 );
			glVertex2i( origin_x + stop_r - 1, origin_y + 1 );
			glVertex2i( origin_x + stop_r - 1, origin_y + size_y - 1 );
		glEnd();

		if( busy_mode == GWIZ_STEP_ZERO_MODE )
			GWIZ_color( COL_BLACK_GREY );
		else
			GWIZ_color( COL_DARK_YELLOW );
		glBegin( GL_LINE_STRIP );
			glVertex2i( origin_x + stop_r - 1, origin_y + size_y - 1 );
			glVertex2i( origin_x + stop_l + 1, origin_y + size_y - 1 );
			glVertex2i( origin_x + stop_l + 1, origin_y + 1 );
		glEnd();

	// RIGHT ARROW:
		GWIZ_color( COL_DARK_GREY );
		glBegin( GL_LINE_LOOP );
			glVertex2i( origin_x + size_x, origin_y + half_y );
			glVertex2i( origin_x + stop_r2 + 1, origin_y + size_y );
			glVertex2i( origin_x + stop_r2 + 1, origin_y );
		glEnd();

		if( busy_mode == GWIZ_STEP_RIGHT_MODE )
			GWIZ_color( COL_DARK_YELLOW );
		else
			GWIZ_color( COL_BLACK_GREY );
		glBegin( GL_LINE_STRIP );
			glVertex2i( origin_x + stop_r2 + 2, origin_y + 1 );
			glVertex2i( origin_x + size_x - tip, origin_y + half_y );
		glEnd();

		if( busy_mode == GWIZ_STEP_RIGHT_MODE )
			GWIZ_color( COL_BLACK_GREY );
		else
			GWIZ_color( COL_DARK_YELLOW );
		glBegin( GL_LINE_STRIP );
			glVertex2i( origin_x + size_x - tip, origin_y + half_y );
			glVertex2i( origin_x + stop_r2 + 2, origin_y + size_y - 1 );
			glVertex2i( origin_x + stop_r2 + 2, origin_y + 1 );
		glEnd();

		GWIZ_color( COL_MID_GREY );
		if( label )	{
			alphabet.print( label, origin_x - (int)strlen( label ) * SGI_GLFONT_SPACE_WID, origin_y + 2 );
		}
		char val_str[128];
		if( round_mode )	{
			print_value( val_str, value, round_mode, ensigned );
			alphabet.print( val_str, origin_x + size_x + RIGHT_LABEL_GAP, origin_y + 2 );
		}
}

////////////////////////////////////////////////////////////////////////////

void gwiz::gwizDiscreteMapper::set_groups( int num_groups, int *groups )	{
	int i, j;

		if( slots ) 
			delete [] slots;
		num_slots = num_steps + num_groups - 1;
		slots = new int[ num_slots ];

		int c = 0;
		int s = 0;
		for( j=0;j<num_groups;j++ )	{
			for( i=0;i<groups[j];i++ ) 
				slots[s++] = c++;
			if( j < ( num_groups-1 ) ) 
				slots[s++] = -1;
		}
		num_slots = s;
}
		
void gwiz::gwizDiscreteMapper::set( int val )	{
	
		value = val; 
		if( value < 0 ) value = 0;
		if( value >= num_steps ) value = num_steps-1;
}

int gwiz::gwizDiscreteMapper::set_slot( int s )	{

		int v = slots[ s ];
		if( v >= 0 )	{
			set( v );
			return( 1 );
		}
		return( 0 );
}

int gwiz::gwizDiscreteMapper::update( int action, int px, int py )	{

		int is_bound = bound( px, py );
		if( action )	{
			if( is_bound )	{
				if( !busy )	{
					set_slot( (int)( get_normal_x( px ) * (float)num_slots ) );
					clicked = 1;
				}
				else
					clicked = 0;
				busy = 1;
			}
		}				
		else	{
			busy = 0;
		}
		if( busy )	{
			if( !on_click )	{
				set_slot( (int)( get_normal_x( px ) * (float)num_slots ) );
			}
		}
		return( busy );
}

void gwiz::gwizDiscreteMapper::draw( char *label, char **names )	{
	int i;
	
		GWIZ_color( COL_DARK_GREY );
		glBegin( GL_LINE_LOOP );
			glVertex2i( origin_x, origin_y );
			glVertex2i( origin_x + size_x, origin_y );
			glVertex2i( origin_x + size_x, origin_y + size_y );
			glVertex2i( origin_x, origin_y + size_y );
		glEnd();
		
		float step_wid = (float)size_x / (float)num_slots;
		float step_x = 0.0;

		for( i=0;i<num_slots;i++ )	{

			if( slots[ i ] >= 0 )	{
				int curr;
				if( slots[ i ] == value ) 
					curr = 1;
				else 
					curr = 0;
				
			// lower
				if( curr )
					GWIZ_color( COL_DARK_YELLOW );
				else
					GWIZ_color( COL_BLACK_GREY );
				glBegin( GL_LINE_STRIP );
					glVertex2i( (int)( origin_x + step_x + 1 ), origin_y + 1 );
					glVertex2i( (int)( origin_x + step_x + step_wid - 1 ), origin_y + 1 );
					glVertex2i( (int)( origin_x + step_x + step_wid - 1 ), origin_y + size_y - 1 );
				glEnd();
	
			// upper
				if( curr )
					GWIZ_color( COL_BLACK_GREY );
				else
					GWIZ_color( COL_DARK_YELLOW );
				glBegin( GL_LINE_STRIP );
					glVertex2i( (int)( origin_x + step_x + step_wid - 1 ), origin_y + size_y - 1 );
					glVertex2i( (int)( origin_x + step_x + 1 ), origin_y + size_y - 1 );
					glVertex2i( (int)( origin_x + step_x + 1 ), origin_y + 1 );
				glEnd();
			}
			else	{
				GWIZ_color( COL_DARK_GREY );
				glBegin( GL_POLYGON );
					glVertex2i( (int)( origin_x + step_x ), origin_y + 1 );
					glVertex2i( (int)( origin_x + step_x + step_wid + 1 ), origin_y + 1 );
					glVertex2i( (int)( origin_x + step_x + step_wid + 1 ), origin_y + size_y );
					glVertex2i( (int)( origin_x + step_x ), origin_y + size_y );
				glEnd();
			}

			step_x += step_wid;
		}
		
		GWIZ_color( COL_MID_GREY );
		if( label )	{
			alphabet.print( label, origin_x - (int)strlen( label ) * SGI_GLFONT_SPACE_WID, origin_y + 2 );
		}
		char val_str[128];
		if( names )	{
			if( names[ value ] )	{
				sprintf( val_str, " %s", names[ value ] );
			}
			else
				sprintf( val_str, " <null>" );
		}
		else
			sprintf( val_str, " %d", value );
		alphabet.print( val_str, origin_x + size_x + RIGHT_LABEL_GAP, origin_y + 2 );
}

////////////////////////////////////////////////////////////////////////////

void gwiz::gwizScalarMapper::set( float val )	{ 

		value = val; 
		if( value < range_min ) value = range_min;
		if( value > range_max ) value = range_max;
		normal = GWIZ_unmap_normal_from_power_range( value, range_min, range_max, range_power );
}

void gwiz::gwizScalarMapper::wrap( float val )	{
		if( val < range_min ) 
			set( range_max - ( range_min - val ) );
		else
		if( val > range_max ) 
			set( range_min + ( val - range_max ) );
		else
			set( val );
}

int gwiz::gwizScalarMapper::update( int action, int px, int py )	{

		int is_bound = bound_sloppy_x( px, py );
		if( action )	{
			if( is_bound )	{
				busy = 1;
			}
		}				
		else	{
			busy = 0;
		}
		if( busy )	{
			set( GWIZ_map_normal_to_power_range( get_normal_x( px ), range_min, range_max, range_power ) );
		}
		return( busy );
}

void gwiz::gwizScalarMapper::draw( char *label, int round_mode )	{

		GWIZ_color( COL_DARK_GREY );
		int half_y = size_y/2;
		glBegin( GL_LINES );
			glVertex2i( origin_x, origin_y );
			glVertex2i( origin_x, origin_y + size_y );
			glVertex2i( origin_x + size_x, origin_y );
			glVertex2i( origin_x + size_x, origin_y + size_y );
			glVertex2i( origin_x, origin_y + half_y );
			glVertex2i( origin_x + size_x, origin_y + half_y );
		glEnd();
		
#if 0
		int value_x = (int)( normal * size_x );
#else
		int value_x = (int)( normal * (float)size_x + 0.5 );
#endif

		GWIZ_color( COL_PALE_YELLOW );
		glBegin( GL_POLYGON );
			glVertex2i( origin_x + value_x - size_y/2, origin_y );
			glVertex2i( origin_x + value_x, origin_y );
			glVertex2i( origin_x + value_x, origin_y + size_y );
		glEnd();

		GWIZ_color( COL_MID_GREY );
		if( label )	{
			int displace = 0;
			if( value_x < size_y/2 )	{
				displace = value_x - size_y/2;
			}
			alphabet.print( 
				label, 
				origin_x - (int)strlen( label ) * SGI_GLFONT_SPACE_WID + displace, 
				origin_y + 2 
			);
		}
		char val_str[128];
		if( round_mode )	{
			print_value( val_str, value, round_mode );
			alphabet.print( val_str, origin_x + size_x + RIGHT_LABEL_GAP, origin_y + 2 );
		}
}

void gwiz::gwizScalarMapper::draw_ghost( char *label, int round_mode )	{

#if 0
		int value_x = (int)( normal * size_x );
#else
		int value_x = (int)( normal * (float)size_x + 0.5 );
#endif

		GWIZ_color( COL_DARK_YELLOW );
		glBegin( GL_LINE_LOOP );
			glVertex2i( origin_x + value_x - size_y/2, origin_y );
			glVertex2i( origin_x + value_x, origin_y );
			glVertex2i( origin_x + value_x, origin_y + size_y );
		glEnd();

		GWIZ_color( COL_MID_GREY );
		if( label )	{
			int displace = 0;
			if( value_x < size_y/2 )	{
				displace = value_x - size_y/2;
			}
			alphabet.print( 
				label, 
				origin_x - (int)strlen( label ) * SGI_GLFONT_SPACE_WID + displace, 
				origin_y + 2 
			);
		}
		char val_str[128];
		if( round_mode )	{
			print_value( val_str, value, round_mode );
			alphabet.print( val_str, origin_x + size_x + RIGHT_LABEL_GAP, origin_y + 2 );
		}
}

////////////////////////////////////////////////////////////////////////////
#define DIFF_EPSILON 0.001

void gwiz::gwizRangeMapper::set_a( float a )	{ 

		value_a = a; 
		if( value_a < range_min ) 
			value_a = range_min;
		if( value_a > range_max ) 
			value_a = range_max;
		if( diff_lock_min > -HUGE )	{
			if( ( value_b - value_a ) < ( diff_lock_min - DIFF_EPSILON ) )	{
				set_b( value_a + diff_lock_min );
			}
		}
		if( diff_lock_max < HUGE )	{
			if( ( value_b - value_a ) > ( diff_lock_max + DIFF_EPSILON ) ) 
				set_b( value_a + diff_lock_max );
		}
		normal_a = GWIZ_unmap_normal_from_power_range( value_a, range_min, range_max, range_power );
}

void gwiz::gwizRangeMapper::set_b( float b )	{ 

		value_b = b; 
		if( value_b < range_min ) 
			value_b = range_min;
		if( value_b > range_max ) 
			value_b = range_max;
		if( diff_lock_min > -HUGE )	{
			if( ( value_b - value_a ) < ( diff_lock_min - DIFF_EPSILON ) )
				set_a( value_b - diff_lock_min );
		}
		if( diff_lock_max < HUGE )	{
			if( ( value_b - value_a ) > ( diff_lock_max + DIFF_EPSILON ) ) 
				set_a( value_b - diff_lock_max );
		}
		normal_b = GWIZ_unmap_normal_from_power_range( value_b, range_min, range_max, range_power );
}

void gwiz::gwizRangeMapper::wrap_a( float a )	{

		if( a < range_min ) 
			set_a( range_max - ( range_min - a ) );
		else
		if( a > ( range_max - diff_lock_min ) ) 
			set_a( range_min + ( a - range_max ) );
		else
			set_a( a );
}

void gwiz::gwizRangeMapper::wrap_b( float b )	{

		if( b < ( range_min + diff_lock_max ) ) 
			set_b( range_max - ( range_min - b ) );
		else
		if( b > range_max ) 
			set_b( range_min + ( b - range_max ) );
		else
			set_b( b );
}

int gwiz::gwizRangeMapper::update( int action, int px, int py )	{

		int is_bound = bound_sloppy_x( px, py );
		if( action )	{
			if( is_bound )	{
				float vertical = get_normal_y( py );
				if( !busy_b )	{
					if( vertical < 0.5 )	{
						busy_a = 1;
					}
				}
				if( !busy_a )	{
					if( vertical >= 0.5 )	{
						busy_b = 1;
					}
				}
			}
		}
		else	{
			busy_a = 0;
			busy_b = 0;
		}
		busy = busy_a || busy_b;
		if( busy )	{
			float value = GWIZ_map_normal_to_power_range( 
				get_normal_x( px ), 
				range_min, 
				range_max, 
				range_power 
			);
			if( busy_a ) 
				set_a( value );
			else
				set_b( value );
		}
		return( busy );
}

int gwiz::gwizRangeMapper::update( int action1, int action2, int px, int py )	{

		int is_bound = bound_sloppy_x( px, py );
		if( action1 || action2 )	{
			if( is_bound )	{
				float vertical =  get_normal_y( py );
				if( !busy_b )	{
					if( vertical < 0.5 )	{
						busy_a = 1;
					}
				}
				if( !busy_a )	{
					if( vertical >= 0.5 )	{
						busy_b = 1;
					}
				}
			}
		}
		else	{
			busy_a = 0;
			busy_b = 0;
		}
		busy = busy_a || busy_b;
		if( busy )	{
			float value = GWIZ_map_normal_to_power_range( 
				get_normal_x( px ), 
				range_min, 
				range_max, 
				range_power 
			);
			if( action2 )	{
				set_a( value );
				set_b( value );
			}
			else
			if( busy_a ) 
				set_a( value );
			else
				set_b( value );
		}
		return( busy );
}

void gwiz::gwizRangeMapper::draw( char *label, int round_mode )	{

		int half_y = size_y/2;
		GWIZ_color( COL_DARK_GREY );
		glBegin( GL_LINES );
			glVertex2i( origin_x, origin_y );
			glVertex2i( origin_x, origin_y + size_y );
			glVertex2i( origin_x + size_x, origin_y );
			glVertex2i( origin_x + size_x, origin_y + size_y );
			glVertex2i( origin_x, origin_y + half_y );
			glVertex2i( origin_x + size_x, origin_y + half_y );
		glEnd();

#if 0
		int value_ax = (int)( normal_a * size_x );
		int value_bx = (int)( normal_b * size_x );
#else
		int value_ax = (int)( normal_a * size_x + 0.5 );
		int value_bx = (int)( normal_b * size_x + 0.5 );
#endif
		GWIZ_color( COL_PALE_YELLOW );
		if( ( diff_lock_min == -HUGE )&&( diff_lock_max == HUGE ) )	{
			glBegin( GL_POLYGON );
				glVertex2i( origin_x + value_ax - size_y/2, origin_y + half_y );
				glVertex2i( origin_x + value_ax, origin_y );
				glVertex2i( origin_x + value_ax, origin_y + half_y );
			glEnd();
			glBegin( GL_POLYGON );
				glVertex2i( origin_x + value_bx - size_y/2, origin_y + half_y );
				glVertex2i( origin_x + value_bx, origin_y + half_y );
				glVertex2i( origin_x + value_bx, origin_y + size_y );
			glEnd();
		}
		else	{
			glBegin( GL_POLYGON );
				glVertex2i( origin_x + value_ax - size_y/2, origin_y + half_y );
				glVertex2i( origin_x + value_ax, origin_y );
				glVertex2i( origin_x + value_ax, origin_y + half_y );
			glEnd();
			glBegin( GL_POLYGON );
				glVertex2i( origin_x + value_bx, origin_y + half_y );
				glVertex2i( origin_x + value_bx + size_y/2, origin_y + half_y );
				glVertex2i( origin_x + value_bx, origin_y + size_y );
			glEnd();
		}

		GWIZ_color( COL_MID_GREY );
		if( label )	{
			int min_x = value_ax;
			if( value_bx < min_x ) 
				min_x = value_bx;
			int displace = 0;
			if( min_x < size_y/2 ) 
				displace = min_x - size_y/2;
			alphabet.print( 
				label, 
				origin_x - (int)strlen( label ) * SGI_GLFONT_SPACE_WID + displace, 
				origin_y + 2
			);
		}
		char val_a_str[128];
		char val_b_str[128];
		char range_str[128];
		if( round_mode )	{
			print_value( val_a_str, value_a, round_mode );
			print_value( val_b_str, value_b, round_mode );
			sprintf( range_str, "%s : %s", val_a_str, val_b_str );
			alphabet.print( range_str, origin_x + size_x + RIGHT_LABEL_GAP, origin_y + 2 );
		}
}

////////////////////////////////////////////////////////////////////////////

int gwiz::gwizCrystalBall::bound_ortho_circle( float_t normal_xpos, float_t normal_ypos )	{
	
		if( normal_xpos > 1.0 ) return( 0 );
		if( normal_xpos < -1.0 ) return( 0 );
		if( normal_ypos > 1.0 ) return( 0 );
		if( normal_ypos < -1.0 ) return( 0 );
		return( ( normal_xpos * normal_xpos + normal_ypos * normal_ypos ) < 1.0 );
}

vector_t gwiz::gwizCrystalBall::intersect_ortho_sphere( float_t normal_xpos, float_t normal_ypos )	{

		return( 
			vector_t( 
				normal_xpos, 
				normal_ypos, 
				cos( asin( sqrt( normal_xpos*normal_xpos + normal_ypos*normal_ypos ) ) ) 
			)
		);
}

int gwiz::gwizCrystalBall::update( int action1, int action2, int xpos, int ypos, float dt, float rot_factor, float pos_factor )	{
		
		float_t ball_radius = (float_t)size_x * 0.5;
		float_t normal_xpos = (float_t)( xpos - ( origin_x + ball_radius ) )/ball_radius;
		float_t normal_ypos = (float_t)( ypos - ( origin_y + ball_radius ) )/ball_radius;

		int is_bound = bound_ortho_circle( normal_xpos, normal_ypos );
		if( action1 )	{
			if( is_bound )	{
				if( !busy_rot )	{
					busy_rot = 1;
					r_from = intersect_ortho_sphere( normal_xpos, normal_ypos );
				}
			}
		}
		else	{
			busy_rot = 0;
		}

		if( action2 )	{
			if( is_bound )	{
				if( !busy_pos )	{
					busy_pos = 1;
					p_from = ( -curr_q ) * vector_t( normal_xpos, normal_ypos, 0.0 );
				}
			}
		}
		else	{
			busy_pos = 0;
		}
		
		if( busy_rot )	{
			vector_t r_to = intersect_ortho_sphere( normal_xpos, normal_ypos );
			if( ( r_to - r_from ).length() > EPSILON4 )	{
				float_t degrees = DEG( acos( r_to.dot( r_from ) ) );
				vector_t axis = r_from.cross( r_to );
				delta_q = quat_t( degrees * rot_factor / ( dt * ANGLE_DELTA_FACTOR ), axis );
				r_from = r_to;
			}
			else
			if( is_bound ) {
				delta_q = quat_t(); 
			}
		}
		if( busy_pos )	{
			vector_t p_to = ( -curr_q ) * vector_t( normal_xpos, normal_ypos, 0.0 );
			curr_p += ( ( p_to - p_from ) * pos_factor );
			p_from = p_to;
			delta_q = quat_t(); 
		}

		curr_q = ( delta_q * ( dt * ANGLE_DELTA_FACTOR ) ) * curr_q;
		return( busy_rot || busy_pos );
}

void gwiz::gwizCrystalBall::draw( void )	{
	static GLuint gl_list = 0x0;
	
		if( gl_list == 0x0 )	{
			vector_t unit = vector_t( 1.0, 0.0, 0.0 );
			int i;
			gl_list = glGenLists(1);
			glNewList( gl_list, GL_COMPILE );
				glBegin( GL_LINE_LOOP );
					for( i=0;i<64;i++ )	{
						vector_t v = euler_t( 0.0, 0.0, (float_t)i/64.0 * 360.0 ) * unit;
						glVertex2f( v.x() + 1.0, v.y() + 1.0 );
					}
				glEnd();
			glEndList();
		}

		float ball_radius = (float)size_x * 0.5;
		if( busy_rot || busy_pos )
			GWIZ_color( COL_ALPHA_GREY );
		else
			GWIZ_color( COL_ALPHA2_GREY );
		glPushMatrix();
			glTranslatef( (float)origin_x, (float)origin_y, 0.0 );
			glScalef( ball_radius, ball_radius, 1.0 );
			glCallList( gl_list );
		glPopMatrix();
}
