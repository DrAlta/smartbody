
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
#define TIMER_MSEC			20
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
static gw_camera_t G_camera;

///////////////////////////////////////////////////////////

#define USER_FIRST_INDEX	1

#define ENABLE_DUPLICATE_TEST	1
#define ENABLE_DUPLICATE_BLOCK	0


class NodRecordBuffer {

	public:
	
	NodRecordBuffer( void ) {
		count = 0;
		meta_data_arr = NULL;
		rot_data_arr = NULL;
	}
	~NodRecordBuffer( void ) {}
	
	void allocate_meta( int num )	{
		count = num;
		if( meta_data_arr ) {
			delete [] meta_data_arr;
		}
		meta_data_arr = new int [ count ];
	}
	
	void allocate_buff( void )	{

		int i;
		if( rot_data_arr ) {
			for( i=0; i<count; i++ )	{
				if( rot_data_arr[ i ] ) {
					delete [] rot_data_arr[ i ];
				}
			}
			delete [] meta_data_arr;
		}

		rot_data_arr = new float * [ count ];
		for( i=0; i<count; i++ )	{
			rot_data_arr[ i ] = new float [ meta_data_arr[ i ] * 4 ];
		}

		quat_data_arr = new quat_t * [ count ];
		for( i=0; i<count; i++ )	{
			quat_data_arr[ i ] = new quat_t [ meta_data_arr[ i ] ];
		}
	}

	void set_meta_data( int track, int len )	{
		meta_data_arr[ track ] = len;
	}
	
	void set_rot_data( int track, int offset, float t, float rx, float ry, float rz )	{
		rot_data_arr[ track ][ offset * 4 ] = t;
		rot_data_arr[ track ][ offset * 4 + 1 ] = rx;
		rot_data_arr[ track ][ offset * 4 + 2 ] = ry;
		rot_data_arr[ track ][ offset * 4 + 3 ] = rz;
	}
	
	bool read_file( const char *filename )	{

		if( filename == NULL )	{
	    	printf( "NodRecordBuffer::read_file ERR: NULL filename\n" ); 
			return( false );
		}

		FILE *in_fp;
		if( ( in_fp = fopen( filename, "r" ) ) == NULL ) { 
			printf( "NodRecordBuffer::read_file ERR: file '%s' not found\n", filename ); 
			return( false );
		}

		float t, rx, ry, rz;
		
		int tracks = 0;
		int len = 0;
		int max_len = 0;

		bool done = false;
		while( !done )	{

			int ret = fscanf( in_fp, "%f,%f,%f,%f\n", &t, &rx, &ry, &rz );
			if( ret == 4 )	{
			
				len++;
			}
			else	{

				if( len ) { 
					tracks++; 
					if( len > max_len ) {
						max_len = len;
					}
					len = 0;
				}
				if( ret == EOF )	{
					done = true;
				}
				else	{
					fscanf( in_fp, "--\n" );
				}
			}
		}
		rewind( in_fp );

		allocate_meta( tracks );

		tracks = 0;
		len = 0;
		done = false;
		while( !done )	{

			int ret = fscanf( in_fp, "%f,%f,%f,%f\n", &t, &rx, &ry, &rz );
			if( ret == 4 )	{
			
				len++;
			}
			else	{
				
				if( len ) { 
					set_meta_data( tracks, len );
					tracks++;
					len = 0;
				}
				if( ret == EOF )	{
					done = true;
				}
				else	{
					fscanf( in_fp, "--\n" );
				}
			}
		}
		rewind( in_fp );

		allocate_buff();

		tracks = 0;
		done = false;
		while( !done )	{

			int ret = fscanf( in_fp, "%f,%f,%f,%f\n", &t, &rx, &ry, &rz );
			if( ret == 4 )	{
			
				set_rot_data( tracks, len, t, rx, ry, rz );
				len++;
			}
			else	{
				
				if( len )	{
					tracks++;
					len = 0;
				}
				if( ret == EOF )	{
					done = true;
				}
				else	{
					fscanf( in_fp, "--\n" );
				}
			}
		}
		fclose( in_fp );

		printf( "found %d tracks, maximum: %d\n", tracks, max_len );
		int i;
		for( i=0; i<tracks; i++ )	{
			printf( "meta: %d\n", meta_data_arr[ i ] );
		}
		
		int j;
		for( j=0; j<tracks; j++ )	{
		
			quat_t accum = quat_t(); // 0.0
			quat_t P = accum;

			for( i=0; i< meta_data_arr[ j ]; i++ )	{
#if ENABLE_DUPLICATE_TEST
				quat_t Q = get_rot( j, i );
				if( 
					( P.x() == Q.x() ) &&
					( P.y() == Q.y() ) &&
					( P.z() == Q.z() ) ) {
					printf( "dupe: { %f %f %f }: Tr:%d i:%d \n", P.x(), P.y(), P.z(), j, i );
				}
#if ENABLE_DUPLICATE_BLOCK
				else	{
					accum *= Q;
				}
#else
				accum *= Q;
#endif

				P = Q;
#else
				float dt = get_dt( j, i );
				quat_t Q = get_rot( j, i );
				accum *= ( Q * dt );
#endif
				
				quat_data_arr[ j ][ i ] = accum;
			}
		}
		
		return( true );
	}
	
