#include "SbmPhysicsSimODE.h"
#include <sbm/mcontrol_util.h>
#include <sbm/SBScene.h>
#include <sbm/SBCollisionManager.h>

/************************************************************************/
/* Physics Sim ODE                                                      */
/************************************************************************/

SbmPhysicsSimODE::SbmPhysicsSimODE(void)
{
	hasInit = false;
	initSimulation();
}
SbmPhysicsSimODE::~SbmPhysicsSimODE(void)
{
}

void SbmPhysicsSimODE::nearCallBack(void *data, dGeomID o1, dGeomID o2)
{		
	const int N = 4;
	SbmPhysicsSimODE* phyODE = static_cast<SbmPhysicsSimODE*>(data);
	if (!phyODE)
		return;

	//return;
//  	if (o1 != phyODE->groundID && o2 != phyODE->groundID)
//  		return;

	bool collideFloor = (o1 == phyODE->groundID || o2 == phyODE->groundID);

	dContact contact[N];
	dBodyID b1,b2;
	b1 = dGeomGetBody(o1);
	b2 = dGeomGetBody(o2);

	if (b1 && !dBodyIsEnabled(b1)) b1 = 0;
	if (b2 && !dBodyIsEnabled(b2)) b2 = 0;
	if (!b1 && !b2) // both bodies are disable, no need for collision check
		return;
	bool joined = b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact);
	//bool joined = b1 && b2 && dAreConnected(b1,b2);
 	if (joined) 
 	{		
 		return;	
 	}
	int n =  dCollide(o1,o2,N,&contact[0].geom,sizeof(dContact));
	if (n > 0) // handle collision event
	{
		SbmPhysicsObj* obj1 = NULL; if (b1) obj1 = (SbmPhysicsObj*)(dBodyGetData(b1));
		SbmPhysicsObj* obj2 = NULL; if (b2) obj2 = (SbmPhysicsObj*)(dBodyGetData(b2));
 		dVector3& hpos = contact[0].geom.pos;
 		SrVec hitPt = SrVec((float)hpos[0],(float)hpos[1],(float)hpos[2]);		
 		if (obj1 && !collideFloor) obj1->handleCollision(hitPt,obj2);		
 		if (obj2 && !collideFloor) obj2->handleCollision(hitPt,obj1);			
	}

	for (int i = 0; i < n; i++) {	
		contact[i].surface.mode = dContactSoftERP | dContactSoftCFM |dContactBounce | dContactApprox1;			
		{
			contact[i].surface.bounce     = 0.0f; // (0.0~1.0) restitution parameter
			contact[i].surface.bounce_vel = 0.0f;;
			contact[i].surface.mu = 1.0f;//1000.f;		
			contact[i].surface.soft_cfm = 1e-3;
			contact[i].surface.soft_erp = 0.1;
		}
		//contact[i].surface.bounce_vel = 1000.f; // minimum incoming velocity for bounce 
		dJointID c = dJointCreateContact(phyODE->getWorldID(),phyODE->getContactGroupID(),&contact[i]);
		dJointAttach (c,b1,b2);	
	}
}

static void myMessageFunction(int errnum, const char* msg, va_list ap)
{
	LOG("ODE Error %d %s", errnum, msg);
}

void SbmPhysicsSimODE::initSimulation()
{	
	dSetErrorHandler(&myMessageFunction);
	dSetDebugHandler(&myMessageFunction);
	dSetMessageHandler(&myMessageFunction);


	dInitODE();

	worldID = dWorldCreate();
	//dWorldSetAutoDisableFlag(worldID,1);
	dWorldSetGravity(worldID,0.f,-980.f,0.f);
	//dWorldSetLinearDamping(worldID,0.002f);
	//dWorldSetAngularDamping(worldID,0.01f);
	dWorldSetLinearDamping(worldID,0.0001f);
	dWorldSetAngularDamping(worldID,0.03f);
	//dWorldSetAngularDamping(worldID,0.001f);

	//dWorldSetERP(worldID,0.5);
	//dWorldSetCFM(worldID,1e-3);	

	//spaceID = dHashSpaceCreate(0);
	spaceID = dSimpleSpaceCreate(0);

	groundID = dCreatePlane(spaceID,0,1,0,0.0f); // create a plane at y = 0

	contactGroupID = dJointGroupCreate(0);

	hasInit = true;
}

