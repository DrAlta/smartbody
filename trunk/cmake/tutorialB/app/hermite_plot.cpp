
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "sgi_ogl_font.h"
#include "gwiz_math.h"
#include "gwiz_camera.h"
#include "sr_linear_curve.h"
#include "sr_spline_curve.h"
#include "sr_curve_builder.h"

using namespace gwiz;

///////////////////////////////////////////////////////////

#define GWIZ_ESC_KEY		27
#define TIMER_MSEC			30
//#define TIMER_MSEC			200

///////////////////////////////////////////////////////////

static int G_window_resx = 768;
static int G_window_resy = 512;
static int G_count = 0;

static int G_leftmouse = 0;
static int G_middlemouse = 0;
static int G_rightmouse = 0;

static int G_shift_key = 0;
static int G_ctrl_key = 0;
static int G_alt_key = 0;

static gw_viewport_t G_master_port;

static float G_scale = 100.0;

//#define WIDGET_NONE 	0
//#define WIDGET_SCALE 	1

//static int G_active_widget = WIDGET_NONE;
//static int G_hot_widget = WIDGET_NONE;
//static bool G_busy_widget = false;

//#define WIDGET_DIAM		16
//#define WIDGET_DIAM		25
#define WIDGET_DIAM		64

#define WIDGET_DIAM_F	( (float)WIDGET_DIAM )
#define WIDGET_DIAM_F2	( ( (float)WIDGET_DIAM ) * 0.5 )

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

#define NUM_SAMPLE_SEGS		3
#define NUM_RESAMPLE_SEGS	8

#define NUM_SAMPLE_ITERS  	7
#define NUM_RESAMPLE_ITERS  ( NUM_SAMPLE_ITERS )

srLinearCurve Linear_arr[ NUM_SAMPLE_ITERS ];
srLinearCurve Resample_arr[ NUM_RESAMPLE_ITERS ];

#define RESAMPLE_POINTSIZE	2
#define NUM_SAMPLES 	( 1000 )

int 	G_hot_key = -1;
int 	G_active_key = -1;

bool G_enable_legend = true;

bool G_enable_tangents = true;
bool G_enable_nodes = true;
bool G_enable_keys = true; // dev dfl...

bool G_enable_direct = true;
bool G_enable_samples = false;
bool G_enable_resamples = false;

// HEADS and TAILS:

int G_head_extend_mode = srSplineCurve::EXTEND_REPEAT;
//int G_head_extend_mode = srSplineCurve::EXTEND_ACCEL;

int G_tail_extend_mode = srSplineCurve::EXTEND_REPEAT;
//int G_tail_extend_mode = srSplineCurve::EXTEND_ACCEL;

int G_curve_algorithm = srSplineCurve::ALG_CATMULL;

int G_sample_boundary_mode = srLinearCurve::EXTRAPOLATE;

////////////////////////////////

#define TEST_KEY_INSERTION	0
#define TEST_KEY_EDITING	0
#define TEST_KEY_GROWING	0

#if TEST_KEY_INSERTION
srSplineCurve	Curve( srSplineCurve::INSERT_KEYS );
#else
srSplineCurve	Curve( srSplineCurve::INSERT_NODES );
#endif
srCurveBuilder	Builder;

///////////////////////////////////////////////////////////

void update_samples( void ) {
	int i;

	Curve.set_extensions( G_head_extend_mode, G_tail_extend_mode );
	Curve.set_algorithm( G_curve_algorithm );

	int segs = NUM_SAMPLE_SEGS;
	for( i=0; i< NUM_SAMPLE_ITERS; i++ )	{
		Builder.get_spline_curve( Linear_arr + i, Curve, segs, true );
		Linear_arr[ i ].set_boundary_mode( G_sample_boundary_mode, G_sample_boundary_mode );
		segs *= 2;
	}

	Builder.set_input_range( 1.5, 7.0 );

	segs = NUM_RESAMPLE_SEGS;
	for( i=0; i< NUM_RESAMPLE_ITERS; i++ )	{
		if( i < NUM_SAMPLE_ITERS )	{
			Builder.get_resample_curve( Resample_arr + i, Linear_arr[ i ], segs, false );
			segs *= 2;
		}
		else	{
			printf( "err %d %d\n", i, NUM_SAMPLE_ITERS );
		}
	}
}

