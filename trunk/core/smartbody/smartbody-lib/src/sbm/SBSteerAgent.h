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

		void setCurrentSBCharacter(SBCharacter* sbCharacter);
		SBCharacter* getCurrentSBCharacter();
};
}

#endif