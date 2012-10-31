#include "vhcl.h"

#include <iostream>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp> 
#include <sb/SBAnimationState.h>

#include "bml_states.hpp"
#include "sbm/mcontrol_util.h"
#include "bml_xml_consts.hpp"
#include "bml_event.hpp"

using namespace std;
using namespace BML;
using namespace xml_utils;

BML::BehaviorRequestPtr BML::parse_bml_states( DOMElement* elem, const std::string& unique_id, BehaviorSyncPoints& behav_syncs, bool required, BmlRequestPtr request, mcuCBHandle *mcu )
{
	// local Id
	std::string localId;
	const XMLCh* attrID = elem->getAttribute(BMLDefs::ATTR_ID);
	xml_utils::xml_translate(&localId, attrID);

	// get character
	std::string characterName = request->actor->getName();
	SbmCharacter* character = mcu->getCharacter(characterName);
	if (character == NULL)
	{
		LOG("parse_bml_states ERR: cannot find character with name %s.", characterName.c_str());
		return BehaviorRequestPtr();
	}

	// get state
	std::string stateName = xml_parse_string(BMLDefs::ATTR_NAME, elem);
	if (stateName == "")
	{
		LOG("parse_bml_states ERR: expecting a state name.");
		return BehaviorRequestPtr();
	}
	PABlend* state = mcu->lookUpPABlend(stateName);
	if (!state)
	{
		LOG("parse_bml_states WARNING: Can't find state name %s, will schedule PseudoIdle state under schedule mode", stateName.c_str());
	}

	// get parameters
	std::string mode = xml_parse_string(BMLDefs::ATTR_MODE, elem);
	if (mode == "")
		mode = "schedule";
	std::string xString = xml_parse_string(BMLDefs::ATTR_X, elem);
	std::string yString = xml_parse_string(BMLDefs::ATTR_Y, elem);
	std::string zString = xml_parse_string(BMLDefs::ATTR_Z, elem);

	// get weights from parameters
	std::vector<double> weights;
	if (state)
	{
		weights.resize(state->getNumMotions());
	}

	if (weights.size() > 0)
		weights[0] = 1.0f;

	double x = 0;
	double y = 0;
	double z = 0;
	if ((xString != "" || yString != "" || zString != "") && state)
	{
		x = atof(xString.c_str());
		y = atof(yString.c_str());
		z = atof(zString.c_str());
		int parameterType = state->getType();
		if (parameterType == 0)
			state->getWeightsFromParameters(x, weights);
		if (parameterType == 1)
			state->getWeightsFromParameters(x, y, weights);
		if (parameterType == 2)
			state->getWeightsFromParameters(x, y, z, weights);
	}

	// wrap mode
	ScheduleType scType;
	std::string wrap = xml_parse_string(BMLDefs::ATTR_WRAPMODE, elem);
	if (wrap == "")
		wrap = "Loop";
	boost::algorithm::to_lower(wrap);
	scType.wrap = PABlendData::Loop;
	if (wrap == "once")
		scType.wrap = PABlendData::Once;

	// schedule mode
	std::string schedule = xml_parse_string(BMLDefs::ATTR_SCHEDULEMODE, elem);
	if (schedule == "")
		schedule = "Queued";
	boost::algorithm::to_lower(schedule);
	scType.schedule = PABlendData::Queued;
	if (schedule == "now")
		scType.schedule = PABlendData::Now;

	// blend mode
	std::string blend = xml_parse_string(BMLDefs::ATTR_BLENDMODE, elem);
	if (blend == "")
		blend = "Overwrite";
	boost::algorithm::to_lower(blend);
	scType.blend =PABlendData::Overwrite;
	if (blend == "additive")
		scType.blend = PABlendData::Additive;

	// partial joint name
	std::string joint = xml_parse_string(BMLDefs::ATTR_PARTIALJOINT, elem);
	if (joint == "")
		joint = "null";

	// take time offset0
	scType.jName = joint;
	scType.timeOffset = xml_parse_double(BMLDefs::ATTR_START, elem);
	scType.stateTimeOffset = xml_parse_double(BMLDefs::ATTR_OFFSET, elem, 0.0);
	scType.stateTimeTrim = xml_parse_double(BMLDefs::ATTR_TRIM, elem, 0.0);
	scType.transitionLen = xml_parse_double(BMLDefs::ATTR_TRANSITION_LENGTH, elem, -1.0);
	scType.playSpeed = xml_parse_double(BMLDefs::ATTR_SPEED, elem, 1.0);
	std::string directPlayStr = xml_parse_string(BMLDefs::ATTR_DIRECTPLAY, elem, "false");
	scType.directPlay = false;
	if (directPlayStr == "true")
		scType.directPlay = true;

	SmartBody::SBAnimationBlend0D* ZeroDState = dynamic_cast<SmartBody::SBAnimationBlend0D*>(state);
	if (!ZeroDState) // don't use state offset unless it is a 0-D state
	{
		scType.stateTimeOffset = 0.0;
		scType.stateTimeTrim = 0.0;
	}
	// schedule a state
	if (mode == "schedule")
	{
		if (state)
			//character->param_animation_ct->schedule(state, x, y, z, wrapMode, scheduleMode, blendMode, joint, timeOffset, stateStartOffset, stateEndTrim, transitionLen, directPlay);
			character->param_animation_ct->schedule(state, weights, scType);
		else
			character->param_animation_ct->schedule(state, weights);
	}

	// update parameter
	if (mode == "update")
	{
		character->param_animation_ct->updateWeights(weights);
	}

	return BehaviorRequestPtr( new EventRequest(unique_id, localId, "", behav_syncs, ""));
}
