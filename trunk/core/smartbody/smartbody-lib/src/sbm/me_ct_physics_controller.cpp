#include "me_ct_physics_controller.h"
#include <sr/sr_euler.h>
#include <sbm/mcontrol_util.h>

std::string MeCtPhysicsController::CONTROLLER_TYPE = "PhysicsController";

MeCtPhysicsController::MeCtPhysicsController(SbmCharacter* character) : SmartBody::SBController()
{
	_character = character;
	_valid = true;
	_prevTime = 0.0;
	_duration = 0.0;
}

MeCtPhysicsController::~MeCtPhysicsController()
{
	
}


bool MeCtPhysicsController::controller_evaluate(double t, MeFrameData& frame)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (_prevTime == 0)
		_dt = 0.016;
	else
	{
		_dt = t - _prevTime;
		_prevTime = t;
	}	
	bool hasPhy = mcu.physicsEngine->getBoolAttribute("enable");
#if 1
	if (_valid && _context && hasPhy)
	{
		SbmPhysicsCharacter* phyChar = _character->getPhysicsCharacter();
		std::vector<SbmJointObj*> jointObjList = phyChar->getJointObjList();

		// recompute joint torque 
		// update physics simulation results to character channel arrays
		for (unsigned int i=0;i<jointObjList.size();i++)
		{
			SbmJointObj* obj = jointObjList[i];
			SbmPhysicsJoint* phyJoint = obj->getPhyJoint();
			SBJoint* joint = obj->getPhyJoint()->getSBJoint();

			if (joint->getName() == "base")
				continue;

			if (!joint)	continue;

			SrMat tran = obj->getRelativeOrientation();					
			std::string jname = joint->getName();
			std::string jPosName = joint->getName();
			
			int channelId = _context->channels().search(jname, SkChannel::Quat);
			if (channelId < 0)	continue;
			int bufferId = frame.toBufferIndex(channelId);
			if (bufferId < 0)	continue;	

			

			SrQuat phyQuat = SrQuat(tran);
			SrQuat inQuat;
			// input reference pose
			inQuat.w = frame.buffer()[bufferId + 0];
			inQuat.x = frame.buffer()[bufferId + 1];
			inQuat.y = frame.buffer()[bufferId + 2];
			inQuat.z = frame.buffer()[bufferId + 3];
			phyJoint->setRefQuat(inQuat);
			// compute current relative ang velocity	
			/*
			SrQuat pQuat;			
			if (obj->getParentObj())
				pQuat = obj->getParentObj()->getGlobalTransform().rot;
			SrVec relW = obj->getAngularVel()*pQuat.inverse(); 
			if (obj->getParentObj())
				relW = relW - obj->getParentObj()->getAngularVel()*pQuat.inverse();
			SrVec relWD = SrVec(0,0,0);

			SrVec torque = computePDTorque(phyQuat,inQuat,relW,relWD)*pQuat;	
			//if (jname == "r_hip")
			//	sr_out << "torque = " << torque << srnl;
			if (obj->getParentObj())// && obj->getParentObj()->getParentObj())
			{
				SrVec oldTorque = phyJoint->getJointTorque();
				SrVec torqueDiff = torque - oldTorque;

				float maxTorqueRateOfChange = 2000;

				torqueDiff.x = (torqueDiff.x<-maxTorqueRateOfChange)?(-maxTorqueRateOfChange):(torqueDiff.x);
				torqueDiff.x = (torqueDiff.x>maxTorqueRateOfChange)?(maxTorqueRateOfChange):(torqueDiff.x);
				torqueDiff.y = (torqueDiff.y<-maxTorqueRateOfChange)?(-maxTorqueRateOfChange):(torqueDiff.y);
				torqueDiff.y = (torqueDiff.y>maxTorqueRateOfChange)?(maxTorqueRateOfChange):(torqueDiff.y);
				torqueDiff.z = (torqueDiff.z<-maxTorqueRateOfChange)?(-maxTorqueRateOfChange):(torqueDiff.z);
				torqueDiff.z = (torqueDiff.z>maxTorqueRateOfChange)?(maxTorqueRateOfChange):(torqueDiff.z);

				//phyJoint->setJointTorque(oldTorque+torqueDiff);
				phyJoint->setJointTorque(torque);
			}
			*/

			//if (joint->getParent()) quat = SrQuat();
			frame.buffer()[bufferId + 0] = phyQuat.w;
			frame.buffer()[bufferId + 1] = phyQuat.x;
			frame.buffer()[bufferId + 2] = phyQuat.y;
			frame.buffer()[bufferId + 3] = phyQuat.z;
			
			int positionChannelID = _context->channels().search(jPosName, SkChannel::XPos);
			if (positionChannelID < 0) continue;
			int posBufferID = frame.toBufferIndex(positionChannelID);
			if (posBufferID < 0)	continue;
			SrVec pos = tran.get_translation();
			frame.buffer()[posBufferID + 0] = pos.x;
			frame.buffer()[posBufferID + 1] = pos.y;
			frame.buffer()[posBufferID + 2] = pos.z;			
		}				
	}
#endif
	return true;
}

