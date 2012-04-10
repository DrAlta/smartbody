#include "SBSteerAgent.h"
#include <sbm/mcontrol_util.h>
#include <sbm/SBCharacter.h>

namespace SmartBody {

SBSteerAgent::SBSteerAgent() : SteeringAgent(NULL)
{
}

SBSteerAgent::SBSteerAgent(SBCharacter* sbCharacter) : SteeringAgent(sbCharacter)
{
}

SBSteerAgent::~SBSteerAgent()
{
}

void SBSteerAgent::setSteerStateNamePrefix(std::string prefix)
{
	_stateNamePrefix = prefix;
	SteeringAgent* agent = dynamic_cast<SteeringAgent*>(this);
	SbmCharacter* character = agent->getCharacter();
	if (character)
		character->statePrefix = _stateNamePrefix;
}

const std::string& SBSteerAgent::getSteerStateNamePrefix()
{
	return _stateNamePrefix;
}

void SBSteerAgent::setSteerType(std::string type)
{
	_steerType = type;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SteeringAgent* agent = dynamic_cast<SteeringAgent*>(this);
	SbmCharacter* character = agent->getCharacter();
	if (!character)
		return;

	if (_steerType == "example")
	{
		if (character->checkExamples())
			character->locomotion_type = character->Example;
		else
			character->locomotion_type = character->Basic;
	}
	if (type == "procedural")
	{
		character->locomotion_type = character->Procedural;
		if (character->steeringAgent)
			character->steeringAgent->desiredSpeed = 1.6f;
	}
	if (type == "basic")
	{
		character->locomotion_type = character->Basic;
	}
}

const std::string& SBSteerAgent::getSteerType()
{
	return _steerType;
}

void SBSteerAgent::setCurrentSBCharacter(SBCharacter* sbCharacter)
{
	setCharacter(sbCharacter);
	SteeringAgent* agent = dynamic_cast<SteeringAgent*>(this);
	sbCharacter->steeringAgent = agent;

	setSteerStateNamePrefix(_stateNamePrefix);
	setSteerType(_steerType);
	// reset steering parameters based on current new character
	setSteerParamsDirty(true);
	initSteerParams();
}

SBCharacter* SBSteerAgent::getCurrentSBCharacter()
{
	return dynamic_cast<SBCharacter*>(getCharacter());
}

}