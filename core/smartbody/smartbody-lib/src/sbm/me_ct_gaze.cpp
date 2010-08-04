/*
 *  me_ct_gaze.cpp - part of SmartBody-lib
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
 *      Andrew n marshall, USC
 *      Ed Fast, USC
 */

#include "me_ct_gaze.h"

#define MAX_JOINT_LABEL_LEN	32

//#define DFL_GAZE_HEAD_SPEED 180.0
//#define DFL_GAZE_EYE_SPEED  1000.0

// Default Values per Gaze Key:                      LUMBAR,   THORAX, CERVICAL, CRANIAL,  OPTICAL
float MeCtGaze::DEFAULT_LIMIT_PITCH_UP[]   = { 15.0,     6.0,    25.0,     20.0,     35.0    };
float MeCtGaze::DEFAULT_LIMIT_PITCH_DOWN[] = { -15.0,    -6.0,   -25.0,    -20.0,    -35.0   };
float MeCtGaze::DEFAULT_LIMIT_HEADING[]    = { 30.0,     15.0,   60.0,     45.0,     40.0    };
float MeCtGaze::DEFAULT_LIMIT_ROLL[]       = { 10.0,     5.0,    20.0,     15.0,     0.0     };

// Defaults for the Old APIs
// History of Values:
//   Original Marcus Implementation:
//     Speed     (back, neck, eyes): 1000, 1500, 2000
//     Smoothing (back, neck, eyes): 0.8, 0.8, 0.1
//   Original BML Defaults:
//     Speed     (back, neck, eyes): 1000, 1500, 2000
//     Smoothing (back, neck, eyes): 0.8, 0.8, 0.1
//   Gaze Key Revision (June2007):
//     Speed     (head, eyes, eye arg default): 180, 1000, 10000
//     Smoothing (back, neck, eyes): 0.3, 0.1, 0.0
//   Brent's Edits / Gaze Paper Update (Oct2008):
//     Speed     (head, eyes, default eyes arg): 180, 1000, 10000
//     Smoothing (back, neck, eyes): 0.3, 0.1, 0.0
//   Andrew's Updates, equiv to old BML defaults (Feb2009):
//     Speed     (head, eyes, default eyes arg): 360, 10000, 10000
//     Smoothing (back, neck, eyes): 0.8, 0.8, 0.1
const float MeCtGaze::DEFAULT_SPEED_HEAD         = 2500;
const float MeCtGaze::DEFAULT_SPEED_EYES         = 10000.0;

const float MeCtGaze::DEFAULT_SMOOTHING_LUMBAR   = 0.8f; 
const float MeCtGaze::DEFAULT_SMOOTHING_CERVICAL = 0.8f;
const float MeCtGaze::DEFAULT_SMOOTHING_EYEBALL  = 0.1f;



int G_debug_c = 0;
int G_debug = 0;

#if TEST_SENSOR
#include "me_ct_gaze_sensor.h"
void test_sensor_callback( int id, int status )	{
	printf( "test_sensor_callback: id:%d status:%d\n", id, status );
}
#endif

#if ENABLE_HACK_TARGET_CIRCLE
int G_hack_target_circle = 0;
float G_hack_target_heading = 0.0;
#endif

///////////////////////////////////////////////////////////////////////////

/*
	GAZING HIERARCHY:    { "spine1", "spine2", "spine3", "spine4", "spine5", "skullbase", "face_top_parent", "eyeball_left", "eyeball_right" }
	FULL GAZING JOINTS:  { "spine1", "spine2", "spine3", "spine4", "spine5", "skullbase", "eyeball_left", "eyeball_right" }

	BACK: { "spine1", "spine2", "spine3" }
	NECK: { "spine4", "spine5", "skullbase" }
	EYES: { "eyeball_left", "eyeball_right" }

---

	LUMBAR: 	{ "spine1", "spine2" }
	THORAX: 	{ "spine3" }
	CERVICAL:	{ "spine4", "spine5" }
	HEAD:		{ "skullbase" }
	EYES:		{ "eyeball_left", "eyeball_right" }
*/

/*
	height: segment length
		spine1: 7.8 		7.8
		spine2: 17.7		9.9
		spine3: 28.9		11.2
		spine4: 56.0		27.1
		spine5: 59.7		3.7
		skullbase: 64.7 	5.0
		eyeball: 70.6		5.9
*/

const char* MeCtGaze::CONTROLLER_TYPE = "Gaze";

int MeCtGaze::joint_index( const char *label )	{
	if( label )	{
		if( _stricmp( label, "spine1" ) == 0 )        return( GAZE_JOINT_SPINE1 );
		if( _stricmp( label, "spine2" ) == 0 )        return( GAZE_JOINT_SPINE2 );
		if( _stricmp( label, "spine3" ) == 0 )        return( GAZE_JOINT_SPINE3 );
		if( _stricmp( label, "spine4" ) == 0 )        return( GAZE_JOINT_SPINE4 );
		if( _stricmp( label, "spine5" ) == 0 )        return( GAZE_JOINT_SPINE5 );
		if( _stricmp( label, "skullbase" ) == 0 )     return( GAZE_JOINT_SKULL );
		if( _stricmp( label, "eyeball_left" ) == 0 )  return( GAZE_JOINT_EYE_L );
		if( _stricmp( label, "eyeball_right" ) == 0 ) return( GAZE_JOINT_EYE_R );
	}
	return( -1 ); // default err
}

