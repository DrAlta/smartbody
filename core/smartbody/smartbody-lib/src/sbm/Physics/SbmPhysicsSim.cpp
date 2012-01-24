#include "SbmPhysicsSim.h"
#include <sbm/mcontrol_util.h>
#include <sr/sr_output.h>
//#include <boost/math/special_functions/fpclassify.hpp>

SbmPhysicsSim::SbmPhysicsSim(void)
{
	
	SBObject::createDoubleAttribute("dT",0.0003, true, "Basic", 20, false, false, false, "?");
	SBObject::createDoubleAttribute("gravity",980, true, "Basic", 20, false, false, false, "?");
	SBObject::createDoubleAttribute("Ks",230000, true, "Basic", 20, false, false, false, "?");
	SBObject::createDoubleAttribute("Kd",2000, true, "Basic", 20, false, false, false, "?");
	SBObject::createDoubleAttribute("MaxSimTime",0.01,true,"Basic", 20, false, false, false, "?");
	//SBObject::createDoubleAttribute("gravity",9.8, true, "Basic", 20, false, false, false, "?");
	//SBObject::createDoubleAttribute("Ks",30.0, true, "Basic", 20, false, false, false, "?");
	//SBObject::createDoubleAttribute("Kd",0.0, true, "Basic", 20, false, false, false, "?");	
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
		// update PD torque for each physics character
		SbmPhysicsCharacterMap::iterator ci;
		for ( ci  = characterMap.begin();
			  ci != characterMap.end();
			  ci++)
		{
			SbmPhysicsCharacter* phyChar = ci->second;
			phyChar->updatePDTorque();
		}
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
		if (dynamic_cast<SbmJointObj*>(obj) == NULL) // is not a joint obj
			pawnObjMap[obj->getName()] = obj;
	}
}


void SbmPhysicsSim::addPhysicsCharacter( SbmPhysicsCharacter* phyChar )
{
	if (characterMap.find(phyChar->getPhysicsCharacterName()) == characterMap.end())
	{
		characterMap[phyChar->getPhysicsCharacterName()] = phyChar;
	}
}