void SbmPhysicsSimODE::updateSimulationInternal( float timeStep )
{	
	float gravity = (float)SBObject::getDoubleAttribute("gravity");
	dWorldSetGravity(worldID,0.f,-fabs(gravity),0.f);	
	//dWorldSetGravity(worldID,0.f, -9.8, 0.f);
	//dWorldSetGravity(worldID,0.f,-9.8f,0.f);
	//dWorldSetGravity(worldID,0.f,-0.1f,0.f);


	// set external force & torque for each rigid body
	SbmODEObjMap::iterator vi;
	for ( vi  = odeObjMap.begin();
		  vi != odeObjMap.end();
		  vi++)
	{
		SbmODEObj* odeObj = (vi)->second;
	    SbmPhysicsObj* obj = odeObj->physicsObj;
		if (obj)
		{
			const SrVec& extF = obj->getExternalForce();
			const SrVec& extT = obj->getExternalTorque();
			if (extF.y > 1e-5)
				dBodyAddForce(odeObj->bodyID,extF.x,extF.y,extF.z);
			dBodyAddTorque(odeObj->bodyID,extT.x,extT.y,extT.z);
		}
	}
	
	// set torque for each joint
	SbmODEJointMap::iterator ji;
	for ( ji  = odeJointMap.begin();
		  ji != odeJointMap.end();
		  ji++)
	{
		SbmODEJoint* odeJ = (ji)->second;
		SbmPhysicsJoint* j = odeJ->joint;
		if (j)
		{
			const SrVec& jT = j->getJointTorque()*(1.f);
			dBodyAddTorque(odeJ->parentID,jT.x,jT.y,jT.z);
			dBodyAddTorque(odeJ->childID,-jT.x,-jT.y,-jT.z);
			//dBodyAddTorque(odeJ->childID,jT.x,jT.y,jT.z);
		}
	}

	dSpaceCollide(spaceID,this,SbmPhysicsSimODE::nearCallBack);		
	//dWorldStep(worldID,timeStep);
	//dWorldStepFast1(worldID,timeStep,10);
	dWorldQuickStep(worldID,timeStep);	
	dJointGroupEmpty(contactGroupID);

	//std::for_each(physicsObjList.begin(),physicsObjList.end(),std::mem_fun(&SbmPhysicsObj::updateSbmObj));
	SbmPhysicsObjMap::iterator mi;
	for ( mi  = physicsObjList.begin();
		  mi != physicsObjList.end();
		  mi++)
	{
		SbmPhysicsObj* phyObj = mi->second; 
		phyObj->updateSbmObj();
	}
}

SbmPhysicsSimODE* SbmPhysicsSimODE::getODESim()
{
	SbmPhysicsSimODE* odePhysics = dynamic_cast<SbmPhysicsSimODE*>(SbmPhysicsSim::getPhysicsEngine());
	return odePhysics;
}

void SbmPhysicsSimODE::updatePhysicsJoint( SbmPhysicsJoint* phyJoint )
{
	SbmODEJoint* odeJoint = getODEJoint(phyJoint);
	if (!odeJoint)
		return;

	if (phyJoint->getStringAttribute("type") == "ball")//parent && parent->getParentObj())//jname == "r_shoulder" || jname == "r_elbow" || jname == "r_forearm" || jname == "r_wrist")
	{		
		dJointID aMotor = odeJoint->aMotorID;
		double dT = getDoubleAttribute("dT");
		double Ks = getDoubleAttribute("Ks")*phyJoint->getTotalSupportMass();
		double Kd = getDoubleAttribute("Kd")*phyJoint->getTotalSupportMass();
		dReal cfmValue = 1.0/(Ks*dT+Kd);
		dReal erpValue = (Ks*dT)/(Ks*dT+Kd);

		dJointSetAMotorParam(aMotor, dParamStopCFM, cfmValue);
		dJointSetAMotorParam(aMotor, dParamStopCFM1, cfmValue);
		dJointSetAMotorParam(aMotor, dParamStopCFM2, cfmValue);
		dJointSetAMotorParam(aMotor, dParamStopCFM3, cfmValue);

		dJointSetAMotorParam(aMotor, dParamStopERP, erpValue);
		dJointSetAMotorParam(aMotor, dParamStopERP1, erpValue);
		dJointSetAMotorParam(aMotor, dParamStopERP2, erpValue);
		dJointSetAMotorParam(aMotor, dParamStopERP3, erpValue);	

// 		dJointSetAMotorParam(aMotor, dParamLoStop, phyJoint->getDoubleAttribute("axis0LimitLow"));
// 		dJointSetAMotorParam(aMotor, dParamHiStop, phyJoint->getDoubleAttribute("axis0LimitHigh"));
// 		dJointSetAMotorParam(aMotor, dParamLoStop1, phyJoint->getDoubleAttribute("axis0LimitLow"));
// 		dJointSetAMotorParam(aMotor, dParamHiStop1, phyJoint->getDoubleAttribute("axis0LimitHigh"));
// 		dJointSetAMotorParam(aMotor, dParamLoStop2, phyJoint->getDoubleAttribute("axis1LimitLow"));
// 		dJointSetAMotorParam(aMotor, dParamHiStop2, phyJoint->getDoubleAttribute("axis1LimitHigh"));
// 		dJointSetAMotorParam(aMotor, dParamLoStop3, phyJoint->getDoubleAttribute("axis2LimitLow"));
// 		dJointSetAMotorParam(aMotor, dParamHiStop3, phyJoint->getDoubleAttribute("axis2LimitHigh"));
	}
	else // fixed joint
	{

	}
}