char * MeCtGaze::joint_label( const int index ) {
	switch( index )	{
		case GAZE_JOINT_SPINE1: return( "spine1" );
		case GAZE_JOINT_SPINE2: return( "spine2" );
		case GAZE_JOINT_SPINE3: return( "spine3" );
		case GAZE_JOINT_SPINE4: return( "spine4" );
		case GAZE_JOINT_SPINE5: return( "spine5" );
		case GAZE_JOINT_SKULL:  return( "skullbase" );
		case GAZE_JOINT_EYE_L:  return( "eyeball_left" );
		case GAZE_JOINT_EYE_R:  return( "eyeball_right" );
	}
	return( "UNKNOWN" ); // default err
}

int MeCtGaze::key_index( const char *label )	{
	if( label )	{
		if( _stricmp( label, "lumbar" ) == 0 )   return( GAZE_KEY_LUMBAR );
		if( _stricmp( label, "thorax" ) == 0 )   return( GAZE_KEY_THORAX );
		if( _stricmp( label, "cervical" ) == 0 ) return( GAZE_KEY_CERVICAL );
		if( _stricmp( label, "cranial" ) == 0 )  return( GAZE_KEY_CRANIAL );
		if( _stricmp( label, "optical" ) == 0 )  return( GAZE_KEY_OPTICAL );
		if( _stricmp( label, "back" ) == 0 )     return( GAZE_KEY_LUMBAR );
		if( _stricmp( label, "chest" ) == 0 )    return( GAZE_KEY_THORAX );
		if( _stricmp( label, "neck" ) == 0 )     return( GAZE_KEY_CERVICAL );
		if( _stricmp( label, "head" ) == 0 )     return( GAZE_KEY_CRANIAL );
		if( _stricmp( label, "eyes" ) == 0 )     return( GAZE_KEY_OPTICAL );
	}
	return( -1 ); // default err
}

char * MeCtGaze::key_label( const int key )	{
	switch( key )	{
		case GAZE_KEY_LUMBAR:   return( "LUMBAR" );
		case GAZE_KEY_THORAX:   return( "THORAX" );
		case GAZE_KEY_CERVICAL: return( "CERVICAL" );
		case GAZE_KEY_CRANIAL:  return( "CRANIAL" );
		case GAZE_KEY_OPTICAL:  return( "OPTICAL" );
	}
	return( "UNKNOWN" ); // default err
}

///////////////////////////////////////////////////////////////////////////

MeCtGaze::MeCtGaze( void )	{
	
	started = 0;
	start = 0;
	prev_time = 0.0;

	priority_joint = GAZE_JOINT_EYE_L;
	target_mode = TARGET_POINT;	
	offset_mode = OFFSET_POINT;
	offset_coord = OFFSET_WORLD;
	
	_duration = -1.0f;

	skeleton_ref_p = NULL;

	target_ref_joint_str = NULL;
	target_ref_joint_p = NULL;
	offset_ref_joint_str = NULL;
	offset_ref_joint_p = NULL;
	ref_joint_str = NULL;
	ref_joint_p = NULL;
	
	timing_mode = TASK_SPEED;
	head_speed = 1.0;
	head_time = 1.0;
	fade_interval = 0.0;
	fade_remaining = 0.0;
	fading_normal = 1.0;
	fading_mode = FADING_MODE_OFF;
	
	joint_key_count = 0;
	joint_key_map = NULL;
	joint_key_top_map = NULL;
	joint_key_arr = NULL;
	
	key_smooth_dirty = 0;
	key_bias_dirty = 0;
	key_limit_dirty = 0;
	key_blend_dirty = 0;
	
	joint_count = 0;
	joint_arr = NULL;
}

MeCtGaze::~MeCtGaze( void )	{
	
	if( joint_key_map )	{
		delete [] joint_key_map;
		joint_key_map = NULL;
	}
	if( joint_key_top_map )	{
		delete [] joint_key_top_map;
		joint_key_top_map = NULL;
	}
	if( joint_key_arr )	{
		delete [] joint_key_arr;
		joint_key_arr = NULL;
	}
	if( joint_arr )	{
		delete [] joint_arr;
		joint_arr = NULL;
	}
	if( target_ref_joint_str ) {
		free( target_ref_joint_str );
		target_ref_joint_str = NULL;
	}
	if( offset_ref_joint_str ) {
		free( offset_ref_joint_str );
		offset_ref_joint_str = NULL;
	}
	if( ref_joint_str ) {
		free( ref_joint_str );
		ref_joint_str = NULL;
	}
}

