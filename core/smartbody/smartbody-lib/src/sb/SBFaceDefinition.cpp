#include "SBFaceDefinition.h"
#include "sbm/mcontrol_util.h"
#include <sk/sk_motion.h>
#include "sbm/action_unit.hpp"
#include <sb/SBMotion.h>


namespace SmartBody {

SBFaceDefinition::SBFaceDefinition()
{
	_faceNeutral = NULL;
}

SBFaceDefinition::SBFaceDefinition(const std::string& name)
{
	_faceNeutral = NULL;
	_name = name;
}

const std::string& SBFaceDefinition::getName()
{
	return _name;
}

void SBFaceDefinition::setName(const std::string& name)
{
	_name = name;
}

SBFaceDefinition::SBFaceDefinition(SBFaceDefinition* source)
{
	if (!source)
	{
		_faceNeutral = NULL;
		return;
	}
	_faceNeutral = source->getFaceNeutral();
	int numVisemes = source->getNumVisemes();
	for (int v = 0; v < numVisemes; v++)
	{
		std::string visemeName = source->getVisemeName(v);
		SkMotion* motion = source->getVisemeMotion(visemeName);
		_visemeMap.insert(std::pair<std::string, std::pair<SkMotion*, float> >(visemeName, std::pair<SkMotion*, float>(motion, 1.0f)));
	}

	int numAUs = source->getNumAUs();
	for (int a = 0; a < numAUs; a++)
	{
		int auNum = source->getAUNum(a);
		ActionUnit* sourceAu = source->getAU(auNum);
		ActionUnit* au = new ActionUnit(sourceAu);
		_auMap.insert(std::pair<int, ActionUnit*>(auNum, au));
	}
	
}

SBFaceDefinition::~SBFaceDefinition()
{
	for (std::map<std::string, std::pair<SkMotion*, float> >::iterator iter = _visemeMap.begin();
		 iter != _visemeMap.end();
		 iter++)
	{
		SkMotion* motion = (*iter).second.first;
	}

	_visemeMap.clear();

	for (std::map<int, ActionUnit*>::iterator iter = _auMap.begin();
		 iter != _auMap.end();
		 iter++)
	{
		ActionUnit* au = (*iter).second;
		delete au;
	}
}

void SBFaceDefinition::setFaceNeutral(const std::string& motionName)
{
	SkMotion* motion = NULL;
	if (motionName.length() > 0)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		motion = mcu.getMotion(motionName);
		if (!motion)
		{
			LOG("ERROR: Unknown facial neutral motion \"%s\".", motionName.c_str());
			return;
		}
	}

	_faceNeutral = motion;

	LOG("Face neutral motion is now '%s'.", motionName.c_str());
}

SkMotion* SBFaceDefinition::getFaceNeutral()
{
	return _faceNeutral;
}

bool SBFaceDefinition::hasAU(int auNum)
{
	std::map<int, ActionUnit*>::iterator iter = _auMap.find(auNum);
	if (iter == _auMap.end())
	{
		return false;
	}
	else // au already exists, replace it
	{
		return true;
	}
}

void SBFaceDefinition::setAU(int auNum, const std::string& side, const std::string& motionName)
{
	if (side != "left" &&
		side != "LEFT" &&
		side != "right" &&
		side != "RIGHT" &&
		side != "both" && 
		side != "BOTH" &&
		side != ""
		)
	{
		LOG("Unrecognized side '%s'. Action Unit %d not added.", side.c_str(), auNum);
		return;
	}

	SkMotion* motion = NULL;
	if (motionName.length() > 0)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		motion = mcu.getMotion(motionName);
		if (!motion)
		{
			LOG("ERROR: Unknown facial pose \"%s\".", motionName.c_str());
			return;
		}
	}

	ActionUnit* au = getAU(auNum);
	if (!au)
	{
		if (side == "both" || side == "BOTH" || side == "")
		{
			au = new ActionUnit( motion );
			au->reset_type();
			au->set_bilateral();
		}
		else if (side == "left" || side == "LEFT")
		{
			au = new ActionUnit( motion, NULL );
			au->reset_type();
			au->set_left();
		}
		else if (side == "right" || side == "RIGHT")
		{
			au = new ActionUnit( NULL, motion );
			au->reset_type();
			au->set_right();
		}

		addAU(auNum, au);
		//LOG("AU '%d' added to face definition %s.", auNum, getName().c_str());
		
	} 
	else
	{
		if (side == "")
		{
			if( au->is_left() || au->is_right() )
			LOG("WARNING: Overwritting au #%d", auNum);
			au->set( motion );
			au->reset_type();
			au->set_bilateral();
		}
		else if (side == "left" || side == "LEFT")
		{				
			if( au->is_left() || au->is_bilateral())
			LOG("WARNING: Overwritting au #%d left", auNum);
			au->set( motion, au->right );
			if (au->is_right())
			{
				au->reset_type();
				au->set_left();
				au->set_right();
			}
			else if (au->is_bilateral())
			{
				au->reset_type();
				au->set_left();
			}
		}
		else if (side == "right" || side == "RIGHT")
		{
			if( au->is_right() )
				LOG("WARNING: Overwritting au #%d right", auNum);
			au->set( au->left, motion );
			if (au->is_left())
			{
				au->reset_type();
				au->set_left();
				au->set_right();
			}
			else if (au->is_bilateral())
			{
				au->reset_type();
				au->set_right();
			}
		}
	}

}

