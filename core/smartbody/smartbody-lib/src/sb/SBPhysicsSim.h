#ifndef _SBPhysicsSIM_H_ 
#define _SBPhysicsSIM_H_ 
#include <deque>
#include "SBColObject.h"
#include <sb/SBObject.h>
#include <sb/SBJoint.h>

namespace SmartBody
{

class SBPhysicsObjInterface : public SBTransformObjInterface
{
protected:
	bool          bHasPhysicsSim;
	bool          bHasCollisionSim;	
	SBTransform  globalTransform;	
	SBTransform  refTransform;

public:	
	virtual SBTransform& getGlobalTransform() { return globalTransform; }
	virtual void setGlobalTransform(SBTransform& rt);	

	void setGlobalTransform(const SrMat& gmat);
	SBTransform& getRefTransform() { return refTransform; }
	void setRefTransform(const SrMat& gmat);
	void setRefTransform(SBTransform& rt);

	virtual void enablePhysicsSim(bool bPhy) = 0;
	virtual void enableCollisionSim(bool bCol) = 0;

	bool hasPhysicsSim() { return bHasPhysicsSim; }
	bool hasCollisionSim() { return bHasCollisionSim; }	
	void hasPhysicsSim(bool phySim) { bHasPhysicsSim = phySim; }
	void hasCollisionSim(bool colSim) { bHasCollisionSim = colSim; }
	unsigned long getID();	
};

class SBPhysicsObj : public SBPhysicsObjInterface, public SmartBody::SBObject// abstraction for rigid objects in the physics engine
{
protected:	
	SBGeomObject* colObj;		
	float         objDensity;
	SrVec         externalForce, externalTorque;
	SrVec         linearVel, angularVel;	
	bool          initGeom;
public:
	SBPhysicsObj();
	~SBPhysicsObj();

	virtual void enablePhysicsSim(bool bPhy);
	virtual void enableCollisionSim(bool bCol);	
	virtual void updateSbmObj();
	virtual void updatePhySim();

	void setGeometry(SBGeomObject* obj);
	void changeGeometry(std::string& geomType, SrVec extends);

	SBGeomObject* getColObj() { return colObj; }
	float         getMass();	
	void          setMass(float mass);
	float         getDensity() { return objDensity; }
	void          setDensity(float density) { objDensity = density; }

	SrVec& getExternalTorque() { return externalTorque; }
	void   setExternalTorque(SrVec val) { externalTorque = val; }
	SrVec& getExternalForce() { return externalForce; }
	void   setExternalForce(SrVec val) { externalForce = val; }

	SrVec getLinearVel();
	void  setLinearVel(SrVec val);
	SrVec getAngularVel();
	void  setAngularVel(SrVec val);

	virtual void notify(SBSubject* subject);	
	virtual void handleCollision(SrVec contactPt, SBPhysicsObj* colObj);
};

class SbmJointObj;
class SBPhysicsCharacter;

class SBPhysicsJoint : public SmartBody::SBObject
{
protected:
	SBJoint* sbmJoint;
	SbmJointObj* parentObj;	
	SbmJointObj* childObj;
	SrQuat refQuat;	
	SrVec  refAngularVel;	
	SrVec jointTorque;
	float totalSupportMass; // all 	
public:
	SBPhysicsJoint(SBJoint* joint);
	~SBPhysicsJoint();
	SBJoint* getSBJoint() { return sbmJoint; }
	SrQuat& getRefQuat() { return refQuat; }
	void setRefQuat(SrQuat val) { refQuat = val; }
	SrVec& getRefAngularVel() { return refAngularVel; }
	void setRefAngularVel(SrVec val) { refAngularVel = val; }

	SrVec& getJointTorque() { return jointTorque; }
	void   setJointTorque(SrVec val) { jointTorque = val; }	
	SbmJointObj* getParentObj() const { return parentObj; }
	void setParentObj(SbmJointObj* val) { parentObj = val; }
	SbmJointObj* getChildObj() const { return childObj; }
	void setChildObj(SbmJointObj* val) { childObj = val; }

	unsigned long getID();

	virtual void notify(SBSubject* subject);

public:
	void updateTotalSupportMass();
	float getTotalSupportMass() { return totalSupportMass; }
};

struct CollisionRecord
{
	SrVec collisionPt;
	SrVec momentum; 
	SbmJointObj* hitJointObj;
	SBPhysicsObj* collider;
public:
	CollisionRecord& operator= (const CollisionRecord& rt);
};

class SbmJointObj : public SBPhysicsObj 
// Modeling each body part as a SbmPhyObj
{
protected:	
	SBPhysicsCharacter* phyChar;
	SBPhysicsJoint* phyJoint;	
public:
	SbmJointObj(SBPhysicsCharacter* phyc);
	~SbmJointObj();
	SrMat getRelativeOrientation();
	SBPhysicsJoint* getPhyJoint() { return phyJoint; 	}
	SBJoint*         getSBJoint() { return phyJoint->getSBJoint(); }
	SBPhysicsCharacter* getPhysicsCharacter() { return phyChar; }

	SBPhysicsJoint* getChildJoint(int i);
	int getNumChildJoints();

	SbmJointObj* getParentObj() { return phyJoint->getParentObj(); }
	//void setParentObj(SbmJointObj* parent) { parentObj = parent; }
	virtual void initJoint(SBPhysicsJoint* joint);
	virtual void handleCollision(SrVec contactPt, SBPhysicsObj* colObj);
};


class SBPhysicsCharacter : public SBPhysicsObjInterface, public SmartBody::SBObject // interface for articulated dynamic character 
{
protected:	
	SBPhysicsJoint* root;
	std::map<std::string, SBPhysicsJoint*> jointMap;
	std::map<std::string, SbmJointObj*>     jointObjMap;
	//std::map<std::string, SBGeomObject*>   jointGeometryMap;
	std::string characterName;	
	std::vector<CollisionRecord> collisionRecords;
public:
	SBPhysicsCharacter();
	virtual void initPhysicsCharacter(std::string& charName, std::vector<std::string>& jointNameList, bool buildGeometry = false);	