void SbmPhysicsSimODE::linkJointObj( SbmJointObj* obj )
{
	SbmJointObj* parent = obj->getParentObj();	
	SbmODEObj* odeObj = getODEObj(obj);	
	if (!parent) 
	{		
		return;
	}

	SbmODEObj* pode = getODEObj(parent);
	std::string jname = obj->getSBJoint()->getName();
	
	SBJoint* joint = obj->getSBJoint();
	SbmPhysicsJoint* phyJoint = obj->getPhyJoint();
	SbmODEJoint* odeJoint = new SbmODEJoint();	
	odeJoint->joint = phyJoint;
	odeJoint->parentID = pode->bodyID;
	odeJoint->childID = odeObj->bodyID;
	if (phyJoint->getStringAttribute("type") == "ball")//parent && parent->getParentObj())//jname == "r_shoulder" || jname == "r_elbow" || jname == "r_forearm" || jname == "r_wrist")
	{
		dJointID j = dJointCreateBall(worldID, 0);
		dJointAttach(j, pode->bodyID, odeObj->bodyID);		
		SrVec apoint = obj->getSBJoint()->getMatrixGlobal().get_translation();
		//now we'll set the world position of the ball-and-socket joint. It is important that the bodies are placed in the world
		//properly at this point
		dJointSetBallAnchor(j, apoint.x,apoint.y,apoint.z);

		//create joint limits	
		dJointID aMotor = dJointCreateAMotor(worldID, 0);
		
		dJointAttach(aMotor, pode->bodyID, odeObj->bodyID);
		dJointSetAMotorMode(aMotor, dAMotorEuler);		

		SrVec twistAxis = phyJoint->getVec3Attribute("axis2");
		SrVec swingAxis = phyJoint->getVec3Attribute("axis0");
		dJointSetAMotorAxis (aMotor, 0, 1, swingAxis.x, swingAxis.y, swingAxis.z);
		dJointSetAMotorAxis (aMotor, 2, 2, twistAxis.x, twistAxis.y, twistAxis.z);

		odeJoint->jointID = j;
		odeJoint->aMotorID = aMotor;
	}
	else
	{
		
		dJointID j = dJointCreateFixed(worldID, 0);	
		dJointAttach(j, pode->bodyID, odeObj->bodyID);
		SrVec apoint = parent->getSBJoint()->getMatrixGlobal().get_translation();
		//now we'll set the world position of the ball-and-socket joint. It is important that the bodies are placed in the world
		//properly at this point
		dJointSetFixed(j);//, apoint.x,apoint.y,apoint.z);		
		odeJoint->jointID = j;
	}

	unsigned long jID = phyJoint->getID();
	odeJointMap[jID] = odeJoint;	
	physicsJointList[jID] = phyJoint;	

	updatePhysicsJoint(phyJoint);
}

