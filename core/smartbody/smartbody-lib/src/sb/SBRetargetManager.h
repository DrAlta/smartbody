#ifndef _SBRETARGETMANAGER_H_
#define _SBRETARGETMANAGER_H_

#include <sb/SBTypes.h>
#include <string>
#include <map>

namespace SmartBody {

class SBRetarget;

typedef std::pair<std::string,std::string> StringPair;

class SBRetargetManager
{
	public:
		SBAPI SBRetargetManager();
		SBAPI ~SBRetargetManager();

		SBAPI SBRetarget* createRetarget(std::string sourceSk, std::string targetSk);		
		SBAPI SBRetarget* getRetarget(std::string sourceSk, std::string targetSk);

	protected:
		std::map<StringPair, SBRetarget*> _retargets;
};

}
#endif