#include "SbmPhysicsSim.h"

SbmPhysicsSim::SbmPhysicsSim(void)
{
	DObject::createDoubleAttribute("gravity",980, true, "Basic", 20, false, false, false, "?");
	DObject::createBoolAttribute("enable",false,true, "Basic", 20, false, false, false, "?");	
}

SbmPhysicsSim::~SbmPhysicsSim(void)
{
}

void SbmPhysicsSim::updateSimulation( float timestep )
{
	bool enableSim = DObject::getBoolAttribute("enable");
	if (enableSim)
	{
		updateSimulationInternal(timestep);
	}
}

void SbmPhysicsSim::setEnable( bool enable )
{
	DObject::setBoolAttribute("enable",enable);
}

void SbmPhysicsSim::setGravity( float gravity )
{
	DObject::setDoubleAttribute("gravity",gravity);
}

SbmPhysicsObj::SbmPhysicsObj()
{
	colObj = NULL;
	objMass = 0.f;
	bHasPhysicsSim = true;
	bHasCollisionSim = true;	
}