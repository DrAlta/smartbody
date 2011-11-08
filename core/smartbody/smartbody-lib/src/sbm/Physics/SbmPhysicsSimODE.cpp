#include "SbmPhysicsSimODE.h"
#include <sbm/mcontrol_util.h>

/************************************************************************/
/* Physics Obj ODE                                                      */
/************************************************************************/

SbmPhysicsObjODE::SbmPhysicsObjODE()
{
	bodyID = 0;
	geomID = 0;
	meshdataID = 0;
}

SbmPhysicsObjODE::~SbmPhysicsObjODE()
{
// 	SbmPhysicsSimODE* odeSim = SbmPhysicsSimODE::getODESim();
// 	if (odeSim)
// 	{
// 		dSpaceRemove(odeSim->getSpaceID(),geomID);	
// 	}
	dGeomDestroy(geomID);
	dGeomTriMeshDataDestroy(meshdataID);	
	dBodyDestroy(bodyID);	
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
	SbmTransform curT;// = colObj->getGlobalTransform();
	const dReal* quat = dBodyGetQuaternion(bodyID);
	curT.rot.w = quat[0];
	curT.rot.x = quat[1];
	curT.rot.y = quat[2];
	curT.rot.z = quat[3];

	const dReal* pos = dBodyGetPosition(bodyID);
	curT.tran[0] = pos[0];
	curT.tran[1] = pos[1];
	curT.tran[2] = pos[2];
	colObj->setWorldState(curT);
	//sr_out << "new pos = " << curT.tran << srnl;
}

void SbmPhysicsObjODE::updateSimObj()
{
	if (!colObj)
		return;

	SbmTransform& curT = colObj->getWorldState();
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
		SbmTransform& curT = obj->getWorldState();
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
	else if (dynamic_cast<SbmGeomTriMesh*>(obj))
	{
		SbmGeomTriMesh* tri = dynamic_cast<SbmGeomTriMesh*>(obj);
		SrModel* model = tri->geoMesh;
		//model->invert_faces();
		SrBox bbox;		
		model->get_bounding_box(bbox);		
		meshdataID = dGeomTriMeshDataCreate();
		//dGeomTriMeshDataBuildSimple(meshdataID,(const dReal*)(&model->V[0]),model->V.size(),(const dTriIndex*)(&model->F[0]),model->F.size()*3);
		dGeomTriMeshDataBuildSingle(meshdataID,(const dReal*)(&model->V[0]),3*sizeof(float),model->V.size(),(const dTriIndex*)(&model->F[0]),model->F.size()*3,3*sizeof(int));
		geomID = dCreateTriMesh(odeSim->getSpaceID(),meshdataID,NULL,NULL,NULL);		
		dMassSetTrimesh(&odeMass,(dReal)mass,geomID);		
		if (dMassCheck(&odeMass) != 1) // set the default mass to its bounding box
			dMassSetBox(&odeMass,(dReal)mass,bbox.size().x*0.5f,bbox.size().y*0.5f,bbox.size().z*0.5f);		
		//dGeomSetPosition(geomID,-odeMass.c[0], -odeMass.c[1], -odeMass.c[2]);
		dMassTranslate( &odeMass, -odeMass.c[0], -odeMass.c[1], -odeMass.c[2]);		
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
	const int N = 40;
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
		contact[i].surface.bounce     = 0.00f; // (0.0~1.0) restitution parameter
		contact[i].surface.mu = 5000.f;//1000.f;
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

	//spaceID = dHashSpaceCreate(0);
	spaceID = dSimpleSpaceCreate(0);

	groundID = dCreatePlane(spaceID,0,1,0,1.0f); // create a plane at y = 0

	contactGroupID = dJointGroupCreate(0);

	hasInit = true;
}

void SbmPhysicsSimODE::updateSimulationInternal( float timeStep )
{	
	float gravity = (float)SBObject::getDoubleAttribute("gravity");
	dWorldSetGravity(worldID,0.f,-fabs(gravity),0.f);	

	dSpaceCollide(spaceID,this,SbmPhysicsSimODE::nearCallBack);		
	//dWorldStep(worldID,timeStep);
	//dWorldStepFast1(worldID,timeStep,10);
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
