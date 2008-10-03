/*
 *  me_ct_gaze.h - part of SmartBody-lib
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
 */

#ifndef ME_CT_GAZE_H
#define ME_CT_GAZE_H

#include <SK/sk_skeleton.h>
#include <ME/me_controller.h>

#include "gwiz_math.h"
#include "me_ct_gaze_joint.h"
#include "me_blend_cover.h"

#define TEST_SENSOR 0

#if TEST_SENSOR
class MeCtGazeSensor;
#endif

#define ENABLE_HACK_TARGET_CIRCLE 0
#if ENABLE_HACK_TARGET_CIRCLE
extern int G_hack_target_circle;
#endif

///////////////////////////////////////////////////////////////////////////

class MeCtGazeKey	{
	public:
	
		MeCtGazeKey() {
			id = -1;
//			task_weight = 1.0;
			limit_p_up = 90.0;
			limit_p_dn = 90.0;
			limit_h = 180.0;
			limit_r = 180.0;
			blend_weight = 1.0;
		}
		virtual ~MeCtGazeKey() {}
		
		int id;
		
		euler_t	bias_rot;
		float	limit_p_up, limit_p_dn, limit_h, limit_r;
//		float	task_weight;
		float	blend_weight;
};

class MeCtGaze : public MeController	{
	
	private:
		enum timing_mode_enum_set	{
			TASK_SPEED,
			TASK_TIME_HINT
		};

		enum target_mode_enum_set	{
			TARGET_POINT,
			TARGET_ORIENT
		};

		enum offset_mode_enum_set	{
			OFFSET_POINT,
			OFFSET_ORIENT
		};

		enum offset_coord_enum_set	{
			OFFSET_WORLD,
			OFFSET_JOINT_LOCAL,
			OFFSET_PARENT_LOCAL
		};

		enum gaze_joint_enum_set	{
			GAZE_JOINT_SPINE1,
			GAZE_JOINT_SPINE2,
			GAZE_JOINT_SPINE3,
			GAZE_JOINT_SPINE4,
			GAZE_JOINT_SPINE5,
			GAZE_JOINT_SKULL,
			GAZE_JOINT_EYE_L,
			GAZE_JOINT_EYE_R,
			NUM_GAZE_JOINTS
		};

#if TEST_SENSOR
		MeCtGazeSensor* sensor_p;
#endif

	public:
		enum gaze_key_enum_set	{
			GAZE_KEY_LUMBAR,
			GAZE_KEY_THORAX,
			GAZE_KEY_CERVICAL,
			GAZE_KEY_CRANIAL,
			GAZE_KEY_OPTICAL,
			NUM_GAZE_KEYS,
			GAZE_KEY_BACK = GAZE_KEY_LUMBAR,
			GAZE_KEY_CHEST = GAZE_KEY_THORAX,
			GAZE_KEY_NECK = GAZE_KEY_CERVICAL,
			GAZE_KEY_HEAD = GAZE_KEY_CRANIAL,
			GAZE_KEY_EYES = GAZE_KEY_OPTICAL
		};

		static int joint_index( const char *label );
		static char * joint_label( const int key );

		static int key_index( const char *label );
		static char * key_label( const int key );

		static int valid_key( int key ) { return( ( key >= 0 )&&( key < NUM_GAZE_KEYS ) ); }
		static const char* type_name;

		MeCtGaze();
		virtual ~MeCtGaze();
		
		void init( void )	{ init( GAZE_KEY_EYES, GAZE_KEY_LUMBAR ); }
		void init( int key_fr, int key_to );
//		void init( int key_fr, int key_to, int priority_key = GAZE_JOINT_EYE_L );
		void set_task_priority( int key ); // which joint to attempt to satisfy target task

