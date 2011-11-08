#ifndef _SBMPHYSICSSIMODE_H_
#define _SBMPHYSICSSIMODE_H_

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

	virtual void updateSbmObj();
	virtual void updatePhySim();
	virtual void setPhysicsSim(bool bSim);	
	virtual void setCollisionSim(bool bCol);
	virtual void initGeometry(SbmGeomObject* obj, float density);
	virtual unsigned long  getID() { return (unsigned long)bodyID; }	
protected:
	void createODEGeometry(SbmGeomObject* obj, float mass);
};

class SbmODEObj
{
public:
	dBodyID bodyID;
	dGeomID geomID;
	dMass   odeMass;
	dTriMeshDataID meshdataID; // optional mesh data ID
	SbmPhysicsObj* physicsObj; // rigid body
public:
	SbmODEObj();
	~SbmODEObj();
	void cleanGeometry();
};

typedef std::map<unsigned long,SbmODEObj*> SbmODEObjMap;

class SbmPhysicsSimODE :
	public SbmPhysicsSim
{
protected:
	dWorldID worldID;
	dSpaceID spaceID;	
	dGeomID  groundID;
	dJointGroupID contactGroupID;
	bool   hasInit;	
	SbmODEObjMap odeObjMap;
public:
	SbmPhysicsSimODE(void);
	~SbmPhysicsSimODE(void);
	dWorldID getWorldID() { return worldID; }
	dSpaceID getSpaceID() { return spaceID; }	
	dJointGroupID getContactGroupID() { return contactGroupID; }
	bool   systemIsInit() { return hasInit; }
public:	
	virtual void initSimulation();	
	
	virtual void addPhysicsObj(SbmPhysicsObj* obj);	
	virtual void removePhysicsObj(SbmPhysicsObj* obj);
	virtual void addPhysicsCharacter(SbmPhysicsCharacter* phyChar);
	virtual void removePhysicsCharacter(SbmPhysicsCharacter* phyChar);

	virtual void updateSimulationInternal(float timeStep);	
	virtual SbmPhysicsObj* createPhyObj(); 
	virtual SbmJointObj* createJointObj();

	virtual void updatePhyObjGeometry(SbmPhysicsObj* obj, SbmGeomObject* geom = NULL);
	virtual void enablePhysicsSim(SbmPhysicsObj* obj, bool bSim);
	virtual void enableCollisionSim(SbmPhysicsObj* obj, bool bCol);	
	virtual void updateSbmObj(SbmPhysicsObj* obj); // write sim data to colObj
	virtual void updatePhySim(SbmPhysicsObj* obj); // read sim data from colObj	

public:
	static SbmPhysicsSimODE* getODESim();
protected:
	static void nearCallBack(void *data, dGeomID o1, dGeomID o2);
	SbmODEObj* getODEObj(SbmPhysicsObj* obj);
	dGeomID createODEGeometry(SbmPhysicsObj* obj, float mass);
};
#endif

