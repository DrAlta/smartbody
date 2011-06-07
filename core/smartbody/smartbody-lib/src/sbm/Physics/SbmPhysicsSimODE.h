#pragma once
#include <ode/ode.h>
#include <ode/common.h>
#include "SbmPhysicsSim.h"

class SbmPhysicsObjODE : public SbmPhysicsObj
{
protected:
	dBodyID bodyID;
	dGeomID geomID;
	dMass   odeMass;
	dTriMeshDataID meshdataID; // optional mesh data ID
public:
	SbmPhysicsObjODE();
	~SbmPhysicsObjODE();

	virtual void updateColObj();
	virtual void updateSimObj();
	virtual void setPhysicsSim(bool bSim);	
	virtual void setCollisionSim(bool bCol);
	virtual void initGeometry(SbmGeomObject* obj, float density);
	virtual unsigned long  getID() { return (unsigned long)bodyID; }	
protected:
	void createODEGeometry(SbmGeomObject* obj, float mass);
};


class SbmPhysicsSimODE :
	public SbmPhysicsSim
{
protected:
	dWorldID worldID;
	dSpaceID spaceID;	
	dGeomID  groundID;
	dJointGroupID contactGroupID;
	bool   hasInit;
public:
	SbmPhysicsSimODE(void);
	~SbmPhysicsSimODE(void);
	dWorldID getWorldID() { return worldID; }
	dSpaceID getSpaceID() { return spaceID; }	
	dJointGroupID getContactGroupID() { return contactGroupID; }
	bool   systemIsInit() { return hasInit; }
public:	
	virtual void initSimulation();
	virtual void setGravity(float gravity);
	virtual void addPhysicsObj(SbmPhysicsObj* obj);
	virtual void removePhysicsObj(SbmPhysicsObj* obj);
	virtual void updateSimulation(float timeStep);	
	virtual SbmPhysicsObj* createPhyObj() { return new SbmPhysicsObjODE(); }
public:
	static SbmPhysicsSimODE* getODESim();
protected:
	static void nearCallBack(void *data, dGeomID o1, dGeomID o2);
};
