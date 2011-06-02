#include "SbmPhysicsSim.h"

SbmPhysicsSim::SbmPhysicsSim(void)
{
}

SbmPhysicsSim::~SbmPhysicsSim(void)
{
}

SbmPhysicsObj::SbmPhysicsObj()
{
	colObj = NULL;
	objMass = 0.f;
	bHasPhysicsSim = true;
	bHasCollisionSim = true;
}