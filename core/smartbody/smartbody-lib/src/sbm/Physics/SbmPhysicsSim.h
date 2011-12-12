#ifndef _SBMPHYSICSSIM_H_ 
#define _SBMPHYSICSSIM_H_ 
#include <deque>
#include "SbmColObject.h"
#include <sbm/SBObject.h>
#include <sbm/SBJoint.h>

using namespace SmartBody;

class SbmPhysicsObjInterface
{
protected:
	bool          bHasPhysicsSim;
	bool          bHasCollisionSim;	
	SbmTransform  globalTransform;
public:	
	SbmTransform& getGlobalTransform() { return globalTransform; }
	void setGlobalTransform(const SrMat& gmat);
	void setGlobalTransform(SbmTransform& rt);	
	bool hasPhysicsSim() { return bHasPhysicsSim; }
	bool hasCollisionSim() { return bHasCollisionSim; }	
	void hasPhysicsSim(bool phySim) { bHasPhysicsSim = phySim; }
	void hasCollisionSim(bool colSim) { bHasCollisionSim = colSim; }
	unsigned long getID();
	
};

class SbmPhysicsObj : public SbmPhysicsObjInterface// abstraction for rigid objects in the physics engine
{
protected:
	SbmGeomObject* colObj;	
	float         objMass;	
	float         objDensity;
	SrVec         externalForce, externalTorque;
	SrVec         linearVel, angularVel;	
	
public:
	SbmPhysicsObj();
	~SbmPhysicsObj() {}	

	virtual void enablePhysicsSim(bool bPhy);
	virtual void enableCollisionSim(bool bCol);

	virtual void setGeometry(SbmGeomObject* obj, float density);
	virtual void updateSbmObj();
	virtual void updatePhySim();

	SbmGeomObject* getColObj() { return colObj; }
	float         getMass() { return objMass; }		
	void          setMass(float mass) { objMass = mass; }
	float         getDensity() { return objDensity; }
	void          setDensity(float density) { objDensity = density; }

	SrVec& getExternalTorque() { return externalTorque; }
	void   setExternalTorque(SrVec val) { externalTorque = val; }
	SrVec& getExternalForce() { return externalForce; }
	void   setExternalForce(SrVec val) { externalForce = val; }

	SrVec& getLinearVel() { return linearVel; }
	void   setLinearVel(SrVec val) { linearVel = val; }
	SrVec& getAngularVel() { return angularVel; }
	void   setAngularVel(SrVec val) { angularVel = val; }
};

class SbmJointObj;
class SbmPhysicsCharacter;

class SbmPhysicsJoint : public SmartBody::SBObject
{
protected:
	SBJoint* sbmJoint;
	SbmJointObj* parentObj;	
	SbmJointObj* childObj;
	SrVec jointTorque;
public:
	SbmPhysicsJoint(SBJoint* joint);
	~SbmPhysicsJoint();
	SBJoint* getSBJoint() { return sbmJoint; }

	SrVec& getJointTorque() { return jointTorque; }
	void   setJointTorque(SrVec val) { jointTorque = val; }	

	SbmJointObj* getParentObj() const { return parentObj; }
	void setParentObj(SbmJointObj* val) { parentObj = val; }
	SbmJointObj* getChildObj() const { return childObj; }
	void setChildObj(SbmJointObj* val) { childObj = val; }
	unsigned long getID();
};

class SbmJointObj : public SbmPhysicsObj 
// Modeling each body part as a SbmPhyObj
{
protected:	
	SbmPhysicsCharacter* phyChar;
	SbmPhysicsJoint* phyJoint;	
public:
	SbmJointObj(SbmPhysicsCharacter* phyc);
	~SbmJointObj();
	SrMat getRelativeOrientation();
	SbmPhysicsJoint* getPhyJoint() { return phyJoint; 	}
	SBJoint*         getSBJoint() { return phyJoint->getSBJoint(); }

	SbmPhysicsJoint* getChildJoint(int i);
	int getNumChildJoints();

	SbmJointObj* getParentObj() { return phyJoint->getParentObj(); }
	//void setParentObj(SbmJointObj* parent) { parentObj = parent; }
	virtual void initJoint(SbmPhysicsJoint* joint);
};

class SbmPhysicsCharacter : public SbmPhysicsObjInterface // interface for articulated dynamic character 
{
protected:	
	SbmJointObj* rootObj;
	std::map<std::string, SbmPhysicsJoint*> jointMap;
	std::map<std::string, SbmJointObj*>     jointObjMap;
	std::map<std::string, SbmGeomObject*>   jointGeometryMap;
	std::string characterName;
public:
	std::string getName() { return characterName; }
	virtual void initPhysicsCharacter(std::string& charName, std::vector<std::string>& jointNameList, bool buildGeometry = false);	
	SbmJointObj* getJointObj(const std::string& jointName); // get body part associated with this joint
	SbmPhysicsJoint* getPhyJoint(const std::string& jointName);
	std::vector<SbmJointObj*> getJointObjList();
	std::map<std::string,SbmJointObj*>& getJointObjMap();
protected:
	void cleanUpJoints();
	SbmGeomObject* createJointGeometry(SBJoint* joint, float radius = -1);
};

//typedef std::deque<SbmPhysicsObj*> SbmPhysicsObjList;
typedef std::map<unsigned long, SbmPhysicsJoint*> SbmPhysicsJointMap;
typedef std::map<unsigned long,SbmPhysicsObj*> SbmPhysicsObjMap;
typedef std::map<std::string, SbmPhysicsCharacter*> SbmPhysicsCharacterMap;
class SbmPhysicsSim : public SmartBody::SBObject
{
friend class SbmPhyObj;
protected:
	SbmPhysicsObjMap physicsObjList;	
	SbmPhysicsJointMap physicsJointList;
	SbmPhysicsCharacterMap characterMap;
public:
	SbmPhysicsSim(void);
	~SbmPhysicsSim(void);		
	void updateSimulation(float timestep);
	void setEnable(bool enable);
	void setGravity(float gravity);	
	virtual bool hasPhysicsObj(SbmPhysicsObj* obj);
	virtual bool hasPhysicsCharacter(SbmPhysicsCharacter* phyChar);

	virtual void addPhysicsObj(SbmPhysicsObj* obj);
	virtual void removePhysicsObj(SbmPhysicsObj* obj);
	virtual void addPhysicsCharacter(SbmPhysicsCharacter* phyChar) = 0;	
	virtual void removePhysicsCharacter(SbmPhysicsCharacter* phyChar) = 0;	

	virtual void enablePhysicsSim(SbmPhysicsObj* obj, bool bSim) = 0;
	virtual void enableCollisionSim(SbmPhysicsObj* obj, bool bCol) = 0;	
	virtual void updateSbmObj(SbmPhysicsObj* obj) = 0; // write sim data to colObj
	virtual void updatePhySim(SbmPhysicsObj* obj) = 0; // read sim data from colObj	
	virtual void updatePhyObjGeometry(SbmPhysicsObj* obj, SbmGeomObject* geom = NULL) = 0;

	//virtual void applyTorque(SBJoint* joint, )

	virtual SrVec getJointConstraintPos(SbmPhysicsJoint* joint) = 0;

	virtual void initSimulation() = 0;		
	virtual void updateSimulationInternal(float timeStep) = 0;
	virtual SbmPhysicsObj* createPhyObj() = 0;	
};


#endif

