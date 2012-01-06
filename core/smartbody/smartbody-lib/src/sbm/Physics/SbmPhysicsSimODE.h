#ifndef _SBMPHYSICSSIMODE_H_
#define _SBMPHYSICSSIMODE_H_

#include <ode/ode.h>
#include <ode/common.h>
#include "SbmPhysicsSim.h"

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

class SbmODEJoint
{
public:
	dJointID jointID;
	dJointID aMotorID;
	dBodyID  parentID, childID;
	SbmPhysicsJoint* joint;
public:
	SbmODEJoint();
	~SbmODEJoint();
};

typedef std::map<unsigned long,SbmODEObj*> SbmODEObjMap;
typedef std::map<unsigned long, SbmODEJoint*> SbmODEJointMap;

class SbmPhysicsSimODE :
	public SbmPhysicsSim
{
public:
	dGeomID  groundID;
protected:
	dWorldID worldID;
	dSpaceID spaceID;		
	dJointGroupID contactGroupID;
	bool   hasInit;	
	SbmODEObjMap odeObjMap;
	SbmODEJointMap odeJointMap;
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

	virtual SrVec getJointConstraintPos(SbmPhysicsJoint* joint);
	virtual SrVec getJointRotationAxis(SbmPhysicsJoint* joint, int axis);

	virtual void updatePhysicsJoint(SbmPhysicsJoint* phyJoint); // update joint parameters		

	virtual void updatePhyObjGeometry(SbmPhysicsObj* obj, SbmGeomObject* geom = NULL);
	virtual void enablePhysicsSim(SbmPhysicsObj* obj, bool bSim);
	virtual void enableCollisionSim(SbmPhysicsObj* obj, bool bCol);	
	virtual void writeToPhysicsObj(SbmPhysicsObj* obj); // write sim data to colObj
	virtual void readFromPhysicsObj(SbmPhysicsObj* obj); // read sim data from colObj	

public:
	static SbmPhysicsSimODE* getODESim();
protected:
	static void nearCallBack(void *data, dGeomID o1, dGeomID o2);
	SbmODEObj* getODEObj(SbmPhysicsObj* obj);
	SbmODEJoint* getODEJoint(SbmPhysicsJoint* joint);
	dGeomID createODEGeometry(SbmPhysicsObj* obj, float mass);
	void linkJointObj(SbmJointObj* obj);
};
#endif

