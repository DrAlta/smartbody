/*
 *  me_ct_gaze_joint.h - part of SmartBody-lib
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

#ifndef ME_CT_GAZE_JOINT_H
#define ME_CT_GAZE_JOINT_H

#include <SK/sk_skeleton.h>
#include <ME/me_controller.h>

#include "gwiz_math.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

///////////////////////////////////////////////////////////////////////////

class MeCtGazeJoint	{

	public:
		MeCtGazeJoint();
		virtual ~MeCtGazeJoint() {}
		
		int id;
		int active;
		
		float 	limit_p_up, limit_p_dn, limit_h, limit_r;
		float 	task_weight;
		float 	blend_weight;
		float 	speed_ratio;
		float 	speed;
		float 	duration;
		float 	smooth;
		
		void init( SkJoint* j_p );
		void start( void );
		
		vector_t	forward_pos;
		vector_t	forward_ref; // default forward direction
		quat_t		forward_rot;
		
		vector_t	local_pos;
		quat_t		local_rot;
		quat_t		prev_local_rot;
		
		vector_t	world_pos;
		quat_t		world_rot;
		quat_t		prev_world_rot; // to measure absolute speed of head
		float t_elapse;
	
		vector_t	parent_loc_pos; // local coord of immediate parent joint
		quat_t		parent_loc_rot;

		vector_t	parent_pos; // world coord of immediate parent joint
		quat_t		parent_rot;

		quat_t	evaluate( float dt, vector_t target_pos, quat_t off_rot, float scale_factor = 1.0 );
		quat_t	evaluate( float dt, quat_t target_rot, quat_t off_rot, float scale_factor = 1.0 );
		
	private:
		void	capture_joint_state( void );
		quat_t	rotation_to_target( vector_t target_pos );
		quat_t	rotation_to_target( quat_t target_rot );
		
//		quat_t	constrain_box( quat_t task_rot );
//		quat_t	constrain_swing( quat_t task_rot );
//		quat_t	constrain_quat( quat_t task_rot );
		quat_t	constrain_ellipse( quat_t task_rot );
		
		quat_t	constrain_quat_speed( float dt, quat_t task_rot );
		quat_t	constrain_smooth( float dt, quat_t task_rot );

		quat_t	constrain( float dt, quat_t task_rot );

		SkJoint* joint_p;
};

///////////////////////////////////////////////////////////////////////////
#endif