void MeCtGaze::init( int key_fr, int key_to )	{
	int i;
	
	joint_key_count = NUM_GAZE_KEYS;

	joint_key_map = new int[ joint_key_count ];
	joint_key_map[ GAZE_KEY_LUMBAR ] = GAZE_JOINT_SPINE1;
	joint_key_map[ GAZE_KEY_THORAX ] = GAZE_JOINT_SPINE3;
	joint_key_map[ GAZE_KEY_CERVICAL ] = GAZE_JOINT_SPINE4;
	joint_key_map[ GAZE_KEY_HEAD ] = GAZE_JOINT_SKULL;
	joint_key_map[ GAZE_KEY_EYES ] = GAZE_JOINT_EYE_L;

	joint_key_top_map = new int[ joint_key_count ];
	joint_key_top_map[ GAZE_KEY_LUMBAR ] = GAZE_JOINT_SPINE2;
	joint_key_top_map[ GAZE_KEY_THORAX ] = GAZE_JOINT_SPINE3;
	joint_key_top_map[ GAZE_KEY_CERVICAL ] = GAZE_JOINT_SPINE5;
	joint_key_top_map[ GAZE_KEY_HEAD ] = GAZE_JOINT_SKULL;
	joint_key_top_map[ GAZE_KEY_EYES ] = GAZE_JOINT_EYE_L;

	joint_key_arr = new MeCtGazeKey[ joint_key_count ];
	for( i = 0; i < NUM_GAZE_KEYS; i++ )	{
		joint_key_arr[ i ].id = i;
	}

	joint_count = NUM_GAZE_JOINTS;
	joint_arr = new MeCtGazeJoint[ joint_count ];
	for( i = 0; i < NUM_GAZE_JOINTS; i++ )	{
		joint_arr[ i ].id = i;
	}
	
	// Sort key range:
	if( key_fr < 0 )	{
		key_fr = GAZE_KEY_LUMBAR;
	}
	if( key_fr >= NUM_GAZE_KEYS )	{
		key_fr = GAZE_KEY_EYES;
	}
	if( key_to < 0 )	{
		key_to = GAZE_KEY_EYES;
	}
	if( key_to >= NUM_GAZE_KEYS )	{
		key_to = GAZE_KEY_EYES;
	}
	if( key_fr > key_to )	{
		int tmp = key_fr; key_fr = key_to; key_to = tmp;
	}
	key_min = key_fr;
	key_max = key_to;

	// Activate joints based on key range
	int joint_fr = joint_key_map[ key_fr ];
	int joint_to = joint_key_map[ key_to ];
	for( i = joint_fr; i <= joint_to; i++ )	{
		joint_arr[ i ].active = 1;
	}
	joint_arr[ GAZE_JOINT_EYE_R ].active = joint_arr[ GAZE_JOINT_EYE_L ].active;
	
	// Add names to channel array
	for( i = 0; i < joint_count; i++ )	{
		_channels.add( SkJointName( joint_label( i ) ), SkChannel::Quat );
	}
	
	set_task_priority( key_max );
	//set_speed( DFL_GAZE_HEAD_SPEED, DFL_GAZE_EYE_SPEED ); // initializes timing_mode = TASK_SPEED;
	set_speed( DEFAULT_SPEED_HEAD, DEFAULT_SPEED_EYES );

	//set_smooth( 0.3f, 0.1f, 0.0f );
	set_smooth( DEFAULT_SMOOTHING_LUMBAR, DEFAULT_SMOOTHING_CERVICAL, DEFAULT_SMOOTHING_EYEBALL );
	/*
	set_limit( GAZE_KEY_LUMBAR,   15.0, 30.0, 10.0 );
	set_limit( GAZE_KEY_THORAX,   6.0,  15.0, 5.0 );
	set_limit( GAZE_KEY_CERVICAL, 25.0, 60.0, 20.0 );
	set_limit( GAZE_KEY_HEAD,     20.0, 45.0, 15.0 );
	set_limit( GAZE_KEY_EYES,     50.0, 75.0, 0.0 );
	*/

	set_limit( GAZE_KEY_LUMBAR,   DEFAULT_LIMIT_PITCH_UP[GAZE_KEY_LUMBAR],
								  DEFAULT_LIMIT_PITCH_DOWN[GAZE_KEY_LUMBAR],
								  DEFAULT_LIMIT_HEADING[GAZE_KEY_LUMBAR],
								  DEFAULT_LIMIT_ROLL[GAZE_KEY_LUMBAR]);

	set_limit( GAZE_KEY_THORAX,   DEFAULT_LIMIT_PITCH_UP[GAZE_KEY_THORAX],
								  DEFAULT_LIMIT_PITCH_DOWN[GAZE_KEY_THORAX],
								  DEFAULT_LIMIT_HEADING[GAZE_KEY_THORAX],
								  DEFAULT_LIMIT_ROLL[GAZE_KEY_THORAX]);

	set_limit( GAZE_KEY_CERVICAL, DEFAULT_LIMIT_PITCH_UP[GAZE_KEY_CERVICAL],
								  DEFAULT_LIMIT_PITCH_DOWN[GAZE_KEY_CERVICAL],
								  DEFAULT_LIMIT_HEADING[GAZE_KEY_CERVICAL],
								  DEFAULT_LIMIT_ROLL[GAZE_KEY_CERVICAL]);

	set_limit( GAZE_KEY_CRANIAL,  DEFAULT_LIMIT_PITCH_UP[GAZE_KEY_CRANIAL],
								  DEFAULT_LIMIT_PITCH_DOWN[GAZE_KEY_CRANIAL],
								  DEFAULT_LIMIT_HEADING[GAZE_KEY_CRANIAL],
								  DEFAULT_LIMIT_ROLL[GAZE_KEY_CRANIAL]);

	set_limit( GAZE_KEY_OPTICAL,  DEFAULT_LIMIT_PITCH_UP[GAZE_KEY_OPTICAL],
								  DEFAULT_LIMIT_PITCH_DOWN[GAZE_KEY_OPTICAL],
								  DEFAULT_LIMIT_HEADING[GAZE_KEY_OPTICAL],
								  DEFAULT_LIMIT_ROLL[GAZE_KEY_OPTICAL]);

	for( i = 0; i < NUM_GAZE_KEYS; i++ )	{
		set_bias( i, 0.0f, 0.0f, 0.0f );
		set_blend( i, 1.0f );
	}

	MeController::init();

#if TEST_SENSOR
	sensor_p = new MeCtGazeSensor;
	sensor_p->init(
		1.0,
		10.0,
		99,
		this,
		test_sensor_callback
	);
#endif
}

void MeCtGaze::set_task_priority( int key )	{

	priority_joint = joint_key_map[ key ];
//	priority_joint = joint_key_top_map[ key ];
}

///////////////////////////////////////////////////////////////////////////

