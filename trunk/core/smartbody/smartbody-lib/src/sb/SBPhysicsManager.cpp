#include "SBPhysicsManager.h"
#ifndef __native_client__
#include <sb/SBPythonClass.h>
#endif
#include <sbm/mcontrol_util.h>
#include <sb/SBScene.h>
#include <sbm/Physics/SbmPhysicsSimODE.h>

#ifdef __ANDROID__
#define USE_PHYSICS_CHARACTER 0
#elif __native_client__
#define USE_PHYSICS_CHARACTER 0
#else
#define USE_PHYSICS_CHARACTER 1	
#endif

namespace SmartBody {

SBPhysicsManager::SBPhysicsManager()
{
	setName("physics");

	physicsTime = 0;
	_ode = new SbmPhysicsSimODE();
	_ode->initSimulation();
}

SBPhysicsManager::~SBPhysicsManager()
{
	delete _ode;
}

SbmPhysicsSim* SBPhysicsManager::getPhysicsEngine()
{
	return _ode;
}


bool SBPhysicsManager::isEnable()
{
	return getPhysicsEngine()->getBoolAttribute("enable");
}

void SBPhysicsManager::setEnable(bool enable)
{
	if (enable)
	{
		// ...
		physicsTime = mcuCBHandle::singleton().time;		
	}
	else
	{
		// ...
	}
	getPhysicsEngine()->setBoolAttribute("enable",enable);
	
}

void SBPhysicsManager::start()
{
}

void SBPhysicsManager::beforeUpdate(double time)
{

}

void SBPhysicsManager::update(double time)
{
	SbmPhysicsSim* physicsEngine = getPhysicsEngine();
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	static double prevTime = -1;
	if (isEnable())
	{
		float dt = (float)physicsEngine->getDoubleAttribute("dT");//timeStep*0.03f;	
		float simLimit = (float)physicsEngine->getDoubleAttribute("MaxSimTime");
		double timeDiff = time - physicsTime;
		double timeElapse = 0.0;
		vhcl::Timer timer;
		vhcl::StopWatch watch(timer);			
		while (physicsTime < time && timeElapse < simLimit)			
		{	
			watch.Start();
			physicsEngine->updateSimulation(dt);
			physicsTime += dt;
			watch.Stop();
			timeElapse += watch.GetTime();
		}	
		physicsTime = time;		
	}
	else
	{
		physicsTime = time;
	}

	// update character
	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		 iter != mcu.getCharacterMap().end();
		 iter++)
	{
		SbmCharacter* character = (*iter).second;
		//character->updateJointPhyObjs(isEnable());
		updatePhysicsCharacter(character->getName());
	}

	for (std::map<std::string, SbmPawn*>::iterator iter = mcu.getPawnMap().begin();
		 iter != mcu.getPawnMap().end();
		 iter++)
	{
		SbmPawn* pawn = (*iter).second;
		updatePhysicsPawn(pawn->getName());
	}
}

void SBPhysicsManager::afterUpdate(double time)
{
}

void SBPhysicsManager::stop()
{
}

SmartBody::SBObject* SBPhysicsManager::getPhysicsSimulationEngine()
{
	return getPhysicsEngine();
}

SmartBody::SBObject* SBPhysicsManager::getPhysicsCharacter( std::string charName )
{
	return this->getPhysicsEngine()->getPhysicsCharacter(charName);
}

SmartBody::SBObject* SBPhysicsManager::getPhysicsJoint( std::string charName, std::string jointName )
{
	SbmPhysicsCharacter* phyChar = this->getPhysicsEngine()->getPhysicsCharacter(charName);
	SbmPhysicsJoint* phyJoint = NULL;
	if (phyChar)
	{
		phyJoint = phyChar->getPhyJoint(jointName);		
	}
	return phyJoint;
}

