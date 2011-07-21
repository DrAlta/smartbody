/*
 *  bml_saccade.cpp - part of Motion Engine and SmartBody-lib
 *  Copyright (C) 2011  University of Southern California
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
 *      Yuyu Xu, USC
 */

#include "vhcl.h"
#include <iostream>
#include <sstream>
#include <string>

#include <xercesc/util/XMLStringTokenizer.hpp>

#include "bml_saccade.hpp"

#include "mcontrol_util.h"
#include "me_ct_saccade.h"

#include "bml_xml_consts.hpp"
#include "bml_event.hpp"
#include "xercesc_utils.hpp"
#include "BMLDefs.h"

using namespace std;
using namespace BML;
using namespace xml_utils;
BehaviorRequestPtr BML::parse_bml_saccade( DOMElement* elem, const std::string& unique_id, BML::BehaviorSyncPoints& behav_syncs, bool required, BML::BmlRequestPtr request, mcuCBHandle *mcu )
{
	MeCtSaccade* saccade_ct = request->actor->saccade_ct;
	saccade_ct->setValid(true);
	saccade_ct->setUseModel(true);	
	const XMLCh* id = elem->getAttribute(BMLDefs::ATTR_ID);
	std::string localId;
	if (id)
		localId = XMLString::transcode(id);
	
	float duration = 0.03f;
	float magnitude = 3.0f;
	float direction = 45.0f;
	const char* dur = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_DURATION));
	if (strcmp(dur, "") != 0)
	{
		saccade_ct->setUseModel(false);
		duration = (float)atof(dur);
	}
	const char* mag = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_MAGNITUDE));
	if (strcmp(mag, "") != 0)
	{
		saccade_ct->setUseModel(false);
		magnitude = (float)atof(mag);
	}
	const char* dir = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_DIRECTION));
	if (strcmp(dir, "") != 0)
	{
		saccade_ct->setUseModel(false);
		direction = (float)atof(dir);
	}
	if (!saccade_ct->getUseModel())
	{
		saccade_ct->spawnOnce(direction, magnitude, duration);
		return BehaviorRequestPtr( new EventRequest(unique_id, localId, "", behav_syncs, ""));
	}

	//-----
	const char* bMode = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_MODE));
	if (strcmp(bMode, "") != 0)
	{
		std::string bModeString = bMode;
		if (bModeString == "talk")
			saccade_ct->setBehaviorMode(MeCtSaccade::Talking);
		else if (bModeString == "listen")
			saccade_ct->setBehaviorMode(MeCtSaccade::Listening);
		else if (bModeString == "think")
			saccade_ct->setBehaviorMode(MeCtSaccade::Thinking);
		else
		{
			LOG("BML::parse_bml_saccade ERR: this mode not recognized");
			return BehaviorRequestPtr(); 
		}
	}

	const char* limitAngle = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_ANGLELIMIT));
	if (strcmp(limitAngle, "") != 0)
	{
		float angle = (float)atof(limitAngle);
		MeCtSaccade::BehaviorMode mode = saccade_ct->getBehaviorMode();
		if (mode == MeCtSaccade::Talking)
			saccade_ct->setTalkingAngleLimit(angle);
		if (mode == MeCtSaccade::Listening)
			saccade_ct->setListeningAngleLimit(angle);
		if (mode == MeCtSaccade::Thinking)
			saccade_ct->setThinkingAngleLimit(angle);
	}

	const char* finishFlag = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_FINISH));
	if (strcmp(finishFlag, "") != 0)
	{
		std::string finish = finishFlag;
		if (finish == "true")
			saccade_ct->setValid(false);
		else
		{
			LOG("BML::parse_bml_saccade ERR: this tag not recognized");
			return BehaviorRequestPtr();
		}
	}

	return BehaviorRequestPtr( new EventRequest(unique_id, localId, "", behav_syncs, ""));
}
