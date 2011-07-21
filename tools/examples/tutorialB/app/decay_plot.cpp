
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

void draw_trimmed_segment( 
	double x0, double y0, double r0,
	double x1, double y1, double r1
);
void draw_unit_circle( int filled = 0 );
void draw_unit_rect( void );

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
			break;

		case GLUT_KEY_F2:
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
}

void GW_update( void )	{

	glutPostRedisplay();
	G_count++;
}

///////////////////////////////////////////////////////////

float calc_waggle_curve( float t, float length, float pitch, float warp, float accel_pow, float decay_pow )	{

	double u = t / length;					// normalized traversal
	double a = warp * pow( t, accel_pow );	// sine acceleration curve

	double A = pow( u, decay_pow ); 		// decay acceleration
	double d = sin( A * M_PI ) * 0.5;		// decay envelope
	
	double p = -M_PI * 0.5 * pitch; 		// pitch adjustment
	double v = ( sin( p + a * M_PI ) - sin( p ) ) * d;
	return( (float)v );
}

void draw_waggle_curve( float length, float pitch, float warp, float accel_pow, float decay_pow, int mode = 0 )	{

	double p = -M_PI * 0.5 * pitch;

	glBegin( GL_LINE_STRIP );
	for( int i = 1; i<= 8000; i++ ) {

		double u = (float)i/8000.0; // normalized traversal
		
		double t = length * u; // local time

		double a = warp * pow( t, accel_pow ); // sine acceleration curve

		double A = pow( u, decay_pow );
		double d = sin( A * M_PI ) * 0.5;  // decay envelope

		float x = t;

		float y;
		if( mode == 0 )
//			y = ( ( 1.0 + cos( M_PI + a * M_PI ) ) * 0.5 ) * d;
//			y = sin( a * M_PI ) * d;
//			y = ( sin( p + a * M_PI ) - sin( p ) ) * d;
			y = calc_waggle_curve( t, length, pitch, warp, accel_pow, decay_pow );
		else
		if( mode == 1 )
			y = a;
		else
			y = d;

		glVertex3f( x, y, 0.0 );
	}
	glEnd();
}

void draw_test_nod_curve( void ) {

	static double width = 6.0;
	if( G_leftmouse )	{
		width = G_master_port.mousex / G_scale;
	}
	// pitch: range: -1.0 -> 0.0 -> 1.0
	static double pitch = 0.0;
	if( G_leftmouse )	{
		pitch = G_master_port.norm_mousey - 0.5;
	}
	
	glPushMatrix();
	glScalef( G_scale, G_scale, 1.0 );
	glTranslatef( 0.0, 1.0, 0.0 );
	
	glColor3f( 0.25, 0.25, 0.25 );
	glBegin( GL_LINE_STRIP );
	for( int i = 0; i< 1000; i++ ) {
		float x = width * (float)i/1000.0;
		float y = ( 1.0 + cos( M_PI + x * M_PI ) ) * 0.5;
//		double p = -M_PI * 0.5 * pitch;
//		y = ( sin( p + a * M_PI ) - sin( p ) ) * d;
//		float y = ( sin( M_PI + x * M_PI ) ) * 0.5;
		glVertex3f( x, y, 0.0 );
	}
	glEnd();

	float warp = 0.6; // useful range: 0.5 - 1.0

	// float power = 1.0; // useful range: 0.5 - 1.5
	glColor3f( 0.2, 0.0, 0.2 );
	draw_waggle_curve( width, pitch, warp, 0.5, 1.5, 2 );

	glColor3f( 0.4, 0.0, 0.4 );
	draw_waggle_curve( width, pitch, warp, 1.0, 1.0, 2 );

	glColor3f( 0.6, 0.0, 0.6 );
	draw_waggle_curve( width, pitch, warp, 1.5, 0.5, 2 );

	glColor3f( 0.2, 0.0, 0.0 );
	draw_waggle_curve( width, pitch, warp, 0.5, 1.5, 1 );

	glColor3f( 0.4, 0.0, 0.0 );
	draw_waggle_curve( width, pitch, warp, 1.0, 1.0, 1 );

	glColor3f( 0.6, 0.0, 0.0 );
	draw_waggle_curve( width, pitch, warp, 1.5, 0.5, 1 );


	glColor3f( 0.0, 0.0, 1.0 );
	draw_waggle_curve( width, pitch, warp, 0.5, 1.5 );

	glColor3f( 0.0, 1.0, 0.0 );
	draw_waggle_curve( width, pitch, warp, 1.0, 1.0 );

	glColor3f( 1.0, 1.0, 0.0 );
	draw_waggle_curve( width, pitch, warp, 1.5, 0.5 );

	glColor3f( 1.0, 1.0, 1.0 );
	draw_waggle_curve( width, pitch, 0.5, 2.0, 0.1 );

	glPopMatrix();
}