void SBFaceDefinition::addAU(int auNum, ActionUnit* au)
{
	if (!au)
	{
		LOG("No AU given. Cannot add AU '%d'.", auNum);
		return;
	}

	std::map<int, ActionUnit*>::iterator iter = _auMap.find(auNum);
	if (iter == _auMap.end())
	{
		_auMap.insert(std::pair<int, ActionUnit*>(auNum, au));
		return;
	}
	else // au already exists, replace it
	{
		_auMap.erase(iter);
		_auMap.insert(std::pair<int, ActionUnit*>(auNum, au));
		LOG("Action unit '%d' replaced with new rules.", auNum);
	}
}

bool SBFaceDefinition::hasViseme(const std::string& visemeName)
{
	std::map<std::string, std::pair<SkMotion*, float> >::iterator iter = _visemeMap.find(visemeName);
	if (iter == _visemeMap.end())
	{
		return false;
	}
	else
	{
		return true;
	}
}

void SBFaceDefinition::setViseme(const std::string& visemeName, const std::string& motionName)
{
	std::map<std::string, std::pair<SkMotion*, float> >::iterator iter = _visemeMap.find(visemeName);
	if (iter == _visemeMap.end())
	{
		// no motion given, add the viseme only
		if (motionName == "")
		{
			_visemeMap.insert(std::pair<std::string, std::pair<SkMotion*, float> >(visemeName,std::pair<SkMotion*, float>((SkMotion*)NULL, 0.0f)));
			return;
		}

		// motion name given, find the associated motion
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		SkMotion* motion = mcu.getMotion(motionName);
		if (!motion)
		{
			LOG("Cannot find viseme named '%s'. Viseme named '%s' not added.", visemeName.c_str(), motionName.c_str());
			return;
		}

		_visemeMap.insert(std::pair<std::string, std::pair<SkMotion*, float> >(visemeName, std::pair<SkMotion*, float>(motion, 1.0f)));
		//LOG("Viseme '%s' added to face definition %s.", visemeName.c_str(), getName().c_str());
		return;
	}
	else // viseme already exists - replace it with the new definition
	{
		SkMotion* motion = (*iter).second.first;
		if (motion)
		{
			// no motion given, add the viseme only
			if (motionName == "")
			{
				_visemeMap.erase(iter);
				_visemeMap.insert(std::pair<std::string, std::pair<SkMotion*, float> >(visemeName, std::pair<SkMotion*, float>((SkMotion*)NULL, 1.0f)));
				LOG("Viseme '%s' with motion '%s' replaced with no motion.", visemeName.c_str(), motion->getName().c_str()); 
				return;
			}
			else
			{
				mcuCBHandle& mcu = mcuCBHandle::singleton();
				SkMotion* newMotion = mcu.getMotion(motionName);
				if (!newMotion)
				{
					LOG("Cannot find viseme named '%s'. Viseme named '%s' not added.", visemeName.c_str(), motionName.c_str());
					return;
				}
				else
				{
					_visemeMap.erase(iter);
					_visemeMap.insert(std::pair<std::string, std::pair<SkMotion*, float> >(visemeName, std::pair<SkMotion*, float>(newMotion, 1.0f)));
					LOG("Viseme '%s' with motion '%s' replaced with motion '%s'.", visemeName.c_str(), motion->getName().c_str(), motionName.c_str());
					return;
				}
			}
		}
	}
}

void SBFaceDefinition::setVisemeWeight(const std::string& visemeName, float weight)
{
	std::map<std::string, std::pair<SkMotion*, float> >::iterator iter = _visemeMap.find(visemeName);
	if (iter == _visemeMap.end())
	{
		LOG("Viseme '%s' does not exist, cannot set its' weight.", visemeName.c_str());
		return;
	}
	else // viseme already exists - replace it with the new definition
	{
		(*iter).second.second = weight;
		LOG("Viseme '%s' now has weight %f", visemeName.c_str(), weight);
	}
}

