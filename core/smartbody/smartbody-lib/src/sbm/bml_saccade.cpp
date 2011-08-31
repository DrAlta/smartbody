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

	const char* bin0 = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_PERCENTAGE_BIN0));
	const char* bin45 = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_PERCENTAGE_BIN45));
	const char* bin90 = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_PERCENTAGE_BIN90));
	const char* bin135 = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_PERCENTAGE_BIN135));
	const char* bin180 = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_PERCENTAGE_BIN180));
	const char* bin225 = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_PERCENTAGE_BIN225));
	const char* bin270 = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_PERCENTAGE_BIN270));
	const char* bin315 = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_PERCENTAGE_BIN315));
	if (strcmp(bin0, "") != 0 && strcmp(bin45, "") != 0 && strcmp(bin90, "") != 0 && strcmp(bin135, "") != 0 &&
		strcmp(bin180, "") != 0 && strcmp(bin225, "") != 0 && strcmp(bin270, "") != 0 && strcmp(bin315, "") != 0)
	{
		saccade_ct->setPercentageBins((float)atof(bin0), (float)atof(bin45), (float)atof(bin90), (float)atof(bin135), (float)atof(bin180), (float)atof(bin225), (float)atof(bin270), (float)atof(bin315)); 
	}

	const char* mean = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_SACCADE_INTERVAL_MEAN));
	const char* variant = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_SACCADE_INTERVAL_VARIANT));
	if (strcmp(mean, "") != 0 && strcmp(variant, "") != 0)
	{
		saccade_ct->setGaussianParameter((float)atof(mean), (float)atof(variant));
	}

	const char* limitAngle = xml_utils::asciiString(elem->getAttribute(BMLDefs::ATTR_ANGLELIMIT));
	if (strcmp(limitAngle, "") != 0)
	{
		float angle = (float)atof(limitAngle);
		saccade_ct->setAngleLimit(angle);
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
