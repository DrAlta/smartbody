#include "SBReach.h"
#include <sb/SBCharacter.h>
#include <sb/SBMotion.h>
#include <controllers/MeCtBodyReachState.h>

namespace SmartBody {

SBReach::SBReach()
{
	_character = NULL;
	interpolatorType = "KNN";
}

SBReach::SBReach(SBCharacter* character)
{
	_character = character;
	interpolatorType = "KNN";
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

	int reachType = MeCtReachEngine::getReachType(type);
	if (reachType != -1)
	{
		_character->addReachMotion(reachType,motion);
	}
	else
	{
		LOG("Please use 'LEFT' or 'RIGHT'");
		return;
	}
}

void SBReach::removeMotion(std::string type, SBMotion* motion)
{
	if (!_character)
	{
		LOG("No character present, motion %s was not removed from reach.", motion->getName().c_str());
		return;
	}

	int reachType = MeCtReachEngine::getReachType(type);
	if (reachType != -1)
	{
		_character->removeReachMotion(reachType,motion);
	}	
	else
	{
		LOG("Please use 'LEFT' or 'RIGHT'");
		return;
	}
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
	MotionDataSet& motionSet = const_cast<MotionDataSet&>(_character->getReachMotionDataSet());
	MotionDataSet::iterator vi;
	for ( vi  = motionSet.begin();
		  vi != motionSet.end();
		  vi++)
	{
		SkMotion* motion = (*vi).second;
		motionNames.push_back(motion->getName());
	}	
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
			re->updateMotionExamples(_character->getReachMotionDataSet(), interpolatorType);
		}
	}
}


bool SBReach::isPawnAttached( std::string pawnName )
{	
	if (!_character)
		return false;

	for (ReachEngineMap::iterator mi = _character->getReachEngineMap().begin();
		mi != _character->getReachEngineMap().end();
		mi++)
	{
		MeCtReachEngine* re = mi->second;
		if (re)
		{
			if (re->getReachData()->effectorState.attachedPawnName == pawnName)
			{
				return true;
			}
		}
	}
	return false;
}

void SBReach::setPointHandMotion( std::string type, SBMotion* pointMotion )
{
	if (!_character)
		return;
	MotionDataSet& pointMotionSet = const_cast<MotionDataSet&>(_character->getPointHandData());
	int reachType = MeCtReachEngine::getReachType(type);
	if (reachType != -1)
	{
		TagMotion motion = TagMotion(reachType,pointMotion);
		pointMotionSet.insert(motion);
	}
	else
	{
		LOG("Please use 'LEFT' or 'RIGHT'");
		return;
	}	

}

SBMotion* SBReach::getPointHandMotion( std::string type )
{
	MotionDataSet& pointMotionSet = const_cast<MotionDataSet&>(_character->getPointHandData());
	int reachType = MeCtReachEngine::getReachType(type);
	SkMotion* skMotion = SbmCharacter::findTagSkMotion(reachType,pointMotionSet);
	SBMotion* sbMotion = dynamic_cast<SBMotion*>(skMotion);
	return sbMotion;
}

void SBReach::setGrabHandMotion(std::string type, SBMotion* grabMotion)
{
	if (!_character)
		return;
	MotionDataSet& grabMotionSet = const_cast<MotionDataSet&>(_character->getGrabHandData());
	int reachType = MeCtReachEngine::getReachType(type);
	if (reachType != -1)
	{
		TagMotion motion = TagMotion(reachType,grabMotion);
		grabMotionSet.insert(motion);
	}
	else
	{
		LOG("Please use 'LEFT' or 'RIGHT'");
		return;
	}	
}

SBMotion* SBReach::getGrabHandMotion(std::string type)
{
	MotionDataSet& grabMotionSet = const_cast<MotionDataSet&>(_character->getGrabHandData());
	int reachType = MeCtReachEngine::getReachType(type);
	SkMotion* skMotion = SbmCharacter::findTagSkMotion(reachType,grabMotionSet);
	SBMotion* sbMotion = dynamic_cast<SBMotion*>(skMotion);
	return sbMotion;
}

void SBReach::setReleaseHandMotion(std::string type, SBMotion* releasebMotion)
{
	if (!_character)
		return;
	MotionDataSet& releaseMotionSet = const_cast<MotionDataSet&>(_character->getReleaseHandData());
	int reachType = MeCtReachEngine::getReachType(type);
	if (reachType != -1)
	{
		TagMotion motion = TagMotion(reachType,releasebMotion);
		releaseMotionSet.insert(motion);
	}
	else
	{
		LOG("Please use 'LEFT' or 'RIGHT'");
		return;
	}	
}

SBMotion* SBReach::getReleaseHandMotion(std::string type)
{
	MotionDataSet& releaseMotionSet = const_cast<MotionDataSet&>(_character->getReleaseHandData());
	int reachType = MeCtReachEngine::getReachType(type);
	SkMotion* skMotion = SbmCharacter::findTagSkMotion(reachType,releaseMotionSet);
	SBMotion* sbMotion = dynamic_cast<SBMotion*>(skMotion);
	return sbMotion;
}

void SBReach::setReachHandMotion(std::string type, SBMotion* reachMotion)
{
	if (!_character)
		return;
	MotionDataSet& reachMotionSet = const_cast<MotionDataSet&>(_character->getReachHandData());
	int reachType = MeCtReachEngine::getReachType(type);
	if (reachType != -1)
	{
		TagMotion motion = TagMotion(reachType,reachMotion);
		reachMotionSet.insert(motion);
	}
	else
	{
		LOG("Please use 'LEFT' or 'RIGHT'");
		return;
	}	
}

SBMotion* SBReach::getReachHandMotion(std::string type)
{
	MotionDataSet& reachMotionSet = const_cast<MotionDataSet&>(_character->getReachHandData());
	int reachType = MeCtReachEngine::getReachType(type);
	SkMotion* skMotion = SbmCharacter::findTagSkMotion(reachType,reachMotionSet);
	SBMotion* sbMotion = dynamic_cast<SBMotion*>(skMotion);
	return sbMotion;
}

void SBReach::setInterpolatorType( std::string type )
{
	interpolatorType = type;
}


}