	int get_len( int track )	{

		return( meta_data_arr[ track ] );
	}

	float get_time( int track, int offset )	{
		
		return( rot_data_arr[ track ][ offset * 4 ] );
	}

	float get_dt( int track, int offset )	{
	
		return( rot_data_arr[ track ][ offset * 4 ] - rot_data_arr[ track ][ ( offset - 1 ) * 4 ] );
	}

	quat_t get_rot( int track, int offset ) {
	
		float drx = rot_data_arr[ track ][ offset * 4 + 1 ];
		float dry = rot_data_arr[ track ][ offset * 4 + 2 ];
		float drz = rot_data_arr[ track ][ offset * 4 + 3 ];
#if 1
		euler_t dex( DEG( drx ), 0.0, 0.0 );
		euler_t dey( 0.0, DEG( dry ), 0.0 );
		euler_t dez( 0.0, 0.0, DEG( drz ) );
#else
		euler_t dex( drx, 0.0, 0.0 );
		euler_t dey( 0.0, dry, 0.0 );
		euler_t dez( 0.0, 0.0, drz );
#endif
#if 1
		return( quat_t( dex * dey * dez ) );
#else
		return( quat_t( dez * dey * dex ) );
#endif
	}
	
	quat_t get_quat( int track, int offset )	{
		return( quat_data_arr[ track ][ offset ] );
	}
	
	int count;
	int *meta_data_arr;
	float **rot_data_arr;
	quat_t **quat_data_arr;
};

class NodSequenceBuffer {

	public:
	
	NodSequenceBuffer( void ) {
		count = 0;
		meta_data_arr = NULL;
		time_data_arr = NULL;
	}
	~NodSequenceBuffer( void ) {}
	
	void allocate( int num )	{
		count = num;
		if( meta_data_arr ) {
			delete [] meta_data_arr;
		}
		if( time_data_arr ) {
			delete [] time_data_arr;
		}
		meta_data_arr = new int [ count * 2 ];
		time_data_arr = new float [ count * 2 ];
	}
	
	void set_data( int index, int track, int type, float fr, float to )	{
		meta_data_arr[ index * 2 ] = track;
		meta_data_arr[ index * 2 + 1 ] = type;
		time_data_arr[ index * 2 ] = fr;
		time_data_arr[ index * 2 + 1 ] = to;
	}