void Hermite_init( void )	{

	Curve.set_extensions( G_head_extend_mode, G_tail_extend_mode );
	Curve.set_auto_extend();
	
#if 0

	Curve.insert( 2.0, 1.0 );
	Curve.insert( 1.0, 1.0 );
	Curve.insert( 3.0, 1.0 );
	Curve.insert( 1.5, 1.0 );
	Curve.insert( 3.5, 1.5 );
	Curve.insert( 2.5, 1.5 );

	Curve.print();

#else
//	Curve.insert( 0.0, 2.0 );
	Curve.insert( 1.0, 1.0 );
	Curve.insert( 3.0, 3.5 );
	Curve.insert( 5.0, 1.2 );
	Curve.insert( 6.0, 2.0 );
//	Curve.insert( 7.5, 4.0 );
//	Curve.insert( 8.0, 3.75 );
	Curve.print();
#endif


	printf( "Hermite_init: update_samples():\n" );
	update_samples();
}

void Hermite_update( void )	{
	
	float mx = G_master_port.mousex;
	float my = G_master_port.mousey;
	float px = mx / G_scale;
	float py = my / G_scale;
//	double radius = WIDGET_DIAM_F2 / G_scale;
	float s = WIDGET_DIAM_F / G_scale;
//	float s = WIDGET_DIAM_F / 100.0;
	float radii_scale = 1.0 * s;
	
	if( G_leftmouse == false )	{
//		if( G_active_key == -1 ) {
//			G_hot_key = Curve.probe_bbox_key( px, py, radius, true, true );
			G_hot_key = Curve.probe_bbox_node( px, py, radii_scale, true, true, true );
//		}
	}
	
	if( G_leftmouse )	{
		if( G_hot_key >= 0 )
			G_active_key = 0;
	}
	else	{
		G_active_key = -1;
	}

	if( G_active_key >= 0 )	{
		Curve.edit( px, py );
		update_samples();
	}
}

void draw_trimmed_segment( 
	double x0, double y0, double r0,
	double x1, double y1, double r1
);
void draw_unit_circle( int filled = 0 );
void draw_unit_rect( void );

void draw_samples( void ) {
	int i;
	double x, y;
	float alpha = 1.0;
	
	if( G_enable_samples )	{
		for( i = 0; i < NUM_SAMPLE_ITERS; i++ )	{

			glColor4f( 1.0, 0.0, 0.0, alpha );
			Linear_arr[ i ].query_reset();
			glBegin( GL_LINE_STRIP );
				while( Linear_arr[ i ].query_next( &x, &y, true ) )	{ 
					glVertex3f( x, y, 0.0 );
				}
			glEnd();
//			alpha *= 0.6667;
			alpha *= 0.6;
		}
	}

	if( G_enable_resamples )	{
		alpha = 0.75;
		double green = 0.75;
		double blue = 1.0;
		glPointSize( RESAMPLE_POINTSIZE );
		for( i = 0; i < NUM_RESAMPLE_ITERS; i++ )	{

			glColor4f( 0.0, green, blue, alpha );
			Resample_arr[ i ].query_reset();
			glBegin( GL_POINTS );
				while( Resample_arr[ i ].query_next( &x, &y, true ) )	{ glVertex3f( x, y, 0.0 ); }
			glEnd();
	//		green *= 0.75;
			blue *= 0.8;
	//		alpha *= 0.667;
			alpha *= 0.9;
		}
		glPointSize( 1 );
	}
}