void SbmPhysicsSim::removePhysicsObj( SbmPhysicsObj* obj )
{
	if (physicsObjList.find(obj->getID()) != physicsObjList.end())
	{
		physicsObjList.erase(obj->getID());
		if (dynamic_cast<SbmJointObj*>(obj) == NULL) // is not a joint obj
			pawnObjMap.erase(obj->getName());
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
	if (characterMap.find(phyChar->getPhysicsCharacterName()) != characterMap.end())
		return true;
	else
		return false;
}


SbmPhysicsObj* SbmPhysicsSim::getPhysicsPawn( const std::string& pawnName )
{
	SbmPhysicsPawnMap::iterator iter = pawnObjMap.find(pawnName);
	if (iter == pawnObjMap.end())
		return NULL;
	else
		return (*iter).second;
}

SbmPhysicsCharacter* SbmPhysicsSim::getPhysicsCharacter( const std::string& charName )
{
	SbmPhysicsCharacterMap::iterator iter = characterMap.find(charName);
	if (iter == characterMap.end())
		return NULL;
	else
		return (*iter).second;
}

void SbmPhysicsSim::notify( SBSubject* subject )
{
	SBAttribute* attribute = dynamic_cast<SBAttribute*>(subject);
	if (attribute)
	{
		if (attribute->getName() == "Kd" || attribute->getName() == "Ks" || attribute->getName() == "dT")
		{
			updateAllPhysicsJoints();			
		}	
		else if (attribute->getName() == "enable")
		{
			SbmPhysicsObjMap::iterator mi;
			for ( mi  = physicsObjList.begin();
				  mi != physicsObjList.end();
				  mi++)
			{
				SbmPhysicsObj* obj = mi->second;				
				obj->setLinearVel(obj->getVec3Attribute("refLinearVelocity"));
				obj->setAngularVel(obj->getVec3Attribute("refAngularVelocity"));
				obj->updatePhySim();
			}
		}
	}
}

void SbmPhysicsSim::updateAllPhysicsJoints()
{
	SbmPhysicsCharacterMap::iterator mi;
	for ( mi  = characterMap.begin();
		  mi != characterMap.end();
		  mi++)
	{
		SbmPhysicsCharacter* phyChar = mi->second;
		std::vector<SbmPhysicsJoint*> jointList = phyChar->getPhyJointList();
		for (unsigned int i=0;i<jointList.size();i++)
		{
			SbmPhysicsJoint* phyJoint = jointList[i];
			updatePhysicsJoint(phyJoint);
		}
	}
}

SbmPhysicsSim* SbmPhysicsSim::getPhysicsEngine()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	return mcu._scene->getPhysicsManager()->getPhysicsEngine();
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

void SbmPhysicsObjInterface::setRefTransform( SbmTransform& rt )
{			
	refTransform.rot  = rt.rot;
	refTransform.tran = rt.tran;	
}


void SbmPhysicsObjInterface::setRefTransform( const SrMat& newState )
{	
	SRT gtran; gtran.gmat(newState);
	setRefTransform(gtran);	
}

unsigned long SbmPhysicsObjInterface::getID()
{
	return (unsigned long)this;
}

SbmPhysicsObj::SbmPhysicsObj()
{
	colObj = NULL;		
	objDensity = 0.01f;
	bHasPhysicsSim = false;
	bHasCollisionSim = true;	
	initGeom = false;

	SBObject::createBoolAttribute("enable",false,true, "Basic", 20, false, false, false, "?");		
	SBObject::createBoolAttribute("constraint",false,true, "Basic", 20, false, false, false, "?");
	
	SBObject::createDoubleAttribute("mass",1.f,true, "Basic", 20, false, false, false, "?");
	SBObject::createVec3Attribute("refLinearVelocity",0.f,0.f,0.f,true, "Physics", 20, false, false, false, "?");
	SBObject::createVec3Attribute("refAngularVelocity",0.f,0.f,0.f,true, "Physics", 20, false, false, false, "?");

	SBObject::createStringAttribute("constraintTarget","none",true, "Basic", 80, false, false, false, "?");
	
	SmartBody::StringAttribute* geomTypeAttr = createStringAttribute("geomType","box",true,"Geom", 80, false, false, false,"the geometry type");	
	std::vector<std::string> geomTypes;
	geomTypes.push_back("null");
	geomTypes.push_back("sphere");	
	geomTypes.push_back("box");	
	geomTypes.push_back("capsule");	
	geomTypeAttr->setValidValues(geomTypes);
	SBObject::createVec3Attribute("geomSize",1.f,1.f,1.f,true, "Geom", 20, false, false, false, "?");
}

SbmPhysicsObj::~SbmPhysicsObj()
{
	if (colObj)
		delete colObj;
}


void SbmPhysicsObj::setGeometry( SbmGeomObject* obj)
{
	initGeom = false;
	colObj = obj;
	colObj->attachToPhyObj(this);
	setStringAttribute("geomType",colObj->geomType());
	SrVec geomSize = colObj->getGeomSize();
	setVec3Attribute("geomSize",geomSize[0],geomSize[1],geomSize[2]);
	initGeom = true;
}

void SbmPhysicsObj::enablePhysicsSim( bool bPhy )
{	
	SbmPhysicsSim* phySim = SbmPhysicsSim::getPhysicsEngine();
	phySim->enablePhysicsSim(this,bPhy);	
}

void SbmPhysicsObj::enableCollisionSim( bool bCol )
{
	SbmPhysicsSim* phySim = SbmPhysicsSim::getPhysicsEngine();
	phySim->enableCollisionSim(this,bCol);
}

void SbmPhysicsObj::updateSbmObj()
{
	SbmPhysicsSim* phySim = SbmPhysicsSim::getPhysicsEngine();
	phySim->writeToPhysicsObj(this);
}

void SbmPhysicsObj::updatePhySim()
{
	SbmPhysicsSim* phySim = SbmPhysicsSim::getPhysicsEngine();
	phySim->readFromPhysicsObj(this);
}

void SbmPhysicsObj::notify( SBSubject* subject )
{
	SBAttribute* attribute = dynamic_cast<SBAttribute*>(subject);
	if (attribute)
	{
		if (attribute->getName() == "enable")
		{
			bool val = this->getBoolAttribute("enable");
			this->setLinearVel(this->getVec3Attribute("refLinearVelocity"));
			this->setAngularVel(this->getVec3Attribute("refAngularVelocity"));
			this->updatePhySim();
			enablePhysicsSim(val);
		}		
		else if (attribute->getName() == "geomType" || attribute->getName() == "geomSize" || attribute->getName() == "mass")
		{
			std::string geomType = getStringAttribute("geomType");
			if (initGeom)
				changeGeometry(geomType,getVec3Attribute("geomSize"));
		}
	}
}

void SbmPhysicsObj::changeGeometry( std::string& geomType, SrVec extends )
{
	initGeom = false;
	SbmTransform localTran;	
	if (colObj)
	{
		localTran = colObj->getLocalTransform();
		delete colObj;
	}
	colObj = SbmGeomObject::createGeometry(geomType,extends);
	colObj->setLocalTransform(localTran);
	colObj->attachToPhyObj(this);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPhysicsSim* phySim = SbmPhysicsSim::getPhysicsEngine();
	phySim->updatePhyObjGeometry(this);

	setStringAttribute("geomType",colObj->geomType());
	SrVec geomSize = colObj->getGeomSize();
	setVec3Attribute("geomSize",geomSize[0],geomSize[1],geomSize[2]);
	initGeom = true;
}

float SbmPhysicsObj::getMass()
{
	return (float)getDoubleAttribute("mass");
}

void SbmPhysicsObj::setMass( float mass )
{
	setDoubleAttribute("mass",(double)mass);
}

SrVec SbmPhysicsObj::getLinearVel()
{
	return linearVel;
}

void SbmPhysicsObj::setLinearVel( SrVec val )
{
	linearVel = val;
}

SrVec SbmPhysicsObj::getAngularVel()
{
	return angularVel;
}

void SbmPhysicsObj::setAngularVel( SrVec val )
{
	angularVel = val;
}

void SbmPhysicsObj::handleCollision( SrVec contactPt, SbmPhysicsObj* colObj )
{

}
/************************************************************************/
/* Physics Joint                                                        */
/************************************************************************/

SbmPhysicsJoint::SbmPhysicsJoint( SBJoint* joint ) 
{
	sbmJoint = joint;
	jointTorque = SrVec(0,0,0);
	totalSupportMass = 0.f;
	parentObj = NULL;
	childObj = NULL;
	SBObject::createDoubleAttribute("KScale",1.0, true, "Basic", 20, false, false, false, "?");
}

unsigned long SbmPhysicsJoint::getID()
{
	return (unsigned long)(this);
}

void SbmPhysicsJoint::updateTotalSupportMass()
{
	totalSupportMass = 0.f;
	SbmJointObj* obj = getChildObj();
	totalSupportMass += obj->getMass();

	for (int i=0;i<obj->getNumChildJoints();i++)
	{
		SbmPhysicsJoint* cj = obj->getChildJoint(i);
		if (cj)
		{
			cj->updateTotalSupportMass();
			totalSupportMass += cj->getTotalSupportMass();
		}		
	}
	LOG("joint %s, total mass = %f",this->getSBJoint()->getName().c_str(),totalSupportMass);
}

void SbmPhysicsJoint::notify( SBSubject* subject )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPhysicsSim* phySim = SbmPhysicsSim::getPhysicsEngine();
	phySim->updatePhysicsJoint(this);	
}

