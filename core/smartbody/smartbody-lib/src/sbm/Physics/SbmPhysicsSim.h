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
	SbmTransform  refTransform;
	
public:	
	SbmTransform& getGlobalTransform() { return globalTransform; }
	void setGlobalTransform(const SrMat& gmat);
	void setGlobalTransform(SbmTransform& rt);	

	SbmTransform& getRefTransform() { return refTransform; }
	void setRefTransform(const SrMat& gmat);
	void setRefTransform(SbmTransform& rt);

	virtual void enablePhysicsSim(bool bPhy) = 0;
	virtual void enableCollisionSim(bool bCol) = 0;

	bool hasPhysicsSim() { return bHasPhysicsSim; }
	bool hasCollisionSim() { return bHasCollisionSim; }	
	void hasPhysicsSim(bool phySim) { bHasPhysicsSim = phySim; }
	void hasCollisionSim(bool colSim) { bHasCollisionSim = colSim; }
	unsigned long getID();	
};

class SbmPhysicsObj : public SbmPhysicsObjInterface, public SmartBody::SBObject// abstraction for rigid objects in the physics engine
{
protected:	
	SbmGeomObject* colObj;		
	float         objDensity;
	SrVec         externalForce, externalTorque;
	SrVec         linearVel, angularVel;	
	bool          initGeom;
public:
	SbmPhysicsObj();
	~SbmPhysicsObj() {}	

	virtual void enablePhysicsSim(bool bPhy);
	virtual void enableCollisionSim(bool bCol);	
	virtual void updateSbmObj();
	virtual void updatePhySim();

	void setGeometry(SbmGeomObject* obj);
	void changeGeometry(std::string& geomType, SrVec extends);

	SbmGeomObject* getColObj() { return colObj; }
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
};

class SbmJointObj;
class SbmPhysicsCharacter;

class SbmPhysicsJoint : public SmartBody::SBObject
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
	SbmPhysicsJoint(SBJoint* joint);
	~SbmPhysicsJoint();
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

class SbmPhysicsCharacter : public SbmPhysicsObjInterface, public SmartBody::SBObject // interface for articulated dynamic character 
{
protected:	
	SbmPhysicsJoint* root;
	std::map<std::string, SbmPhysicsJoint*> jointMap;
	std::map<std::string, SbmJointObj*>     jointObjMap;
	std::map<std::string, SbmGeomObject*>   jointGeometryMap;
	std::string characterName;
public:
	SbmPhysicsCharacter();
	virtual void initPhysicsCharacter(std::string& charName, std::vector<std::string>& jointNameList, bool buildGeometry = false);	

	virtual void enablePhysicsSim(bool bPhy);
	virtual void enableCollisionSim(bool bCol);

	std::string getPhysicsCharacterName() { return characterName; }		
	SbmJointObj* getJointObj(const std::string& jointName); // get body part associated with this joint
	SbmPhysicsJoint* getPhyJoint(const std::string& jointName);
	SbmPhysicsJoint* getPhyJointRoot() { return root; }
	std::vector<SbmJointObj*> getJointObjList();
	std::vector<SbmPhysicsJoint*> getPhyJointList();
	std::map<std::string,SbmJointObj*>& getJointObjMap();
	virtual void notify(SBSubject* subject);

	void updatePDTorque();
protected:
	void cleanUpJoints();
	SbmGeomObject* createJointGeometry(SBJoint* joint, float radius = -1);
	void updateJointAxis(SbmPhysicsJoint* phyJoint);
	SrVec computePDTorque(SrQuat& q, SrQuat& qD, SrVec& w, SrVec& vD, float Ks, float Kd);
	SrVec computeSPDTorque(SrQuat& q, SrQuat& qD, SrVec& w, SrVec& vD, float Ks, float Kd, float dt, float mass);
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
	SbmPhysicsCharacterMap& getCharacterMap() { return characterMap; }
	SbmPhysicsObjMap&       getPhysicsObjMap() { return physicsObjList; }
	SbmPhysicsCharacter* getPhysicsCharacter(std::string& charName);

	virtual void addPhysicsObj(SbmPhysicsObj* obj);
	virtual void removePhysicsObj(SbmPhysicsObj* obj);
	virtual void addPhysicsCharacter(SbmPhysicsCharacter* phyChar);	
	virtual void removePhysicsCharacter(SbmPhysicsCharacter* phyChar) = 0;	

	virtual void enablePhysicsSim(SbmPhysicsObj* obj, bool bSim) = 0;
	virtual void enableCollisionSim(SbmPhysicsObj* obj, bool bCol) = 0;	

	virtual void writeToPhysicsObj(SbmPhysicsObj* obj) = 0; // write sim data to colObj
	virtual void readFromPhysicsObj(SbmPhysicsObj* obj) = 0; // read sim data from colObj	

	void updateAllPhysicsJoints();
	virtual void updatePhysicsJoint(SbmPhysicsJoint* phyJoint) = 0; // update joint parameters		
	virtual void updatePhyObjGeometry(SbmPhysicsObj* obj, SbmGeomObject* geom = NULL) = 0;

	//virtual void applyTorque(SBJoint* joint, )

	virtual SrVec getJointConstraintPos(SbmPhysicsJoint* joint) = 0;
	virtual SrVec getJointRotationAxis(SbmPhysicsJoint* joint, int axis) = 0;

	virtual void initSimulation() = 0;		
	virtual void updateSimulationInternal(float timeStep) = 0;
	virtual SbmPhysicsObj* createPhyObj() = 0;	

	virtual void notify(SBSubject* subject);
	static SbmPhysicsSim* getPhysicsEngine();
};


#endif

