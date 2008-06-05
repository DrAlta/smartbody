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

/*
TODO: SBM commands:

        ctrl <> quickdraw <qdraw-motion-name>
        quickdraw <> L|R dur|time|speed <sec|dps> local|world <pitch-deg> <heading-deg> <roll-deg>

 * The input drawing motion can be any direction, any reasonable magnitude.
 * Tokens 'dur' and 'time' are synonymous.

TODO: Class API members:

        void init( SkMotion* mot_p );
        void set_time( float sec );
        void set_speed( float dps );
        void set_heading_local( float h );
        void set_heading_world( float h );

Step motion:

        sf-smartbody/data/sbm-testdata/doctor/qdraw/AdultM_FastDraw001.skm

TODO: Example sequence:

        0.0 load motions ../../testdata/doctor/qdraw
        0.0 ctrl QD quickdraw AdultM_FastDraw001
        0.0 quickdraw QD dur 0.9 local 0.0 45.0
        0.0 char doctor ctrl QD begin

 * This will induce the doctor to shoot 45 degrees to his left.

TODO: Testing sequence:

        seq sbm-quickdraw-init

 * This runs the doctor through a few hoops.
*/

class MeCtQuickDraw : public MeController	{ 

	private:
		enum timing_mode_enum_set	{
			TASK_SPEED,
			TASK_TIME
		};
		enum coord_coord_enum_set	{
			HEADING_LOCAL,
			HEADING_WORLD
		};

		struct joint_param_index_t {
			int x, y, z, q;
		};

		SkMotion*            _left_motion; 
		SkMotion*            _right_motion; 
		SkMotion*            _motion; 
		SkMotion::InterpType _play_mode; // its play mode
		double               _duration;  // the time-warped duration
		int                  _last_apply_frame; // to optimize shared motion evaluation
		SrBuffer<int>        _mChan_to_buff; // motion's channels to context's buffer index
		SkChannelArray		_channels; // override motion channels, to include world_offset

		SkSkeleton* 	skeleton_ref_p;
		float * interim_pose_buff_p;
		
		vector_t world_offset_pos; // joint state at controller-start
		quat_t   world_offset_rot;
		
		joint_param_index_t world_offset_chan;
		joint_param_index_t world_offset_idx;
		joint_param_index_t base_joint_chan;
		joint_param_index_t base_joint_idx;
		
		int timing_mode;
		int heading_mode;
		int dirty_action_bit;
		
		float raw_angle;
		float world_turn_angle;
		float turn_angle;
		float turn_angle_scale;

		float raw_time;
		float turn_speed;
		float turn_time;
		float turn_time_scale;
		
		SkMotion* build_mirror_motion( SkMotion* ref_motion_p );
		void capture_world_offset_state( void );
		float calc_raw_turn_angle( SkMotion* mot_p, char *joint_name );
		void update_action_params( void );

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

		void set_time( float sec );
		void set_speed( float dps );
		
		void set_heading_local( float h );
		void set_heading_world( float h );

		/*! Returns a pointer to the current motion of this controller */
		SkMotion* motion () { return _motion; }

		/*! Set the play mode, default is linear */
		void play_mode ( SkMotion::InterpType it ) { _play_mode=it; }

		/*! Returns the current play mode */
		SkMotion::InterpType play_mode () const { return _play_mode; }

		virtual double controller_duration ();

	private:

		// callbacks for the base class
		virtual void context_updated( void );
		virtual void controller_start();
		virtual void controller_map_updated();
		virtual bool controller_evaluate ( double t, MeFrameData& frame );
		virtual SkChannelArray& controller_channels ();
		virtual const char* controller_type ();
		virtual void print_state( int tabCount );
};

//////////////////////////////////////////////////////////////////////////////////
# endif // ME_CT_QUICK_DRAW_H

