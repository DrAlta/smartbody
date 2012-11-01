#include "SBGestureMap.h"
#include <sbm/mcontrol_util.h>
#include <sb/SBCharacter.h>
#include <sb/SBScene.h>
#include <sb/SBMotion.h>

namespace SmartBody {

SBGestureMap::SBGestureMap() : SBObject()
{
	defaultGestureInfo._animation = "";
	defaultGestureInfo._lexeme = "";
	defaultGestureInfo._type = "";
	defaultGestureInfo._hand = "";
	defaultGestureInfo._style = "";
	defaultGestureInfo._posture = "";
}

SBGestureMap::SBGestureMap(const std::string& name)
{
	setName(name);

	defaultGestureInfo._animation = "";
	defaultGestureInfo._lexeme = "";
	defaultGestureInfo._type = "";
	defaultGestureInfo._hand = "";
	defaultGestureInfo._style = "";
	defaultGestureInfo._posture = "";
}

SBGestureMap::~SBGestureMap()
{
	_gestureMaps.clear();
}

SBGestureMap* SBGestureMap::copy()
{
	// TODO:
	return NULL;
}

void SBGestureMap::addGestureMapping(const std::string& name, const std::string& lexeme, const std::string& type, const std::string& hand, const std::string& style, const std::string& posture)
{
	GestureInfo gInfo;
	gInfo._animation = name;
	gInfo._lexeme = lexeme;
	gInfo._type = type;
	gInfo._hand = hand;
	gInfo._style = style;
	gInfo._posture = posture;

	_gestureMaps.push_back(gInfo);
}

std::string SBGestureMap::getGestureByInfo(const std::string& lexeme, const std::string& type, const std::string& hand, const std::string& style, const std::string& posture)
{
	
	for (std::vector<GestureInfo>::iterator iter = _gestureMaps.begin(); 
		iter != _gestureMaps.end(); 
		iter++)
	{
		if (iter->_lexeme == lexeme &&
			iter->_type == type &&
			iter->_hand == hand &&
			iter->_style == style &&
			iter->_posture == posture)
		{
			return iter->_animation;
		}
	}
	LOG("Gesture %s cannot find gesture with type %s, posture %s, hand %s.", getName().c_str(), type.c_str(), posture.c_str(), hand.c_str());
	return "";
}

SBGestureMap::GestureInfo& SBGestureMap::getGestureByIndex(int i)
{
	if (i < 0 || i >= int(_gestureMaps.size()))
	{
		LOG("Index %d out of range of gesture map.", i);
		return defaultGestureInfo;
	}

	return _gestureMaps[i];
}

int SBGestureMap::getNumMappings()
{
	return _gestureMaps.size();
}

void SBGestureMap::validate()
{
	SBScene* scene = SBScene::getScene();
	std::vector<GestureInfo>::iterator iter = _gestureMaps.begin();
	for (; iter != _gestureMaps.end(); iter++)
	{
		LOG("Gesture: motion='%s' idle='%s' lexeme='%s', type='%s', hand='%s', style='%s'", 
			 iter->_animation.c_str(), iter->_posture.c_str(), iter->_lexeme.c_str(), iter->_type.c_str(), iter->_hand.c_str(), iter->_style.c_str());
		SBMotion* animation = scene->getMotion(iter->_animation);
		if (!animation)
		{
			LOG("WARNING: Animation '%s' is not loaded.", iter->_animation.c_str());
		}
		SBMotion* idle = scene->getMotion(iter->_posture);
		if (!idle)
		{
			LOG("WARNING: Idle '%s' is not loaded.", iter->_posture.c_str());
		}
	}

}


}