bool MeCtGaze::is_covered( MeBlendCoverMap & map, float lim )	{
	bool covered = true;
	int i;
	
	for( i=0; i<NUM_GAZE_KEYS; i++ )	{
		if( 
			( get_blend( i ) >= ( 1.0 - lim ) )&&
			( map.read( key_label( i ) ) < lim ) 
		)	{
			covered = false;
		}
	}
	return( covered );
}

void MeCtGaze::apply_coverage( MeBlendCoverMap & map ) {
	int i;
	
	for( i=0; i<NUM_GAZE_KEYS; i++ )	{
		map.update( key_label( i ), get_blend( i ) );
	}
}

bool MeCtGaze::check_and_apply_coverage( MeBlendCoverMap & map, float lim )	{
	bool covered = true;
	int i;
	
	for( i=0; i<NUM_GAZE_KEYS; i++ )	{
		const char *label = key_label( i );
		float this_blend = get_blend( i );
		if( 
			( this_blend >= ( 1.0 - lim ) )&&
			( map.read( label ) < lim ) 
		)	{
			covered = false;
			map.update( label, this_blend );
		}
	}
	return( covered );
}

bool MeCtGaze::calc_real_angle_to_target( float& get_deg )	{

#if 0
static int once = 1;
if( once )	{
once = 0;
vector_t a( 0.0, 0.0, 1.0 ), b( 1.0, 0.0, 1.0 );
float deg = DEG( acos( a.normal().dot( b.normal() ) ) );
printf( "test dot: %f deg\n", deg );
printf( "CALC: joint: %s\n", joint_label( priority_joint ) );
}
#endif

	if( !skeleton_ref_p ) return( false );
	
	char *joint_str = joint_label( priority_joint );
	SkJoint* joint_p = skeleton_ref_p->search_joint( joint_str );
	if( !joint_p ) {
		fprintf( stderr, "MeCtGaze::calc_real_angle_to_target ERR: joint '%s' NOT FOUND in skeleton\n", joint_str );
		return( false );
	}
	joint_p->update_gmat_up();

	SrMat sr_M;
	matrix_t M;
	int i, j;
	
	sr_M = joint_p->gmat();
	for( i=0; i<4; i++ )	{
		for( j=0; j<4; j++ )	{
			M.set( i, j, sr_M.get( i, j ) );
		}
	}
	vector_t w_joint_pos = M.translation( GWIZ_M_TR );
	quat_t w_joint_rot = M.quat( GWIZ_M_TR );

	if( target_mode == TARGET_POINT )	{
		vector_t w_tgt_pos = world_target_point();

#if 1
	// Assume head/eye gaze, no bias
	
		vector_t tgt_dir = ( w_tgt_pos - w_joint_pos ).normalize();
		vector_t fwd_dir = w_joint_rot * (-offset_rot) * vector_t( 0.0, 0.0, 1.0 );
		get_deg = (float)DEG( acos( fwd_dir.dot( tgt_dir ) ) );
#else

// quat_t l_task_q = joint_arr[ priority_joint ].rotation_to_target( w_point ) * offset_rot; // see: MeCtGazeJoint::evaluate()

// NEED TO INTEGRATE offset_rot, forward-ray, bias (see TASK_TIME_HINT)
/*
		// construct an euler from ( z-axis, roll ):
		w_target_orient = euler_t( 
			w_point - (
				joint_arr[ GAZE_JOINT_SKULL ].world_pos +
				joint_arr[ GAZE_JOINT_SKULL ].local_rot *
				joint_arr[ GAZE_JOINT_SKULL ].forward_pos
			), 
			0.0 
		);
*/
		
		vector_t biased_offset_joint_fwd = 
		
//		vector_t tgt_dir = ( w_tgt_pos - world_pos ).normalize();
		joint_arr[ priority_joint ].forward_pos;
		vector_t tgt_dir = ( w_tgt_pos - w_joint_pos ).normalize();

//		vector_t fwd_dir = w_joint_rot * vector_t( 0.0, 0.0, 1.0 );
		vector_t fwd_dir = w_joint_rot * (-offset_rot) * vector_t( 0.0, 0.0, 1.0 );

		get_deg = (float)DEG( acos( fwd_dir.dot( tgt_dir ) ) );
#endif

//printf( "deg: %f\n", get_deg );
	}
	else	{
		quat_t w_tgt_rot = world_target_orient();
// quat_t l_task_q = joint_arr[ priority_joint ].rotation_to_target( w_orient ) * offset_rot; // see: MeCtGazeJoint::evaluate()

// DON"T FORGET offset_rot, bias
		
		vector_t tgt_dir = w_tgt_rot * vector_t( 0.0, 0.0, 1.0 );
		vector_t fwd_dir = w_joint_rot * (-offset_rot) * vector_t( 0.0, 0.0, 1.0 );
		get_deg = (float)DEG( acos( fwd_dir.dot( tgt_dir ) ) );
	}
	
	return( true );
}

///////////////////////////////////////////////////////////////////////////

void MeCtGaze::set_speed( float head_dps, float eyes_dps )	{
	timing_mode = TASK_SPEED;
	head_speed = head_dps;
	joint_arr[ GAZE_JOINT_EYE_L ].speed = eyes_dps;
	joint_arr[ GAZE_JOINT_EYE_R ].speed = eyes_dps;
}

void MeCtGaze::set_time_hint( float head_sec )	{
	
	timing_mode = TASK_TIME_HINT;
	head_time = head_sec;
}

