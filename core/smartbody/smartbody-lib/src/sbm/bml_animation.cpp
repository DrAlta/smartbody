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

#include "vhcl.h"

#include <iostream>
#include <sstream>
#include <string>

#include "bml_animation.hpp"
#include "mcontrol_util.h"
#include "bml_xml_consts.hpp"



using namespace std;
using namespace BML;
using namespace xml_utils;

const XMLCh ATTR_ANIM1[] = L"anim1";
const XMLCh ATTR_ANIM2[] = L"anim2";
const XMLCh ATTR_LOOP[] = L"loop";

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
			double twarp = 1.0;
			const char* speedStr = xml_utils::asciiString( elem->getAttribute( BML::ATTR_SPEED ) );
			if( speedStr[0] != 0 ) {  // speed attribute is not empty
//				motionCt->warp_limits( (float)0.01, 100 );  // override limits
//				motionCt->twarp( (float) atof( speedStr ) );
				twarp = atof( speedStr );
				if( twarp == 0.0 ) twarp = 1.0;
			}
			delete [] speedStr;

			SkMotion* motion = (*motionIter).second;
			MeCtMotion* motionCt = new MeCtMotion();

			// Name controller with behavior unique_id
			ostringstream name;
			name << unique_id << ' ' << motion->name();
			motionCt->name( name.str().c_str() );  // TODO: include BML act and behavior ids

			motionCt->init( motion, 0.0, 1.0 / twarp );
#if 0

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
#endif

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


BML::BehaviorRequestPtr BML::parse_bml_panimation( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {
	//type = BML_MOTION;

	const XMLCh* animName1 = elem->getAttribute( ATTR_ANIM1 );
	const XMLCh* animName2 = elem->getAttribute( ATTR_ANIM2 );
	const XMLCh* id = elem->getAttribute( ATTR_ID );
	std::string localId;
	if (id)
		localId = XMLString::transcode(id);
	

	std::string charName;
	int nameStartPos = unique_id.find_first_of("_");
	int nameEndPos = unique_id.find_first_of("_", nameStartPos + 1);
	charName = unique_id.substr(nameStartPos+1, nameEndPos-nameStartPos-1);
	SbmCharacter* character = mcu->character_map.lookup(charName);

	if( animName1 != 0 && *animName1 != 0 && animName2 != 0 && *animName2 != 0 )	
	{
		string motion1( xml_utils::asciiString( animName1 ) );
		string motion2( xml_utils::asciiString( animName2 ) );
		MeCtMotion* motion1Ct = NULL;
		MeCtMotion* motion2Ct = NULL;
		std::map<std::string, SkMotion*>::iterator motion1Iter = mcu->motion_map.find(motion1);

		// -- Initialize two motion controllers
		if (motion1Iter != mcu->motion_map.end())
		{
			SkMotion* motion1 = (*motion1Iter).second;
			motion1->connect(character->skeleton_p);
			motion1Ct = new MeCtMotion();
			motion1Ct->init( motion1 );
			// Name controller with behavior unique_id
			ostringstream name;
			name << unique_id << ' ' << motion1->name();
			motion1Ct->name( name.str().c_str() );  // TODO: include BML act and behavior ids

#if 0
			float duration = motion1->duration();
			float ready = motion1->time_ready();
			float indt = (ready >= 0)? ready : motion1Ct->indt();
			float relax = motion1->time_relax();
//				float outdt = (relax<0)? motionCt->outdt()
//									: ( (relax<duration)? duration-relax : 0 );
			// is outdt an absolute or a relative time? shapiro 5/28/10
			float outdt = (relax<0)? motion1Ct->outdt()
								: ( (relax<duration)? duration - relax : 0 );
			motion1Ct->inoutdt( indt, outdt );
			float stroke_emphasis = motion1->time_stroke_emphasis();
			if( stroke_emphasis >= 0 )
				motion1Ct->emphasist( stroke_emphasis );
#endif
		} 
		else 
		{
			LOG("WARNING: BML::parse_bml_panimation(): behavior \"%s\": name=\"%s\" not loaded; ignoring behavior.", unique_id.c_str(), motion1.c_str());
			return BehaviorRequestPtr();  // a.k.a., NULL
		}
		std::map<std::string, SkMotion*>::iterator motion2Iter = mcu->motion_map.find(motion2);
		if (motion2Iter != mcu->motion_map.end())
		{
			SkMotion* motion2 = (*motion2Iter).second;
			motion2->connect(character->skeleton_p);
			motion2Ct = new MeCtMotion();
			motion2Ct->init( motion2 );
			// Name controller with behavior unique_id
			ostringstream name;
			name << unique_id << ' ' << motion2->name();
			motion2Ct->name( name.str().c_str() );  // TODO: include BML act and behavior ids

#if 0
			float duration = motion2->duration();
			float ready = motion2->time_ready();
			float indt = (ready >= 0)? ready : motion2Ct->indt();
			float relax = motion2->time_relax();
//				float outdt = (relax<0)? motionCt->outdt()
//									: ( (relax<duration)? duration-relax : 0 );
			// is outdt an absolute or a relative time? shapiro 5/28/10
			float outdt = (relax<0)? motion2Ct->outdt()
								: ( (relax<duration)? duration - relax : 0 );
			motion2Ct->inoutdt( indt, outdt );
			float stroke_emphasis = motion2->time_stroke_emphasis();
			if( stroke_emphasis >= 0 )
				motion2Ct->emphasist( stroke_emphasis );
#endif
		}
		else 
		{
			LOG("WARNING: BML::parse_bml_panimation(): behavior \"%s\": name=\"%s\" not loaded; ignoring behavior.", unique_id.c_str(), motion2.c_str());
			return BehaviorRequestPtr();  // a.k.a., NULL
		}

		// TODO: Check the validation of input parameters here
		// Initialize the blending weight
		float value = 1.0;
		const char* pvalue = xml_utils::asciiString( elem->getAttribute( BML::ATTR_PVALUE ) );
		if( pvalue[0] != 0 )   // param value is not empty
			value = (float)atof(pvalue);

		// Initialize the play mode (loop or not)
		std::string isLoopString = xml_utils::asciiString(elem->getAttribute(ATTR_LOOP));
		bool isLoop;
		if (isLoopString == "true") isLoop = true;
		else						isLoop = false;

		BehaviorRequestPtr behavPtr(new ParameterizedMotionRequest( unique_id, localId, motion1Ct, motion2Ct, request->actor->motion_sched_p, behav_syncs, value, isLoop) );
		return behavPtr;
	} 
	else 
	{
		std::wstringstream wstrstr;
		cerr<<"WARNING: BML::parse_bml_animation(): behavior \""<<unique_id<<"\": missing name= attribute; ignoring <animation>.";
		LOG(convertWStringToString(wstrstr.str()).c_str());
		return BehaviorRequestPtr();  // a.k.a., NULL
	}
}
