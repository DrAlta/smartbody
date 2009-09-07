/*
 *  bml_animation.cpp - part of SmartBody-lib
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
 *      Andrew n marshall, USC
 */

#include <iostream>
#include <sstream>
#include <string>

#include "mcontrol_util.h"
#include "bml_animation.hpp"



using namespace std;
using namespace BML;
using namespace xml_utils;



BehaviorRequestPtr BML::parse_bml_animation( DOMElement* elem, const std::string& unique_id, SyncPoints& tms, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
	//type = BML_MOTION;

	const XMLCh* animName = elem->getAttribute( ATTR_NAME );
	if( animName && XMLString::stringLen( animName ) ) {
		// Look up motion
		string asciiName( xml_utils::asciiString( animName ) );
		SkMotion* motion = mcu->motion_map.lookup( asciiName.c_str() );

		if( motion ) {
			MeCtMotion* motionCt = new MeCtMotion();
			motionCt->init( motion );

			// Name controller with behavior unique_id
			ostringstream name;
			name << unique_id << ' ' << motion->name();
			motionCt->name( name.str().c_str() );  // TODO: include BML act and behavior ids

			// Copy motion metadata
			{	float duration = motion->duration();
				float ready = motion->time_ready();
				float indt = (ready >= 0)? ready : motionCt->indt();
				float relax = motion->time_relax();
				float outdt = (relax<0)? motionCt->outdt()
									: ( (relax<duration)? duration-relax : 0 );
				motionCt->inoutdt( indt, outdt );
				float stroke_emphasis = motion->time_stroke_emphasis();
				if( stroke_emphasis >= 0 )
					motionCt->emphasist( stroke_emphasis );
			}

			const char* speedStr = xml_utils::asciiString( elem->getAttribute( ATTR_SPEED ) );
			if( speedStr[0] != 0 ) {  // speed attribute is not empty
				motionCt->warp_limits( (float)0.01, 100 );  // override limits
				motionCt->twarp( (float) atof( speedStr ) );
			}
			delete [] speedStr;

			return BehaviorRequestPtr( new MotionRequest( unique_id, motionCt, request->actor->motion_sched_p, tms ) );
		} else {
			// TODO: exception?
			cerr<<"WARNING: BML::parse_bml_animation(): behavior \""<<unique_id<<"\": name=\""<<asciiName<<"\" not loaded; ignoring behavior."<<endl;
			return BehaviorRequestPtr();  // a.k.a., NULL
		}
	} else {
		// TODO: exception?
		cerr<<"WARNING: BML::parse_bml_animation(): behavior \""<<unique_id<<"\": missing name= attribute; ignoring <animation>."<<endl;
		return BehaviorRequestPtr();  // a.k.a., NULL
	}
}