#if 0
void MeCtGaze::set_smooth( float smooth_basis ) {

	if( smooth_basis < 0.0 ) smooth_basis = 0.0;
	if( smooth_basis > 1.0 ) smooth_basis = 1.0;

	joint_arr[ GAZE_JOINT_SPINE1 ].smooth = smooth_basis;
	joint_arr[ GAZE_JOINT_SPINE2 ].smooth = smooth_basis;
	joint_arr[ GAZE_JOINT_SPINE3 ].smooth = smooth_basis;
	joint_arr[ GAZE_JOINT_SPINE4 ].smooth = smooth_basis;
	joint_arr[ GAZE_JOINT_SPINE5 ].smooth = smooth_basis;
	joint_arr[ GAZE_JOINT_SKULL ].smooth = smooth_basis;
}

void MeCtGaze::set_smooth( float back_sm, float neck_sm, float eyes_sm )	{
	
	if( back_sm < 0.0 ) back_sm = 0.0;
	if( back_sm > 1.0 ) back_sm = 1.0;
	if( neck_sm < 0.0 ) neck_sm = 0.0;
	if( neck_sm > 1.0 ) neck_sm = 1.0;
	if( eyes_sm < 0.0 ) eyes_sm = 0.0;
	if( eyes_sm > 1.0 ) eyes_sm = 1.0;
	
	joint_arr[ GAZE_JOINT_SPINE1 ].smooth = back_sm;
	joint_arr[ GAZE_JOINT_SPINE2 ].smooth = back_sm;
	joint_arr[ GAZE_JOINT_SPINE3 ].smooth = back_sm;

	joint_arr[ GAZE_JOINT_SPINE4 ].smooth = neck_sm;
	joint_arr[ GAZE_JOINT_SPINE5 ].smooth = neck_sm;
	joint_arr[ GAZE_JOINT_SKULL ].smooth = neck_sm;

	joint_arr[ GAZE_JOINT_EYE_L ].smooth = eyes_sm;
	joint_arr[ GAZE_JOINT_EYE_R ].smooth = eyes_sm;
}
#endif

/////////////////////////////////////////////////////////////////////////////

void MeCtGaze::inspect_skeleton( SkJoint* joint_p, int depth )	{
	int i, j, n;
	
	if( joint_p )	{
		const char *name = joint_p->name();
		for( j=0; j<depth; j++ ) { printf( " " ); }
		printf( "%s\n", name );
		n = joint_p->num_children();
		for( i=0; i<n; i++ )	{
			inspect_skeleton( joint_p->child( i ), depth + 1 );
		}
	}
}

void MeCtGaze::inspect_skeleton_local_transform( SkJoint* joint_p, int depth )	{
	
	if( joint_p )	{
		const char *name = joint_p->name();
		matrix_t M;
		int i, j;

		SrMat sr_M = joint_p->lmat();
		for( i=0; i<4; i++ )	{
			for( j=0; j<4; j++ )	{
				M.set( i, j, sr_M.get( i, j ) );
			}
		}
		vector_t pos = M.translation( GWIZ_M_TR );
		euler_t rot = M.euler( GWIZ_M_TR );

		for( j=0; j<depth; j++ ) { printf( " " ); }
		printf( "%s : pos{ %.3f %.3f %.3f } : phr{ %.2f %.2f %.2f }\n", 
			name,
			pos.x(), pos.y(), pos.z(),
			rot.p(), rot.h(), rot.r()
		);

		int n = joint_p->num_children();
		for( i=0; i<n; i++ )	{
			inspect_skeleton_local_transform( joint_p->child( i ), depth + 1 );
		}
	}
}

void MeCtGaze::inspect_skeleton_world_transform( SkJoint* joint_p, int depth )	{
	
	if( joint_p )	{
		const char *name = joint_p->name();
		matrix_t M;
		int i, j;

		joint_p->update_gmat_up();
		SrMat sr_M = joint_p->gmat();
		for( i=0; i<4; i++ )	{
			for( j=0; j<4; j++ )	{
				M.set( i, j, sr_M.get( i, j ) );
			}
		}
		vector_t pos = M.translation( GWIZ_M_TR );
		euler_t rot = M.euler( GWIZ_M_TR );

		for( j=0; j<depth; j++ ) { printf( " " ); }
		printf( "%s : pos{ %.3f %.3f %.3f } : phr{ %.2f %.2f %.2f }\n", 
			name,
			pos.x(), pos.y(), pos.z(),
			rot.p(), rot.h(), rot.r()
		);
		
		int n = joint_p->num_children();
		for( i=0; i<n; i++ )	{
			inspect_skeleton_world_transform( joint_p->child( i ), depth + 1 );
		}
	}
}

void MeCtGaze::update_skeleton_gmat( void )	{

	if( skeleton_ref_p )	{
		SkJoint* joint_p = skeleton_ref_p->search_joint( "skullbase" );
		if( joint_p ) {
			joint_p->update_gmat_up();
		}
		else	{
			fprintf( stderr, "MeCtGaze::update_skeleton_gmat ERR: joint 'skullbase' NOT FOUND in skeleton\n" );
		}

		joint_p = skeleton_ref_p->search_joint( "eyeball_left" );
		if( joint_p ) {
			joint_p->update_gmat_up();
		}
		else	{
			fprintf( stderr, "MeCtGaze::update_skeleton_gmat ERR: joint 'eyeball_left' NOT FOUND in skeleton\n" );
		}

		joint_p = skeleton_ref_p->search_joint( "eyeball_right" );
		if( joint_p ) {
			joint_p->update_gmat_up();
		}
		else	{
			fprintf( stderr, "MeCtGaze::update_skeleton_gmat ERR: joint 'eyeball_right' NOT FOUND in skeleton\n" );
		}
	}
	else	{
		fprintf( stderr, "MeCtGaze::update_skeleton_gmat ERR: skeleton NOT FOUND\n" );
	}
}