void SbmPhysicsSimODE::addPhysicsCharacter( SbmPhysicsCharacter* phyChar )
{
	std::vector<SbmJointObj*> jointObjList = phyChar->getJointObjList();	
	// add rigid body object
	if (hasPhysicsCharacter(phyChar))
		return;

	// add rigid body
	SbmPhysicsSim::addPhysicsCharacter(phyChar);
	for (unsigned int i=0;i<jointObjList.size();i++)
	{
		SbmJointObj* obj = jointObjList[i];
		addPhysicsObj(obj);
		updatePhyObjGeometry(obj,obj->getColObj());	
		LOG("joint obj name = %s",obj->getSBJoint()->getName().c_str());
		
// 		if (obj->getParentObj() == NULL || obj->getSBJoint()->getName() == "base")
// 		{
// 			obj->enableCollisionSim(false);
// 		}			
// 		if (obj->getSBJoint()->getName() == "l_sternoclavicular" || obj->getSBJoint()->getName() == "r_sternoclavicular" )
// 		{
// 			obj->enableCollisionSim(false);
// 		}
		obj->updatePhySim();
	}
	// add joint constraints	
	
	for (unsigned int i=0;i<jointObjList.size();i++)
	{
		SbmJointObj* obj = jointObjList[i];		
		linkJointObj(obj);
	}	
	//phyChar->getJointObj("base")->enablePhysicsSim(false);
	//std::string jname = "world_offset";
	//phyChar->getJointObj(jname)->enablePhysicsSim(false);
		
}

void SbmPhysicsSimODE::removePhysicsCharacter( SbmPhysicsCharacter* phyChar )
{

}


void SbmPhysicsSimODE::addPhysicsObj( SbmPhysicsObj* obj )
{
	if (hasPhysicsObj(obj)) return;
	SbmPhysicsSimODE* odeSim = SbmPhysicsSimODE::getODESim();
	if (!odeSim)	return;
	if (!odeSim->systemIsInit())   return;

	SbmPhysicsSim::addPhysicsObj(obj);
	if (odeObjMap.find(obj->getID()) == odeObjMap.end())
	{
		odeObjMap[obj->getID()] = new SbmODEObj();
	}

	SbmODEObj* odeObj = getODEObj(obj);		
	odeObj->bodyID = dBodyCreate(odeSim->getWorldID());
	dBodySetData(odeObj->bodyID,obj); // attach physics obj with ode body
	odeObj->physicsObj = obj;
	if (obj)
	{	
		SbmTransform& curT = obj->getGlobalTransform();
		dQuaternion quat;
		quat[0] = (dReal)curT.rot.w;	
		quat[1] = (dReal)curT.rot.x;
		quat[2] = (dReal)curT.rot.y;
		quat[3] = (dReal)curT.rot.z;
		dBodySetQuaternion(odeObj->bodyID,quat);
		dBodySetPosition(odeObj->bodyID,(dReal)curT.tran[0],(dReal)curT.tran[1],(dReal)curT.tran[2]);
	}
	if (obj->getColObj())
		updatePhyObjGeometry(obj,obj->getColObj());
}


void SbmPhysicsSimODE::updateODEGeometryTransform( SbmGeomObject* geomObj, dGeomID geomID )
{	
	SbmTransform& curT = geomObj->getCombineTransform();
	dQuaternion quat; 
	quat[0] = (dReal)curT.rot.w;	
	quat[1] = (dReal)curT.rot.x;
	quat[2] = (dReal)curT.rot.y;
	quat[3] = (dReal)curT.rot.z;

	dGeomSetQuaternion(geomID,quat);
	dGeomSetPosition(geomID,(dReal)curT.tran[0],(dReal)curT.tran[1],(dReal)curT.tran[2]);	
}


void SbmPhysicsSimODE::removePhysicsObj( SbmPhysicsObj* obj )
{
	if (!hasPhysicsObj(obj)) return;
	SbmPhysicsSimODE* odeSim = SbmPhysicsSimODE::getODESim();
	if (!odeSim)	return;
	if (!odeSim->systemIsInit())   return;

	SbmPhysicsSim::removePhysicsObj(obj);
	SbmODEObjMap::iterator li = odeObjMap.find(obj->getID());
	if (li != odeObjMap.end())
	{
		SbmODEObj* odeObj = li->second;
		delete odeObj;
		odeObjMap.erase(li);
	}
}

SbmPhysicsObj* SbmPhysicsSimODE::createPhyObj()
{
	return new SbmPhysicsObj();
}

SbmODEObj* SbmPhysicsSimODE::getODEObj( SbmPhysicsObj* obj )
{
	SbmODEObj* odeObj = NULL;
	if (hasPhysicsObj(obj))
	{
		odeObj = odeObjMap[obj->getID()];
	}	
	return odeObj;	
}


