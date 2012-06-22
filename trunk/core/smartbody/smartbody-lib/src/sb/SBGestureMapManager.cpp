#include "SBGestureMapManager.h"
#include <sbm/mcontrol_util.h>
#include <sb/SBScene.h>
#include <sb/SBGestureMap.h>

namespace SmartBody {

SBGestureMapManager::SBGestureMapManager()
{
}

SBGestureMapManager::~SBGestureMapManager()
{
	std::map<std::string, SBGestureMap*>::iterator iter = _gestureMaps.begin();
	for (; iter != _gestureMaps.end(); iter++)
	{
		delete iter->second;
	}
	_gestureMaps.clear();
}

SBGestureMap* SBGestureMapManager::createGestureMap(std::string characterName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SBScene* scene = mcu._scene;
	// get the character
	SBCharacter* character = scene->getCharacter(characterName);
	if (!character)
	{
		LOG("Character named %s does not exist.", characterName.c_str());
		return NULL;
	}

	std::map<std::string, SBGestureMap*>::iterator iter = _gestureMaps.find(characterName);
	if (iter != _gestureMaps.end())
	{
		delete iter->second;
		_gestureMaps.erase(iter);
	}
	
	SBGestureMap* map = new SBGestureMap(character);
	_gestureMaps.insert(std::make_pair(characterName, map));
	return map;
}

void SBGestureMapManager::removeGestureMap(std::string characterName)
{
	std::map<std::string, SBGestureMap*>::iterator iter = _gestureMaps.find(characterName);
	if (iter != _gestureMaps.end())
	{
		delete iter->second;
		_gestureMaps.erase(iter);
	}
	else
		LOG("Character %s doesn't has gesture map!", characterName.c_str());
}

int SBGestureMapManager::getNumGestureMaps()
{
	return _gestureMaps.size();
}

SBGestureMap* SBGestureMapManager::getGestureMap(std::string characterName)
{
	std::map<std::string, SBGestureMap*>::iterator iter = _gestureMaps.find(characterName);
	if (iter != _gestureMaps.end())
	{
		return (*iter).second;
	}
	else
	{
		return NULL;
	}
}

}