void Hermite_display( void )	{

	glPushMatrix();
		glScalef( G_scale, G_scale, 1.0 );

		double x, y;
		float s = WIDGET_DIAM_F / G_scale;
		float radii_scale = 1.0 * s;
		int c;

		// draw blackout for node highlighter...
		Curve.query_reset();
		c = 0;
		glColor4f( 0.0, 0.0, 0.0, 0.8 );
		while( Curve.query_node( &x, &y, NULL, NULL, NULL, NULL, true ) )	{
			
			float rad = 0.0;
			if( c == G_hot_key )	{
				rad = radii_scale * 1.2;
			}
			else
			if( G_enable_nodes )	{
				rad = radii_scale * 0.5;
			}
			else
			if( G_enable_keys )	{
				rad = radii_scale * 0.4;
			}
			if( rad > 0.0 ) {
				glPushMatrix();

					glTranslatef( x, y, 0.0 );
					glPushMatrix();
						glScalef( rad, rad, 1.0 );
						draw_unit_circle( true );
					glPopMatrix();

				glPopMatrix();
			}
			c++;
		}

		glBlendFunc( GL_SRC_ALPHA,  GL_ONE ); 

		if( G_enable_direct )	{
			glEnable( GL_LINE_SMOOTH );
			glBegin( GL_LINE_STRIP );

				glColor4f( 0.0, 0.0, 1.0, 0.5 );
				double t_start, t_stop;
				Curve.query_span( &t_start, &t_stop );
				double t_span = t_stop - t_start;

				if( t_span > 0.0 )	{

					double dt = t_span / (double)( NUM_SAMPLES - 1 );
					double t = t_start;

					for( int i = 0; i< NUM_SAMPLES; i++ ) {

						double v = Curve.evaluate( t );
						glVertex3f( t, v, 0.0 );
						t += dt;
					}
				}
			glEnd();
			glDisable( GL_LINE_SMOOTH );
		}

		glEnable( GL_POINT_SMOOTH );
		draw_samples();
		glDisable( GL_POINT_SMOOTH );
	

		if( G_enable_keys ) {
		
			glEnable( GL_LINE_SMOOTH );
			glColor4f( 0.0, 1.0, 0.0, 0.5 );
			Curve.query_reset();
			while( Curve.query_key( &x, &y, true ) )	{

				glPushMatrix();

					glTranslatef( x, y, 0.0 );
//					glScalef( s, s, 1.0 );

					float rad = radii_scale * 0.4;
					glPushMatrix();
						glScalef( rad, rad, 1.0 );
						draw_unit_circle();
					glPopMatrix();
				glPopMatrix();
			}
			glDisable( GL_LINE_SMOOTH );
		}

		Curve.query_reset();
		double ml, mr, dl, dr;
		c = 0;
		while( Curve.query_node( &x, &y, &ml, &mr, &dl, &dr, true ) )	{
			
			glEnable( GL_LINE_SMOOTH );
			glColor4f( 1.0, 1.0, 0.0, 0.9 );
			glPushMatrix();

				glTranslatef( x, y, 0.0 );
//				glScalef( s, s, 1.0 );

				glPushMatrix();
					if( c == G_hot_key )	{
						float rad = radii_scale * 1.2;
						glScalef( rad, rad, 1.0 );
						draw_unit_circle();
					}
					else
					if( G_enable_nodes )	{
						float rad = radii_scale * 0.5;
						glScalef( rad, rad, 1.0 );
						draw_unit_circle();
					}
				glPopMatrix();

			glPopMatrix();
			glDisable( GL_LINE_SMOOTH );

			if( G_enable_tangents ) {
				
				glPushMatrix();
					glTranslatef( x, y, 0.0 );

					float x1 = -dl;
					float y1 = -ml * dl;
					float x2 = dr;
					float y2 = mr * dr;

					double tan_pt_rad = 0.0;
					if( c == G_hot_key )	{
						tan_pt_rad = radii_scale * 1.2;
					}
					else
					if( G_enable_nodes )	{
						tan_pt_rad = radii_scale * 0.5;
					}
					else
					if( G_enable_keys )	{
						tan_pt_rad = radii_scale * 0.4;
					}

					float end_pt_rad = radii_scale * 0.2;

					glColor4f( 1.0, 1.0, 0.0, 0.33 );
					draw_trimmed_segment( 0.0, 0.0, tan_pt_rad, x1, y1, end_pt_rad );
					draw_trimmed_segment( 0.0, 0.0, tan_pt_rad, x2, y2, end_pt_rad );

					glEnable( GL_LINE_SMOOTH );
					glPushMatrix();
						glTranslatef( x1, y1, 0.0 );
						glScalef( end_pt_rad, end_pt_rad, 1.0 );
						draw_unit_circle();
					glPopMatrix();

					glColor4f( 1.0, 1.0, 1.0, 0.33 );
					glPushMatrix();
						glTranslatef( x2, y2, 0.0 );
						glScalef( end_pt_rad, end_pt_rad, 1.0 );
						draw_unit_circle();
					glPopMatrix();
					glDisable( GL_LINE_SMOOTH );

				glPopMatrix();
			}
			c++;
		}

		glBlendFunc( GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA ); 

	glPopMatrix();
}