SbmODEJoint* SbmPhysicsSimODE::getODEJoint( SbmPhysicsJoint* joint )
{
	SbmODEJoint* odeJoint = NULL;
	unsigned long jointID = (unsigned long)joint;
	if (odeJointMap.find(jointID) != odeJointMap.end())
	{
		odeJoint = odeJointMap[jointID];
	}	
	return odeJoint;	
}


SrVec SbmPhysicsSimODE::getJointConstraintPos( SbmPhysicsJoint* joint )
{
	SbmODEJoint* odeJoint = getODEJoint(joint);
	if (!odeJoint) return SrVec(0,0,0);

	dVector3 jointPoint;
	if (joint->getStringAttribute("type") == "ball")	
		dJointGetBallAnchor(odeJoint->jointID,jointPoint);	
	return SrVec((float)jointPoint[0],(float)jointPoint[1],(float)jointPoint[2]);
}

SrVec SbmPhysicsSimODE::getJointRotationAxis( SbmPhysicsJoint* joint, int axis )
{
	SbmODEJoint* odeJoint = getODEJoint(joint);
	if (!odeJoint) return SrVec(0,0,0);

	dVector3 jointAxis;
	if (joint->getStringAttribute("type") == "ball")	
		dJointGetAMotorAxis(odeJoint->aMotorID,axis,jointAxis);	
	return SrVec((float)jointAxis[0],(float)jointAxis[1],(float)jointAxis[2])*10.f;	
}

void SbmPhysicsSimODE::updatePhyObjGeometry( SbmPhysicsObj* obj, SbmGeomObject* geom /*= NULL*/ )
{
	SbmPhysicsSimODE* odeSim = SbmPhysicsSimODE::getODESim();
	if (!odeSim)	return;
	if (!odeSim->systemIsInit())   return;
	SbmODEObj* odeObj = getODEObj(obj);	
	if (odeObj)
	{
		if (geom && geom != obj->getColObj())
		{
			obj->setGeometry(geom);		
		}
		odeObj->cleanGeometry();
		//odeObj->geomID = createODEGeometry(obj,obj->getDensity());
		//obj->setMass(1.f);		
		odeObj->geomID = createODEGeometry(obj,obj->getMass());
		if (odeObj->geomID && obj->getColObj())
		{			
			dGeomSetBody(odeObj->geomID ,odeObj->bodyID);
			SbmTransform& offsetT = obj->getColObj()->getLocalTransform();
			dGeomSetOffsetPosition(odeObj->geomID,(dReal)offsetT.tran[0],(dReal)offsetT.tran[1],(dReal)offsetT.tran[2]);
			dQuaternion quat;
			quat[0] = (dReal)offsetT.rot.w;	
			quat[1] = (dReal)offsetT.rot.x;
			quat[2] = (dReal)offsetT.rot.y;
			quat[3] = (dReal)offsetT.rot.z;
			dGeomSetOffsetQuaternion(odeObj->geomID,quat);	
			odeObj->odeMass.translate((dReal)offsetT.tran[0],(dReal)offsetT.tran[1],(dReal)offsetT.tran[2]);
			//obj->setMass((float)odeObj->odeMass.mass);
		}
		LOG("obj mass = %f",odeObj->odeMass.mass);
		dBodySetMass(odeObj->bodyID,&odeObj->odeMass);				
	}	
}
void SbmPhysicsSimODE::enablePhysicsSim( SbmPhysicsObj* obj, bool bSim )
{
	SbmODEObj* odeObj = getODEObj(obj);	
	obj->hasPhysicsSim(bSim);
	if (obj->hasPhysicsSim())
		//dBodyEnable(odeObj->bodyID);
		dBodySetDynamic(odeObj->bodyID);
	else
		dBodySetKinematic(odeObj->bodyID);
		//dBodyDisable(odeObj->bodyID);	
}

void SbmPhysicsSimODE::enableCollisionSim( SbmPhysicsObj* obj, bool bCol )
{
	SbmODEObj* odeObj = getODEObj(obj);	
	obj->hasCollisionSim(bCol);
	if (obj->hasCollisionSim())
		dGeomEnable(odeObj->geomID);
	else
		dGeomDisable(odeObj->geomID);
}