float calc_wiggle_curve( float t, float warp, float accel_pow )	{

	double a = warp * pow( t, accel_pow );	// sine acceleration curve
	double p = pow( t, 2.0 * accel_pow );	// decay power function
	if( p < 0.000000001 ) p = 0.000000001;

	double d = t / ( p * warp );			// decay envelope
	double v = ( ( 1.0 + cos( M_PI + a * M_PI ) ) * 0.5 ) * d;
	
	return( (float)v );
}

void draw_wiggle_curve( float length, float warp, float power, int mode = 0 )	{

	glBegin( GL_LINE_STRIP );
	for( int i = 1; i<= 8000; i++ ) {

		double u = (float)i/8000.0; // normalized traversal
		
		double t = length * u; // local time

//		double a = ( pow( t + 0.5, power ) - pow( 0.5, power ) ) * pow( 0.5, power ); // old formula
		double a = warp * pow( t, power ); // sine acceleration curve

		double p = pow( t, 2.0 * power ); // decay power function
		if( p < 0.000000001 ) p = 0.000000001;

		double d = t / ( p * warp ); // decay

		float x = t;

		float y;
		if( mode == 0 )
//			y = ( ( 1.0 + cos( M_PI + a * M_PI ) ) * 0.5 ) * d;
			y = calc_wiggle_curve( t, warp, power );
		else
		if( mode == 1 )
			y = a;
		else
			y = d;

		glVertex3f( x, y, 0.0 );
	}
	glEnd();
}

void draw_wiggle_curveX( float length, float power, int mode = 0 )	{

	glBegin( GL_LINE_STRIP );
	for( int i = 0; i< 1000; i++ ) {

		float x = length * (float)i/1000.0;
		
		double t = 1.0 * pow( x, power );
		double u = pow( x, 2.0 * power );
		if( u < 0.000000001 ) u = 0.000000001;
		
		float y;
		if( mode == 0 )
			y = ( ( 1.0 + cos( M_PI + t * M_PI ) ) * 0.5 ) * ( x / u );
		else
		if( mode == 1 )
			y = t;
		else
			y = ( x / u );

		glVertex3f( x, y, 0.0 );
	}
	glEnd();
}

void draw_test_nod_wiggle( void ) {

	static double width = 7.0;
	if( G_leftmouse )	{
		width = G_master_port.mousex / G_scale;
	}

	glPushMatrix();
	glScalef( G_scale, G_scale, 1.0 );
	glTranslatef( 0.0, 1.0, 0.0 );
	
	glColor3f( 0.25, 0.25, 0.25 );
	glBegin( GL_LINE_STRIP );
	for( int i = 0; i< 1000; i++ ) {
		float x = width * (float)i/1000.0;
		float y = ( 1.0 + cos( M_PI + x * M_PI ) ) * 0.5;
		glVertex3f( x, y, 0.0 );
	}
	glEnd();

	float warp = 1.0; // useful range: 0.5 - 1.0
	// float power = 2.0; // useful range: 1.0 - 3.0

	glColor3f( 0.2, 0.0, 0.2 );
	draw_wiggle_curve( width, warp, 1.0, 2 );

	glColor3f( 0.4, 0.0, 0.4 );
	draw_wiggle_curve( width, warp, 2.0, 2 );

	glColor3f( 0.6, 0.0, 0.6 );
	draw_wiggle_curve( width, warp, 3.0, 2 );

	glColor3f( 0.5, 0.0, 0.0 );
	draw_wiggle_curve( width, warp, 1.0, 1 );

	glColor3f( 0.7, 0.0, 0.0 );
	draw_wiggle_curve( width, warp, 2.0, 1 );

	glColor3f( 1.0, 0.0, 0.0 );
	draw_wiggle_curve( width, warp, 3.0, 1 );

	glColor3f( 0.0, 0.0, 1.0 );
	draw_wiggle_curve( width, warp, 1.0 );

	glColor3f( 0.0, 1.0, 0.0 );
	draw_wiggle_curve( width, warp, 2.0 );

	glColor3f( 1.0, 1.0, 0.0 );
	draw_wiggle_curve( width, warp, 3.0 );

	glPopMatrix();
}