/************************************************************************/
/* SbmJointObj                                                          */
/************************************************************************/

SbmJointObj::SbmJointObj(SbmPhysicsCharacter* phyc) : SbmPhysicsObj()
{
	phyJoint = NULL;
	phyChar = phyc;	
}

SbmJointObj::~SbmJointObj()
{

}

SbmPhysicsJoint* SbmJointObj::getChildJoint( int i )
{
	SBJoint* joint = getSBJoint();
	if (i>=0 && i<joint->getNumChildren())
	{	
		return phyChar->getPhyJoint(joint->getChild(i)->getName());		
	}
	else
		return NULL;
}

int SbmJointObj::getNumChildJoints()
{
	SBJoint* joint = getSBJoint();
	return joint->getNumChildren();
}

void SbmJointObj::initJoint( SbmPhysicsJoint* phyj )
{	
	phyJoint = phyj;
	SBJoint* joint = phyj->getSBJoint();
	joint->calculateLocalCenter();
	SrMat tranMat; tranMat.translation(joint->getLocalCenter());	
	//if (joint->parent()) 
	SrMat gmat = tranMat*joint->gmat();		
	setGlobalTransform(gmat);

	// create physics attributes for the joint
	if (!phyJoint->getAttribute("type"))
		phyJoint->createStringAttribute("type","ball",true,"Basic",41,false,false,false,"joint type");	
	if (!phyJoint->getAttribute("axis0"))
		phyJoint->createVec3Attribute("axis0",1,0,0,true,"Basic",42,false,false,false,"rotation axis 0");
	if (!phyJoint->getAttribute("axis1"))
		phyJoint->createVec3Attribute("axis1",0,1,0,true,"Basic",43,false,false,false,"rotation axis 1");
	if (!phyJoint->getAttribute("axis2"))
		phyJoint->createVec3Attribute("axis2",0,0,1,true,"Basic",44,false,false,false,"rotation axis 2");
	
	SrVec defaultHigh = SrVec(1.9f,1.9f,1.9f);
	SrVec defaultLow  = SrVec(-1.9f,-1.9f,-1.9f);
	std::string jname = joint->getName();
	if (jname.find("sternoclavicular") != std::string::npos || jname.find("acromioclavicular") != std::string::npos || jname.find("forefoot") != std::string::npos)
	{
		defaultHigh = SrVec(0.f,0.f,0.f); defaultLow = SrVec(-0.f,-0.f,-0.f);
	}
	else if (jname.find("spine") != std::string::npos)
	{
		defaultHigh = SrVec(0.1f,0.1f,0.1f); defaultLow = SrVec(-0.1f,-0.1f,-0.1f);
	}
	else if (jname.find("shoulder") != std::string::npos)
	{
		defaultHigh = SrVec(2.9f,2.9f,2.5f); defaultLow = SrVec(-1.9f,-2.9f,-2.5f);
	}
	else if (jname.find("elbow") != std::string::npos)
	{
		defaultHigh = SrVec(0.2f,1.9f,0.2f); defaultLow = SrVec(-0.2f,-0.0f,-0.2f);
	}
	else if (jname.find("forearm") != std::string::npos)
	{
		defaultHigh = SrVec(0.0f,0.f,0.8f); defaultLow = SrVec(-0.0f,-0.f,-0.8f);
	}
	else if (jname.find("wrist") != std::string::npos)
	{
		defaultHigh = SrVec(0.5f,0.5f,0.1f); defaultLow = SrVec(-0.5f,-0.5f,-0.1f);
	}
	else if (jname.find("hip") != std::string::npos)
	{
		defaultHigh = SrVec(0.7f,0.7f,0.6f); defaultLow = SrVec(-0.7f,-0.7f,-0.6f);
	}
	else if (jname.find("ankle") != std::string::npos)
	{
		defaultHigh = SrVec(0.5f,0.5f,0.0f); defaultLow = SrVec(-0.5f,-0.5f,-0.0f);
	}
	else if (jname.find("knee") != std::string::npos)
	{
		defaultHigh = SrVec(0.1f,1.3f,0.1f); defaultLow = SrVec(-0.1f,-0.0f,-0.1f);		
	}

	
	if (!phyJoint->getAttribute("axis0LimitHigh"))
		phyJoint->createDoubleAttribute("axis0LimitHigh", defaultHigh[0], true,"Basic",45,false,false,false,"upper limit for axis0 rotation" );
	if (!phyJoint->getAttribute("axis0LimitLow"))
		phyJoint->createDoubleAttribute("axis0LimitLow", defaultLow[0], true,"Basic",45,false,false,false,"lower limit for axis0 rotation" );

	if (!phyJoint->getAttribute("axis1LimitHigh"))
		phyJoint->createDoubleAttribute("axis1LimitHigh", defaultHigh[1], true,"Basic",45,false,false,false,"upper limit for axis1 rotation" );
	if (!phyJoint->getAttribute("axis1LimitLow"))
		phyJoint->createDoubleAttribute("axis1LimitLow", defaultLow[1], true,"Basic",45,false,false,false,"lower limit for axis1 rotation" );

	if (!phyJoint->getAttribute("axis2LimitHigh"))
		phyJoint->createDoubleAttribute("axis2LimitHigh", defaultHigh[2], true,"Basic",45,false,false,false,"upper limit for axis2 rotation" );
	if (!phyJoint->getAttribute("axis2LimitLow"))
		phyJoint->createDoubleAttribute("axis2LimitLow", defaultLow[2], true,"Basic",45,false,false,false,"lower limit for axis2 rotation" );

	//if (joint->getName() == "spine4")
	//	setExternalForce(SrVec(0,18000,0));	
}

