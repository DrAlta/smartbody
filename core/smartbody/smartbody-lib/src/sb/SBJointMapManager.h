#ifndef _SBJOINTMAPMANAGER_
#define _SBJOINTMAPMANAGER_

#include <string>
#include <vector>
#include <map>

namespace SmartBody {

class SBJointMap;

class SBJointMapManager 
{
	public:
		SBJointMapManager();
		~SBJointMapManager();
	
		SmartBody::SBJointMap* createJointMap(const std::string& name);
		SmartBody::SBJointMap* getJointMap(const std::string& name);
		std::vector<std::string> getJointMapNames();
		void removeJointMap(const std::string& name);
		void removeAllJointMaps();

	private:
		std::map<std::string, SmartBody::SBJointMap*> _jointMaps;

};


}

#endif
