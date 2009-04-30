/*
 *  me_ct_eyelid.h - part of SmartBody-lib
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

#ifndef ME_CT_EYELID_H
#define ME_CT_EYELID_H

#include <SK/sk_skeleton.h>
#include <ME/me_controller.h>

//////////////////////////////////////////////////////////////////////////////////

class MeCtEyeLid : public MeController	{ 

	private:

//		SkMotion*            _neutral_pose_p;
//		SkMotion*            _blink_pose_p;
//		SkMotion*            _lookup_pose_p;
//		SkMotion*            _lookdown_pose_p;
//		SkMotion*            _flat_pose_p;

		SkChannelArray		_channels; // override motion channels, to include world_offset
		SkSkeleton* 	    _skeleton_ref_p;
		
//		float neut_lid_deg;
//		float blink_lid_deg;
//		float raise_lid_deg;

//		SrBuffer<int>        LL_lid_indices; // lid channels in raw motion
//		int        UL_lid_quat_index;

	public:
		static const char* type_name;

		/*! Constructor */
		MeCtEyeLid( void );

		/*! Destructor is public but pay attention to the use of ref()/unref() */
		virtual ~MeCtEyeLid( void );
		
		void clear( void );

//		void init( SkMotion* neutral_p, SkMotion* blink_p, SkMotion* lkup_p, SkMotion* lkdn_p, SkMotion* flat_p );
//		void init( SkMotion* neutral_p, SkMotion* blink_p, SkMotion* raise_p );
//		void init( SkMotion* neutral_p, SkMotion* blink_p );
		void init( void );
		
	private:

		SkJoint*		get_joint( char *joint_str, SkJoint *joint_p );
		SkJoint*		source_ref_joint( void );

		void	print_motion_channel( SkMotion* mot_p, const char *chan_name );
		float	get_motion_joint_pitch( SkMotion* mot_p, const char *chan_name );
//		float	calculate_upper_correction( float in_lid, float in_eye );
			
		// callbacks for the base class
		virtual void context_updated( void );
		virtual void controller_map_updated();
		virtual void controller_start();
		virtual bool controller_evaluate ( double t, MeFrameData& frame );
		virtual SkChannelArray& controller_channels ();
		virtual double controller_duration ();
		virtual const char* controller_type ();
		virtual void print_state( int tabCount );
};

//////////////////////////////////////////////////////////////////////////////////
#endif // ME_CT_EYELID_H