	virtual void enablePhysicsSim(bool bPhy);
	virtual void enableCollisionSim(bool bCol);
	std::vector<CollisionRecord>& getCollisionRecords() { return collisionRecords; }

	std::string getPhysicsCharacterName() { return characterName; }		
	SbmJointObj* getJointObj(const std::string& jointName); // get body part associated with this joint
	SBPhysicsJoint* getPhyJoint(const std::string& jointName);
	SBPhysicsJoint* getPhyJointRoot() { return root; }
	std::vector<SbmJointObj*> getJointObjList();
	std::vector<SBPhysicsJoint*> getPhyJointList();
	std::map<std::string,SbmJointObj*>& getJointObjMap();
	virtual void notify(SBSubject* subject);
	void updatePDTorque();
protected:
	void cleanUpJoints();
	SBGeomObject* createJointGeometry(SBJoint* joint, float radius = -1);
	void updateJointAxis(SBPhysicsJoint* phyJoint);
	SrVec computePDTorque(SrQuat& q, SrQuat& qD, SrVec& w, SrVec& vD, float Ks, float Kd);
	SrVec computeSPDTorque(SrQuat& q, SrQuat& qD, SrVec& w, SrVec& vD, float Ks, float Kd, float dt, float mass);
};

//typedef std::deque<SBPhysicsObj*> SBPhysicsObjList;
typedef std::map<unsigned long, SBPhysicsJoint*> SBPhysicsJointMap;
typedef std::map<unsigned long,SBPhysicsObj*> SBPhysicsObjMap;
typedef std::map<std::string, SBPhysicsCharacter*> SBPhysicsCharacterMap;
typedef std::map<std::string, SBPhysicsObj*> SBPhysicsPawnMap;
class SBPhysicsSim : public SmartBody::SBObject
{
friend class SbmPhyObj;
protected:
	SBPhysicsObjMap physicsObjList;		
	SBPhysicsJointMap physicsJointList;
	SBPhysicsCharacterMap characterMap;
	SBPhysicsPawnMap      pawnObjMap;
public:
	SBPhysicsSim(void);
	~SBPhysicsSim(void);		
	void updateSimulation(float timestep);
	void setEnable(bool enable);
	void setGravity(float gravity);	
	virtual bool hasPhysicsObj(SBPhysicsObj* obj);
	virtual bool hasPhysicsCharacter(SBPhysicsCharacter* phyChar);	
	SBPhysicsCharacterMap& getCharacterMap() { return characterMap; }
	SBPhysicsPawnMap&       getPawnObjMap() { return pawnObjMap; }
	SBPhysicsCharacter* getPhysicsCharacter(const std::string& charName);
	SBPhysicsObj*       getPhysicsPawn(const std::string& pawnName);

	virtual void addPhysicsObj(SBPhysicsObj* obj);
	virtual void removePhysicsObj(SBPhysicsObj* obj);
	virtual void addPhysicsCharacter(SBPhysicsCharacter* phyChar);	
	virtual void removePhysicsCharacter(SBPhysicsCharacter* phyChar) = 0;	

	virtual void enablePhysicsSim(SBPhysicsObj* obj, bool bSim) = 0;
	virtual void enableCollisionSim(SBPhysicsObj* obj, bool bCol) = 0;	

	virtual void writeToPhysicsObj(SBPhysicsObj* obj) = 0; // write sim data to colObj
	virtual void readFromPhysicsObj(SBPhysicsObj* obj) = 0; // read sim data from colObj	

	void updateAllPhysicsJoints();
	virtual void updatePhysicsJoint(SBPhysicsJoint* phyJoint) = 0; // update joint parameters		
	virtual void updatePhyObjGeometry(SBPhysicsObj* obj, SBGeomObject* geom = NULL) = 0;

	//virtual void applyTorque(SBJoint* joint, )

	virtual SrVec getJointConstraintPos(SBPhysicsJoint* joint) = 0;
	virtual SrVec getJointRotationAxis(SBPhysicsJoint* joint, int axis) = 0;

	virtual void initSimulation() = 0;		
	virtual void updateSimulationInternal(float timeStep) = 0;
	virtual SBPhysicsObj* createPhyObj() = 0;	

	virtual void notify(SBSubject* subject);
	static SBPhysicsSim* getPhysicsEngine();
};

}


#endif

