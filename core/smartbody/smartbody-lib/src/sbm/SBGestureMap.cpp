#include "SBGestureMap.h"
#include <sbm/mcontrol_util.h>
#include <sbm/SBCharacter.h>

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
	std::map<std::string, SBGestureInfo*>::iterator iter = _gestureMap.begin();
	for (; iter != _gestureMap.end(); iter++)
	{
		delete iter->second;
	}
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

void SBGestureMap::addGestureMapping(const std::string& name, const std::string& type, const std::string& posture, const std::string& hand, const std::string& style)
{
	SBGestureInfo* gestureInfo = new SBGestureInfo(type, posture, hand, style);
	std::map<std::string, SBGestureInfo*>::iterator iter = _gestureMap.find(name);
	if (iter != _gestureMap.end())
	{
		delete iter->second;
		_gestureMap.erase(iter);
	}
	_gestureMap.insert(std::make_pair(name, gestureInfo));
}

std::string SBGestureMap::getGestureByInfo(const std::string& type, const std::string& posture, const std::string& hand, const std::string& style)
{
	std::map<std::string, SBGestureInfo*>::iterator iter = _gestureMap.begin();
	for (; iter != _gestureMap.end(); iter++)
	{
		if (iter->second->getType() == type &&
			iter->second->getPosture() == posture &&
			iter->second->getHand() == hand &&
			iter->second->getStyle() == style)
		{
			return iter->first;
		}
	}
	LOG("Character %s cannot find gesture with type %s, posture %s, hand %s.", _character->getName().c_str(), type.c_str(), posture.c_str(), hand.c_str());
	return "";
}

std::string SBGestureMap::getGestureByIndex(int i)
{
	std::map<std::string, SBGestureInfo*>::iterator iter = _gestureMap.begin();
	int index = 0;
	for (; iter != _gestureMap.end(); iter++)
	{
		if (index == i)
			return iter->first;
		index++;
	}
	LOG("Index out of range of gesture map.");
	return "";

}

int SBGestureMap::getNumMappings()
{
	return _gestureMap.size();
}

std::string SBGestureMap::getGestureType(const std::string& name)
{
	std::map<std::string, SBGestureInfo*>::iterator iter = _gestureMap.find(name);
	if (iter != _gestureMap.end())
		return iter->second->getType();
	LOG("Gesture %s doesn't exist", name.c_str());
	return "";
}

std::string SBGestureMap::getGesturePosture(const std::string& name)
{
	std::map<std::string, SBGestureInfo*>::iterator iter = _gestureMap.find(name);
	if (iter != _gestureMap.end())
		return iter->second->getPosture();
	LOG("Gesture %s doesn't exist", name.c_str());
	return "";
}

std::string SBGestureMap::getGestureHand(const std::string& name)
{
	std::map<std::string, SBGestureInfo*>::iterator iter = _gestureMap.find(name);
	if (iter != _gestureMap.end())
		return iter->second->getHand();
	LOG("Gesture %s doesn't exist", name.c_str());
	return "";
}

std::string SBGestureMap::getGestureStyle(const std::string& name)
{
	std::map<std::string, SBGestureInfo*>::iterator iter = _gestureMap.find(name);
	if (iter != _gestureMap.end())
		return iter->second->getStyle();
	LOG("Gesture %s doesn't exist", name.c_str());
	return "";
}

SBGestureInfo::SBGestureInfo(const std::string& type, const std::string& posture, const std::string& hand, const std::string& style)
{
	_type = type;
	_posture = posture;
	_hand = hand;
	_style = style;
}

SBGestureInfo::~SBGestureInfo()
{
}

void SBGestureInfo::setType(const std::string& type)
{
	_type = type;
}

const std::string& SBGestureInfo::getType()
{
	return _type;
}

void SBGestureInfo::setPosture(const std::string& posture)
{
	_posture = posture;
}

const std::string& SBGestureInfo::getPosture()
{
	return _posture;
}

void SBGestureInfo::setHand(const std::string& hand)
{
	_hand = hand;
}

const std::string& SBGestureInfo::getHand()
{
	return _hand;
}

void SBGestureInfo::setStyle(const std::string& style)
{
	_style = style;
}
const std::string& SBGestureInfo::getStyle()
{
	return _style;
}

}