void draw_bobble_curve( float width, float height, float power, int drawable = 5 )	{

//	w = cycles

	glBegin( GL_LINE_STRIP );
	for( int i = 0; i< 1000; i++ ) {

		float x = width * (float)i/1000.0;

//		float t = ( pow( x + 0.5, power ) - pow( 0.5, power ) ) * pow( 0.5, power ); // old formula
		float t = 1.0 * pow( x, power );
//		float t = 1.0 * pow( power, x );

//		float u = 2.0 * pow( x, 2.0 * power ); // same as *0.5 below
//		float u = pow( x, 2.0 * power );
		double u = pow( x, 2.0 * power );
		if( u < 0.000000001 ) u = 0.000000001;

		float y = 0.0;
		if( drawable == 0 )
			y = ( 1.0 + cos( t * M_PI + M_PI ) );
		else
		if( drawable == 1 )
			y = ( 1.0 + cos( t * M_PI + M_PI ) ) * 0.5;
		else
		if( drawable == 2 )
			y = ( 1.0 + cos( t * M_PI + M_PI ) ) * ( 1.0 / t ) * 0.5;
		else
		if( drawable == 3 )
			y = ( 1.0 + cos( t * M_PI + M_PI ) ) * ( 1.0 / u ) * 0.5;
		else
		if( drawable == 4 )
			y = ( 1.0 + cos( t * M_PI + M_PI ) ) * ( x / t ) * 0.5;
		else
		if( drawable == 5 )
			y = ( 1.0 + cos( t * M_PI + M_PI ) ) * ( x / u ) * 0.5;
		else
		if( drawable == 6 )
			y = t;
		else
		if( drawable == 7 )
			y = u;
		else
		if( drawable == 8 )
			y = ( x / t );
		else
		if( drawable == 9 )
			y = ( x / u );

		glVertex3f( x, height * y, 0.0 );
	}
	glEnd();
}

void draw_test_nod_curveX( void ) {

	glPushMatrix();
	glScalef( G_scale, G_scale, 1.0 );
	glTranslatef( 0.0, 1.0, 0.0 );
	
	double width = 8.0;
	double height = 1.0;
	
	glColor3f( 0.25, 0.25, 0.25 );
	glBegin( GL_LINE_STRIP );
	for( int i = 0; i< 1000; i++ ) {
		float x = width * (float)i/1000.0;
		float y = height * ( ( 1.0 + cos( M_PI + x * M_PI ) ) * 0.5 );
		glVertex3f( x, y, 0.0 );
	}
	glEnd();

	int tester = 9;

	glColor3f( 0.3, 0.0, 0.0 );
	draw_bobble_curve( width, height, 1.0, tester );

	glColor3f( 0.5, 0.0, 0.0 );
	draw_bobble_curve( width, height, 2.0, tester );

	glColor3f( 0.7, 0.0, 0.0 );
	draw_bobble_curve( width, height, 3.0, tester );


	glColor3f( 0.0, 0.0, 0.3 );
	draw_bobble_curve( width, height, 0.5 );

	glColor3f( 0.0, 0.0, 0.5 );
	draw_bobble_curve( width, height, 0.75 );

	glColor3f( 0.0, 0.0, 1.0 );
	draw_bobble_curve( width, height, 1.0 );

	glColor3f( 0.0, 0.3, 0.7 );
	draw_bobble_curve( width, height, 1.25 );

	glColor3f( 0.0, 0.5, 0.5 );
	draw_bobble_curve( width, height, 1.5 );

	glColor3f( 0.0, 0.7, 0.3 );
	draw_bobble_curve( width, height, 1.75 );

	glColor3f( 0.0, 1.0, 0.0 );
	draw_bobble_curve( width, height, 2.0 );

	glColor3f( 1.0, 1.0, 0.0 );
	draw_bobble_curve( width, height, 3.0 );

	glPopMatrix();
}

///////////////////////////////////////////////////////////

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

#if 1
		draw_test_nod_curve();
#else
		draw_test_nod_wiggle();
#endif

	glPopMatrix();
	
	glPopAttrib();
	glutSwapBuffers();
}

void GW_DisplayFunc(void)	{

	Hermite_DisplayFunc();
}

///////////////////////////////////////////////////////////

void GW_TimerRepeatFunc( int )	{
	
	glutTimerFunc( TIMER_MSEC, GW_TimerRepeatFunc, 0 );
	GW_update();
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
	glutCreateWindow( "gwiz Decay Function plot" );

	glutTimerFunc( TIMER_MSEC, GW_TimerRepeatFunc, 0 );
	glutKeyboardFunc( GW_KeyboardFunc );
	glutSpecialFunc( GW_SpecialFunc );
	glutMouseFunc( GW_MouseFunc );
	glutMotionFunc( GW_MotionFunc );
	glutPassiveMotionFunc( GW_MotionFunc );
    glutReshapeFunc( GW_ReshapeFunc );
	glutDisplayFunc( GW_DisplayFunc );
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

