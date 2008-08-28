/*
 *  me_ct_quick_draw.h - part of SmartBody-lib
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

# ifndef ME_CT_QUICK_DRAW_H
# define ME_CT_QUICK_DRAW_H

//////////////////////////////////////////////////////////////////////////////////

#include <SR/sr_hash_table.h>
#include <SR/sr_buffer.h>
#include <SK/sk_motion.h>
#include <SK/sk_skeleton.h>
#include <ME/me_controller.h>

#include "gwiz_math.h"

// ASSUME: right handed quick-draw motion only
// DETERMINE: if handedness can be extracted from raw motion

/*
TODO: SBM commands:

        ctrl <> quickdraw <qdraw-motion-name>
        quickdraw <> <dur-sec> local|world <pitch-deg> <heading-deg> <roll-deg>
#        quickdraw <> L|R <sec|dps> local|world <pitch-deg> <heading-deg> <roll-deg>

 * The input drawing motion can be any direction, any reasonable magnitude.
 * Tokens 'dur' and 'time' are synonymous.

TODO: Class API members:

        void init( SkMotion* mot_p );
        void set_time( float sec );
        void set_aim_local( float h );
        void set_aim_world( float h );

Step motion:

        sf-smartbody/data/sbm-testdata/doctor/qdraw/AdultM_FastDraw001.skm

TODO: Example sequence:

        0.0 load motions ../../../../data/sbm-test/common-sk/qdraw
        0.0 ctrl Q quickdraw AdultM_FastDraw001
        0.0 quickdraw Q 0.9 point 0.0 450.0 1000.0
        0.0 char doctor ctrl Q begin

 * This will induce the doctor to shoot 45 degrees to his left.

TODO: Testing sequence:

        seq sbm-qdraw-init
        seq sbm-qdraw

 * This runs the doctor through a few hoops.
*/

class MeCtQuickDraw : public MeController	{ 

	private:
		enum aim_coord_enum_set	{
			AIM_LOCAL,
			AIM_WORLD
		};

		struct joint_state_t	{
			vector_t	parent_pos; // world coord of immediate parent joint
			quat_t		parent_rot;
			vector_t	local_pos;
			quat_t		local_rot;
			vector_t	world_pos;
			quat_t		world_rot;
		};

		SkMotion*            _motion; 
		SkMotion::InterpType _play_mode; // its play mode
		double               _duration;  // the time-warped duration
		int                  _last_apply_frame; // to optimize shared motion evaluation
		SrBuffer<int>        _motion_chan_to_buff; // motion's channels to context's buffer index
		SrBuffer<int>        _arm_chan_indices; // arm channels in raw motion

		SkSkeleton* 	skeleton_ref_p;
		vector_t		point_target_pos;
		char*			target_ref_joint_str;
		SkJoint*		target_ref_joint_p;

		float raw_time;
		float play_time;
		float play_time_scale;

		vector_t world_offset_pos; // joint state at controller-start
		quat_t   world_offset_rot;

		float * interim_pose_buff_p;
		
	public:
		static const char* type_name;

		/*! Constructor */
		MeCtQuickDraw ();

		/*! Destructor is public but pay attention to the use of ref()/unref() */
		virtual ~MeCtQuickDraw ();

		/*! Set the motion to be used. A valid motion must be set using
    		this method before calling any other method.
    		The old motion is unreferenced, and the new one is referenced.
    		(SkMotion derives SrSharedClass and has ref/unref methods)
    		The keytimes of m are translated to ensure start from zero. 
    		MeController::init() is automatically called. */
		void init( SkMotion* mot_p );

		// Target API
		void set_target_point( float x, float y, float z );
		void set_target_coord_joint( SkJoint* joint_p );

		void set_target_joint( float x, float y, float z, SkJoint* ref_joint_p = NULL );

		void set_time( float sec );

		/*! Set the play mode, default is linear */
		void play_mode ( SkMotion::InterpType it ) { _play_mode=it; }

		/*! Returns the current play mode */
		SkMotion::InterpType play_mode () const { return _play_mode; }

	private:
		joint_state_t capture_joint_state( SkJoint *joint_p );
		quat_t rotation_to_target( vector_t l_forward_dir, vector_t w_target_pos, joint_state_t* state_p );

		SkJoint*	find_joint( char *joint_str, SkJoint **joint_pp );
		SkJoint*	target_ref_joint( void );
		vector_t	world_target_point( void );

		// callbacks for the base class
		virtual void context_updated( void );
		virtual void controller_start();
		virtual void controller_map_updated();
		virtual bool controller_evaluate ( double t, MeFrameData& frame );
		virtual SkChannelArray& controller_channels ();
		virtual double controller_duration ();
		virtual const char* controller_type ();
		virtual void print_state( int tabCount );
};

//////////////////////////////////////////////////////////////////////////////////
# endif // ME_CT_QUICK_DRAW_H

