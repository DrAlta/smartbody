#ifndef _SBREACHMANAGER_H_
#define _SBREACHMANAGER_H_

#include <string>
#include <map>

namespace SmartBody {

class SBReach;

class SBReachManager
{
	public:
		SBReachManager();
		~SBReachManager();

		SBReach* createReach(std::string characterName);
		void removeReach(SBReach* reach);
		int getNumReaches();
		SBReach* getReach(std::string characterName);

	protected:
		std::map<std::string, SBReach*> _reaches;
};


}

#endif