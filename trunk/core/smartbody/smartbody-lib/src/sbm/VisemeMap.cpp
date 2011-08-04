#include "VisemeMap.hpp"
#include "mcontrol_util.h"

FaceDefinition::FaceDefinition()
{
	_faceNeutral = NULL;
}

std::string FaceDefinition::getName()
{
	return _name;
}

void FaceDefinition::setName(std::string name)
{
	_name = name;
}

FaceDefinition::FaceDefinition(FaceDefinition* source)
{
	_faceNeutral = source->getFaceNeutral();
	if (_faceNeutral)
		_faceNeutral->ref();

	int numVisemes = source->getNumVisemes();
	for (int v = 0; v < numVisemes; v++)
	{
		std::string visemeName = source->getVisemeName(v);
		SkMotion* motion = source->getVisemeMotion(visemeName);
		_visemeMap.insert(std::pair<std::string, SkMotion*>(visemeName, motion));
		if (motion)
			motion->ref();
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

FaceDefinition::~FaceDefinition()
{
	for (std::map<std::string, SkMotion*>::iterator iter = _visemeMap.begin();
		 iter != _visemeMap.end();
		 iter++)
	{
		SkMotion* motion = (*iter).second;
		if (motion)
			motion->unref();
	}

	_visemeMap.clear();
	if (_faceNeutral)
		_faceNeutral->unref();


	for (std::map<int, ActionUnit*>::iterator iter = _auMap.begin();
		 iter != _auMap.end();
		 iter++)
	{
		ActionUnit* au = (*iter).second;
		delete au;
	}
}

void FaceDefinition::setFaceNeutral(std::string motionName)
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

	if (_faceNeutral)
		_faceNeutral->unref();
	_faceNeutral = motion;
	if (_faceNeutral)
		_faceNeutral->ref();

	LOG("Face neutral motion is now '%s'.", motionName.c_str());
}

SkMotion* FaceDefinition::getFaceNeutral()
{
	return _faceNeutral;
}

bool FaceDefinition::hasAU(int auNum)
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

void FaceDefinition::setAU(int auNum, std::string side, std::string motionName)
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
		LOG("Unrecognized side '%s'. Action Unit %d not added.", side, auNum);
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
		LOG("AU '%d' added to face definition %s.", auNum, getName().c_str());
		
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

void FaceDefinition::addAU(int auNum, ActionUnit* au)
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

bool FaceDefinition::hasViseme(std::string visemeName)
{
	std::map<std::string, SkMotion*>::iterator iter = _visemeMap.find(visemeName);
	if (iter == _visemeMap.end())
	{
		return false;
	}
	else
	{
		return true;
	}
}

void FaceDefinition::setViseme(std::string visemeName, std::string motionName)
{
	std::map<std::string, SkMotion*>::iterator iter = _visemeMap.find(visemeName);
	if (iter == _visemeMap.end())
	{
		// no motion given, add the viseme only
		if (motionName == "")
		{
			_visemeMap.insert(std::pair<std::string, SkMotion*>(visemeName, NULL));
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

		motion->ref();
		_visemeMap.insert(std::pair<std::string, SkMotion*>(visemeName, motion));
		LOG("Viseme '%s' added to face definition %s.", visemeName.c_str(), getName().c_str());
		return;
	}
	else // viseme already exists - replace it with the new definition
	{
		SkMotion* motion = (*iter).second;
		if (motion)
		{
			// no motion given, add the viseme only
			if (motionName == "")
			{
				motion->unref();
				_visemeMap.erase(iter);
				_visemeMap.insert(std::pair<std::string, SkMotion*>(visemeName, NULL));
				LOG("Viseme '%s' with motion '%s' replaced with no motion.", visemeName.c_str(), motion->name()); 
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
					motion->unref();
					_visemeMap.erase(iter);
					_visemeMap.insert(std::pair<std::string, SkMotion*>(visemeName, newMotion));
					LOG("Viseme '%s' with motion '%s' replaced with motion '%s'.", visemeName.c_str(), motion->name(), motionName.c_str());
					return;
				}
			}
		}
	}
}

int FaceDefinition::getNumVisemes()
{
	return _visemeMap.size();
}

std::string FaceDefinition::getVisemeName(int index)
{
	int counter = 0;
	for (std::map<std::string, SkMotion*>::iterator iter = _visemeMap.begin();
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

	return "";
}

SkMotion* FaceDefinition::getVisemeMotion(std::string viseme)
{
	std::map<std::string, SkMotion*>::iterator iter = _visemeMap.find(viseme);
	if (iter != _visemeMap.end())
	{
		return (*iter).second;
	}

	return NULL;
}

int FaceDefinition::getNumAUs()
{
	return _auMap.size();
}

int FaceDefinition::getAUNum(int index)
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

ActionUnit* FaceDefinition::getAU(int index)
{
	std::map<int, ActionUnit*>::iterator iter = _auMap.find(index);
	if (iter == _auMap.end())
		return NULL;
	else
		return (*iter).second;
}
