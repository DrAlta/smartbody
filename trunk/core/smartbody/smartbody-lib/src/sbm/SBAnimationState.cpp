#include "SBAnimationState.h"
#include <sbm/mcontrol_util.h>

namespace SmartBody {

SBAnimationState::SBAnimationState() : PAState()
{
	_isFinalized = false;
}

SBAnimationState::SBAnimationState(const std::string& name) : PAState(name)
{
	_isFinalized = false;
}

SBAnimationState::~SBAnimationState()
{
}

void SBAnimationState::addCorrespondencePoints(const std::vector<std::string>& motionNames, const std::vector<double>& points)
{
	if (motions.size() == 0)
	{
		LOG("Add motions before add correspondence points for state");
		return;
	}
	if (motionNames.size() != motions.size())
	{
		LOG("Add correspondence points error, input motion number is not the same with that when adding motions");
		return;		
	}
	for (size_t i = 0; i < motionNames.size(); i++)
	{
		if (motionNames[i] != motions[i]->getName())
		{
			LOG("Add correspondence points error, input motion names are not in the same order with that when adding motions");
			return;
		}
	}
	if (motionNames.size() != points.size())
	{
		LOG("Add correspondence points error, input motion number is not the same with points number!");
		return;
	}	
	int num = motionNames.size();
	// first time
// 	if (keys.size() == 0)
// 	{
// 		for (int i = 0; i < num; i++)
// 		{
// 			std::vector<double> keyVec;
// 			keys.push_back(keyVec);
// 		}
// 	}

	// find the right place to insert the keys
	int insertPosition = -1;
	if (keys.size() > 0)
	{
		for (size_t i = 0; i < keys[0].size(); i++)
		{
			if (points[0] <= keys[0][i])
			{
				insertPosition = i;
				break;
			}
		}
		if (insertPosition == -1)
		{
			insertPosition = keys[0].size();
		}
	}

	for (int i = 0; i < num; i++)
	{
		keys[i].insert(keys[i].begin() + insertPosition, points[i]);
	}

	validateCorrespondencePoints();
}

void SBAnimationState::setCorrespondencePoints(int motionIndex, int pointIndex, double value)
{
	if (motionIndex < 0 || pointIndex < 0 || (keys.size() == 0) || (pointIndex >= (int) keys[0].size()))
		return;

	keys[motionIndex][pointIndex] = value;
	validateCorrespondencePoints();
}

void SBAnimationState::removeCorrespondencePoints(int index)
{
	if (index < 0 || (keys.size() == 0) || (index >= (int) keys[0].size()))
		return;

	for (std::vector< std::vector<double> >::iterator iter = keys.begin();
		 iter != keys.end();
		 iter++)
	{
		std::vector<double>& keyArray = (*iter);
		keyArray.erase(keyArray.begin() + index);
	}	
}

int SBAnimationState::getNumMotions()
{
	return motions.size();
}

std::string SBAnimationState::getMotion(int num)
{
	if (motions.size() > (size_t) num && num >= 0)
	{
		return motions[num]->getName();
	}
	else
	{
		return "";
	}
}

int SBAnimationState::getNumCorrespondencePoints()
{
	return getNumKeys();
}

std::vector<double> SBAnimationState::getCorrespondencePoints(int num)
{
	if (keys.size() > (size_t) num && num >= 0)
	{
		return keys[num];
	}
	else
	{
		return std::vector<double>();
	}
}

std::string SBAnimationState::getDimension()
{
	return _dimension;
}

void SBAnimationState::removeMotion(const std::string& motionName)
{
	SmartBody::SBMotion* motion = SmartBody::SBScene::getScene()->getMotion(motionName);
	if (!motion)
	{
		LOG("No motion named %s found, cannot remove from state %s.", motionName.c_str(), this->stateName.c_str());
	}
}

bool SBAnimationState::addSkMotion(const std::string& motion)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SkMotion* skMotion = mcu.getMotion(motion);
	if (skMotion)
	{
		motions.push_back(skMotion);


		// add a zero-correspondence point for this new motion
		int numPoints = 0;
		if (keys.size() > 0)
			numPoints  = keys[keys.size() - 1].size();
		std::vector<double> keyVec;
		if (numPoints > 0)
		{
			keyVec.resize(numPoints);
			// uniformly space the correspondence points
			double time = skMotion->duration();
			double step = time / double(numPoints);
			for (int i = 0; i < numPoints; i++)
			{
				keyVec[i] = double(i) * step;
			}
		}
		keys.push_back(keyVec);
	
		getParameters().push_back(SrVec());
	}
	else
	{
		LOG("SBAnimationState add sk motion failure, %s doesn't exist", motion.c_str());
		return false;
	}
	return true;
}

bool SBAnimationState::removeSkMotion(const std::string& motionName)
{
	// find the index of the motion
	int index = -1;
	for (int i = 0; i < getNumMotions(); i++)
	{
		SkMotion* m = motions[i];
		if (m->getName() == motionName)
		{
			index = i;
			break;
		}
	}
	if (index < 0)
	{
		LOG("SBAnimationState delete motion failure, %s doesn't exist", motionName.c_str());
		return false;
	}

	// first delete corresponding time markers
	removeParameter(motionName);

	// delete the motion and correspondence point
	motions.erase(motions.begin() + index);
	keys.erase(keys.begin() + index);
	return true;
}