	bool read_file( const char *filename )	{

		if( filename == NULL )	{
	    	printf( "NodSequenceBuffer::read_file ERR: NULL filename\n" ); 
			return( false );
		}

		FILE *in_fp;
		if( ( in_fp = fopen( filename, "r" ) ) == NULL ) { 
			printf( "NodSequenceBuffer::read_file ERR: file '%s' not found\n", filename ); 
			return( false );
		}

		float f1, tin, tout, f2;
		int len = 0;

		bool done = false;
		while( !done )	{

			int ret = fscanf( in_fp, "%f,%f,%f,%f\n", &f1, &tin, &tout, &f2 );
			if( ret == 4 )	{
			
				len++;
			}
			else	{

				if( ret == EOF )	{
					done = true;
				}
				else	{
					printf( "NodSequenceBuffer::read_file: ERROR in file '%s'\n", filename );
				}
			}
		}
		rewind( in_fp );

		allocate( len );

		len = 0;
		int seq, type;
		done = false;
		while( !done )	{

			int ret = fscanf( in_fp, "%f,%f,%f,%f\n", &f1, &tin, &tout, &f2 );
			if( ret == 4 )	{
			
				seq = (int)f1;
				type = (int)f2;
				set_data( len, seq, type, tin, tout );
				len++;
			}
			else	{

				if( ret == EOF )	{
					done = true;
				}
				else	{
					printf( "NodSequenceBuffer::read_file: ERROR in file '%s'\n", filename );
				}
			}
		}

		printf( "found %d sequences\n", count );
		return( true );
	}

	int get_track( int index )	{
		return( meta_data_arr[ index * 2 ] - USER_FIRST_INDEX ); // counting from 1, not 0
	}

	int get_type( int index )	{
		return( meta_data_arr[ index * 2 + 1] );
	}

	float get_tin( int index )	{
		return( time_data_arr[ index * 2 ] );
	}

	float get_tout( int index )	{
		return( time_data_arr[ index * 2 + 1 ] );
	}

	int count;
	int *meta_data_arr;
	float *time_data_arr;
};

///////////////////////////////////////////////////////////

#include <sys/time.h>

double get_real_time( void )	{
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return( tv.tv_sec + ( tv.tv_usec / 1000000.0 ) );
}

///////////////////////////////////////////////////////////

#define MODE_PLAY_THROUGH	0
#define MODE_PLAY_SEQUENCES 1
#define MODE_PLAY_SEQ_LOOP	2

int G_play_mode = MODE_PLAY_THROUGH;

double G_start_time = 0.0;
double G_time = 0.0;
double G_dt = 0.01;

double G_play_time = 0.0; // set to initial track time

int G_sequence = 0;
int G_track = 0;
int G_index = 0;

quat_t G_orientation;
float G_magnification = 1.0;

NodRecordBuffer nod_record;
NodSequenceBuffer seq_buffer;

///////////////////////////////////////////////////////////

void print_skm_array( void )	{

	if( G_play_mode == MODE_PLAY_THROUGH )	{
		printf( "print_skm_array ERR: must be in sequence mode (F2,F3)\n" );
	}
	else	{

		int i = 0;
		while( nod_record.get_time( G_track, i ) < seq_buffer.get_tin( G_sequence ) ) {
			i++;
		}

		int start = i;
		int c = 0;
		float t = nod_record.get_time( G_track, start );
		while( t < seq_buffer.get_tout( G_sequence ) )	{
			c++;
			t = nod_record.get_time( G_track, start + c );
		}
		fprintf( stdout, "frames %d\n", c );

		t = nod_record.get_time( G_track, i );
		while( t < seq_buffer.get_tout( G_sequence ) )	{

			quat_t q = nod_record.get_quat( G_track, i );
			vector_t axang = q.axisangle();

	//		fprintf( stdout, "kt %f fr %.8f %.8f %.8f ", axang.xf(), axang.yf(), axang.zf() );
			fprintf( stdout, "kt %f fr %.8f %.8f %.8f ", t, axang.x(), axang.y(), axang.z() );
			fprintf( stdout, "\n" );

			i++;
			t = nod_record.get_time( G_track, i );
		}
	}
}

///////////////////////////////////////////////////////////