SmartBody::SBObject* SBPhysicsManager::getJointObj( std::string charName, std::string jointName )
{
	SbmPhysicsCharacter* phyChar = this->getPhysicsEngine()->getPhysicsCharacter(charName);
	SbmJointObj* jointObj = NULL;
	if (phyChar)
	{
		jointObj = phyChar->getJointObj(jointName);		
	}
	return jointObj;	
}

SmartBody::SBObject* SBPhysicsManager::getPhysicsPawn( std::string pawnName )
{
	return getPhysicsEngine()->getPhysicsPawn(pawnName);
}


SmartBody::SBObject* SBPhysicsManager::createPhysicsPawn( std::string pawnName, std::string geomType, SrVec geomSize )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SmartBody::SBScene* scene = mcu._scene;
	SmartBody::SBPawn* pawn = scene->getPawn(pawnName);
	if (!pawn) return NULL;
	SbmPhysicsObj* phyObj = pawn->getPhysicsObject();//new SbmPhysicsObj();
	if (phyObj) getPhysicsEngine()->removePhysicsObj(phyObj); // remove existing physics object

	phyObj = new SbmPhysicsObj();
	phyObj->setName(pawnName);
	SbmGeomObject* geomObj = SbmGeomObject::createGeometry(geomType,geomSize);
	phyObj->setGeometry(geomObj);
	//phyObj->changeGeometry(geomType,geomSize);
	getPhysicsEngine()->addPhysicsObj(phyObj);
	getPhysicsEngine()->updatePhyObjGeometry(phyObj);
	return phyObj;
}

SmartBody::SBObject* SBPhysicsManager::createPhysicsCharacter( std::string charName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SmartBody::SBScene* scene = mcu._scene;
	SmartBody::SBCharacter* sbmChar = scene->getCharacter(charName);
	if (!sbmChar) return NULL; // no character with the name
	SbmPhysicsCharacter* phyChar = NULL;
	phyChar = getPhysicsEngine()->getPhysicsCharacter(charName);
	if (phyChar) return phyChar; // physics character of the same name already exist

	// create physics character
	const std::vector<SkJoint*>& joints = sbmChar->getSkeleton()->joints();	
	//printf("init physics obj\n");	
	phyChar = new SbmPhysicsCharacter();
	std::queue<SkJoint*> tempJointList;
	std::vector<std::string> jointNameList;
	std::set<std::string> excludeNameList; 
	excludeNameList.insert("r_wrist");
	excludeNameList.insert("l_wrist");
	excludeNameList.insert("spine5");
	excludeNameList.insert("l_forefoot");
	excludeNameList.insert("r_forefoot");
	excludeNameList.insert("l_ankle");
	excludeNameList.insert("r_ankle");
	
	SkJoint* rootJoint = sbmChar->getSkeleton()->root();	
	tempJointList.push(rootJoint->child(0));

	while (!tempJointList.empty())
	{
		SkJoint* joint = tempJointList.front(); tempJointList.pop();
		std::string jName = joint->name();
		if (joint->num_children() == 0) // don't process leaves
			continue;
		jointNameList.push_back(jName);
		if (excludeNameList.find(jName) != excludeNameList.end())
			continue;
		for (int i=0;i<joint->num_children();i++)
		{
			SkJoint* cj = joint->child(i);	
			if (std::find(joints.begin(),joints.end(),cj) != joints.end())
				tempJointList.push(cj);
		}
	}	
	phyChar->initPhysicsCharacter(charName,jointNameList,true);
#if USE_PHYSICS_CHARACTER
	getPhysicsEngine()->addPhysicsCharacter(phyChar);
#endif
	return phyChar;
}

