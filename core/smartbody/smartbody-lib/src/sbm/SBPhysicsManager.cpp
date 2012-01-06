#include "SBPhysicsManager.h"

namespace SmartBody {

SBPhysicsManager::SBPhysicsManager()
{
	setName("physics");

	_ode = new SbmPhysicsSimODE();
	_ode->initSimulation();
}

SBPhysicsManager::~SBPhysicsManager()
{
	delete _ode;
}

void SBPhysicsManager::setEnable(bool enable)
{
	if (enable)
	{
		// ...
	}
	else
	{
		// ...
	}
}

void SBPhysicsManager::start()
{
}

void SBPhysicsManager::beforeUpdate(double time)
{
}

void SBPhysicsManager::update(double time)
{
}

void SBPhysicsManager::afterUpdate(double time)
{
}

void SBPhysicsManager::stop()
{
}

}