void reset_player( void )	{

	if( G_play_mode == MODE_PLAY_THROUGH )	{
	
		G_play_time = nod_record.get_time( G_track, G_index );
	}
	else	{
	
		G_track = seq_buffer.get_track( G_sequence );
		while( G_track >= nod_record.count )	{
		
			printf( 
				"reset_player ERR: track overrun: seq:%d track:%d index:%d\n", 
				G_sequence, G_track + USER_FIRST_INDEX, G_index 
			);
			G_sequence++;
			if( G_sequence >= seq_buffer.count )	{
				printf( "reset_player WRAP...\n" );
				G_sequence = 0;
			}
			G_track = seq_buffer.get_track( G_sequence );
		}
		G_index = 0;

		while( nod_record.get_time( G_track, G_index ) < seq_buffer.get_tin( G_sequence ) ) {
			G_index++;
		}

		G_play_time = nod_record.get_time( G_track, G_index );
	}

#if 1
	printf( "RESET: t:%f Sq:%d Tr:%d [i:%d]\n", 
		G_play_time, G_sequence, G_track + USER_FIRST_INDEX, G_index
	);
#endif
}


void update_orientation( void ) {

	G_play_time += G_dt;
	
	if( G_play_mode == MODE_PLAY_THROUGH )	{
	
		if( G_index >= ( nod_record.get_len( G_track ) - 1 ) )	{

			G_index = 0;
			G_play_time = nod_record.get_time( G_track, G_index );
		}
		else
		while( G_play_time > nod_record.get_time( G_track, G_index + 1 ) )	{

//printf( "gt: %d +1: %f > %f\n", G_index, G_play_time, nod_record.get_time( G_track, G_index + 1 ) );
			G_index++;
			if( G_index >= ( nod_record.get_len( G_track ) - 1 ) )	{
//printf( "hitch\n" );
				G_index = 0;
				G_play_time = nod_record.get_time( G_track, G_index );
			}
		}
	}
	else	{

		if( G_play_time > seq_buffer.get_tout( G_sequence ) )	{

			if( G_play_mode == MODE_PLAY_SEQUENCES )	{

				G_sequence++;
				if( G_sequence >= seq_buffer.count )	{
					G_sequence = 0;
				}
			}
			reset_player();
		}

		if( G_index >= ( nod_record.get_len( G_track ) - 1 ) )	{

			printf( 
				"ERR sequence overrun: seq:%d track:%d index:%d\n", 
				G_sequence, G_track + USER_FIRST_INDEX, G_index 
			);
			if( G_play_mode == MODE_PLAY_SEQUENCES )	{

				G_sequence++;
				if( G_sequence >= seq_buffer.count )	{
					G_sequence = 0;
				}
				reset_player();
			}
			else	{

				G_index = 0;
				G_play_time = nod_record.get_time( G_track, G_index );
			}
		}
		else
		while( G_play_time > nod_record.get_time( G_track, G_index + 1 ) )	{

			G_index++;
		}
	}

//printf( "UD: Tr:%d i:%d\n", G_track, G_index );
	G_orientation = nod_record.get_quat( G_track, G_index );
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

		case GLUT_KEY_UP:
			G_magnification *= 2.0;
			printf( "Mag: %f\n", G_magnification );
			break;

		case GLUT_KEY_DOWN:
			G_magnification *= 0.5;
			printf( "Mag: %f\n", G_magnification );
			break;

		case GLUT_KEY_LEFT:

			if( G_play_mode == MODE_PLAY_THROUGH )	{
				G_track--;
				if( G_track < 0 ) G_track = nod_record.count - 1;
				printf( "Track: %d of %d\n", G_track, nod_record.count );
				G_index = 0;
			}
			else	{
				G_sequence--;
				if( G_sequence < 0 )	{
					G_sequence = seq_buffer.count - 1;
				}
				printf( "Seq: %d of %d\n", G_sequence, seq_buffer.count );
			}
			reset_player();
			break;

		case GLUT_KEY_RIGHT:

			if( G_play_mode == MODE_PLAY_THROUGH )	{
				G_track++;
				if( G_track >= nod_record.count ) G_track = 0;
				printf( "Track: %d of %d\n", G_track, nod_record.count );
				G_index = 0;
			}
			else	{
				G_sequence++;
				if( G_sequence >= seq_buffer.count )	{
					G_sequence = 0;
				}
				printf( "Seq: %d of %d\n", G_sequence, seq_buffer.count );
			}
			reset_player();
			break;

		case GLUT_KEY_F1:
			G_play_mode = MODE_PLAY_THROUGH;
			printf( "Mode: PLAY_THROUGH\n" );
			reset_player();
			break;

		case GLUT_KEY_F2:
			G_play_mode = MODE_PLAY_SEQUENCES;
			printf( "Mode: PLAY_SEQUENCES\n" );
			reset_player();
			break;

		case GLUT_KEY_F3:
			G_play_mode = MODE_PLAY_SEQ_LOOP;
			printf( "Mode: PLAY_SEQ_LOOP\n" );
			reset_player();
			break;

		case GLUT_KEY_F4:
			print_skm_array();
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

	float aspect = (float)G_window_resx / (float)G_window_resy;
	G_camera.set_perspective( 70.0, aspect );

	G_master_port.resize( width, height );
}


