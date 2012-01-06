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

BML::BehaviorRequestPtr BML::parse_bml_animation( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu ) {

	if (!request->actor->motion_sched_p)
	{
		LOG("Character %s does not have a motion scheduler, so cannot schedule motion.", request->actor->getName().c_str());
		return BehaviorRequestPtr();
	}

	const XMLCh* animName = elem->getAttribute( BMLDefs::ATTR_NAME );

	const XMLCh* id = elem->getAttribute(BMLDefs::ATTR_ID);
	std::string localId;
	xml_utils::xml_translate(&localId, id);
	
	if( animName != 0 && *animName != 0 )	{

		// Look up motion
		string asciiName( xml_utils::asciiString( animName ) );

		std::map<std::string, SkMotion*>::iterator motionIter = mcu->motion_map.find(asciiName);
		if (motionIter != mcu->motion_map.end())
		{
/*
			double twarp = 1.0;
			const char* speedStr = xml_utils::asciiString( elem->getAttribute( BMLDefs::ATTR_SPEED ) );
			if( speedStr[0] != 0 ) {  // speed attribute is not empty
				twarp = atof( speedStr );
				if( twarp == 0.0 ) twarp = 1.0;
			}
			delete [] speedStr;
*/
			double twarp = xml_utils::xml_parse_double( BMLDefs::ATTR_SPEED, elem, 1.0 );

			SkMotion* motion = (*motionIter).second;
			MeCtMotion* motionCt = new MeCtMotion();

			// Name controller with behavior unique_id
			ostringstream name;
			name << unique_id << ' ' << motion->getName();
			motionCt->setName( name.str().c_str() );  // TODO: include BML act and behavior ids
			motionCt->init( const_cast<SbmCharacter*>(request->actor), motion, 0.0, 1.0 / twarp );

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

	const XMLCh* animName1 = elem->getAttribute( BMLDefs::ATTR_ANIM1 );
	const XMLCh* animName2 = elem->getAttribute( BMLDefs::ATTR_ANIM2 );
	const XMLCh* id = elem->getAttribute( BMLDefs::ATTR_ID );
	std::string localId;
	xml_utils::xml_translate(&localId, id);

	std::string charName;
	int nameStartPos = unique_id.find_first_of("_");
	int nameEndPos = unique_id.find_first_of("_", nameStartPos + 1);
	charName = unique_id.substr(nameStartPos+1, nameEndPos-nameStartPos-1);
	SbmCharacter* character = mcu->getCharacter(charName);

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
			motion1->connect(character->getSkeleton());
			motion1Ct = new MeCtMotion();
			motion1Ct->init( const_cast<SbmCharacter*>(request->actor), motion1 );
			// Name controller with behavior unique_id
			ostringstream name;
			name << unique_id << ' ' << motion1->getName();
			motion1Ct->setName( name.str().c_str() );  // TODO: include BML act and behavior ids
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
			motion2->connect(character->getSkeleton());
			motion2Ct = new MeCtMotion();
			motion2Ct->init( const_cast<SbmCharacter*>(request->actor), motion2 );
			// Name controller with behavior unique_id
			ostringstream name;
			name << unique_id << ' ' << motion2->getName();
			motion2Ct->setName( name.str().c_str() );  // TODO: include BML act and behavior ids
		}
		else 
		{
			LOG("WARNING: BML::parse_bml_panimation(): behavior \"%s\": name=\"%s\" not loaded; ignoring behavior.", unique_id.c_str(), motion2.c_str());
			return BehaviorRequestPtr();  // a.k.a., NULL
		}

		// TODO: Check the validation of input parameters here
		// Initialize the blending weight
		float value = 1.0;
		const char* pvalue = xml_utils::asciiString( elem->getAttribute( BMLDefs::ATTR_PVALUE ) );
		if( pvalue[0] != 0 )   // param value is not empty
			value = (float)atof(pvalue);

		// Initialize the play mode (loop or not)
		std::string isLoopString = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_LOOP));
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
