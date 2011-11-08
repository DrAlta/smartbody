#include "SBState.h"

SBState::SBState() : PAStateData()
{
}

SBState::SBState(std::string name) : PAStateData(name)
{
}

SBState::~SBState()
{
}

void SBState::addCorrespondancePoints(std::vector<std::string> motions, std::vector<double> points)
{
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

void SBState2D::addMotion(std::string motion, float parameter1, float paramter2)
{
}

void SBState2D::addTriangle(std::string motion1, std::string motion2, std::string motion3)
{
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


void SBState3D::addMotion(std::string motion, float parameter1, float paramter2, float paramter3)
{
}

void SBState3D::addTetrahedron(std::string motion1, std::string motion2, std::string motion3,std::string motion4)
{
}


