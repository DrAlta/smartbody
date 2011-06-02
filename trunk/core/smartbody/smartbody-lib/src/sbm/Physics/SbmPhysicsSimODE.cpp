#include "SbmPhysicsSimODE.h"
#include <sbm/mcontrol_util.h>

/************************************************************************/
/* Physics Obj ODE                                                      */
/************************************************************************/

SbmPhysicsObjODE::SbmPhysicsObjODE()
{
	bodyID = 0;
	geomID = 0;
}

SbmPhysicsObjODE::~SbmPhysicsObjODE()
{
	dBodyDestroy(bodyID);
	dGeomDestroy(geomID);
}

void SbmPhysicsObjODE::setPhysicsSim( bool bSim )
{
	bHasPhysicsSim = bSim;
	if (bHasPhysicsSim)
		dBodyEnable(bodyID);
	else
		dBodyDisable(bodyID);
}


void SbmPhysicsObjODE::setCollisionSim( bool bCol )
{
	bHasCollisionSim = bCol;
	if (bHasCollisionSim)
		dGeomEnable(geomID);
	else
		dGeomDisable(geomID);
}


void SbmPhysicsObjODE::updateColObj()
{
	if (!colObj || !hasPhysicsSim())
		return;
	// write the current simulation result into object world state
	SbmTransform& curT = colObj->worldState;
	const dReal* quat = dBodyGetQuaternion(bodyID);
	curT.rot.w = quat[0];
	curT.rot.x = quat[1];
	curT.rot.y = quat[2];
	curT.rot.z = quat[3];

	const dReal* pos = dBodyGetPosition(bodyID);
	curT.tran[0] = pos[0];
	curT.tran[1] = pos[1];
	curT.tran[2] = pos[2];
	//sr_out << "new pos = " << curT.tran << srnl;
}

void SbmPhysicsObjODE::updateSimObj()
{
	if (!colObj)
		return;

	SbmTransform& curT = colObj->worldState;
	dQuaternion quat;
	quat[0] = (dReal)curT.rot.w;	
	quat[1] = (dReal)curT.rot.x;
	quat[2] = (dReal)curT.rot.y;
	quat[3] = (dReal)curT.rot.z;
	dBodySetQuaternion(bodyID,quat);
	dBodySetPosition(bodyID,(dReal)curT.tran[0],(dReal)curT.tran[1],(dReal)curT.tran[2]);
}

void SbmPhysicsObjODE::initGeometry( SbmGeomObject* obj, float mass )
{
	SbmPhysicsSimODE* odeSim = SbmPhysicsSimODE::getODESim();
	if (!odeSim)	return;
	if (!odeSim->systemIsInit())   return;

	bodyID = dBodyCreate(odeSim->getWorldID());
	if (obj)
	{
		SbmTransform& curT = obj->worldState;
		dQuaternion quat;
		quat[0] = (dReal)curT.rot.w;	
		quat[1] = (dReal)curT.rot.x;
		quat[2] = (dReal)curT.rot.y;
		quat[3] = (dReal)curT.rot.z;
		dBodySetQuaternion(bodyID,quat);
		dBodySetPosition(bodyID,(dReal)curT.tran[0],(dReal)curT.tran[1],(dReal)curT.tran[2]);
		
		colObj = obj;
		objMass = mass;		
		createODEGeometry(obj,mass);
		dBodySetMass(bodyID,&odeMass);
		if (geomID)
			dGeomSetBody(geomID,bodyID);
		//dBodySetAutoDisableFlag(bodyID,1);
	}	
}

void SbmPhysicsObjODE::createODEGeometry( SbmGeomObject* obj, float mass )
{
	SbmPhysicsSimODE* odeSim = SbmPhysicsSimODE::getODESim();
	if (!odeSim)	return;
	if (!odeSim->systemIsInit())   return;

	if (dynamic_cast<SbmGeomSphere*>(obj))
	{
		SbmGeomSphere* sph = dynamic_cast<SbmGeomSphere*>(obj);
		geomID = dCreateSphere(odeSim->getSpaceID(),(dReal)sph->radius);
		//dMassSetSphereTotal(&odeMass,mass,(dReal)sph->radius);
		dMassSetSphere(&odeMass,mass,(dReal)sph->radius);
	}
	else if (dynamic_cast<SbmGeomBox*>(obj))
	{
		SbmGeomBox* box = dynamic_cast<SbmGeomBox*>(obj);
		SrVec extent = box->extent*2.f;
		geomID = dCreateBox(odeSim->getSpaceID(),(dReal)extent.x,(dReal)extent.y,(dReal)extent.z);
		//dMassSetBoxTotal(&odeMass,(dReal)1.f,(dReal)extent.x,(dReal)extent.y,(dReal)extent.z);
		dMassSetBox(&odeMass,(dReal)mass,(dReal)extent.x,(dReal)extent.y,(dReal)extent.z);
	}
	else if (dynamic_cast<SbmGeomCapsule*>(obj))
	{
		SbmGeomCapsule* cap = dynamic_cast<SbmGeomCapsule*>(obj);		
		geomID = dCreateCapsule(odeSim->getSpaceID(),(dReal)cap->radius,(dReal)cap->extent);
		//dMassSetCapsuleTotal(&odeMass,(dReal)mass,3,(dReal)cap->radius,(dReal)cap->extent);
		dMassSetCapsule(&odeMass,(dReal)mass,3,(dReal)cap->radius,(dReal)cap->extent);
	}
	else // no geoemtry
	{
		//dMassSetZero(&odeMass);			
		dMassAdjust(&odeMass,(dReal)mass);
	}
}
/************************************************************************/
/* Physics Sim ODE                                                      */
/************************************************************************/

