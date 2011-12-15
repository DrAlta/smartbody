#include "SbmPhysicsSim.h"
#include <sbm/mcontrol_util.h>
#include <sr/sr_output.h>

SbmPhysicsSim::SbmPhysicsSim(void)
{
	SBObject::createDoubleAttribute("gravity",980, true, "Basic", 20, false, false, false, "?");
	SBObject::createDoubleAttribute("dT",0.0002, true, "Basic", 20, false, false, false, "?");
	SBObject::createDoubleAttribute("Ks",230000, true, "Basic", 20, false, false, false, "?");
	SBObject::createDoubleAttribute("Kd",2000, true, "Basic", 20, false, false, false, "?");
	SBObject::createDoubleAttribute("KScale",1, true, "Basic", 20, false, false, false, "?");
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
	objMass = 0.1f;
	objDensity = 0.01f;
	bHasPhysicsSim = true;
	bHasCollisionSim = true;	
}

void SbmPhysicsObj::setGeometry( SbmGeomObject* obj, float density )
{
	colObj = obj;
	colObj->attachToPhyObj(this);
	objDensity = density;
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
/* Physics Joint                                                        */
/************************************************************************/

SbmPhysicsJoint::SbmPhysicsJoint( SBJoint* joint ) 
{
	sbmJoint = joint;
	jointTorque = SrVec(0,0,0);
	totalSupportMass = 0.f;
	parentObj = NULL;
	childObj = NULL;
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

	float defaultHigh = 1.5f, defaultLow = -1.5f;
	if (!phyJoint->getAttribute("axis0LimitHigh"))
		phyJoint->createDoubleAttribute("axis0LimitHigh", defaultHigh, true,"Basic",45,false,false,false,"upper limit for axis0 rotation" );
	if (!phyJoint->getAttribute("axis0LimitLow"))
		phyJoint->createDoubleAttribute("axis0LimitLow", defaultLow, true,"Basic",45,false,false,false,"lower limit for axis0 rotation" );

	if (!phyJoint->getAttribute("axis1LimitHigh"))
		phyJoint->createDoubleAttribute("axis1LimitHigh", defaultHigh, true,"Basic",45,false,false,false,"upper limit for axis1 rotation" );
	if (!phyJoint->getAttribute("axis1LimitLow"))
		phyJoint->createDoubleAttribute("axis1LimitLow", defaultLow, true,"Basic",45,false,false,false,"lower limit for axis1 rotation" );

	if (!phyJoint->getAttribute("axis2LimitHigh"))
		phyJoint->createDoubleAttribute("axis2LimitHigh", defaultHigh, true,"Basic",45,false,false,false,"upper limit for axis2 rotation" );
	if (!phyJoint->getAttribute("axis2LimitLow"))
		phyJoint->createDoubleAttribute("axis2LimitLow", defaultLow, true,"Basic",45,false,false,false,"lower limit for axis2 rotation" );

	//if (joint->getName() == "spine4")
	//	setExternalForce(SrVec(0,18000,0));	
}

SrMat SbmJointObj::getRelativeOrientation()
{
	updateSbmObj();
	if (!getParentObj())	
		return getGlobalTransform().gmat();
	else
	{
		return getGlobalTransform().gmat()*getParentObj()->getGlobalTransform().gmat().inverse();
	}
}



/************************************************************************/
/* Physics Character                                                    */
/************************************************************************/
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

std::map<std::string,SbmJointObj*>& SbmPhysicsCharacter::getJointObjMap()
{
	return jointObjMap;
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
		SbmPhysicsJoint* phyJoint = new SbmPhysicsJoint(joint);
		SbmJointObj* jointObj = new SbmJointObj(this);//phySim->createJointObj();		
		if (joint->mass() > 0)
			jointObj->setMass(joint->mass()*0.01f);
		if (buildGeometry)
		{
			SbmGeomObject* jointGeom = createJointGeometry(joint);
			jointObj->setGeometry(jointGeom,0.01f);
			jointGeometryMap[jointNameList[i]] = jointGeom;
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
	SbmPhysicsSim* phySim = mcuCBHandle::singleton().physicsEngine;
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
		radius = curCharacter->getHeight()*0.02f;
	float extend = curCharacter->getHeight()*0.01f;
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

void SbmPhysicsCharacter::updateJointAxis( SbmPhysicsJoint* phyJoint )
{
	SBJoint* joint = phyJoint->getSBJoint();
	SbmJointObj* childObj = phyJoint->getChildObj();

	SrVec twistAxis = phyJoint->getVec3Attribute("axis0");
	SrVec swingAxis = phyJoint->getVec3Attribute("axis1");
	SrVec swingAxis2 = phyJoint->getVec3Attribute("axis2");
	if (joint)
	{
		twistAxis = joint->getLocalCenter()*joint->getMatrixGlobal() -  joint->getMatrixGlobal().get_translation();
		twistAxis.normalize();
		sr_out << "twist axis = " << twistAxis << srnl;
		if (twistAxis.len() == 0) twistAxis = SrVec(0,1,0);	
		swingAxis = SrVec(0.3f,0.3f,0.3f); SrVec newswingAxis = swingAxis - twistAxis*dot(twistAxis,swingAxis); newswingAxis.normalize();
		swingAxis = newswingAxis;
		swingAxis2 = cross(twistAxis,swingAxis);
	}
	phyJoint->setVec3Attribute("axis2",twistAxis[0],twistAxis[1],twistAxis[2]);	
	phyJoint->setVec3Attribute("axis1",swingAxis[0],swingAxis[1],swingAxis[2]);
	phyJoint->setVec3Attribute("axis0",swingAxis2[0],swingAxis2[1],swingAxis2[2]);		

}

void SbmPhysicsCharacter::updatePDTorque()
{
	std::vector<SbmJointObj*> jointObjList = getJointObjList();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SbmPhysicsSim* phySim = mcu.physicsEngine;
	float Ks = (float)phySim->getDoubleAttribute("Ks");
	float Kd = (float)phySim->getDoubleAttribute("Kd");
	float KScale = (float)phySim->getDoubleAttribute("KScale");

	for (unsigned int i=0;i<jointObjList.size();i++)
	{
		SbmJointObj* obj = jointObjList[i];
		SbmPhysicsJoint* phyJoint = obj->getPhyJoint();
		SBJoint* joint = obj->getPhyJoint()->getSBJoint();
		if (!joint)	continue;

		if (joint->getName() == "base")
		{			
			SrMat tranMat; tranMat.translation(joint->getLocalCenter());				
			SrMat gmat = tranMat*joint->gmat();		
			obj->setGlobalTransform(gmat);			
			obj->enablePhysicsSim(false);
			obj->updatePhySim();
			continue;
		}

		SrMat tran = obj->getRelativeOrientation();		
		SrQuat phyQuat = SrQuat(tran);

		SrQuat pQuat;			
		if (obj->getParentObj())
			pQuat = obj->getParentObj()->getGlobalTransform().rot;

		SrQuat inQuat = phyJoint->getRefQuat();
		SrVec relW = obj->getAngularVel()*pQuat.inverse() - obj->getParentObj()->getAngularVel()*pQuat.inverse(); 
		//if (obj->getParentObj())
		//	relW = relW - obj->getParentObj()->getAngularVel()*pQuat.inverse();
		SrVec relWD = SrVec(0,0,0);
		float scaleKs, scaleKd;
		scaleKs = Ks*KScale*phyJoint->getTotalSupportMass();
		scaleKd = Kd*KScale*phyJoint->getTotalSupportMass();//2.0 * sqrtf( scaleKs);//;

		SrVec torque = computePDTorque(phyQuat,inQuat,relW,relWD,scaleKs,scaleKd)*pQuat;	
		//if (jname == "r_hip")
		//	sr_out << "torque = " << torque << srnl;
		
		if (obj->getParentObj())// && obj->getParentObj()->getParentObj())
		{
			SrVec oldTorque = phyJoint->getJointTorque();
			SrVec torqueDiff = torque - oldTorque;

			float maxTorqueRateOfChange = 2000*phyJoint->getTotalSupportMass();

			torqueDiff.x = (torqueDiff.x<-maxTorqueRateOfChange)?(-maxTorqueRateOfChange):(torqueDiff.x);
			torqueDiff.x = (torqueDiff.x>maxTorqueRateOfChange)?(maxTorqueRateOfChange):(torqueDiff.x);
			torqueDiff.y = (torqueDiff.y<-maxTorqueRateOfChange)?(-maxTorqueRateOfChange):(torqueDiff.y);
			torqueDiff.y = (torqueDiff.y>maxTorqueRateOfChange)?(maxTorqueRateOfChange):(torqueDiff.y);
			torqueDiff.z = (torqueDiff.z<-maxTorqueRateOfChange)?(-maxTorqueRateOfChange):(torqueDiff.z);
			torqueDiff.z = (torqueDiff.z>maxTorqueRateOfChange)?(maxTorqueRateOfChange):(torqueDiff.z);

			//phyJoint->setJointTorque(oldTorque+torqueDiff);
			phyJoint->setJointTorque(torque);
		}
	}
}

SrVec SbmPhysicsCharacter::computePDTorque( SrQuat& q, SrQuat& qD, SrVec& w, SrVec& vD, float Ks, float Kd )
{
	SrVec torque;	
	SrQuat qErr = qD*q.inverse();
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
		torque = v/sinTheta*qAngle*(-Ks)*(float)MeCtMath::sgn(qErr.w);
	}
	torque = torque*q; // rotate back to the parent frame

	// torque for angular velocity damping
	torque += (vD - w)*(-Kd);
	return torque;
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