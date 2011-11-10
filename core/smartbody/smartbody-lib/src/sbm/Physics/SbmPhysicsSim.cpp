#include "SbmPhysicsSim.h"
#include <sbm/mcontrol_util.h>

SbmPhysicsSim::SbmPhysicsSim(void)
{
	SBObject::createDoubleAttribute("gravity",980, true, "Basic", 20, false, false, false, "?");
	SBObject::createBoolAttribute("enable",false,true, "Basic", 20, false, false, false, "?");	
}

SbmPhysicsSim::~SbmPhysicsSim(void)
{
}

void SbmPhysicsSim::updateSimulation( float timestep )
{
	bool enableSim = SBObject::getBoolAttribute("enable");
	if (enableSim)
	{
		updateSimulationInternal(timestep);
	}
}

void SbmPhysicsSim::setEnable( bool enable )
{
	SBObject::setBoolAttribute("enable",enable);
}

void SbmPhysicsSim::setGravity( float gravity )
{
	SBObject::setDoubleAttribute("gravity",gravity);
}

void SbmPhysicsSim::addPhysicsObj( SbmPhysicsObj* obj )
{
	//physicsObjList.push_back(obj);
	if (physicsObjList.find(obj->getID()) == physicsObjList.end())
	{
		physicsObjList[obj->getID()] = obj;
	}
}

void SbmPhysicsSim::removePhysicsObj( SbmPhysicsObj* obj )
{
	if (physicsObjList.find(obj->getID()) != physicsObjList.end())
	{
		physicsObjList.erase(obj->getID());
	}
}

bool SbmPhysicsSim::hasPhysicsObj( SbmPhysicsObj* obj )
{
	if (physicsObjList.find(obj->getID()) != physicsObjList.end())
		return true;
	else
		return false;
}

bool SbmPhysicsSim::hasPhysicsCharacter( SbmPhysicsCharacter* phyChar )
{
	if (characterMap.find(phyChar->getName()) != characterMap.end())
		return true;
	else
		return false;
}

/************************************************************************/
/* SbmPhyObjInterface                                                   */
/************************************************************************/

void SbmPhysicsObjInterface::setGlobalTransform( SbmTransform& rt )
{	
		globalTransform.rot  = rt.rot;
		globalTransform.tran = rt.tran;	
}


void SbmPhysicsObjInterface::setGlobalTransform( const SrMat& newState )
{	
	SRT gtran; gtran.gmat(newState);
	setGlobalTransform(gtran);	
}

unsigned long SbmPhysicsObjInterface::getID()
{
	return (unsigned long)this;
}

SbmPhysicsObj::SbmPhysicsObj()
{
	colObj = NULL;	
	objMass = 0.f;
	bHasPhysicsSim = true;
	bHasCollisionSim = true;	
}

void SbmPhysicsObj::setGeometry( SbmGeomObject* obj, float density )
{
	colObj = obj;
	colObj->attachToPhyObj(this);
	objMass = density;
}

void SbmPhysicsObj::enablePhysicsSim( bool bPhy )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPhysicsSim* phySim = mcuCBHandle::singleton().physicsEngine;
	phySim->enablePhysicsSim(this,bPhy);	
}

void SbmPhysicsObj::enableCollisionSim( bool bCol )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPhysicsSim* phySim = mcuCBHandle::singleton().physicsEngine;
	phySim->enableCollisionSim(this,bCol);
}

void SbmPhysicsObj::updateSbmObj()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPhysicsSim* phySim = mcuCBHandle::singleton().physicsEngine;
	phySim->updateSbmObj(this);
}

void SbmPhysicsObj::updatePhySim()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPhysicsSim* phySim = mcuCBHandle::singleton().physicsEngine;
	phySim->updatePhySim(this);
}

/************************************************************************/
/* SbmPhyiscJoint                                                       */
/************************************************************************/
SbmJointObj::SbmJointObj()
{
	sbmJoint = NULL;
    parentObj = NULL;
}

SbmJointObj::~SbmJointObj()
{

}

