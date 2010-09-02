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
#include "vhcl_log.h"
#include "bml_animation.hpp"
#include "mcontrol_util.h"
#include "bml_xml_consts.hpp"



using namespace std;
using namespace BML;
using namespace xml_utils;



BML::BehaviorRequestPtr BML::parse_bml_animation( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
	//type = BML_MOTION;

	const XMLCh* animName = elem->getAttribute( ATTR_NAME );

	const XMLCh* id = elem->getAttribute(ATTR_ID);
	std::string localId;
	if (id)
	{
		localId = XMLString::transcode(id);
	}
	
	if( animName != 0 && *animName != 0 )	{
//	if( animName && XMLString::stringLen( animName ) ) {

		// Look up motion
		string asciiName( xml_utils::asciiString( animName ) );

		std::map<std::string, SkMotion*>::iterator motionIter = mcu->motion_map.find(asciiName);
		if (motionIter != mcu->motion_map.end())
		{
			SkMotion* motion = (*motionIter).second;
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
//				float outdt = (relax<0)? motionCt->outdt()
//									: ( (relax<duration)? duration-relax : 0 );
				// is outdt an absolute or a relative time? shapiro 5/28/10
				float outdt = (relax<0)? motionCt->outdt()
									: ( (relax<duration)? duration - relax : 0 );
				motionCt->inoutdt( indt, outdt );
				float stroke_emphasis = motion->time_stroke_emphasis();
				if( stroke_emphasis >= 0 )
					motionCt->emphasist( stroke_emphasis );
			}

			const char* speedStr = xml_utils::asciiString( elem->getAttribute( BML::ATTR_SPEED ) );
			if( speedStr[0] != 0 ) {  // speed attribute is not empty
				motionCt->warp_limits( (float)0.01, 100 );  // override limits
				motionCt->twarp( (float) atof( speedStr ) );
			}
			delete [] speedStr;

			BehaviorRequestPtr behavPtr(new MotionRequest( unique_id, localId, motionCt, request->actor->motion_sched_p, behav_syncs ) );
			return behavPtr; 
		} else {
			// TODO: exception?
			//cerr<<"WARNING: BML::parse_bml_animation(): behavior \""<<unique_id<<"\": name=\""<<asciiName<<"\" not loaded; ignoring behavior."<<endl;
			LOG("WARNING: BML::parse_bml_animation(): behavior \"%s\": name=\"%s\" not loaded; ignoring behavior.", unique_id.c_str(), asciiName.c_str());
			
			return BehaviorRequestPtr();  // a.k.a., NULL
		}
	} else {
		// TODO: exception?
		std::wstringstream wstrstr;
		cerr<<"WARNING: BML::parse_bml_animation(): behavior \""<<unique_id<<"\": missing name= attribute; ignoring <animation>.";
		LOG(convertWStringToString(wstrstr.str()).c_str());

		return BehaviorRequestPtr();  // a.k.a., NULL
	}
}
