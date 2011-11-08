#include "SBState.h"
#include <sbm/mcontrol_util.h>

namespace SmartBody {

SBState::SBState() : PAStateData()
{
}

SBState::SBState(std::string name) : PAStateData(name)
{
}

SBState::~SBState()
{
}

void SBState::addCorrespondancePoints(std::vector<std::string> motionNames, std::vector<double> points)
{
	if (motions.size() == 0)
	{
		LOG("Add motions before add correspondance points for state");
		return;
	}
	if (motionNames.size() != motions.size())
	{
		LOG("Add correspondance points error, input motion number is not the same with that when adding motions");
		return;		
	}
	for (size_t i = 0; i < motionNames.size(); i++)
	{
		if (motionNames[i] != motions[i]->getName())
		{
			LOG("Add correspondance points error, input motion names are not in the same order with that when adding motions");
			return;
		}
	}
	if (motionNames.size() != points.size())
	{
		LOG("Add correspondance points error, input motion number is not the same with points number!");
		return;
	}
	int num = motionNames.size();

	// first time
	if (keys.size() == 0)
	{
		for (int i = 0; i < num; i++)
		{
			std::vector<double> keyVec;
			keys.push_back(keyVec);
		}
	}

	for (int i = 0; i < num; i++)
	{
		keys[i].push_back(points[i]);
	}
}

int SBState::getNumMotions()
{
	return motions.size();
}

std::string SBState::getMotion(int num)
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

int SBState::getNumCorrespondancePoints()
{
	return getNumKeys();
}

std::vector<double> SBState::getCorrespondancePoints(int num)
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

std::string SBState::getDimension()
{
	return _dimension;
}

bool SBState::addSkMotion(std::string motion)
{
	//TODO: remove weights from SBState
	if (motions.size() == 0)
		weights.push_back(1.0);
	else
		weights.push_back(0.0);
	//---

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SkMotion* skMotion = mcu.lookUpMotion(motion.c_str());
	if (skMotion)
		motions.push_back(skMotion);
	else
	{
		LOG("SBState add sk motion failure, %s doesn't exist", motion.c_str());
		return false;
	}
	return true;
}

SBState0D::SBState0D() : SBState("unknown")
{
}

SBState0D::SBState0D(std::string name) : SBState(name)
{
	_dimension = "0D";
}

SBState0D::~SBState0D()
{
}

void SBState0D::addMotion(std::string motion)
{
	addSkMotion(motion);
}

SBState1D::SBState1D() : SBState("unknown")
{
}


SBState1D::SBState1D(std::string name) : SBState(name)
{
	_dimension = "1D";
}

SBState1D::~SBState1D()
{
}

void SBState1D::addMotion(std::string motion, float parameter)
{
	addSkMotion(motion);

	paramManager->setType(0);
	paramManager->addParameter(motion, parameter);
}

SBState2D::SBState2D() : SBState("unknown")
{
}

SBState2D::SBState2D(std::string name) : SBState(name)
{
	_dimension = "2D";
}

SBState2D::~SBState2D()
{
}

void SBState2D::addMotion(std::string motion, float parameter1, float parameter2)
{
	addSkMotion(motion);
	
	paramManager->setType(1);
	paramManager->addParameter(motion, parameter1, parameter2);
}

void SBState2D::addTriangle(std::string motion1, std::string motion2, std::string motion3)
{
	paramManager->addTriangle(motion1, motion2, motion3);
}

SBState3D::SBState3D() : SBState("unknown")
{
}


SBState3D::SBState3D(std::string name) : SBState(name)
{
	_dimension = "3D";
}

SBState3D::~SBState3D()
{
}


void SBState3D::addMotion(std::string motion, float parameter1, float parameter2, float parameter3)
{
	addSkMotion(motion);
	
	paramManager->setType(1);
	paramManager->addParameter(motion, parameter1, parameter2, parameter3);
}

void SBState3D::addTetrahedron(std::string motion1, std::string motion2, std::string motion3,std::string motion4)
{
}

}