void Hermite_draw_legend(void)	{
	char label[ 256 ];

	int y_pos = G_window_resy - SGI_GLFONT_SPACE_HGT;

	glColor4f( 0.3, 0.3, 0.3, 1.0 );
	if( G_enable_legend )	{
		alphabet.print( (char*)"F1: <hide>", 10, y_pos );
	}
	else	{
		alphabet.print( (char*)"F1: <legend>", 10, y_pos );
		return;
	}

	y_pos -= SGI_GLFONT_SPACE_HGT;
	glColor4f( 0.3, 0.3, 0.3, 1.0 );
	alphabet.print( (char*)"F2: <dflts>", 10, y_pos );


	y_pos -= SGI_GLFONT_SPACE_HGT / 2;
	y_pos -= SGI_GLFONT_SPACE_HGT;

	glColor4f( 1.0, 1.0, 0.0, 1.0 );
	alphabet.print( (char*)"F3:", 10, y_pos );
	if( G_enable_nodes == false )
		glColor4f( 0.5, 0.5, 0.5, 1.0 );
	alphabet.print( (char*)" nodes", 10 + SGI_GLFONT_SPACE_WID * 3, y_pos );

	y_pos -= SGI_GLFONT_SPACE_HGT;
	glColor4f( 0.5, 0.5, 0.0, 1.0 );
	alphabet.print( (char*)"F4:", 10, y_pos );
	if( G_enable_tangents == false )
		glColor4f( 0.5, 0.5, 0.5, 1.0 );
	alphabet.print( (char*)" slope", 10 + SGI_GLFONT_SPACE_WID * 3, y_pos );
		
	y_pos -= SGI_GLFONT_SPACE_HGT;
	glColor4f( 0.0, 0.5, 0.0, 1.0 );
	alphabet.print( (char*)"F5:", 10, y_pos );
	if( G_enable_keys == false )
		glColor4f( 0.5, 0.5, 0.5, 1.0 );
	alphabet.print( (char*)" keys", 10 + SGI_GLFONT_SPACE_WID * 3, y_pos );
		
	y_pos -= SGI_GLFONT_SPACE_HGT;
	glColor4f( 0.0, 0.0, 1.0, 1.0 );
	alphabet.print( (char*)"F6:", 10, y_pos );
	if( G_enable_direct == false )
		glColor4f( 0.5, 0.5, 0.5, 1.0 );
	alphabet.print( (char*)" curve", 10 + SGI_GLFONT_SPACE_WID * 3, y_pos );
		
	y_pos -= SGI_GLFONT_SPACE_HGT;
	glColor4f( 1.0, 0.0, 0.0, 1.0 );
	alphabet.print( (char*)"F7:", 10, y_pos );
	if( G_enable_samples == false )
		glColor4f( 0.5, 0.5, 0.5, 1.0 );
	alphabet.print( (char*)" sample", 10 + SGI_GLFONT_SPACE_WID * 3, y_pos );
		
	y_pos -= SGI_GLFONT_SPACE_HGT;
	glColor4f( 0.0, 1.0, 1.0, 1.0 );
	alphabet.print( (char*)"F8:", 10, y_pos );
	if( G_enable_resamples == false )
		glColor4f( 0.5, 0.5, 0.5, 1.0 );
	alphabet.print( (char*)" re-samp", 10 + SGI_GLFONT_SPACE_WID * 3, y_pos );
		
	y_pos -= SGI_GLFONT_SPACE_HGT / 2;
	glColor4f( 0.75, 0.75, 0.75, 1.0 );

	y_pos -= SGI_GLFONT_SPACE_HGT;
	sprintf( label, "F9  HEAD: %s", srSplineCurve::extend_label( G_head_extend_mode ) );
	alphabet.print( (char*)label, 10, y_pos );

	y_pos -= SGI_GLFONT_SPACE_HGT;
	sprintf( label, "F10 TAIL: %s", srSplineCurve::extend_label( G_tail_extend_mode ) );
	alphabet.print( (char*)label, 10, y_pos );

	y_pos -= SGI_GLFONT_SPACE_HGT;
	sprintf( label, "F11 ALGO: %s", srSplineCurve::algorithm_label( G_curve_algorithm ) );
	alphabet.print( (char*)label, 10, y_pos );

	y_pos -= SGI_GLFONT_SPACE_HGT;
	sprintf( label, "F12 SMPL: %s", srLinearCurve::mode_label( G_sample_boundary_mode ) );
	alphabet.print( (char*)label, 10, y_pos );
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

void draw_trimmed_segment( 
	double x0, double y0, double r0,
	double x1, double y1, double r1
)	{

	vector_t v0( x0, y0, 0.0 );
	vector_t v1( x1, y1, 0.0 );
	vector_t diff = v1 - v0;
	vector_t n_diff = diff.normal();
	vector_t off0 = v0 + n_diff * r0;
	vector_t off1 = v1 - n_diff * r1;
	
	glBegin( GL_LINES );
		glVertex3f( off0.x(), off0.y(), 0.0 );
		glVertex3f( off1.x(), off1.y(), 0.0 );
	glEnd();
}

void draw_unit_circle( int filled )	{
	euler_t de( 0.0, 0.0, 360.0 / 64.0 );
	vector_t v( 1.0, 0.0, 0.0 );

	if( filled )	{
		glBegin( GL_POLYGON );
	}
	else	{
		glBegin( GL_LINE_LOOP );
	}
		for( int i=0; i<64; i++ )	{
			glVertex3f( v.x(), v.y(), v.z() );
			v = de * v;
		}
	glEnd();
}

void draw_unit_rect( void )	{
	static float corners[ 4 ][ 3 ] = {
		{ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 },
		{ 1.0, 1.0, 0.0 }, { 0.0, 1.0, 0.0 }
	};
	
	glBegin( GL_LINE_LOOP );
		glVertex3fv( corners[ 0 ] ); glVertex3fv( corners[ 1 ] );
		glVertex3fv( corners[ 2 ] ); glVertex3fv( corners[ 3 ] );
	glEnd();
}

void draw_unit_grid( float max_x, float max_y ) {
	int i;
	int c;
	
	glBegin( GL_LINES );

	c = (int)max_x;
	for( i=1; i<=c; i++ )	{
		float px = (float)i;
		glVertex3f( px, 0.0, 0.0 );
		glVertex3f( px, max_y, 0.0 );
	}
	
	c = (int)max_y;
	for( i=1; i<=c; i++ )	{
		float py = (float)i;
		glVertex3f( 0.0, py, 0.0 );
		glVertex3f( max_x, py, 0.0 );
	}
	
	glEnd();
}

///////////////////////////////////////////////////////////

void GW_check_modifiers( void )	{

	int mod = glutGetModifiers();
	G_shift_key = ( mod & GLUT_ACTIVE_SHIFT ) ? 1 : 0;
	G_ctrl_key = ( mod & GLUT_ACTIVE_CTRL ) ? 1 : 0;
	G_alt_key = ( mod & GLUT_ACTIVE_ALT ) ? 1 : 0;
}

void GW_KeyPressFunc( int key ) {
	
	GW_check_modifiers();

	switch( key )	{

		case GLUT_KEY_F1:
			G_enable_legend = !G_enable_legend;
			break;

		case GLUT_KEY_F2:
			G_enable_tangents = true;
			G_enable_nodes = true;
			G_enable_keys = false;
			G_enable_direct = true;
			G_enable_samples = false;
			G_enable_resamples = false;
			break;

		case GLUT_KEY_F3:
			G_enable_nodes = !G_enable_nodes;
			break;
		case GLUT_KEY_F4:
			G_enable_tangents = !G_enable_tangents;
			break;
		case GLUT_KEY_F5:
			G_enable_keys = !G_enable_keys;
			break;

		case GLUT_KEY_F6:
			G_enable_direct = !G_enable_direct;
			break;
		case GLUT_KEY_F7:
			G_enable_samples = !G_enable_samples;
			break;
		case GLUT_KEY_F8:
			G_enable_resamples = !G_enable_resamples;
			break;

		case GLUT_KEY_F9:
			G_head_extend_mode = ( G_head_extend_mode + 1 ) % ( srSplineCurve::NUM_EXTEND_MODES );
			break;
		case GLUT_KEY_F10:
			G_tail_extend_mode = ( G_tail_extend_mode + 1 ) % ( srSplineCurve::NUM_EXTEND_MODES );
			break;
		case GLUT_KEY_F11:
			G_curve_algorithm = ( G_curve_algorithm + 1 ) % ( srSplineCurve::NUM_ALG_MODES );
			break;
		case GLUT_KEY_F12:
			G_sample_boundary_mode = ( G_sample_boundary_mode + 1 ) % ( srLinearCurve::NUM_BOUNDARY_MODES );
			break;

		case GLUT_KEY_LEFT:
			printf( "LEFT\n" );
			break;
		case GLUT_KEY_RIGHT:
			printf( "RIGHT\n" );
			break;

		case GLUT_KEY_UP:
			break;
		case GLUT_KEY_DOWN:
			break;

		case GWIZ_ESC_KEY:
			exit( 1 );
			break;
	}
	
	// assume it's important:
	update_samples();
}

void GW_KeyboardFunc( unsigned char unsigned_key, int, int )	{
	GW_KeyPressFunc( (int)unsigned_key );
}

void GW_SpecialFunc( int key, int, int )	{
	GW_KeyPressFunc( (int)key );
}

void GW_MouseFunc( int button, int state, int x, int y )	{

	GW_check_modifiers();
	switch( button ) {
		case GLUT_LEFT_BUTTON:
			if( state == GLUT_DOWN ) {
				G_leftmouse = 1;
			}
			else	{
				G_leftmouse = 0;
			}
		break;
		case GLUT_MIDDLE_BUTTON:
			if( state == GLUT_DOWN ) {
				G_middlemouse = 1;
			}
			else	{
				G_middlemouse = 0;
			}
		break;
		case GLUT_RIGHT_BUTTON:
			if( state == GLUT_DOWN ) {
				G_rightmouse = 1;
			}
			else	{
				G_rightmouse = 0;
			}
		break;
	}
	
	G_master_port.set_mouse( x, G_window_resy - y );
}

void GW_MotionFunc( int x, int y )	{
	
	G_master_port.set_mouse( x, G_window_resy - y );
}

void GW_ReshapeFunc( int width, int height )	{
	
	G_window_resx = width;
	G_window_resy = height;

	G_master_port.resize( width, height );
}


///////////////////////////////////////////////////////////

void GW_init( void ) {
	
	G_master_port.resize( G_window_resx, G_window_resy );

	Hermite_init();
}

void SmPl_update( void )	{
	
	Hermite_update();
#if 0
	float mx = G_master_port.mousex;
	float my = G_master_port.mousey;
	
	if( G_leftmouse == false )	{
		if( G_active_widget == WIDGET_NONE ) {

			if( ( mx < WIDGET_DIAM_F )&&( fabs( G_scale - my ) <= WIDGET_DIAM_F2 ) )	{
				G_hot_widget = WIDGET_SCALE;
			}
			else
			if( ( my < WIDGET_DIAM_F )&&( fabs( G_smooth * (GLdouble)G_window_resx - mx ) <= WIDGET_DIAM_F2 ) )	{ 
				G_hot_widget = WIDGET_SMOOTH;
			}
			else
			if( 
				( my > WIDGET_DIAM_F )&&
				( my < 2.0 * WIDGET_DIAM_F )&&
				( fabs( G_tuner * (GLdouble)G_window_resx / WIDGET_TUNER_MAX - mx ) <= WIDGET_DIAM_F2 ) 
				)	{ 
				G_hot_widget = WIDGET_TUNER;
			}
			else
			if( 
				( fabs( ( G_scale * G_clamp ) / G_speed - mx ) <= WIDGET_DIAM_F2 )&&
				( fabs( G_scale * G_clamp - my ) <= WIDGET_DIAM_F2 )
				)	{
				G_hot_widget = WIDGET_SPD_CLMP;
			}
			else	{
				G_hot_widget = WIDGET_NONE;
			}
		}
	}
	
	if( G_leftmouse )	{
		G_active_widget = G_hot_widget;
		G_busy_widget = ( G_active_widget > 0 );
	}
	else	{
		G_active_widget = WIDGET_NONE;
	}
	
	if( G_active_widget == WIDGET_SCALE )	{
		G_scale = G_master_port.mousey;
		if( G_scale < 2.0 ) G_scale = 2.0;
	}
	else
	if( G_active_widget == WIDGET_SMOOTH )	{
		G_smooth = G_master_port.norm_mousex;
	}
	else
	if( G_active_widget == WIDGET_TUNER )	{
		G_tuner = G_master_port.norm_mousex * WIDGET_TUNER_MAX;
	}
	else
	if( G_active_widget == WIDGET_SPD_CLMP )	{
		G_clamp = G_master_port.mousey / G_scale;
		if( G_clamp < 0.0 ) G_clamp = 0.0;
		G_speed = ( G_scale * G_clamp ) / G_master_port.mousex;
		if( G_speed < 0.0 ) G_speed = 0.0;
	}
#endif
	
	glutPostRedisplay();
	G_count++;
}

void Hermite_DisplayFunc(void)	{
	
	float width = (float)G_window_resx/G_scale;
	float height = (float)G_window_resy/G_scale;

	glDrawBuffer( GL_BACK );

	glPushAttrib( GL_ENABLE_BIT | GL_VIEWPORT_BIT | GL_SCISSOR_BIT );
	glEnable( GL_SCISSOR_TEST );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA ); 

	glViewport( 0, 0, G_window_resx, G_window_resy );
	glScissor( 0, 0, G_window_resx, G_window_resy );

	glClearDepth( 1.0 );
//	glClearColor( 0.0, 0.0, 0.2, 0.0 );
	glClearColor( 0.0, 0.0, 0.1, 0.0 );
//	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0.0, (GLdouble)G_window_resx, 0.0, (GLdouble)G_window_resy, -1.0, 1.0 );

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

		glLoadIdentity();
		
		glPushMatrix();
			glScalef( G_scale, G_scale, 1.0 );
			
			glColor4f( 1.0, 1.0, 1.0, 0.2 );
			draw_unit_grid( width, height );
			
		glPopMatrix();

		Hermite_display();

	glPopMatrix();
	
	glPushMatrix();
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		Hermite_draw_legend();

	glPopMatrix();
	glPopAttrib();
	glutSwapBuffers();
}

