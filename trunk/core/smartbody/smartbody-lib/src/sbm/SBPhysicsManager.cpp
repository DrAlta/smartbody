#include "SBPhysicsManager.h"
#include <sbm/mcontrol_util.h>

namespace SmartBody {

SBPhysicsManager::SBPhysicsManager()
{
	setName("physics");

	physicsTime = 0;
	_ode = new SbmPhysicsSimODE();
	_ode->initSimulation();
}

SBPhysicsManager::~SBPhysicsManager()
{
	delete _ode;
}

SbmPhysicsSim* SBPhysicsManager::getPhysicsEngine()
{
	return _ode;
}


bool SBPhysicsManager::isEnable()
{
	return getPhysicsEngine()->getBoolAttribute("enable");
}

void SBPhysicsManager::setEnable(bool enable)
{
	if (enable)
	{
		// ...
		physicsTime = mcuCBHandle::singleton().time;		
	}
	else
	{
		// ...
	}
	getPhysicsEngine()->setBoolAttribute("enable",enable);
	
}

void SBPhysicsManager::start()
{
}

void SBPhysicsManager::beforeUpdate(double time)
{

}

void SBPhysicsManager::update(double time)
{
	SbmPhysicsSim* physicsEngine = getPhysicsEngine();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (isEnable())
	{
		float dt = (float)physicsEngine->getDoubleAttribute("dT");//timeStep*0.03f;	
		while (physicsTime < time)			
		{	
			physicsEngine->updateSimulation(dt);
			physicsTime += dt;
		}	
	}

	// update character
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		 iter != mcu.getCharacterMap().end();
		 iter++)
	{
		SbmCharacter* character = (*iter).second;
		character->updateJointPhyObjs(isEnable());
	}
}

void SBPhysicsManager::afterUpdate(double time)
{
}

void SBPhysicsManager::stop()
{
}

}

