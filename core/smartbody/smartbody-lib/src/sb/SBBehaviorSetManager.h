#ifndef _SBBEHAVIORSETMANAGER_
#define _SBBEHAVIORSETMANAGER_

#include <map>
#include <vector>
#include "SBBehaviorSet.h"

class SBBehaviorSet;

namespace SmartBody
{

class SBBehaviorSetManager
{
	public:
		SBBehaviorSetManager();
		~SBBehaviorSetManager();

		SBBehaviorSet* createBehaviorSet(const std::string& name);
		int getNumBehaviorSets();
		std::map<std::string, SBBehaviorSet*>& getBehaviorSets();
		SBBehaviorSet* getBehaviorSet(const std::string& name);
		void addBehaviorSet(const std::string& name, SBBehaviorSet* set);
		void removeBehaviorSet(const std::string& name);
		void removeAllBehaviorSets();

	protected:
		std::map<std::string, SBBehaviorSet*> _behaviorSets;

};

}

#endif