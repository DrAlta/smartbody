#include "SBGestureMap.h"
#include <sbm/mcontrol_util.h>
#include <sb/SBCharacter.h>

namespace SmartBody {

SBGestureMap::SBGestureMap()
{
	_character = NULL;
}

SBGestureMap::SBGestureMap(SBCharacter* character)
{
	_character = character;
}

SBGestureMap::~SBGestureMap()
{
	_gestureMap.clear();
}

SBCharacter* SBGestureMap::getCharacter()
{
	return _character;
}

SBGestureMap* SBGestureMap::copy()
{
	// TODO:
	return NULL;
}

void SBGestureMap::addGestureMapping(const std::string& name, const std::string& lexeme, const std::string& type, const std::string& hand, const std::string& style, const std::string& posture)
{
	GestureInfo gInfo;
	gInfo._lexeme = lexeme;
	gInfo._type = type;
	gInfo._hand = hand;
	gInfo._style = style;
	gInfo._posture = posture;

	std::map<std::string, GestureInfo>::iterator iter = _gestureMap.find(name);
	if (iter != _gestureMap.end())
	{
		_gestureMap.erase(iter);
	}
	_gestureMap.insert(std::make_pair(name, gInfo));
}

std::string SBGestureMap::getGestureByInfo(const std::string& lexeme, const std::string& type, const std::string& hand, const std::string& style, const std::string& posture)
{
	std::map<std::string, GestureInfo>::iterator iter = _gestureMap.begin();
	for (; iter != _gestureMap.end(); iter++)
	{
		if (iter->second._lexeme == lexeme &&
			iter->second._type == type &&
			iter->second._hand == hand &&
			iter->second._style == style &&
			iter->second._posture == posture)
		{
			return iter->first;
		}
	}
	LOG("Character %s cannot find gesture with type %s, posture %s, hand %s.", _character->getName().c_str(), type.c_str(), posture.c_str(), hand.c_str());
	return "";
}

std::string SBGestureMap::getGestureByIndex(int i)
{
	if (i < 0 || i >= int(_gestureMap.size()))
	{
		LOG("Index %d out of range of gesture map.", i);
		return "";
	}

	std::map<std::string, GestureInfo>::iterator iter = _gestureMap.begin();
	std::advance(iter, i);
	return iter->first;
}

int SBGestureMap::getNumMappings()
{
	return _gestureMap.size();
}

}