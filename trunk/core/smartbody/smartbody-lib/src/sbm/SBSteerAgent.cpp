#include "SBSteerAgent.h"

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

void SBSteerAgent::setCurrentSBCharacter(SBCharacter* sbCharacter)
{
	setCharacter(sbCharacter);
	SteeringAgent* agent = dynamic_cast<SteeringAgent*>(this);
	sbCharacter->steeringAgent = agent;
}

SBCharacter* SBSteerAgent::getCurrentSBCharacter()
{
	return dynamic_cast<SBCharacter*>(getCharacter());
}

}