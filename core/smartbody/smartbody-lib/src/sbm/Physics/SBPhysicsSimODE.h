#ifndef _SBPhysicsSIMODE_H_
#define _SBPhysicsSIMODE_H_

#include <ode/ode.h>
#include <ode/common.h>
#include "SBPhysicsSim.h"

class SbmODEObj
{
public:
	dBodyID bodyID;
	dGeomID geomID;
	dMass   odeMass;
	dTriMeshDataID meshdataID; // optional mesh data ID
	SmartBody::SBPhysicsObj* physicsObj; // rigid body
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
	SmartBody::SBPhysicsJoint* joint;
public:
	SbmODEJoint();
	~SbmODEJoint();
};

typedef std::map<unsigned long,SbmODEObj*> SbmODEObjMap;
typedef std::map<unsigned long, SbmODEJoint*> SbmODEJointMap;

class SBPhysicsSimODE :
	public SmartBody::SBPhysicsSim
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
	SBPhysicsSimODE(void);
	~SBPhysicsSimODE(void);
	dWorldID getWorldID() { return worldID; }
	dSpaceID getSpaceID() { return spaceID; }	
	dJointGroupID getContactGroupID() { return contactGroupID; }
	bool   systemIsInit() { return hasInit; }
public:	
	virtual void initSimulation();	
	
	virtual void addPhysicsObj(SmartBody::SBPhysicsObj* obj);	
	virtual void removePhysicsObj(SmartBody::SBPhysicsObj* obj);
	virtual void addPhysicsCharacter(SmartBody::SBPhysicsCharacter* phyChar);
	virtual void removePhysicsCharacter(SmartBody::SBPhysicsCharacter* phyChar);

	virtual void updateSimulationInternal(float timeStep);	
	virtual SmartBody::SBPhysicsObj* createPhyObj(); 	
	virtual SrVec getJointConstraintPos(SmartBody::SBPhysicsJoint* joint);
	virtual SrVec getJointRotationAxis(SmartBody::SBPhysicsJoint* joint, int axis);
	virtual void updatePhysicsJoint(SmartBody::SBPhysicsJoint* phyJoint); // update joint parameters		
	virtual void updatePhyObjGeometry(SmartBody::SBPhysicsObj* obj, SbmGeomObject* geom = NULL);
	virtual void enablePhysicsSim(SmartBody::SBPhysicsObj* obj, bool bSim);
	virtual void enableCollisionSim(SmartBody::SBPhysicsObj* obj, bool bCol);	
	virtual void writeToPhysicsObj(SmartBody::SBPhysicsObj* obj); // write sim data to colObj
	virtual void readFromPhysicsObj(SmartBody::SBPhysicsObj* obj); // read sim data from colObj	

public:
	static dGeomID createODERawGeometry(SbmGeomObject* geomObj); 	
	static void updateODEGeometryTransform(SbmGeomObject* geomObj, dGeomID geomID);
	static SBPhysicsSimODE* getODESim();
protected:
	static void updateODEMass(dMass& odeMass, SbmGeomObject* geomObj, float mass);
	static void nearCallBack(void *data, dGeomID o1, dGeomID o2);
	SbmODEObj* getODEObj(SmartBody::SBPhysicsObj* obj);
	SbmODEJoint* getODEJoint(SmartBody::SBPhysicsJoint* joint);
	dGeomID createODEGeometry(SmartBody::SBPhysicsObj* obj, float mass);
	void linkJointObj(SmartBody::SbmJointObj* obj);
};

class SbmCollisionSpaceODE : public SbmCollisionSpace
{
protected:
	dSpaceID spaceID;	
	std::map<std::string,dGeomID> odeGeomMap;
	std::map<dGeomID, std::string> odeGeomNameMap;	
	SbmCollisionPairList curCollisionPairs;
public:
	SbmCollisionSpaceODE();
	~SbmCollisionSpaceODE();

	virtual void addCollisionObjects(const std::string& objName);
	virtual void removeCollisionObjects(const std::string& objName);
	virtual void getPotentialCollisionPairs(SbmCollisionPairList& collisionPairs);
	static  void collisionSpaceNearCallBack(void *data, dGeomID o1, dGeomID o2);
	std::string getODEGeomName(dGeomID geomID);
	SbmCollisionPairList& getCurrentCollisionPairList();
protected:
	dGeomID getODEGeomID(const std::string& geomName);	
};

#endif