SrMat SbmJointObj::getRelativeOrientation()
{
	updateSbmObj();	
	SrMat gmat;
	if (getParentObj())
	{
		gmat = getGlobalTransform().gmat()*getParentObj()->getGlobalTransform().gmat().inverse();		
	}
	else if (getSBJoint()->getParent())
	{
		gmat = getGlobalTransform().gmat()*getSBJoint()->getParent()->getMatrixGlobal().inverse();
	}
	else
	{
		gmat = getGlobalTransform().gmat();
	}	
	return gmat;
}


void SbmJointObj::handleCollision( SrVec contactPt, SbmPhysicsObj* colObj )
{
	SbmJointObj* colJointObj = dynamic_cast<SbmJointObj*>(colObj);
	if (colJointObj && colJointObj->getPhysicsCharacter() == phyChar) // do not handle self-collision
		return;
	std::vector<CollisionRecord>& colRecords = phyChar->getCollisionRecords();
	if (colRecords.size() > 0) return;
	CollisionRecord rec;
	rec.collisionPt = contactPt;
	rec.hitJointObj = this;
	rec.collider = colObj;	
	if (colObj)
	{
		rec.momentum = colObj->getLinearVel()*colObj->getMass();
	}
	colRecords.push_back(rec);	
}


/************************************************************************/
/* Physics Character                                                    */
/************************************************************************/

