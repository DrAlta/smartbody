#include "SBJointMapManager.h"
#include <sbm/SBJointMap.h>

namespace SmartBody {

SBJointMapManager::SBJointMapManager()
{
}

SBJointMapManager::~SBJointMapManager()
{
}

SmartBody::SBJointMap* SBJointMapManager::getJointMap(const std::string& name)
{
	std::map<std::string, SmartBody::SBJointMap*>::iterator iter = _jointMaps.find(name);
	if (iter != _jointMaps.end())
		return (*iter).second;
	else
		return NULL;
}

SmartBody::SBJointMap* SBJointMapManager::createJointMap(const std::string& name)
{
	std::map<std::string, SmartBody::SBJointMap*>::iterator iter = _jointMaps.find(name);
	if (iter == _jointMaps.end())
	{
		SmartBody::SBJointMap* map = new SmartBody::SBJointMap();
		_jointMaps[name] = map;
		return map;
	}
	else
	{
		return NULL;
	}
}

std::vector<std::string> SBJointMapManager::getJointMapNames()
{
	std::vector<std::string> names;
	for (std::map<std::string, SmartBody::SBJointMap*>::iterator iter = _jointMaps.begin();
		 iter != _jointMaps.end();
		 iter++)
	{
		names.push_back((*iter).first);
	}

	return names;
}

}