void SBPhysicsManager::updatePhysicsPawn( std::string pawnName )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SmartBody::SBScene* scene = mcu._scene;
	SbmPhysicsSim* phyEngine = getPhysicsEngine();
	SbmPhysicsObj* phyObj = phyEngine->getPhysicsPawn(pawnName);
	SBPawn* pawn = scene->getPawn(pawnName);
	if (!phyObj || !pawn) return;

	bool pawnPhySim = (phyEngine->getBoolAttribute("enable") && pawn->getBoolAttribute("enablePhysics"));
	if (pawnPhySim)
	{
		phyObj->updateSbmObj();
		pawn->setWorldOffset(phyObj->getGlobalTransform().gmat());				
	}
	else
	{
		SRT newWorldState; 
		//newWorldState.gmat(pawn->get_world_offset_joint()->gmat());				
		newWorldState.gmat(pawn->get_world_offset());
		phyObj->setRefTransform(phyObj->getGlobalTransform()); // save previous transform
		phyObj->setGlobalTransform(newWorldState);		
		phyObj->updatePhySim();		
	}	
}

void SBPhysicsManager::updatePhysicsCharacter( std::string charName )
{	
	SbmPhysicsSim* phyEngine = getPhysicsEngine();
	SbmPhysicsCharacter* phyChar = phyEngine->getPhysicsCharacter(charName);
	if (!phyChar) return; 

	bool charPhySim = (phyEngine->getBoolAttribute("enable") && phyChar->getBoolAttribute("enable"));
	std::map<std::string,SbmJointObj*> jointPhyObjMap = phyChar->getJointObjMap();	
	for ( std::map<std::string, SbmJointObj*>::iterator mi  = jointPhyObjMap.begin();
		  mi != jointPhyObjMap.end();
		  mi++)
	{
		SbmJointObj* phyObj = mi->second;
		SBJoint* joint = phyObj->getSBJoint();
		const std::string& jointName = joint->name();		
		bool kinematicRoot = (jointName == "base" || jointName == "JtPelvis") && phyChar->getBoolAttribute("kinematicRoot");	
#if USE_PHYSICS_CHARACTER		
		bool constraintObj = false;
		SBPawn* constraintPawn = SmartBody::SBScene::getScene()->getPawn(phyObj->getStringAttribute("constraintTarget"));
		if (charPhySim && constraintPawn && phyObj->getBoolAttribute("constraint"))
		{				
			phyObj->enablePhysicsSim(false);				
			phyObj->setRefTransform(phyObj->getGlobalTransform()); // save previous transform
			phyObj->setGlobalTransform(constraintPawn->get_world_offset());					
			phyObj->setAngularVel(phyObj->getPhyJoint()->getRefAngularVel());
			phyObj->updatePhySim();						
		}

		else if (charPhySim && phyObj->getBoolAttribute("constraint"))
		{
			SrMat tranMat; tranMat.translation(joint->getLocalCenter());	
			phyObj->enablePhysicsSim(false);					
			SrMat gmat = tranMat*phyObj->getRefTransform().gmat();		
			phyObj->setRefTransform(phyObj->getGlobalTransform()); // save previous transform
			phyObj->setGlobalTransform(gmat);					
			phyObj->setAngularVel(phyObj->getPhyJoint()->getRefAngularVel());
			phyObj->updatePhySim();						
		}
		else if (charPhySim && !kinematicRoot && !constraintObj)
		{
			phyObj->enablePhysicsSim(true);
			phyObj->updateSbmObj();
		}
		else
		{		
			SrMat tranMat; tranMat.translation(joint->getLocalCenter());	
			phyObj->enablePhysicsSim(false);			
			SrMat gmat = tranMat*joint->gmat();		
			phyObj->setRefTransform(phyObj->getGlobalTransform()); // save previous transform
			phyObj->setGlobalTransform(gmat);				
			phyObj->setAngularVel(phyObj->getPhyJoint()->getRefAngularVel());
			phyObj->updatePhySim();
		}		
#else
		{			
			SrMat tranMat; tranMat.translation(joint->getLocalCenter());					
			SrMat gmat = tranMat*joint->gmat();		
			phyObj->setGlobalTransform(gmat);
		}			
#endif
		}
}	


}