void SbmPhysicsSimODE::writeToPhysicsObj( SbmPhysicsObj* obj )
{
	if (!obj->hasPhysicsSim())
		return;
	SbmODEObj* odeObj = getODEObj(obj);	
	// write the current simulation result into object world state
	SbmTransform curT;//colObj->getGlobalTransform();
	const dReal* quat = dBodyGetQuaternion(odeObj->bodyID);
	curT.rot.w = (float)quat[0];
	curT.rot.x = (float)quat[1];
	curT.rot.y = (float)quat[2];
	curT.rot.z = (float)quat[3];

	const dReal* pos = dBodyGetPosition(odeObj->bodyID);
	curT.tran[0] = (float)pos[0];
	curT.tran[1] = (float)pos[1];
	curT.tran[2] = (float)pos[2];
	obj->setGlobalTransform(curT);

	const dReal* linVel = dBodyGetLinearVel(odeObj->bodyID);
	obj->setLinearVel(SrVec((float)linVel[0],(float)linVel[1],(float)linVel[2]));

	const dReal* angVel = dBodyGetAngularVel(odeObj->bodyID);
	obj->setAngularVel(SrVec((float)angVel[0],(float)angVel[1],(float)angVel[2]));
}

void SbmPhysicsSimODE::readFromPhysicsObj( SbmPhysicsObj* obj )
{
	SbmODEObj* odeObj = getODEObj(obj);	
	SbmTransform& curT = obj->getGlobalTransform();
	dQuaternion quat;
	quat[0] = (dReal)curT.rot.w;	
	quat[1] = (dReal)curT.rot.x;
	quat[2] = (dReal)curT.rot.y;
	quat[3] = (dReal)curT.rot.z;
	dBodySetQuaternion(odeObj->bodyID,quat);
	dBodySetPosition(odeObj->bodyID,(dReal)curT.tran[0],(dReal)curT.tran[1],(dReal)curT.tran[2]);	
	
 	SrVec lvel = obj->getLinearVel();
 	dBodySetLinearVel(odeObj->bodyID,(dReal)lvel[0],(dReal)lvel[1],(dReal)lvel[2]);
 	SrVec avel = obj->getAngularVel();
 	dBodySetAngularVel(odeObj->bodyID,(dReal)avel[0],(dReal)avel[1],(dReal)avel[2]);	
}

dGeomID SbmPhysicsSimODE::createODERawGeometry( SbmGeomObject* geomObj )
{
	if (!geomObj) return 0;
	dGeomID geomID = 0;
	if (geomObj->geomType() == "sphere")
	{
		SbmGeomSphere* sph = dynamic_cast<SbmGeomSphere*>(geomObj);
		geomID = dCreateSphere(0,(dReal)sph->radius);
	}
	else if (geomObj->geomType() == "box")
	{
		SbmGeomBox* box = dynamic_cast<SbmGeomBox*>(geomObj);
		SrVec extent = box->extent*2.f;
		geomID = dCreateBox(0,(dReal)extent.x,(dReal)extent.y,(dReal)extent.z);
	}
	else if (geomObj->geomType() == "capsule")
	{
		SbmGeomCapsule* cap = dynamic_cast<SbmGeomCapsule*>(geomObj);		
		if (cap->extent == 0.0)
			cap->extent = 0.01f;
		geomID = dCreateCapsule(0,(dReal)cap->radius,(dReal)cap->extent);
	}
	dGeomSetData(geomID,geomObj);
	return geomID;
}

void SbmPhysicsSimODE::updateODEMass( dMass& odeMass, SbmGeomObject* geomObj, float mass )
{
	if (!geomObj)
	{
		dMassAdjust(&odeMass,(dReal)mass);
		return;
	}  

	if (geomObj->geomType() == "sphere")
	{
		SbmGeomSphere* sph = dynamic_cast<SbmGeomSphere*>(geomObj);
		dMassSetSphereTotal(&odeMass,(dReal)mass,(dReal)sph->radius);
	}
	else if (geomObj->geomType() == "box")
	{
		SbmGeomBox* box = dynamic_cast<SbmGeomBox*>(geomObj);
		SrVec extent = box->extent*2.f;
		dMassSetBoxTotal(&odeMass,(dReal)mass,(dReal)extent.x,(dReal)extent.y,(dReal)extent.z);
	}
	else if (geomObj->geomType() == "capsule")
	{
		SbmGeomCapsule* cap = dynamic_cast<SbmGeomCapsule*>(geomObj);		
		if (cap->extent == 0.0)
			cap->extent = 0.01f;
		dMassSetCapsuleTotal(&odeMass,(dReal)mass,3,(dReal)cap->radius,(dReal)cap->extent);
	}
	else
	{
		dMassAdjust(&odeMass,(dReal)mass);
	}
}