void MeCtGaze::load_forward_pos( void ) {

	vector_t world_mid_eye_pos = 
		joint_arr[ GAZE_JOINT_EYE_L ].world_pos.lerp( 
			0.5, joint_arr[ GAZE_JOINT_EYE_R ].world_pos 
		) +
		vector_t( 0.0, 0.0, 5.0 );

	joint_arr[ GAZE_JOINT_SPINE1 ].forward_pos =
		world_mid_eye_pos - joint_arr[ GAZE_JOINT_SPINE1 ].world_pos;
		
	joint_arr[ GAZE_JOINT_SPINE2 ].forward_pos =
		world_mid_eye_pos - joint_arr[ GAZE_JOINT_SPINE2 ].world_pos;
		
	joint_arr[ GAZE_JOINT_SPINE3 ].forward_pos =
		world_mid_eye_pos - joint_arr[ GAZE_JOINT_SPINE3 ].world_pos;
		
	joint_arr[ GAZE_JOINT_SPINE4 ].forward_pos =
		world_mid_eye_pos - joint_arr[ GAZE_JOINT_SPINE4 ].world_pos;
		
	joint_arr[ GAZE_JOINT_SPINE5 ].forward_pos =
		world_mid_eye_pos - joint_arr[ GAZE_JOINT_SPINE5 ].world_pos;

	joint_arr[ GAZE_JOINT_SKULL ].forward_pos =
		world_mid_eye_pos - joint_arr[ GAZE_JOINT_SKULL ].world_pos;
}

#if 0
SkJoint* MeCtGaze::get_joint( char *joint_str, SkJoint *joint_p )	{

	if( joint_str )	{
		if( joint_p == NULL )	{
			if( skeleton_ref_p )	{
				joint_p = skeleton_ref_p->search_joint( joint_str );
				if( joint_p == NULL )	{
					fprintf( stderr, "MeCtGaze::get_joint ERR: joint '%s' NOT FOUND in skeleton\n", joint_str );
					free( joint_str );
					joint_str = NULL;
				}
			}
			else	{
				fprintf( stderr, "MeCtGaze::get_joint ERR: skeleton NOT FOUND\n" );
			}
		}
	}
	return( joint_p );
}

SkJoint* MeCtGaze::target_ref_joint( void ) {
//	return( target_ref_joint_p = get_joint( target_ref_joint_str, target_ref_joint_p ) );
	return( get_joint( target_ref_joint_str, &target_ref_joint_p ) );
}

SkJoint* MeCtGaze::offset_ref_joint( void ) {
//	return( offset_ref_joint_p = get_joint( offset_ref_joint_str, offset_ref_joint_p ) );
	return( get_joint( offset_ref_joint_str, &offset_ref_joint_p ) );
}
#endif

SkJoint* MeCtGaze::reference_joint( void )	{

	if( ref_joint_str )	{
		if( ref_joint_p == NULL )	{
			if( skeleton_ref_p )	{
				ref_joint_p = skeleton_ref_p->search_joint( ref_joint_str );
				if( ref_joint_p == NULL )	{
					fprintf( stderr, "MeCtGaze::reference_joint ERR: joint '%s' NOT FOUND in skeleton\n", ref_joint_str );
					free( ref_joint_str );
					ref_joint_str = NULL;
				}
			}
			else	{
				fprintf( stderr, "MeCtGaze::reference_joint ERR: skeleton NOT FOUND\n" );
			}
		}
	}
	return( ref_joint_p );
}

vector_t MeCtGaze::world_target_point( void )	{
	
/*
	add point offset here:

*/

	SkJoint* joint_p = reference_joint();
	if( joint_p )	{
		SrMat sr_M;
		matrix_t M;
		int i, j;
		
		joint_p->update_gmat_up();
		sr_M = joint_p->gmat();
		for( i=0; i<4; i++ )	{
			for( j=0; j<4; j++ )	{
				M.set( i, j, sr_M.get( i, j ) );
			}
		}
		vector_t pos = M.translation( GWIZ_M_TR );
		quat_t rot = M.quat( GWIZ_M_TR );
		return( pos + rot * point_target_pos );
	}
	return( point_target_pos );
}

quat_t MeCtGaze::world_target_orient( void )	{
	
/*
	add point offset here:

*/

	SkJoint* joint_p = reference_joint();
	if( joint_p )	{
		SrMat sr_M;
		matrix_t M;
		int i, j;
		
		joint_p->update_gmat_up();
		sr_M = joint_p->gmat();
		for( i=0; i<4; i++ )	{
			for( j=0; j<4; j++ )	{
				M.set( i, j, sr_M.get( i, j ) );
			}
		}
		quat_t rot = M.quat( GWIZ_M_TR );
		return( rot * orient_target_rot );
	}
	return( orient_target_rot );
}

void MeCtGaze::controller_start( void )	{
	
#if ENABLE_HACK_TARGET_CIRCLE
//G_hack_target_heading = 0.0;  // don't update this for every gaze controller
#endif

	started = 1;
	start = 1;

#if TEST_SENSOR
	sensor_p->controller_start();
#endif
}

