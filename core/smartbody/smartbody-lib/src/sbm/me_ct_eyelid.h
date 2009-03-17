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

		SkChannelArray		_channels; // override motion channels, to include world_offset
		SkSkeleton* 	    _skeleton_ref_p;
		
//		SrBuffer<int>        LL_lid_indices; // lid channels in raw motion
//		int        UL_lid_quat_index;

/*
		struct joint_state_t	{
//			vector_t	parent_pos; // world coord of immediate parent joint
//			quat_t		parent_rot;
			vector_t	local_pos;
			quat_t		local_rot;
			vector_t	world_pos;
			quat_t		world_rot;
		};
*/

	public:
		static const char* type_name;

		/*! Constructor */
		MeCtEyeLid( void );

		/*! Destructor is public but pay attention to the use of ref()/unref() */
		virtual ~MeCtEyeLid( void );
		
		void clear( void );

		void init( void );
		
	private:

		SkJoint*		get_joint( char *joint_str, SkJoint *joint_p );
		SkJoint*		source_ref_joint( void );
//		joint_state_t	capture_joint_state( SkJoint *joint_p );
//		joint_state_t	calc_channel_state( MeCtEyeLid::joint_state_t source );

		char*		source_ref_joint_str;
		SkJoint*	source_ref_joint_p;

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