///////////////////////////////////////////////////////////

void GW_init( void ) {
	
	G_start_time = get_real_time();

	reset_player();

	G_camera.set_placement( vector_t( 0.0, 0.0, 5.0 ), euler_t() );

	float aspect = (float)G_window_resx / (float)G_window_resy;
	G_camera.set_perspective( 70.0, aspect );
	
	G_master_port.resize( G_window_resx, G_window_resy );
}

void SmPl_update( void )	{

	double prev_time = G_time;
	G_time = get_real_time() - G_start_time;
	G_dt = G_time - prev_time;
	
	update_orientation();
	glutPostRedisplay();
	G_count++;
}

void Hermite_DisplayFunc(void)	{
	
	glDrawBuffer( GL_BACK );

	glPushAttrib( GL_ENABLE_BIT | GL_VIEWPORT_BIT | GL_SCISSOR_BIT );
	glEnable( GL_SCISSOR_TEST );
	glEnable( GL_BLEND );
	glEnable( GL_LINE_SMOOTH );
	glBlendFunc( GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA ); 

	glViewport( 0, 0, G_window_resx, G_window_resy );
	glScissor( 0, 0, G_window_resx, G_window_resy );

	glClearDepth( 1.0 );
	glClearColor( 0.0, 0.0, 0.2, 0.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
	glLoadMatrixd( G_camera.frust_mat_arr );

	glLineWidth( 2 );
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

		glLoadIdentity();
		glLoadMatrixd( G_camera.inv_world_mat_arr );
		
		glPushMatrix();
		
			double deg = G_orientation.degrees() * G_magnification;
			vector_t axis = G_orientation.axis();
			
			glRotated( deg, axis.x(), axis.y(), axis.z() );
			
//			glRotated( -90.0, 0.0, 1.0, 0.0 );
			glColor4f( 1.0, 1.0, 1.0, 0.33 );
			
			glLineWidth( 2 );
			glutWireTeapot( 2.0 );
			glLineWidth( 1 );
		
		glPopMatrix();
		
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

	if( ac > 2 )	{
		nod_record.read_file( av[ 1 ] );
		seq_buffer.read_file( av[ 2 ] );
	}
	else
	if( ac > 1 )	{
		printf( "ERR: %s <data-file> <meta-data>\n", av[ 0 ] );
		return( 0 );
	}
	else	{
		printf( "NOTE: %s: loading default files\n", av[ 0 ] );
#if 1
		nod_record.read_file( "../data/data.txt" );
		seq_buffer.read_file( "../data/gesture.txt" );
#elif 0
		nod_record.read_file( "/home/thiebaux/nod/dataMERL05.dat" );
		seq_buffer.read_file( "/home/thiebaux/nod/MERL05gesture.dat" );
#elif 0
		nod_record.read_file( "/home/thiebaux/nod/dataIUI06.dat" );
		seq_buffer.read_file( "/home/thiebaux/nod/IUI06gesture.dat" );
#else
		nod_record.read_file( "/home/thiebaux/dataIUI06.dat" );
//		seq_buffer.read_file( "/home/thiebaux/nod/IUI06gesture.dat" );
#endif
	}

#if 1
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
#endif
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