void MeCtGaze::controller_start_evaluate( void )	{
	int i;

	if( _context->channels().size() > 0 )	{
		skeleton_ref_p = _context->channels().skeleton();
	}
	
	// ensure skeleton global tansforms are up to date
	update_skeleton_gmat();
	
	for( i=0; i<joint_count; i++ )	{

		float local_contrib;
		if( i < priority_joint )	{
			local_contrib = 1.0f / ( (float)( priority_joint + 1 - i ) );
		}
		else	{
			local_contrib = 1.0f;
		}
		joint_arr[ i ].task_weight = local_contrib;
		
		int context_index = _toContextCh[ i ];
		if( context_index < 0 ) {
			fprintf( stderr, "MeCtGaze::controller_start ERR: joint idx:%d NOT FOUND in skeleton\n", i );
		} 
		else {
			SkJoint* joint_p = _context->channels().joint( context_index );
			if( !joint_p )	{
				fprintf( stderr, "MeCtGaze::controller_start ERR: joint context-idx:%d NOT FOUND in skeleton\n", context_index );
			} 
			else {
				joint_arr[ i ].init( joint_p );
				joint_arr[ i ].start();
			}
		}
#if 0
		printf( "start: [%d]", i );
		printf( " wgt: %.4f",
			joint_arr[ i ].task_weight 
		);
#endif
	}
	
	// set forward position to eyeball center in local coords, 
	//  after joints have start()ed, to init world transform state
	load_forward_pos();

	// set head_speed based on head-rot-to-target-rot
	if( timing_mode == TASK_TIME_HINT )	{

		quat_t w_target_orient;
		if( target_mode == TARGET_POINT )	{
			vector_t w_point = world_target_point();
			// construct an euler from ( z-axis, roll )
			w_target_orient = euler_t( 
				w_point - (
					joint_arr[ GAZE_JOINT_SKULL ].world_pos +
					joint_arr[ GAZE_JOINT_SKULL ].local_rot *
					joint_arr[ GAZE_JOINT_SKULL ].forward_pos
				), 
				0.0 
			);
		}
		else	{
			w_target_orient = world_target_orient();
		}
		
		quat_t body_head_rot = 
			-joint_arr[ GAZE_JOINT_SPINE1 ].parent_rot *
			joint_arr[ GAZE_JOINT_SKULL ].world_rot * 
			joint_arr[ GAZE_JOINT_SKULL ].forward_rot;
		
		quat_t body_task_rot = 
			-joint_arr[ GAZE_JOINT_SPINE1 ].parent_rot *
			w_target_orient * 
			joint_arr[ GAZE_JOINT_SKULL ].forward_rot;
		
		quat_t head_task_rot = ( -body_head_rot ).product( body_task_rot );
		gw_float_t head_task_deg = head_task_rot.degrees();
		head_speed = (float)( head_task_deg / head_time );
//printf( "MeCtGaze::controller_start TASK: %f deg  %f dps\n", head_task_deg, head_speed );
	}

	float total_w = 0.0;
	for( i=0; i<=GAZE_JOINT_SKULL; i++ )	{
		total_w += joint_arr[ i ].task_weight;
	}

	// move to -update-joint-speed() if dirty bit is set.
	for( i=0; i<=GAZE_JOINT_SKULL; i++ )	{
		joint_arr[ i ].speed_ratio = joint_arr[ i ].task_weight / total_w;
		joint_arr[ i ].speed = joint_arr[ i ].speed_ratio * head_speed;
//		joint_arr[ i ].speed = head_speed / ( priority_joint + 1 );
	}
}

void MeCtGaze::set_fade_in( float interval )	{
	
	if( fading_normal < 1.0 )	{
		fade_interval = interval * ( 1.0f - fading_normal );
		fade_remaining = fade_interval;
		fading_mode = FADING_MODE_IN;
	}
}

void MeCtGaze::set_fade_out( float interval )	{

	fading_mode = FADING_MODE_OUT;
	if( fading_normal > 0.0 )	{
		fade_interval = interval * fading_normal;
		fade_remaining = fade_interval;
	}
}

/*
	for( i = 0; i < NUM_GAZE_KEYS; i++ )	{
		set_blend( i, 1.0f );
	}
	joint_key_arr[ GAZE_KEY_EYES ].smooth = eyes_sm;
	joint_key_arr[ GAZE_KEY_HEAD ].smooth = neck_sm;
	joint_key_arr[ GAZE_KEY_CERVICAL ].smooth = neck_sm;
	joint_key_arr[ GAZE_KEY_THORAX ].smooth = back_sm;
	joint_key_arr[ GAZE_KEY_LUMBAR ].smooth = back_sm;
*/

#define SMOOTH_RATE_REF (30.0f)

bool MeCtGaze::update_fading( float dt )	{

printf( "fade: %f %f %f\n", fade_interval, fade_remaining, fading_normal );

	if( fade_remaining <= 0.0 ) {
		if( fading_mode = FADING_MODE_IN )	{
			fading_mode = FADING_MODE_OFF;
			fading_normal = 1.0f;
			return( true );
		}
		fading_normal = 0.0;
		return( false );
	}

	if( fading_mode == FADING_MODE_IN )	{
		fading_normal = 1.0f - fade_remaining / fade_interval;
	}
	else
	if( fading_mode == FADING_MODE_OUT )	{
		fading_normal = fade_remaining / fade_interval;
	}

	for( int i = 0; i < joint_key_count; i++ )	{

		MeCtGazeKey* key_p = joint_key_arr + i;
//		float s = (float)( 0.01 + ( 1.0 - powf( key_p->smooth, dt * SMOOTH_RATE_REF ) ) * 0.99 );

printf( " fr[%d] weight: %f\n", i, key_p->blend_weight );

		float s = (float)( 0.01 + ( 1.0f - powf( key_p->smooth, dt * SMOOTH_RATE_REF / fade_interval ) ) * 0.99 );
		float super_sm = s * fading_normal;
		key_p->blend_weight = s * key_p->blend_weight + ( 1.0f - s ) * super_sm;

printf( " to[%d] weight: %f s:%f ssm:%f \n", i, key_p->blend_weight, s, super_sm );
	}
	
	fade_remaining -= dt;
	if( fading_mode == FADING_MODE_IN )	{
	}
	else
	if( fading_mode == FADING_MODE_OUT )	{

	}
	return( true );
}