	// TARGETING AND OFFSETS: me_ct_gaze_target.cpp
#if 0
		// target coordinate
		void set_target_point( float x, float y, float z );
		void set_target_euler( float p, float h, float r );
		void set_target_swing( float sw_p, float sw_h, float tw = 0.0 ); // swing-twist: pitch, heading, roll
		void set_target_polar( float d, float a, float r = 0.0 );        // polar-coord: direction, radial angle, roll

		// target coordinate system
		void set_target_coord_world( void );
		void set_target_coord_joint( char *joint_name );
		void set_target_coord_joint( SkJoint* joint_p );
		
		// offset coordinate
		void set_offset_point( float x, float y, float z );
		void set_offset_euler( float p, float h, float r = 0.0 );        // euler:       pitch, heading, roll
		void set_offset_swing( float sw_p, float sw_h, float tw = 0.0 ); // swing-twist: pitch, heading, roll
		void set_offset_polar( float d, float a, float r = 0.0 );        // polar-coord: direction, radial angle, roll

		// offset coordinate system
		void set_offset_coord_world( void );
		void set_offset_coord_joint( char *joint_name );
		void set_offset_coord_joint( SkJoint* joint_p );
		void set_offset_coord_parent( void ); // relative to each joint's parent
#else
		
		 // deprecate: backwards compatibility
		void set_target_joint( float x, float y, float z, SkJoint* ref_joint_p = NULL );
		void set_target( float x, float y, float z, char *ref_joint_name = NULL ); // world-coord if NULL
		void set_orient_joint( float p, float h, float r, SkJoint* ref_joint_p = NULL );
		void set_orient( float p, float h, float r, char *ref_joint_name = NULL );

		void set_offset_euler( float off_p, float off_h, float off_r = 0.0 );
		void set_offset_swing( float off_p, float off_h, float off_r = 0.0 ); // swing-twist: pitch, heading, roll
		void set_offset_polar( float off_d, float off_a, float off_r = 0.0 ); // direction, radial angle, roll

#endif
	
	// TUNE AND SPEED:
	
		void set_smooth( float smooth_basis );
		void set_speed( float head_dps, float eyes_dps = 10000.0 );
		void set_time_hint( float head_sec ); // derives approximate speed from task angle

		void set_speed( float back_dps, float neck_dps, float eyes_dps );
		void set_smooth( float back_sm, float neck_sm, float eyes_sm );

#if 0
		void set_limits( float spine_lim, float neck_lim, float eyes_lim ); // per component?
		void set_weights( float spine_wt, float neck_wt, float eyes_wt ); // task weight distribution
#endif
		
	// KEY-MAPPED PARAMETERS: me_ct_gaze_keymap.cpp
		
		// BIAS: forward ray joint bias offsets
		void set_bias( int key, float p, float h, float r );
		
		void set_bias_pitch( int key, float p );
		void set_bias_heading( int key, float h );
		void set_bias_roll( int key, float r );

		void set_bias_pitch( int key1, int key2, float p1, float p2 );
		void set_bias_heading( int key1, int key2, float h1, float h2 );
		void set_bias_roll( int key1, int key2, float r1, float r2 );

		void set_bias_pitch( float e_p, float h_p, float c_p, float t_p, float l_p );
		void set_bias_heading( float e_h, float h_h, float c_h, float t_h, float l_h );
		void set_bias_roll( float e_r, float h_r, float c_r, float t_r, float l_r );

		// BLEND: blending weight against underlying pose
		void set_blend( float l_w, float t_w, float c_w, float h_w, float e_w );
		void set_blend( int key, float w );
		void set_blend( int key1, int key2, float w1, float w2 );

		// LIMIT: key-group rotation limit
		void set_limit( int key, float p, float h, float r );
		void set_limit( int key, float p_up, float p_dn, float h, float r );
#if 0
		void set_limit_pitch( int key, float l );
		void set_limit_heading( int key, float l );
		void set_limit_roll( int key, float l );

		void set_limit_pitch( float e_l, float h_l, float c_l, float t_l, float l_l );
		void set_limit_heading( float e_l, float h_l, float c_l, float t_l, float l_l );
		void set_limit_roll( float e_l, float h_l, float c_l, float t_l, float l_l );