/*
	P.S. This is organized way, but is not a efficient way to do it
*/
void SBAnimationState::validateCorrespondencePoints()
{
	for (int i = 0; i < getNumMotions(); i++)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		SkMotion* skMotion = mcu.lookUpMotion(motions[i]->getName().c_str());		
		for (int j = 1; j < getNumCorrespondencePoints(); j++)
		{
			if (keys[i][j] < keys[i][j - 1])
			{
				for (int k = j; k < getNumCorrespondencePoints(); k++)
					keys[i][k] += skMotion->duration();
			}
		}
	}
}

bool SBAnimationState::validateState()
{
	if (_isFinalized)
		return true;

	for (int i=0; i < getNumMotions(); i++)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		SkMotion* skMotion = mcu.lookUpMotion(motions[i]->getName().c_str());		
		if ((int)keys.size() < i) // no keys for this state
		{			
			keys.push_back(std::vector<double>());			
		}		
		std::vector<double>& keyVec = keys[i];
		if (keyVec.size() == 0) // if no keys for the motion, automatically set up this based on motion duration
		{
			keyVec.push_back(0.0);
			keyVec.push_back(skMotion->duration());
		}
	}
	_isFinalized = true;
	return true;
}

void SBAnimationState::addEvent(const std::string& motion, double time, const std::string& type, const std::string& parameters, bool onceOnly)
{
	MotionEvent* motionEvent = new MotionEvent();
	motionEvent->setIsOnceOnly(onceOnly);
	motionEvent->setTime(time);
	motionEvent->setType(type);
	motionEvent->setParameters(parameters);
	addEventToMotion(motion, motionEvent);
}

void SBAnimationState::removeEvent(int index)
{
	if (index < 0 || (int) _events.size() > index)
		return;
}

MotionEvent* SBAnimationState::getEvent(int index)
{
	if (index < 0 || (int) _events.size() > index)
		return NULL;

	return _events[index].first;
}

void SBAnimationState::removeAllEvents()
{
	for (std::vector<std::pair<MotionEvent*, int> >::iterator iter = _events.begin();
		 iter != _events.end();
		 iter++)
	{
		delete (*iter).first;
	}
	_events.clear();
}

int SBAnimationState::getNumEvents()
{
	return _events.size();
}

SBAnimationState0D::SBAnimationState0D() : SBAnimationState("unknown")
{
}

SBAnimationState0D::SBAnimationState0D(const std::string& name) : SBAnimationState(name)
{
	_dimension = "0D";
}

SBAnimationState0D::~SBAnimationState0D()
{
}

void SBAnimationState0D::addMotion(const std::string& motion)
{
	addSkMotion(motion);	
}

void SBAnimationState0D::removeMotion(const std::string& motion)
{
	SBAnimationState::removeMotion(motion);

	// remove correspondence points
	removeSkMotion(motion);
}

SBAnimationState1D::SBAnimationState1D() : SBAnimationState("unknown")
{
}


SBAnimationState1D::SBAnimationState1D(const std::string& name) : SBAnimationState(name)
{
	_dimension = "1D";
}

SBAnimationState1D::~SBAnimationState1D()
{
}

void SBAnimationState1D::addMotion(const std::string& motion, float parameter)
{
	addSkMotion(motion);

	setType(0);
	setParameter(motion, parameter);
}

void SBAnimationState1D::removeMotion(const std::string& motionName)
{
	SBAnimationState::removeMotion(motionName);

	// remove correspondnce points
	removeSkMotion(motionName);
}

void SBAnimationState1D::setParameter(const std::string& motion, float parameter)
{
	PAState::setParameter(motion, parameter);
}

SBAnimationState2D::SBAnimationState2D() : SBAnimationState("unknown")
{
}

SBAnimationState2D::SBAnimationState2D(const std::string& name) : SBAnimationState(name)
{
	_dimension = "2D";
}

SBAnimationState2D::~SBAnimationState2D()
{
}

void SBAnimationState2D::addMotion(const std::string& motion, float parameter1, float parameter2)
{
	addSkMotion(motion);
	
	setType(1);
	PAState::setParameter(motion, parameter1, parameter2);
}

void SBAnimationState2D::removeMotion(const std::string& motionName)
{
	SBAnimationState::removeMotion(motionName);

	// remove correspondence points
	removeSkMotion(motionName);

	// do something about triangle
	removeTriangles(motionName);
}

void SBAnimationState2D::setParameter(const std::string& motion, float parameter1, float parameter2)
{
	PAState::setParameter(motion, parameter1, parameter2);
}

void SBAnimationState2D::addTriangle(const std::string& motion1, const std::string& motion2, const std::string& motion3)
{
	PAState::addTriangle(motion1, motion2, motion3);
}

SBAnimationState3D::SBAnimationState3D() : SBAnimationState("unknown")
{
}


SBAnimationState3D::SBAnimationState3D(const std::string& name) : SBAnimationState(name)
{
	_dimension = "3D";
}

SBAnimationState3D::~SBAnimationState3D()
{
}


void SBAnimationState3D::addMotion(const std::string& motion, float parameter1, float parameter2, float parameter3)
{
	addSkMotion(motion);
	
	setType(1);
	setParameter(motion, parameter1, parameter2, parameter3);
}

void SBAnimationState3D::removeMotion(const std::string& motionName)
{
	SBAnimationState::removeMotion(motionName);

	// remove correspondence points
	removeSkMotion(motionName);

	// do something about tetrahedrons
	removeTetrahedrons(motionName);
}

void SBAnimationState3D::setParameter(const std::string& motion, float parameter1, float parameter2, float parameter3)
{
	PAState::setParameter(motion, parameter1, parameter2, parameter3);
}

void SBAnimationState3D::addTetrahedron(const std::string& motion1, const std::string& motion2, const std::string& motion3, const std::string& motion4)
{
	PAState::addTetrahedron(motion1, motion2, motion3, motion4);
}

}