bool MeCtGaze::controller_evaluate( double t, MeFrameData& frame )	{
	
//printf( "smooth head: %f eye: %f\n", joint_arr[ GAZE_JOINT_SKULL ].smooth, joint_arr[ GAZE_JOINT_EYE_L ].smooth );

#if 0
	if( !started )	{
		printf( "MeCtGaze::controller_evaluate ERR: not started for this: 0x%x\n", this );
	}
	else	{
		printf( "MeCtGaze::controller_evaluate OK: started for this: 0x%x\n", this );
	}
#endif
	
#if 0
	static int once = 1;
	if( once )	{
		once = 0;

#if ENABLE_FORWARD_RAY_TEST
		test_forward_ray();
#endif
#if 0
		printf( "-- skeleton:\n" );
		if( skeleton_ref_p )	{
			SkJoint* joint_p = skeleton_ref_p->search_joint( SbmPawn::WORLD_OFFSET_JOINT_NAME );
			inspect_skeleton( joint_p );
//			inspect_skeleton_local_transform( joint_p );
//			inspect_skeleton_world_transform( joint_p );
		}
		printf( "--\n" );
#endif
	}
#endif
	
	if( _duration > 0.0 )	{
		if( t > (double)_duration )	{
			return( FALSE );
		}
	}
	
	float dt;
	if( start ) {
		start = 0;
		dt = 0.001f;
		controller_start_evaluate();
	}
	else	{
		dt = (float)(t - prev_time);
	}
	prev_time = t;

#if 0
	if( fading_mode != FADING_MODE_OFF )	{
		update_fading( dt );
		if( fading_normal <= 0.0 ) {
			return( TRUE );
		}
	}
#endif

	update_skeleton_gmat();
	
	// map key values to joints if set
	apply_smooth_keys();
	apply_bias_keys();
	apply_limit_keys();
	apply_blend_keys();
	
	vector_t w_point;
	quat_t w_orient;
	if( target_mode == TARGET_POINT )	{
#if ENABLE_HACK_TARGET_CIRCLE
		if( G_hack_target_circle )	{
			G_hack_target_heading += 60.0f * dt;  // 60 degrees per second
			w_point = euler_t( 0.0, G_hack_target_heading, 0.0 ) * vector_t( 0.0, 150.0, 10000.0 );  // 150 up and 10,000 radius
		}
		else
#endif
		w_point = world_target_point();
	}
	else	{
		w_orient = world_target_orient();
	}

#if 0
	if( offset_mode == OFFSET_POINT )	{
		
	}
	else	{
		
	}
#endif

	SrBuffer<float>& buff = frame.buffer();
	int channels_size = _channels.size();

	for( int i=0; i<channels_size; ++i ) {

		if( joint_arr[ i ].active ) {
		
			int index = frame.toBufferIndex( _toContextCh[ i ] );

	if( G_debug_c )	{
		G_debug = 1;
	}
			quat_t Q_in = quat_t(
				buff[ index + 0 ],
				buff[ index + 1 ],
				buff[ index + 2 ],
				buff[ index + 3 ]
			);

			quat_t Q_eval;
			if( target_mode == TARGET_POINT )	{
				Q_eval = joint_arr[ i ].evaluate( dt, w_point, offset_rot, 1.0 );
			}
			else	{
				Q_eval = joint_arr[ i ].evaluate( dt, w_orient, offset_rot, 1.0 );
			}

//printf( "%d: %f\n", i, joint_arr[ i ].blend_weight );
			quat_t Q_out = Q_in.lerp( joint_arr[ i ].blend_weight, Q_eval );
				
#define SPINE_LOCK_HEAD_TEST 0
#if SPINE_LOCK_HEAD_TEST
			if( i == GAZE_JOINT_SPINE1 )	{
	//			euler_t e( 0.0, 0.0, 0.0 );
	//			euler_t e( -90.0, 0.0, 0.0 );
	//			euler_t e( 90.0, 0.0, 0.0 );
	//			euler_t e( 0.0, 90.0, 0.0 );
	//			euler_t e( 0.0, 180.0, 0.0 );
	//			euler_t e( 0.0, 0.0, 90.0 );
	//			Q_out = e;
			}
			if( ( i == GAZE_JOINT_SPINE1 )||( i == GAZE_JOINT_SKULL ) )	{
				buff[ index + 0 ] = (float) Q_out.w();
				buff[ index + 1 ] = (float) Q_out.x();
				buff[ index + 2 ] = (float) Q_out.y();
				buff[ index + 3 ] = (float) Q_out.z();
			}
#else

			buff[ index + 0 ] = (float) Q_out.w();
			buff[ index + 1 ] = (float) Q_out.x();
			buff[ index + 2 ] = (float) Q_out.y();
			buff[ index + 3 ] = (float) Q_out.z();
				
#endif
G_debug = 0;

#if 0
//euler_t e = Q_out; if( i == 6 ) e.print();
SkJointName n = _channels.name( i );
euler_t e = Q_out;
printf( "%d %s %f %f %f\n", i, n.get_string(), e.x(), e.y(), e.z() );
#endif

			// Mark channel changed
			frame.channelUpdated( i );
		}
	}

if( G_debug_c )	{
	G_debug_c--;
	printf( "DEBUG %d ----------------------------------------\n", G_debug_c );
}

#if TEST_SENSOR
	sensor_p->controller_evaluate( t, frame );
#endif
	return( TRUE );
}

void MeCtGaze::print_state( int tabs )	{
	fprintf( stdout, "MeCtGaze" );
	const char* str = name();
	if( str )
		fprintf( stdout, " \"%s\"\n", str );
}

///////////////////////////////////////////////////////////////////////////