		void set_limit_pitch( int key1, int key2, float l1, float l2 );
		void set_limit_heading( int key1, int key2, float l1, float l2 );
		void set_limit_roll( int key1, int key2, float l1, float l2 );

		// WEIGHT: task weight distribution
		void set_weight( float e_w, float h_w, float c_w, float t_w, float l_w );
		void set_weight( int key, float w );
		void set_weight( int key1, int key2, float w1, float w2 );
#endif

		// QUERIES: for coverage comparisons
		int get_min_key( void ) { return( key_min ); }
		int get_max_key( void ) { return( key_max ); }

		float get_blend( int key ) { 
			if( valid_key( key ) ) {
				if( ( key >= key_min )&&( key <= key_max ) )	{
					return( joint_key_arr[ key ].blend_weight );
				}
			}
			return( 0.0 ); 
		}
		bool is_full_blend( int key, float lim = 0.9999 ) { return( get_blend( key ) >= lim ); }
		bool is_covered( MeBlendCoverMap & map, float lim = 0.9999 );
		void apply_coverage( MeBlendCoverMap & map );
		bool check_and_apply_coverage( MeBlendCoverMap & map, float lim = 0.9999 );
		
		// QUERY: for target acquisition sensor
		bool calc_real_angle_to_target( float& deg );
		
	private:

		double	prev_time;
		int 	start;	// to initialize prev_time, dt
		int 	started;

		int		key_min, key_max;
		int 	priority_joint;
		int 	target_mode;
		int 	offset_mode;
		int 	offset_coord;

		float 			_duration;
		SkChannelArray	_channels;
		SkSkeleton* 	skeleton_ref_p;

		char*			target_ref_joint_str;
		SkJoint*		target_ref_joint_p;
		char*			offset_ref_joint_str;
		SkJoint*		offset_ref_joint_p;
		char*			ref_joint_str; // deprecate
		SkJoint*		ref_joint_p; // deprecate
		
		vector_t		point_target_pos;
		quat_t			orient_target_rot;
		vector_t		point_offset_pos;
		quat_t			orient_offset_rot;
		quat_t			offset_rot; // deprecate
		
		int 			timing_mode;
		float 			head_speed;
		float 			head_time;

		int 			joint_key_count;
		int*			joint_key_map;
		int*			joint_key_top_map;
		MeCtGazeKey*	joint_key_arr;
		int 			key_bias_dirty;
		int 			key_limit_dirty;
		int 			key_blend_dirty;
		void			apply_bias_keys( void );
		void			apply_limit_key( int J, int K, float weight );
		void			apply_limit_keys( void );
		void 			apply_blend_keys( void );

		int 			joint_count;
		MeCtGazeJoint*	joint_arr;

		void		inspect_skeleton( SkJoint* joint_p, int depth = 0 );
		void		inspect_skeleton_local_transform( SkJoint* joint_p, int depth = 0 );
		void		inspect_skeleton_world_transform( SkJoint* joint_p, int depth = 0 );

		void		update_skeleton_gmat( void );
		void		load_forward_pos( void );
		SkJoint*	reference_joint( void ); // deprecate
#if 0
		SkJoint*	get_joint( char *joint_str, SkJoint *joint_p );
		SkJoint*	target_ref_joint( void );
		SkJoint*	offset_ref_joint( void );
#endif
		vector_t	world_target_point( void );
		quat_t		world_target_orient( void );
		
		virtual void controller_start();
		virtual bool controller_evaluate( double t, MeFrameData& frame );
		virtual SkChannelArray& controller_channels()	{ return( _channels ); }
		virtual double controller_duration()			{ return( (double)_duration ); }
		virtual const char* controller_type()			{ return( type_name ); }
		virtual void print_state( int tabs );
};

///////////////////////////////////////////////////////////////////////////
#endif
