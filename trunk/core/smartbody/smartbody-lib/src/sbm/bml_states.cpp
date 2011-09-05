#include "vhcl.h"

#include <iostream>
#include <sstream>
#include <string>

#include "bml_states.hpp"
#include "mcontrol_util.h"
#include "bml_xml_consts.hpp"
#include "bml_event.hpp"

using namespace std;
using namespace BML;
using namespace xml_utils;

BML::BehaviorRequestPtr BML::parse_bml_states( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu )
{
	std::string localId;
	const XMLCh* attrID = elem->getAttribute(BMLDefs::ATTR_ID);
	xml_utils::xml_translate(&localId, attrID);
	std::stringstream command;	
	std::string characterName = request->actor->getName();

	std::string stateName = xml_parse_string(BMLDefs::ATTR_NAME, elem);
	if (stateName == "")
		return BehaviorRequestPtr();

	std::string mode = xml_parse_string(BMLDefs::ATTR_MODE, elem);
	if (mode == "")
		mode = "schedule";
	std::string xString = xml_parse_string(BMLDefs::ATTR_X, elem);
	std::string yString = xml_parse_string(BMLDefs::ATTR_Y, elem);
	std::string zString = xml_parse_string(BMLDefs::ATTR_Z, elem);
	PAStateData* state = mcu->lookUpPAState(stateName);
	if (xString != "" || yString != "" || zString != "")
	{
		if (state)
		{
			double x = atof(xString.c_str());
			double y = atof(yString.c_str());
			double z = atof(zString.c_str());
			int parameterType = state->paramManager->getType();
			if (parameterType == 0)
				state->paramManager->setWeight(x);
			if (parameterType == 1)
				state->paramManager->setWeight(x, y);
			if (parameterType == 2)
				state->paramManager->setWeight(x, y, z);
		}
	}

	// schedule parameter
	std::string loopMode = xml_parse_string(BMLDefs::ATTR_LOOP, elem);
	if (loopMode == "")
		loopMode = "false";
	std::string startingNow = xml_parse_string(BMLDefs::ATTR_STARTINGNOW, elem);
	if (startingNow == "")
		startingNow = "false";
	if (mode == "schedule")
	{
		std::stringstream command1;
		command1 << "panim schedule char " << characterName;
		command1 << " state " << stateName << " loop " << loopMode << " playnow " << startingNow;
		if (state)
			for (int i = 0; i < state->getNumMotions(); i++)
				command1 << " " << state->weights[i];
		mcu->execute((char*) command1.str().c_str());	
	}

	// update parameter
	if (mode == "update")
	{
		std::stringstream command1;
		command1 << "panim update char " << characterName;
		if (state)
			for (int i = 0; i < state->getNumMotions(); i++)
				command1 << " " << state->weights[i];
		mcu->execute((char*) command1.str().c_str());	
	}

	return BehaviorRequestPtr( new EventRequest(unique_id, localId, command.str().c_str(), behav_syncs, ""));
}