dGeomID SbmPhysicsSimODE::createODEGeometry( SbmPhysicsObj* obj, float mass )
{
	SbmPhysicsSimODE* odeSim = SbmPhysicsSimODE::getODESim();
	if (!odeSim)	return 0;
	if (!odeSim->systemIsInit())   return 0;
	SbmODEObj* odeObj = getODEObj(obj);	
	SbmGeomObject* geom = obj->getColObj();
	dGeomID geomID = SbmPhysicsSimODE::createODERawGeometry(geom);
	if (geomID) dSpaceAdd(odeSim->getSpaceID(),geomID);
	SbmPhysicsSimODE::updateODEMass(odeObj->odeMass,geom,mass);	
	return geomID;

/*
	dGeomID geomID = 0;
	if (dynamic_cast<SbmGeomSphere*>(geom))
	{
		SbmGeomSphere* sph = dynamic_cast<SbmGeomSphere*>(geom);
		geomID = dCreateSphere(odeSim->getSpaceID(),(dReal)sph->radius);
		dMassSetSphereTotal(&odeObj->odeMass,(dReal)mass,(dReal)sph->radius);
	}
	else if (dynamic_cast<SbmGeomBox*>(geom))
	{
		SbmGeomBox* box = dynamic_cast<SbmGeomBox*>(geom);
		SrVec extent = box->extent*2.f;
		geomID = dCreateBox(odeSim->getSpaceID(),(dReal)extent.x,(dReal)extent.y,(dReal)extent.z);
		dMassSetBoxTotal(&odeObj->odeMass,(dReal)mass,(dReal)extent.x,(dReal)extent.y,(dReal)extent.z);
	}
	else if (dynamic_cast<SbmGeomCapsule*>(geom))
	{
		SbmGeomCapsule* cap = dynamic_cast<SbmGeomCapsule*>(geom);		
		if (cap->extent == 0.0)
			cap->extent = 0.01f;
		geomID = dCreateCapsule(odeSim->getSpaceID(),(dReal)cap->radius,(dReal)cap->extent);
		dMassSetCapsuleTotal(&odeObj->odeMass,(dReal)mass,3,(dReal)cap->radius,(dReal)cap->extent);
	}
#if USE_ODE_MESH
	else if (dynamic_cast<SbmGeomTriMesh*>(geom))
	{
		SbmGeomTriMesh* tri = dynamic_cast<SbmGeomTriMesh*>(geom);
		SrModel* model = tri->geoMesh;
		//model->invert_faces();
		SrBox bbox;		
		model->get_bounding_box(bbox);		
		odeObj->meshdataID = dGeomTriMeshDataCreate();
		//dGeomTriMeshDataBuildSimple(meshdataID,(const dReal*)(&model->V[0]),model->V.size(),(const dTriIndex*)(&model->F[0]),model->F.size()*3);
		dGeomTriMeshDataBuildSingle(odeObj->meshdataID,(const dReal*)(&model->V[0]),3*sizeof(float),model->V.size(),(const dTriIndex*)(&model->F[0]),model->F.size()*3,3*sizeof(int));
		geomID = dCreateTriMesh(odeSim->getSpaceID(),odeObj->meshdataID,NULL,NULL,NULL);		
		dMassSetTrimesh(&odeObj->odeMass,(dReal)mass,geomID);		
		if (dMassCheck(&odeObj->odeMass) != 1) // set the default mass to its bounding box
			dMassSetBox(&odeObj->odeMass,(dReal)mass,bbox.size().x*0.5f,bbox.size().y*0.5f,bbox.size().z*0.5f);		
		//dGeomSetPosition(geomID,-odeMass.c[0], -odeMass.c[1], -odeMass.c[2]);
		dMassTranslate( &odeObj->odeMass, -odeObj->odeMass.c[0], -odeObj->odeMass.c[1], -odeObj->odeMass.c[2]);		
	}
#endif
	else // no geoemtry
	{
		//dMassSetZero(&odeMass);			
		dMassAdjust(&odeObj->odeMass,(dReal)mass);
	}
	*/	
}


SbmODEObj::SbmODEObj()
{
	bodyID = 0;
	geomID = 0;
	meshdataID = 0;
	physicsObj = NULL;
}

SbmODEObj::~SbmODEObj()
{
	cleanGeometry();
	if (bodyID)
		dBodyDestroy(bodyID);
}

