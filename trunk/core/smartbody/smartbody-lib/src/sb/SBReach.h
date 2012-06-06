#ifndef _SBREACH_H_
#define _SBREACH_H_

#include <string>
#include <vector>

namespace SmartBody {

class SBCharacter;
class SBMotion;

class SBReach
{
	public:
		SBReach();
		SBReach(SBCharacter* character);
		~SBReach();

		SBCharacter* getCharacter();
		SBReach* copy();

		void setInterpolatorType(std::string type);
		void addMotion(std::string type, SBMotion* motion);
		void removeMotion(std::string type, SBMotion* motion);
		int getNumMotions();

		std::vector<std::string> getMotionNames(std::string type);
		void build(SBCharacter* character);
		
		void setGrabHandMotion(std::string type, SBMotion* grabMotion);
		SBMotion* getGrabHandMotion(std::string type);
		void setReleaseHandMotion(std::string type,SBMotion* releasebMotion);
		SBMotion* getReleaseHandMotion(std::string type);
		void setReachHandMotion(std::string type,SBMotion* reachMotion);
		SBMotion* getReachHandMotion(std::string type);

	protected:

		SBCharacter* _character;
		std::string interpolatorType;
};

}


#endif