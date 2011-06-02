#pragma once
#include <deque>
#include "SbmColObject.h"

class SbmPhysicsObj // abstraction for objects in the physics engine
{
protected:
	SbmGeomObject* colObj;
	float         objMass;
	bool          bHasPhysicsSim;
	bool          bHasCollisionSim;
public:
	SbmPhysicsObj();
	~SbmPhysicsObj() {}	
	SbmGeomObject* getColObj() { return colObj; }
	float         getMass() { return objMass; }
	
	virtual void updateColObj() = 0; // write sim data to colObj
	virtual void updateSimObj() = 0; // read sim data from colObj
	virtual void setPhysicsSim(bool bSim) = 0;
	virtual void setCollisionSim(bool bCol) = 0;	
	virtual void initGeometry(SbmGeomObject* obj, float mass ) = 0;
	bool         hasPhysicsSim() { return bHasPhysicsSim; }
	bool         hasCollisionSim() { return bHasCollisionSim; }
	virtual unsigned long getID() = 0;
};

typedef std::deque<SbmPhysicsObj*> SbmPhysicsObjList;
class SbmPhysicsSim
{
protected:
	SbmPhysicsObjList physicsObjList;	
public:
	SbmPhysicsSim(void);
	~SbmPhysicsSim(void);	
	virtual void initSimulation() = 0;
	virtual void setGravity(float gravity) = 0;
	virtual void addPhysicsObj(SbmPhysicsObj* obj) = 0;
	virtual void removePhysicsObj(SbmPhysicsObj* obj) = 0;
	virtual void updateSimulation(float timeStep) = 0;
	virtual SbmPhysicsObj* createPhyObj() = 0;
};