SbmPhysicsCharacter::SbmPhysicsCharacter()
{
	SBObject::createBoolAttribute("enable",false,true, "Basic", 20, false, false, false, "?");	
	SBObject::createBoolAttribute("kinematicRoot",false,true, "Basic", 20, false, false, false, "?");	
	SBObject::createBoolAttribute("usePD",false,true, "Basic", 20, false, false, false, "?");	
}

SbmJointObj* SbmPhysicsCharacter::getJointObj(const std::string& jointName )
{
	if (jointObjMap.find(jointName) != jointObjMap.end())
	{
		return jointObjMap[jointName];
	}
	return NULL;
}


SbmPhysicsJoint* SbmPhysicsCharacter::getPhyJoint(const std::string& jointName )
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
	std::map<std::string, SbmJointObj*>::iterator mi = jointObjMap.begin();
	for ( mi  = jointObjMap.begin();
		 mi != jointObjMap.end();
		 mi++)
	{
		jointObjList.push_back(mi->second);
	}
	return jointObjList;	
}


std::vector<SbmPhysicsJoint*> SbmPhysicsCharacter::getPhyJointList()
{
	std::vector<SbmPhysicsJoint*> jointList;
	std::map<std::string, SbmPhysicsJoint*>::iterator mi = jointMap.begin();
	for ( mi  = jointMap.begin();
		  mi != jointMap.end();
		  mi++)
	{
		jointList.push_back(mi->second);
	}
	return jointList;
}


std::map<std::string,SbmJointObj*>& SbmPhysicsCharacter::getJointObjMap()
{
	return jointObjMap;
}

void SbmPhysicsCharacter::initPhysicsCharacter( std::string& charName, std::vector<std::string>& jointNameList, bool buildGeometry )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SBScene* scene = mcu._scene;
	SbmPhysicsSim* phySim = SbmPhysicsSim::getPhysicsEngine();
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
		SbmPhysicsJoint* phyJoint = new SbmPhysicsJoint(joint);
		SbmJointObj* jointObj = new SbmJointObj(this);//phySim->createJointObj();		
		if (joint->mass() > 0)
			jointObj->setMass(joint->mass());
		if (buildGeometry)
		{
			SbmGeomObject* jointGeom = createJointGeometry(joint);
			jointObj->setGeometry(jointGeom);
			//jointGeometryMap[jointNameList[i]] = jointGeom;
		}
		jointObj->initJoint(phyJoint);
		jointObjMap[jointNameList[i]] = jointObj;	
		jointMap[jointNameList[i]] = phyJoint;
		
	}

	// connect each adjacent joints
	std::map<std::string, SbmJointObj*>::iterator mi = jointObjMap.begin();
	for ( mi  = jointObjMap.begin();
		  mi != jointObjMap.end();
		  mi++)
	{
		SbmJointObj* obj = mi->second;
		// set the child rigid body for the joint
		SbmPhysicsJoint* oj = obj->getPhyJoint();
		if (oj && jointObjMap.find(oj->getSBJoint()->getName()) != jointObjMap.end())
		{
			oj->setChildObj(obj);	 
		}
		SBJoint* pj = oj->getSBJoint()->getParent();
		if (pj)
		{
			SbmJointObj* pobj = getJointObj(pj->getName());
			if (pobj)
				oj->setParentObj(pobj);
		}		
	}

	std::map<std::string, SbmPhysicsJoint*>::iterator ji = jointMap.begin();
	for ( ji  = jointMap.begin();
		  ji != jointMap.end();
		  ji++)
	{
		SbmPhysicsJoint* phyJ = ji->second;
		if (!phyJ->getParentObj()) root = phyJ;
		updateJointAxis(phyJ);
	}
	root->updateTotalSupportMass();
}

void SbmPhysicsCharacter::cleanUpJoints()
{
	std::map<std::string, SbmJointObj*>::iterator mi;
	SbmPhysicsSim* phySim = SbmPhysicsSim::getPhysicsEngine();
	// remove all joint objs
	for ( mi  = jointObjMap.begin();
		  mi != jointObjMap.end();
		  mi++)
	{
		SbmJointObj* jointObj = mi->second;
		phySim->removePhysicsObj(jointObj);
		delete jointObj;
	}
	jointObjMap.clear();	
}

