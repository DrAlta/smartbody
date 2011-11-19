#ifndef _SBREACH_H_
#define _SBREACH_H_

#include <sbm/SBCharacter.h>

namespace SmartBody {

class SBReach
{
	public:
		SBReach();
		SBReach(SBCharacter* character);
		~SBReach();

		SBCharacter* getCharacter();
		SBReach* copy();

		void addMotion(std::string type, SBMotion* motion);
		void removeMotion(std::string type, SBMotion* motion);
		int getNumMotions();

		std::vector<std::string> getMotionNames(std::string type);
		void build(SBCharacter* character);
		
		void setGrabHandMotion(SBMotion* grabMotion);
		SBMotion* getGrabHandMotion();
		void setReleaseHandMotion(SBMotion* releasebMotion);
		SBMotion* getReleaseHandMotion();
		void setReachHandMotion(SBMotion* reachbMotion);
		SBMotion* getReachHandMotion();

	protected:


		SBCharacter* _character;

};

}


#endif