int SBFaceDefinition::getNumVisemes()
{
	return _visemeMap.size();
}

std::vector<std::string> SBFaceDefinition::getVisemeNames()
{
	std::vector<std::string> visemeNames;
	for (std::map<std::string, std::pair<SkMotion*, float> >::iterator iter = _visemeMap.begin();
		 iter != _visemeMap.end();
		 iter++)
	{
		visemeNames.push_back((*iter).first);
	}

	return visemeNames;
}

const std::string& SBFaceDefinition::getVisemeName(int index)
{
	int counter = 0;
	for (std::map<std::string, std::pair<SkMotion*, float> >::iterator iter = _visemeMap.begin();
		 iter != _visemeMap.end();
		 iter++)
	{
		if (counter == index)
		{
			return (*iter).first;
			break;
		}
		counter++;
	}

	return _emptyString;
}

SkMotion* SBFaceDefinition::getVisemeMotion(const std::string& viseme)
{
	std::map<std::string, std::pair<SkMotion*, float> >::iterator iter = _visemeMap.find(viseme);
	if (iter != _visemeMap.end())
	{
		return (*iter).second.first;
	}

	return NULL;
}

float SBFaceDefinition::getVisemeWeight(const std::string& viseme)
{
	std::map<std::string, std::pair<SkMotion*, float> >::iterator iter = _visemeMap.find(viseme);
	if (iter != _visemeMap.end())
	{
		return (*iter).second.second;
	}

	return 0.0f;
}

int SBFaceDefinition::getNumAUs()
{
	return _auMap.size();
}

int SBFaceDefinition::getNumAUChannels()
{
	int numAuChannels = 0;
	std::map<int, ActionUnit*>::iterator iter = _auMap.begin();
	for (; iter != _auMap.end(); iter++)
	{
		if (iter->second->is_bilateral())
			numAuChannels += 1;
		else
			numAuChannels += 2;
	}
	return numAuChannels;
}

int SBFaceDefinition::getAUNum(int index)
{
	int counter = 0;
	for (std::map<int, ActionUnit*>::iterator iter = _auMap.begin();
		 iter != _auMap.end();
		 iter++)
	{
		if (counter == index)
		{
			return (*iter).first;
			break;
		}
		counter++;
	}

	return -1;
}

ActionUnit* SBFaceDefinition::getAU(int index)
{
	std::map<int, ActionUnit*>::iterator iter = _auMap.find(index);
	if (iter == _auMap.end())
		return NULL;
	else
		return (*iter).second;
}

std::vector<int> SBFaceDefinition::getAUNumbers()
{
	std::vector<int> auNames;
	for (std::map<int, ActionUnit*>::iterator iter = _auMap.begin();
		 iter != _auMap.end();
		 iter++)
	{
		auNames.push_back((*iter).first);
	}

	return auNames;
}

std::string SBFaceDefinition::getAUSide(int num)
{
	std::map<int, ActionUnit*>::iterator iter = _auMap.find(num);
	if (iter == _auMap.end())
	{
		LOG("AU %d not found.", num);
		return "";
	}

	if ((*iter).second->is_bilateral())
		return "BOTH";
	else if ((*iter).second->is_left() && (*iter).second->is_right())
		return "LEFTRIGHT";
	else if ((*iter).second->is_left())
		return "LEFT";
	else 
		return "RIGHT";

}

SBMotion* SBFaceDefinition::getAUMotion(int num, std::string side)
{
	std::map<int, ActionUnit*>::iterator iter = _auMap.find(num);
	if (iter == _auMap.end())
	{
		LOG("AU %d not found.", num);
		return NULL;
	}

	SkMotion* motion = NULL;
	ActionUnit* au = (*iter).second;
	if (side == "LEFT" || side == "left")
	{
		if (au->is_left())
		{
			motion = au->left;
		}
		else
		{
			LOG("Action Unit %d does not have a LEFT motion.", num);
			return NULL;
		}
	}
	else if (side == "RIGHT" || side == "right")
	{
		if (au->is_right())
		{
			motion = au->right;
		}
		else
		{
			LOG("Action Unit %d does not have a RIGHT motion.", num);
			return NULL;
		}
	}
	else if (side == "BOTH" || side == "both")
	{
		if (au->is_bilateral())
		{
			motion = au->left;
		}
		else
		{
			LOG("Action Unit %d does not have a BOTH motion.", num);
			return NULL;
		}
	}
	else
	{
		LOG("Use: LEFT, RIGHT, BOTH for Action Unit side.");
		return NULL;
	}

	SBMotion* sbmotion = dynamic_cast<SBMotion*>(motion);
	return sbmotion;
}

}
