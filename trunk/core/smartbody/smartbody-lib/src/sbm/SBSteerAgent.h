#ifndef _STEERAGENT_H_
#define _STEERAGENT_H_

#include <sbm/SteeringAgent.h>

class SteeringAgent;

namespace SmartBody {

class SBCharacter;

class SBSteerAgent : public SteeringAgent
{
	public:
		SBSteerAgent();
		SBSteerAgent(SBCharacter* sbCharacter);
		~SBSteerAgent();

		void setSteerStateNamePrefix(std::string prefix);
		void setSteerType(std::string type);

		void setCurrentSBCharacter(SBCharacter* sbCharacter);
		SBCharacter* getCurrentSBCharacter();

	private:
		std::string _steerType;
		std::string _stateNamePrefix;
};
}

#endif