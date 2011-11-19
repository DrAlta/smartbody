#ifndef _STEERAGENT_H_
#define _STEERAGENT_H_

#include <sbm/SteeringAgent.h>
#include <sbm/SBCharacter.h>

namespace SmartBody {

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