SbmPhysicsSimODE::SbmPhysicsSimODE(void)
{
	hasInit = false;
	initSimulation();
}
SbmPhysicsSimODE::~SbmPhysicsSimODE(void)
{
}

void SbmPhysicsSimODE::nearCallBack(void *data, dGeomID o1, dGeomID o2)
{
	const int N = 10;
	SbmPhysicsSimODE* phyODE = static_cast<SbmPhysicsSimODE*>(data);
	if (!phyODE)
		return;

	dContact contact[N];
	dBodyID b1,b2;
	b1 = dGeomGetBody(o1);
	b2 = dGeomGetBody(o2);
	if (b1 && !dBodyIsEnabled(b1)) b1 = 0;
	if (b2 && !dBodyIsEnabled(b2)) b2 = 0;
	if (!b1 && !b2) // both bodies are disable, no need for collision check
		return;

	int n =  dCollide(o1,o2,N,&contact[0].geom,sizeof(dContact));
	for (int i = 0; i < n; i++) {
		contact[i].surface.mode = dContactBounce | dContactApprox1;		
		contact[i].surface.bounce     = 0.2f; // (0.0~1.0) restitution parameter
		contact[i].surface.mu = 0.005f;//1000.f;
		//contact[i].surface.mu2 = 3000.f;
		//contact[i].surface.soft_cfm = 0.01f;
		//contact[i].surface.soft_erp = 0.0001f;
		//contact[i].surface.bounce_vel = 1000.f; // minimum incoming velocity for bounce 
		dJointID c = dJointCreateContact(phyODE->getWorldID(),phyODE->getContactGroupID(),&contact[i]);
		dJointAttach (c,b1,b2);	
	}
}

void SbmPhysicsSimODE::initSimulation()
{	
	dInitODE();

	worldID = dWorldCreate();
	//dWorldSetAutoDisableFlag(worldID,1);
	dWorldSetGravity(worldID,0.f,-9.8f,0.f);
	dWorldSetLinearDamping(worldID,0.01f);
	dWorldSetAngularDamping(worldID,0.01f);

	spaceID = dHashSpaceCreate(0);

	groundID = dCreatePlane(spaceID,0,1,0,1.0f); // create a plane at y = 0

	contactGroupID = dJointGroupCreate(0);

	hasInit = true;
}

void SbmPhysicsSimODE::updateSimulation( float timeStep )
{	
	dSpaceCollide(spaceID,this,SbmPhysicsSimODE::nearCallBack);
	//dWorldStep(worldID,timeStep);
	dWorldQuickStep(worldID,timeStep);	
	dJointGroupEmpty(contactGroupID);

	std::for_each(physicsObjList.begin(),physicsObjList.end(),std::mem_fun(&SbmPhysicsObj::updateColObj));
}

SbmPhysicsSimODE* SbmPhysicsSimODE::getODESim()
{
	SbmPhysicsSimODE* odePhysics = dynamic_cast<SbmPhysicsSimODE*>(mcuCBHandle::singleton().physicsEngine);
	return odePhysics;
}

void SbmPhysicsSimODE::addPhysicsObj( SbmPhysicsObj* obj )
{
	physicsObjList.push_back(obj);
}

void SbmPhysicsSimODE::removePhysicsObj( SbmPhysicsObj* obj )
{
	SbmPhysicsObjList::iterator li = std::find(physicsObjList.begin(),physicsObjList.end(),obj);
	if (li != physicsObjList.end())
		physicsObjList.erase(li);
}

void SbmPhysicsSimODE::setGravity( float gravity )
{
	dWorldSetGravity(worldID,0.f,-fabs(gravity),0.f);	
}