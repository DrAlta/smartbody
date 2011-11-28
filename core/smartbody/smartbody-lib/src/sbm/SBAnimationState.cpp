#include "SBAnimationState.h"
#include <sbm/mcontrol_util.h>

namespace SmartBody {

SBAnimationState::SBAnimationState() : PAStateData()
{
}

SBAnimationState::SBAnimationState(std::string name) : PAStateData(name)
{
}

SBAnimationState::~SBAnimationState()
{
}

void SBAnimationState::addCorrespondancePoints(std::vector<std::string> motionNames, std::vector<double> points)
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

	validateCorrespondancePoints();
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

int SBAnimationState::getNumCorrespondancePoints()
{
	return getNumKeys();
}

std::vector<double> SBAnimationState::getCorrespondancePoints(int num)
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

bool SBAnimationState::addSkMotion(std::string motion)
{
	//TODO: remove weights from SBAnimationState
	if (motions.size() == 0)
		weights.push_back(1.0);
	else
		weights.push_back(0.0);
	//---

	//TODO: remove skMotion maybe
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SkMotion* skMotion = mcu.lookUpMotion(motion.c_str());
	if (skMotion)
	{
		motions.push_back(skMotion);
	}
	else
	{
		LOG("SBAnimationState add sk motion failure, %s doesn't exist", motion.c_str());
		return false;
	}
	return true;
}

/*
	P.S. This is organized way, but is not a efficient way to do it
*/
void SBAnimationState::validateCorrespondancePoints()
{
	for (int i = 0; i < getNumMotions(); i++)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		SkMotion* skMotion = mcu.lookUpMotion(motions[i]->getName().c_str());		
		for (int j = 1; j < getNumCorrespondancePoints(); j++)
		{
			if (keys[i][j] < keys[i][j - 1])
			{
				for (int k = j; k < getNumCorrespondancePoints(); k++)
					keys[i][k] += skMotion->duration();
			}
		}
	}
}

SBAnimationState0D::SBAnimationState0D() : SBAnimationState("unknown")
{
}

SBAnimationState0D::SBAnimationState0D(std::string name) : SBAnimationState(name)
{
	_dimension = "0D";
}

SBAnimationState0D::~SBAnimationState0D()
{
}

void SBAnimationState0D::addMotion(std::string motion)
{
	addSkMotion(motion);
}

SBAnimationState1D::SBAnimationState1D() : SBAnimationState("unknown")
{
}


SBAnimationState1D::SBAnimationState1D(std::string name) : SBAnimationState(name)
{
	_dimension = "1D";
}

SBAnimationState1D::~SBAnimationState1D()
{
}

void SBAnimationState1D::addMotion(std::string motion, float parameter)
{
	addSkMotion(motion);

	paramManager->setType(0);
	paramManager->setParameter(motion, parameter);
}

void SBAnimationState1D::setParameter(std::string motion, float parameter)
{
	paramManager->setParameter(motion, parameter);
}

SBAnimationState2D::SBAnimationState2D() : SBAnimationState("unknown")
{
}

SBAnimationState2D::SBAnimationState2D(std::string name) : SBAnimationState(name)
{
	_dimension = "2D";
}

SBAnimationState2D::~SBAnimationState2D()
{
}

void SBAnimationState2D::addMotion(std::string motion, float parameter1, float parameter2)
{
	addSkMotion(motion);
	
	paramManager->setType(1);
	paramManager->setParameter(motion, parameter1, parameter2);
}

void SBAnimationState2D::setParameter(std::string motion, float parameter1, float parameter2)
{
	paramManager->setParameter(motion, parameter1, parameter2);
}

void SBAnimationState2D::addTriangle(std::string motion1, std::string motion2, std::string motion3)
{
	paramManager->addTriangle(motion1, motion2, motion3);
}

SBAnimationState3D::SBAnimationState3D() : SBAnimationState("unknown")
{
}


SBAnimationState3D::SBAnimationState3D(std::string name) : SBAnimationState(name)
{
	_dimension = "3D";
}

SBAnimationState3D::~SBAnimationState3D()
{
}


void SBAnimationState3D::addMotion(std::string motion, float parameter1, float parameter2, float parameter3)
{
	addSkMotion(motion);
	
	paramManager->setType(1);
	paramManager->setParameter(motion, parameter1, parameter2, parameter3);
}

void SBAnimationState3D::setParameter(std::string motion, float parameter1, float parameter2, float parameter3)
{
	paramManager->setParameter(motion, parameter1, parameter2, parameter3);
}

void SBAnimationState3D::addTetrahedron(std::string motion1, std::string motion2, std::string motion3,std::string motion4)
{
	paramManager->addTetrahedron(motion1, motion2, motion3, motion4);
}

}