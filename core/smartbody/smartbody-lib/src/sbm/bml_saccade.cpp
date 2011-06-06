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
using namespace std;
using namespace BML;
using namespace xml_utils;
BehaviorRequestPtr BML::parse_bml_saccade( DOMElement* elem, const std::string& unique_id, BML::BehaviorSyncPoints& behav_syncs, bool required, BML::BmlRequestPtr request, mcuCBHandle *mcu )
{
	MeCtSaccade* saccade_ct = request->actor->saccade_ct;
	saccade_ct->setValid(true);
	saccade_ct->setUseModel(true);	
	const XMLCh* id = elem->getAttribute(ATTR_ID);
	std::string localId;
	if (id)
		localId = XMLString::transcode(id);
	
	float duration = 0.03f;
	float magnitude = 3.0f;
	float direction = 45.0f;
	const XMLCh* dur = elem->getAttribute(L"duration");
	if (XMLString::compareIString(dur, L"") != 0)
	{
		saccade_ct->setUseModel(false);
		duration = (float)atof(XMLString::transcode(dur));
	}
	const XMLCh* mag = elem->getAttribute(L"magnitude");
	if (XMLString::compareIString(mag, L"") != 0)
	{
		saccade_ct->setUseModel(false);
		magnitude = (float)atof(XMLString::transcode(mag));
	}
	const XMLCh* dir = elem->getAttribute(L"direction");
	if (XMLString::compareIString(dir, L"") != 0)
	{
		saccade_ct->setUseModel(false);
		direction = (float)atof(XMLString::transcode(dir));
	}
	if (!saccade_ct->getUseModel())
	{
		saccade_ct->spawnOnce(direction, magnitude, duration);
		return BehaviorRequestPtr( new EventRequest(unique_id, localId, "", behav_syncs, ""));
	}

	//-----
	const XMLCh* bMode = elem->getAttribute(L"mode");
	if (XMLString::compareIString(bMode, L"") != 0)
	{
		std::string bModeString = XMLString::transcode(bMode);
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

	const XMLCh* limitAngle = elem->getAttribute(L"angle-limit");
	if (XMLString::compareIString(limitAngle, L"") != 0)
	{
		float angle = (float)atof(XMLString::transcode(limitAngle));
		MeCtSaccade::BehaviorMode mode = saccade_ct->getBehaviorMode();
		if (mode == MeCtSaccade::Talking)
			saccade_ct->setTalkingAngleLimit(angle);
		if (mode == MeCtSaccade::Listening)
			saccade_ct->setListeningAngleLimit(angle);
		if (mode == MeCtSaccade::Thinking)
			saccade_ct->setThinkingAngleLimit(angle);
	}

	const XMLCh* finishFlag = elem->getAttribute(L"finish");
	if (XMLString::compareIString(finishFlag, L"") != 0)
	{
		std::string finish = XMLString::transcode(finishFlag);
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