SbmGeomObject* SbmPhysicsCharacter::createJointGeometry( SBJoint* joint, float radius )
{
	SbmGeomObject* newGeomObj = NULL;
	SbmCharacter* curCharacter = mcuCBHandle::singleton().getCharacter(characterName);
	if (radius < 0.0)
		radius = curCharacter->getHeight()*0.03f;
		//radius = curCharacter->getHeight()*0.01f;
	if (joint->getName() == "spine1" || joint->getName() == "spine2" || joint->getName() == "spine3")
		radius = curCharacter->getHeight()*0.06f;
		//radius = curCharacter->getHeight()*0.02f;
	if (joint->getName() == "l_sternoclavicular" || joint->getName() == "r_sternoclavicular" || joint->getName() == "l_acromioclavicular" || joint->getName() == "r_acromioclavicular")
		radius = curCharacter->getHeight()*0.01f;
	if (joint->getName() == "l_hip" || joint->getName() == "r_hip")
		radius = curCharacter->getHeight()*0.04f;
	float extend = curCharacter->getHeight()*0.015f;

	

	if (joint->getName() == "l_wrist" || joint->getName() == "r_wrist")
		extend = curCharacter->getHeight()*0.03f;

// 	if (joint->getName() == "l_ankle" || joint->getName() == "r_ankle")
// 	{
// 		SrBox bbox;		
// 		radius = curCharacter->getHeight()*0.06;
// 		bbox.extend(joint->getLocalCenter());			
// 		bbox.grows(extend,extend,extend); 		
// 		bbox.grows(radius,0,0);
// 		for (int i=0;i<joint->getNumChildren();i++)
// 		{
// 			bbox.extend(joint->getChild(i)->offset());
// 		}
// 		newGeomObj = new SbmGeomBox(bbox);	
// 	}
	if (0)
	{

	}
	else if (joint->getNumChildren() > 1 ) // bounding box
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

void SbmPhysicsCharacter::updateJointAxis( SbmPhysicsJoint* phyJoint )
{
	SBJoint* joint = phyJoint->getSBJoint();
	SbmJointObj* childObj = phyJoint->getChildObj();

	SrVec twistAxis = phyJoint->getVec3Attribute("axis0");
	SrVec swingAxis = phyJoint->getVec3Attribute("axis1");
	SrVec swingAxis2 = phyJoint->getVec3Attribute("axis2");
// 	if (joint)
// 	{
// 		twistAxis = joint->getLocalCenter()*joint->getMatrixGlobal() -  joint->getMatrixGlobal().get_translation();
// 		twistAxis.normalize();
// 		sr_out << "twist axis = " << twistAxis << srnl;
// 		if (twistAxis.len() == 0) twistAxis = SrVec(0,1,0);	
// 		swingAxis = SrVec(1.f,0.f,0.f); 		
// 
// 		SrVec newswingAxis = swingAxis - twistAxis*dot(twistAxis,swingAxis); 		
// 		newswingAxis.normalize();
// 		swingAxis = newswingAxis;
// 		swingAxis2 = cross(twistAxis,swingAxis);
// 	}
	phyJoint->setVec3Attribute("axis2",twistAxis[0],twistAxis[1],twistAxis[2]);	
	phyJoint->setVec3Attribute("axis1",swingAxis[0],swingAxis[1],swingAxis[2]);
	phyJoint->setVec3Attribute("axis0",swingAxis2[0],swingAxis2[1],swingAxis2[2]);		

}

void SbmPhysicsCharacter::updatePDTorque()
{
	std::vector<SbmJointObj*> jointObjList = getJointObjList();

	if (!getBoolAttribute("usePD"))
	{
		// clean up the torque
		for (unsigned int i=0;i<jointObjList.size();i++)
		{
			SbmJointObj* obj = jointObjList[i];
			SbmPhysicsJoint* phyJoint = obj->getPhyJoint();
			phyJoint->setJointTorque(SrVec(0,0,0));
		}
		return;
	}

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPhysicsSim* phySim = SbmPhysicsSim::getPhysicsEngine();
	float Ks = (float)phySim->getDoubleAttribute("Ks");
	float Kd = (float)phySim->getDoubleAttribute("Kd");
	

	for (unsigned int i=0;i<jointObjList.size();i++)
	{
		SbmJointObj* obj = jointObjList[i];
		SbmPhysicsJoint* phyJoint = obj->getPhyJoint();
		SBJoint* joint = obj->getPhyJoint()->getSBJoint();
		if (!joint)	continue;		
		bool kinematicRoot = (joint->getName() == "base" || joint->getName() == "JtPelvis") && this->getBoolAttribute("kinematicRoot");		
		if (kinematicRoot || obj->getBoolAttribute("constraint"))// || joint->getName() == "JtPelvis")
		{			
			continue;
		}
		
		float KScale = (float)phyJoint->getDoubleAttribute("KScale");

		SrMat tran = obj->getRelativeOrientation();		//obj->getGlobalTransform().gmat();//
		SrMat preRot;
		joint->quat()->prerot().inverse().get_mat(preRot);
		SrQuat phyQuat = SrQuat(tran*preRot);

		SrQuat pQuat;	
		SrVec pAvel;
		if (obj->getParentObj())
		{
			pQuat = obj->getParentObj()->getGlobalTransform().rot;
			pAvel = obj->getParentObj()->getAngularVel();
		}

		SrQuat inQuat = phyJoint->getRefQuat();
		
		
		SrVec relW = (obj->getAngularVel() - pAvel)*pQuat.inverse()*joint->quat()->prerot().inverse();//*joint->quat()->prerot().inverse();//obj->getAngularVel()*pQuat.inverse() - obj->getParentObj()->getAngularVel()*pQuat.inverse(); 		
		SrVec relWD = SrVec(0,0,0);
		float scaleKs, scaleKd;
		scaleKs = Ks*KScale*phyJoint->getTotalSupportMass();
		scaleKd = Kd*KScale*phyJoint->getTotalSupportMass();//2.0 * sqrtf( scaleKs);//;

		SrVec torque = computePDTorque(phyQuat,inQuat,relW,relWD,scaleKs,scaleKd)*joint->quat()->prerot()*pQuat;	
		//SrVec torque = computeSPDTorque(phyQuat,inQuat,relW,relWD,scaleKs,scaleKd,phySim->getDoubleAttribute("dT"),phyJoint->getTotalSupportMass())*pQuat;	
		if (obj->getParentObj())// && obj->getParentObj()->getParentObj())
		{
			SrVec oldTorque = phyJoint->getJointTorque();
			SrVec torqueDiff = torque - oldTorque;

			float maxTorqueRateOfChange = 200*phyJoint->getTotalSupportMass();
			float maxAbsTorque = 5000*phyJoint->getTotalSupportMass();

			torqueDiff.x = (torqueDiff.x<-maxTorqueRateOfChange)?(-maxTorqueRateOfChange):(torqueDiff.x);
			torqueDiff.x = (torqueDiff.x>maxTorqueRateOfChange)?(maxTorqueRateOfChange):(torqueDiff.x);
			torqueDiff.y = (torqueDiff.y<-maxTorqueRateOfChange)?(-maxTorqueRateOfChange):(torqueDiff.y);
			torqueDiff.y = (torqueDiff.y>maxTorqueRateOfChange)?(maxTorqueRateOfChange):(torqueDiff.y);
			torqueDiff.z = (torqueDiff.z<-maxTorqueRateOfChange)?(-maxTorqueRateOfChange):(torqueDiff.z);
			torqueDiff.z = (torqueDiff.z>maxTorqueRateOfChange)?(maxTorqueRateOfChange):(torqueDiff.z);

			SrVec newTorque = oldTorque+torqueDiff;

			newTorque.x = (newTorque.x<-maxAbsTorque)?(-maxAbsTorque):(newTorque.x);
			newTorque.x = (newTorque.x>maxAbsTorque)?(maxAbsTorque):(newTorque.x);
			newTorque.y = (newTorque.y<-maxAbsTorque)?(-maxAbsTorque):(newTorque.y);
			newTorque.y = (newTorque.y>maxAbsTorque)?(maxAbsTorque):(newTorque.y);
			newTorque.z = (newTorque.z<-maxAbsTorque)?(-maxAbsTorque):(newTorque.z);
			newTorque.z = (newTorque.z>maxAbsTorque)?(maxAbsTorque):(newTorque.z);
			//phyJoint->setJointTorque(oldTorque+torqueDiff);
			phyJoint->setJointTorque(torque);
			//phyJoint->setJointTorque(newTorque);
			//torque = newTorque;
			//torque = torque;
		}
	    
		static bool showTorque = true;
		static int counter = 0;
	}
}

SrVec SbmPhysicsCharacter::computePDTorque( SrQuat& q, SrQuat& qD, SrVec& w, SrVec& vD, float Ks, float Kd )
{
	SrVec torque;	
	SrQuat qErr = q.inverse()*qD;//qD*q.inverse();
	qErr.normalize();		
	// torque for correcting the orientation to desired angle
	SrVec v = SrVec(qErr.x,qErr.y,qErr.z);	
	float sinTheta = v.len();
	if (sinTheta > 1) sinTheta = 1;
	if (sinTheta > -gwiz::epsilon10() && sinTheta < gwiz::epsilon10())
	{
		//angle is too small
	}
	else
	{
		float qAngle = asin(v.len());//qErr.angle();
		torque = qErr.axisAngle()*(-Ks);//v/sinTheta*qAngle*(-Ks)*(float)MeCtMath::sgn(qErr.w);
	}

	//torque = torque*q; // rotate back to the parent frame
	torque = torque*q;
	//torque = (qD.axisAngle() - q.axisAngle())*(-Ks);

	// torque for angular velocity damping
	torque += (vD - w)*(-Kd);
	return torque;
}

SrVec SbmPhysicsCharacter::computeSPDTorque( SrQuat& q, SrQuat& qD, SrVec& w, SrVec& vD, float Ks, float Kd, float dt, float mass )
{

	SrVec torque;	
	SrQuat qErr = qD*q.inverse();
	qErr.normalize();	
	// torque for correcting the orientation to desired angle
	SrVec v = SrVec(qErr.x,qErr.y,qErr.z);
	float sinTheta = v.len();
	if (sinTheta > 1) sinTheta = 1;

	SrVec kpq;
	if (sinTheta > -gwiz::epsilon10() && sinTheta < gwiz::epsilon10())
	{
		//angle is too small
	}
	else
	{
		float qAngle = asin(v.len());//qErr.angle();
		//kpq = v/sinTheta*qAngle*(float)MeCtMath::sgn(qErr.w); //(-Ks)*
		kpq = v/sinTheta*qAngle*(-Ks)*(float)MeCtMath::sgn(qErr.w);
	}

	// Roughly based off Wu's 2003 GDC presentation -- but his derivation looks broken.
	// this is a different derivation in the same spirit
	// in particular, his derivation eliminated m(!) as well
	// as the argVel term.
	/*
	const double g = 1.0 / ( 1.0 + ( Kd + Ks * dt ) * dt * dt / mass );
	const double gmks = g * Ks;
	const double gmkd = g * Kd;
	return ((kpq*q - w * dt)* gmks  - w*gmkd)*(-1);
	*/
	

	
	kpq = kpq*q; // -Kp(q_d - q)
	// torque for angular velocity damping
	SrVec ktp = w*dt*(-Ks);
	SrVec damp = (vD - w)*(-Kd);
	float scale = mass/(mass+Kd*dt);//+Ks*dt*dt);
	//LOG("torque scale = %f",scale);
	torque = (kpq+damp-ktp)*scale;	

	return torque;
}

void SbmPhysicsCharacter::enablePhysicsSim( bool bPhy )
{
	std::map<std::string, SbmJointObj*>::iterator mi = jointObjMap.begin();
	for ( mi  = jointObjMap.begin();
		  mi != jointObjMap.end();
		  mi++)
	{
		SbmJointObj* obj = mi->second;
		obj->enablePhysicsSim(bPhy);		
	}
}

void SbmPhysicsCharacter::enableCollisionSim( bool bCol )
{
	std::map<std::string, SbmJointObj*>::iterator mi = jointObjMap.begin();
	for ( mi  = jointObjMap.begin();
		mi != jointObjMap.end();
		mi++)
	{
		SbmJointObj* obj = mi->second;
		obj->enableCollisionSim(bCol);		
	}
}

void SbmPhysicsCharacter::notify( SBSubject* subject )
{
	SBAttribute* attribute = dynamic_cast<SBAttribute*>(subject);
	if (attribute)
	{
		if (attribute->getName() == "enable")
		{
			std::map<std::string, SbmJointObj*>::iterator mi;
			for ( mi  = jointObjMap.begin();
				  mi != jointObjMap.end();
				  mi++ )
			{
				SbmJointObj* obj = mi->second;
				obj->setLinearVel(obj->getVec3Attribute("refLinearVelocity"));
				obj->setAngularVel(obj->getVec3Attribute("refAngularVelocity"));
				obj->updatePhySim();
				
			}
		}		
	}
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



/************************************************************************/
/* Collision Record                                                     */
/************************************************************************/
CollisionRecord& CollisionRecord::operator=( const CollisionRecord& rt )
{
	collisionPt = rt.collisionPt;
	collider    = rt.collider;
	hitJointObj = rt.hitJointObj;
	momentum    = rt.momentum;
	return *this;
}