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
 */

#ifndef ME_CT_EYELID_H
#define ME_CT_EYELID_H

#include <SK/sk_skeleton.h>
#include <ME/me_controller.h>

/*
	char <> softeyes [on|off]
	char <> softeyes weight <upper> <lower>
	char <> softeyes eyepitch|upperlid|lowerlid <upper> <lower>
*/

//////////////////////////////////////////////////////////////////////////////////

class MeCtEyeLid : public MeController	{ 

	private:

		SkChannelArray		_channels; // override motion channels, to include world_offset
		SkSkeleton* 	    _skeleton_ref_p;
		
		float				_eyelidWeight[ 2 ];
		float				_eyelidUpperTransRange[ 2 ];
		float				_eyelidLowerTransRange[ 2 ];
		float				_eyeballPitchRange[ 2 ];
		
	public:
		static const char* type_name;

		/*! Constructor */
		MeCtEyeLid( void );

		/*! Destructor is public but pay attention to the use of ref()/unref() */
		virtual ~MeCtEyeLid( void );
		void clear( void );
		void init( void );
		
		void setEyelidWeight( float up, float dn )	{
			_eyelidWeight[ 0 ] = up;
			_eyelidWeight[ 1 ] = dn;
		}
		void getEyelidWeight( float &up, float &dn )	{
			up = _eyelidWeight[ 0 ];
			dn = _eyelidWeight[ 1 ];
		}

		void setEyelidUpperTransRange( float up, float dn ) {
			_eyelidUpperTransRange[ 0 ] = up;
			_eyelidUpperTransRange[ 1 ] = dn;
		}
		void setEyelidLowerTransRange( float up, float dn ) {
			_eyelidLowerTransRange[ 0 ] = up;
			_eyelidLowerTransRange[ 1 ] = dn;
		}
		void getEyelidUpperTransRange( float &up, float &dn ) {
			up = _eyelidUpperTransRange[ 0 ];
			dn = _eyelidUpperTransRange[ 1 ];
		}
		void getEyelidLowerTransRange( float &up, float &dn ) {
			up = _eyelidLowerTransRange[ 0 ];
			dn = _eyelidLowerTransRange[ 1 ];
		}

		void setEyeballPitchRange( float up, float dn ) {
			_eyeballPitchRange[ 0 ] = up;
			_eyeballPitchRange[ 1 ] = dn;
		}
		void getEyeballPitchRange( float &up, float &dn ) {
			up = _eyeballPitchRange[ 0 ];
			dn = _eyeballPitchRange[ 1 ];
		}

	private:
		SkJoint*		source_ref_joint( void );

		float	calc_lid_correction( 
			float in_eye_p, 
			float eye_range[ 2 ], 
			float in_lid_y,
			float lid_range[ 2 ]
		);
			
		virtual void context_updated( void );
		virtual void controller_map_updated();
		virtual void controller_start();
		virtual bool controller_evaluate ( double t, MeFrameData& frame );
		virtual SkChannelArray& controller_channels ();
		virtual double controller_duration ();
		virtual const char* controller_type () const;
		virtual void print_state( int tabCount );
};

//////////////////////////////////////////////////////////////////////////////////
#endif // ME_CT_EYELID_H
