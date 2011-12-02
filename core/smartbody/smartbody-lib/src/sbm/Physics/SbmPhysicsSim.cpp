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
	joint->calculateLocalCenter();
	SrMat tranMat; tranMat.translation(joint->getLocalCenter());	
	//if (joint->parent()) 
	SrMat gmat = tranMat*joint->gmat();		
	setGlobalTransform(gmat);
	sbmJoint = joint;

	// create physics attributes for the joint
	if (!joint->getAttribute("type"))
		joint->createStringAttribute("type","ball",true,"Basic",41,false,false,false,"joint type");	
	if (!joint->getAttribute("axis0"))
		joint->createVec3Attribute("axis0",1,0,0,true,"Basic",42,false,false,false,"rotation axis 0");
	if (!joint->getAttribute("axis1"))
		joint->createVec3Attribute("axis1",0,1,0,true,"Basic",42,false,false,false,"rotation axis 1");
	if (!joint->getAttribute("axis2"))
		joint->createVec3Attribute("axis2",0,0,1,true,"Basic",42,false,false,false,"rotation axis 2");

	
}

SrMat SbmJointObj::getRelativeOrientation()
{
	updateSbmObj();
	if (!parentObj)	
		return getGlobalTransform().gmat();
	else
	{
		return getGlobalTransform().gmat()*parentObj->getGlobalTransform().gmat().inverse();
	}
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

std::map<std::string,SbmJointObj*>& SbmPhysicsCharacter::getJointObjMap()
{
	return jointMap;
}

void SbmPhysicsCharacter::initPhysicsCharacter( std::string& charName, std::vector<std::string>& jointNameList, bool buildGeometry )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SBScene* scene = mcu._scene;
	SbmPhysicsSim* phySim = mcuCBHandle::singleton().physicsEngine;
	SBCharacter* character = scene->getCharacter(charName);
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
		if (buildGeometry)
		{
			SbmGeomObject* jointGeom = createJointGeometry(joint);
			jointObj->setGeometry(jointGeom,0.01f);
			jointGeometryMap[jointNameList[i]] = jointGeom;
		}
		jointObj->initJoint(joint);
		jointMap[jointNameList[i]] = jointObj;		
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
	SbmCharacter* curCharacter = mcuCBHandle::singleton().getCharacter(characterName);
	if (radius < 0.0)
		radius = curCharacter->getHeight()*0.01f;
	float extend = curCharacter->getHeight()*0.002f;
	if (joint->getNumChildren() > 1) // bounding box
	{
		SrBox bbox;		
		bbox.extend(joint->getLocalCenter());			
		bbox.grows(extend,extend,extend); 
		bbox.extend(SrVec(0,0,0));
		for (int i=0;i<joint->getNumChildren();i++)
		{
			bbox.extend(joint->getChild(i)->offset());
		}
		newGeomObj = new SbmGeomBox(bbox);		
	}
	else if (joint->getNumChildren() == 1)
	{
		SBJoint* child = joint->getChild(0);
		SrVec offset = child->offset(); 
		SrVec center = SrVec(0,0,0);//offset*0.5f;
		SrVec dir = offset; dir.normalize();
		float boneLen = offset.len();	
		float len = boneLen+extend*0.1f;	
		// generate new geometry
		newGeomObj = new SbmGeomCapsule(center-dir*len*0.5f, center+dir*len*0.5f,radius);		
	}
	return newGeomObj;
}

// SbmGeomObject* SbmPhysicsCharacter::createJointGeometry( SBJoint* joint, float radius )
// {
// 	SbmGeomObject* newGeomObj = NULL;
// 	SbmCharacter* curCharacter = mcuCBHandle::singleton().getCharacter(characterName);
// 	if (joint->getParent() && joint->getParent()->getParent())
// 	{
// 		SBJoint* parent = joint->getParent();
// 		SrVec offset = joint->offset(); 
// 		SrVec center = SrVec(0,0,0);//offset*0.5f;
// 		SrVec dir = offset; dir.normalize();
// 		float boneLen = offset.len();	
// 		float len = boneLen+0.001f;	
// 		if (radius <= 0.f)
// 			radius = 1.0;//curCharacter->getHeight()*0.05f;//	
// 		// generate new geometry
// 		newGeomObj = new SbmGeomCapsule(center-dir*len*0.5f, center+dir*len*0.5f,radius);
// 		//newGeomObj = new SbmGeomCapsule(SrVec(0,-len*0.3f,0), SrVec(0,len*0.3f,0),radius);
// 	}
// 	else 
// 	{
// 		newGeomObj = new SbmGeomSphere(1.0f);//SbmGeomSphere(0.01f);
// 	}
// 	return newGeomObj;
// }