void SbmODEObj::cleanGeometry()
{
	if (geomID)
		dGeomDestroy(geomID);
	if (meshdataID)
		dGeomTriMeshDataDestroy(meshdataID);	
	geomID = 0;
	meshdataID = 0;
}



SbmODEJoint::SbmODEJoint()
{
	jointID = 0;
	aMotorID = 0;
	parentID = childID = 0;
	joint = NULL;
}

SbmODEJoint::~SbmODEJoint()
{
	
}

/************************************************************************/
/* Collision Space ODE                                                  */
/************************************************************************/

SbmCollisionSpaceODE::SbmCollisionSpaceODE()
{
	// in case ode is not yet initialized
	dInitODE();	
	spaceID = dSimpleSpaceCreate(0);	
}

void SbmCollisionSpaceODE::addCollisionObjects( const std::string& objName )
{
	SBCollisionManager* colManager = SmartBody::SBScene::getScene()->getCollsionManager();
	SbmGeomObject* obj = colManager->getCollisionObject(objName);
	if (obj)
	{
		SbmCollisionSpace::addCollisionObjects(objName);
		dGeomID geomID = SbmPhysicsSimODE::createODERawGeometry(obj);
		SbmPhysicsSimODE::updateODEGeometryTransform(obj,geomID);
		odeGeomMap[objName] = geomID;
		odeGeomNameMap[geomID] = objName;
		dSpaceAdd(spaceID,geomID);		
	}	
}

void SbmCollisionSpaceODE::removeCollisionObjects( const std::string& objName )
{
	SBCollisionManager* colManager = SmartBody::SBScene::getScene()->getCollsionManager();
	SbmGeomObject* obj = colManager->getCollisionObject(objName);
	if (!obj) return;

	dGeomID geomID = getODEGeomID(objName);
	if (geomID) 
	{
		odeGeomMap.erase(objName);
		odeGeomNameMap.erase(geomID);

		dSpaceRemove(spaceID,geomID);
		dGeomDestroy(geomID);		
	}
	SbmCollisionSpace::removeCollisionObjects(objName);
}

dGeomID SbmCollisionSpaceODE::getODEGeomID( const std::string& geomName )
{
	dGeomID geomID = NULL;
	if (odeGeomMap.find(geomName) != odeGeomMap.end())
	{
		geomID = odeGeomMap[geomName];
	}
	return geomID;
}

void SbmCollisionSpaceODE::getPotentialCollisionPairs( SbmCollisionPairList& collisionPairs )
{
	std::map<std::string,SbmGeomObject*>::iterator vi;
	for ( vi  = collsionObjMap.begin();
		  vi != collsionObjMap.end();
		  vi++)
	{
		std::string geomName = (*vi).first;
		SbmGeomObject* geomObj = (*vi).second;
		dGeomID geomID = getODEGeomID(geomName);
		if (geomID)
		{
			SbmPhysicsSimODE::updateODEGeometryTransform(geomObj,geomID);
		}
	}
	curCollisionPairs.clear();
	dSpaceCollide(spaceID,this,SbmCollisionSpaceODE::collisionSpaceNearCallBack);
	collisionPairs = curCollisionPairs;
}

void SbmCollisionSpaceODE::collisionSpaceNearCallBack( void *data, dGeomID o1, dGeomID o2 )
{
	SbmCollisionSpaceODE* colSpace = (SbmCollisionSpaceODE*)data;
	SbmCollisionPairList& curColPairs = colSpace->getCurrentCollisionPairList();
	//SbmGeomObject* g1 = (SbmGeomObject*)dGeomGetData(o1);
	//SbmGeomObject* g2 = (SbmGeomObject*)dGeomGetData(o2);
	curColPairs.push_back(SbmCollisionPair(colSpace->getODEGeomName(o1),colSpace->getODEGeomName(o2)));
}

SbmCollisionSpaceODE::~SbmCollisionSpaceODE()
{

}

std::string SbmCollisionSpaceODE::getODEGeomName( dGeomID geomID )
{
	std::string geomName = "";
	if (odeGeomNameMap.find(geomID) != odeGeomNameMap.end())
	{
		geomName = odeGeomNameMap[geomID];
	}
	return geomName;
}

SbmCollisionPairList& SbmCollisionSpaceODE::getCurrentCollisionPairList()
{
	return curCollisionPairs;
}