///////////////////////////////////////////////////////////

void GW_TimerRepeatFunc( int )	{
	
	glutTimerFunc( TIMER_MSEC, GW_TimerRepeatFunc, 0 );
	SmPl_update();
}

void GW_AtExit(void)	{
	
}

int GWS_main( int ac, char** av )	{
	
	glutInit( &ac, av );
	GW_init();
	
	if( atexit( GW_AtExit ) != 0 )	{
		fprintf( stderr, "%s ERR: atexit() registration FAILED\n", av[0] );
	}

	glutInitWindowPosition( 200, 0 );
	glutInitWindowSize( G_window_resx, G_window_resy );
#if 0
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
#elif 1
	glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH );
#else
	glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STEREO );
#endif
	glutCreateWindow( "gwiz Temporal Hermite plot" );

	glutTimerFunc( TIMER_MSEC, GW_TimerRepeatFunc, 0 );
	glutKeyboardFunc( GW_KeyboardFunc );
	glutSpecialFunc( GW_SpecialFunc );
	glutMouseFunc( GW_MouseFunc );
	glutMotionFunc( GW_MotionFunc );
	glutPassiveMotionFunc( GW_MotionFunc );
    glutReshapeFunc( GW_ReshapeFunc );

	glutDisplayFunc( Hermite_DisplayFunc );
	glutMainLoop();
	return( 0 );
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

int main( int ac, char** av )	{

#if 0
	pid_t id = fork();
	if( id == 0 )
		return( GWS_main( ac, av ) );
	return( 0 );
#else
	return( GWS_main( ac, av ) );
#endif
}

