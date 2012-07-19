#ifndef _SBBEHAVIORSET_
#define _SBBEHAVIORSET_

#include <string>

namespace SmartBody {

class SBBehaviorSet
{
	public:
		SBBehaviorSet();
		~SBBehaviorSet();

		void setName(const std::string& name);
		const std::string& getName();
		void setScript(const std::string& name);
		const std::string& getScript();
	
	protected:
		std::string _name;
		std::string _script;

};

}

#endif