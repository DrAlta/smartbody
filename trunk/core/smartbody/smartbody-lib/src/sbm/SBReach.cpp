#include "SBReach.h"


namespace SmartBody {

SBReach::SBReach()
{
	_character = NULL;
}

SBReach::SBReach(SBCharacter* character)
{
	_character = character;
}

SBReach::~SBReach()
{
}

SBCharacter* SBReach::getCharacter()
{
	return _character;
}

SBReach* SBReach::copy()
{
	// ?????
	// ...
	// ...
	return NULL;
}

void SBReach::addMotion(std::string type, SBMotion* motion)
{
	if (!_character)
	{
		LOG("No character present, motion %s was not added to reach.", motion->getName().c_str());
		return;
	}

	if (type == "left" || type == "LEFT")
	{
		_character->addReachMotion(MeCtReachEngine::LEFT_ARM, motion);
	}
	else if (type == "right" || type == "RIGHT")
	{
		_character->addReachMotion(MeCtReachEngine::RIGHT_ARM, motion);
	}
	else if (type == "both" || type == "BOTH")
	{
		_character->addReachMotion(MeCtReachEngine::LEFT_ARM, motion);
		_character->addReachMotion(MeCtReachEngine::RIGHT_ARM, motion);
	}
	else
	{
		LOG("Please use 'LEFT', 'RIGHT', or 'BOTH'");
		return;
	}
}

void SBReach::removeMotion(std::string type, SBMotion* motion)
{
	// ...
	// ...
}

int SBReach::getNumMotions()
{
	if (!_character)
		return 0;

	return _character->getReachMotionDataSet().size();
}

std::vector<std::string> SBReach::getMotionNames(std::string type)
{
	std::vector<std::string> motionNames;
	// ...
	// ...
	return motionNames;

}
void SBReach::build(SBCharacter* character)
{
	if (!_character)
		return;

	
	for (ReachEngineMap::iterator mi = _character->getReachEngineMap().begin();
		mi != _character->getReachEngineMap().end();
		mi++)
	{
		MeCtReachEngine* re = mi->second;
		if (re)
		{
			re->updateMotionExamples(_character->getReachMotionDataSet());
		}
	}
}

void SBReach::setGrabHandMotion(SBMotion* grabMotion)
{
}

SBMotion* SBReach::getGrabHandMotion()
{
	// ...
	return NULL;
}

void SBReach::setReleaseHandMotion(SBMotion* releasebMotion)
{
}

SBMotion* SBReach::getReleaseHandMotion()
{
	// ...
	return NULL;
}

void SBReach::setReachHandMotion(SBMotion* reachbMotion)
{
}

SBMotion* SBReach::getReachHandMotion()
{
	// ...
	return NULL;
}

}