void SbmJointObj::initJoint( SBJoint* joint )
{
	SRT grt; grt.gmat(joint->getMatrixGlobal());	
	setGlobalTransform(grt);
	sbmJoint = joint;
}

/************************************************************************/
/* Physics Character                                                    */
/************************************************************************/
SbmJointObj* SbmPhysicsCharacter::getJointObj( std::string& jointName )
{
	if (jointMap.find(jointName) != jointMap.end())
	{
		return jointMap[jointName];
	}
	return NULL;
}

std::vector<SbmJointObj*> SbmPhysicsCharacter::getJointObjList()
{
	std::vector<SbmJointObj*> jointObjList;
	std::map<std::string, SbmJointObj*>::iterator mi = jointMap.begin();
	for ( mi  = jointMap.begin();
		 mi != jointMap.end();
		 mi++)
	{
		jointObjList.push_back(mi->second);
	}
	return jointObjList;	
}

std::map<std::string,SbmJointObj*> SbmPhysicsCharacter::getJointObjMap()
{
	return jointMap;
}

void SbmPhysicsCharacter::initPhysicsCharacter( std::string& characterName, std::vector<std::string>& jointNameList, bool buildGeometry )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SBScene* scene = mcu._scene;
	SbmPhysicsSim* phySim = mcuCBHandle::singleton().physicsEngine;
	SBCharacter* character = scene->getCharacter(characterName);
	if (!character) // no character
		return;
	characterName = character->getName();
	cleanUpJoints();
	SBSkeleton* skel = character->getSkeleton();
	for (unsigned int i=0;i<jointNameList.size();i++) // only process the joints that are in the name lists
	{
		SBJoint* joint = skel->getJointByName(jointNameList[i]);
		if (!joint)
			continue;
		SbmJointObj* jointObj = phySim->createJointObj();
		jointObj->initJoint(joint);
		jointMap[jointNameList[i]] = jointObj;		
		if (buildGeometry)
		{
			SbmGeomObject* jointGeom = createJointGeometry(joint);
			jointObj->setGeometry(jointGeom,10.f);
			jointGeometryMap[jointNameList[i]] = jointGeom;
		}
		if (!joint->getParent()) // root joint
			jointRoot = jointObj;
	}

	// connect each adjacent joints
	std::map<std::string, SbmJointObj*>::iterator mi = jointMap.begin();
	for ( mi  = jointMap.begin();
		  mi != jointMap.end();
		  mi++)
	{
		SbmJointObj* obj = mi->second;
		SBJoint* pj = obj->getJoint()->getParent();
		if (pj && jointMap.find(pj->getName()) != jointMap.end())
		{
			obj->setParentObj(jointMap[pj->getName()]);		 
		}
	}
}

void SbmPhysicsCharacter::cleanUpJoints()
{
	std::map<std::string, SbmJointObj*>::iterator mi;
	SbmPhysicsSim* phySim = mcuCBHandle::singleton().physicsEngine;
	// remove all joint objs
	for ( mi  = jointMap.begin();
		  mi != jointMap.end();
		  mi++)
	{
		SbmJointObj* jointObj = mi->second;
		phySim->removePhysicsObj(jointObj);
		delete jointObj;
	}
	jointMap.clear();	
}

SbmGeomObject* SbmPhysicsCharacter::createJointGeometry( SBJoint* joint, float radius )
{
	SbmGeomObject* newGeomObj = NULL;
	if (joint->getParent())
	{
		SBJoint* parent = joint->getParent();
		SrVec offset = joint->offset(); 
		SrVec center = offset*0.5f;
		SrVec dir = offset; dir.normalize();
		float boneLen = offset.len();	
		float len = boneLen+0.001f;	
		if (radius <= 0.f)
			radius = len*0.2f;	
		// generate new geometry
		newGeomObj = new SbmGeomCapsule(center-dir*len*0.5f, center+dir*len*0.5f,radius);
	}
	else 
	{
		newGeomObj = new SbmGeomSphere(3.f);
	}
	return newGeomObj;
}