#ifndef _STEERAGENT_H_
#define _STEERAGENT_H_

#include <sb/SBTypes.h>
#include <sbm/SteeringAgent.h>

class SteeringAgent;

namespace SmartBody {

class SBCharacter;

class SBSteerAgent : public SteeringAgent
{
	public:
		SBAPI SBSteerAgent();
		SBAPI SBSteerAgent(SBCharacter* sbCharacter);
		SBAPI ~SBSteerAgent();

		SBAPI void setSteerStateNamePrefix(std::string prefix);
		SBAPI const std::string& getSteerStateNamePrefix();
		SBAPI void setSteerType(std::string type);
		SBAPI const std::string& getSteerType();

		SBAPI void setCurrentSBCharacter(SBCharacter* sbCharacter);
		SBAPI SBCharacter* getCurrentSBCharacter();

	private:
		std::string _steerType;
		std::string _stateNamePrefix;
};
}

#endif