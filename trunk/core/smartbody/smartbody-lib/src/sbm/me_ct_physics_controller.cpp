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
#if 0
	if (_valid && _context && hasPhy)
	{
		SbmPhysicsCharacter* phyChar = _character->getPhysicsCharacter();
		std::vector<SbmJointObj*> jointObjList = phyChar->getJointObjList();
		for (unsigned int i=0;i<jointObjList.size();i++)
		{
			SbmJointObj* obj = jointObjList[i];
			SBJoint* joint = obj->getJoint();
			if (!joint)	continue;
			SrMat tran = obj->getRelativeOrientation();
			//if (joint->getParent()) continue;
			//if (joint->getName() == "base" || joint->getName() == "world_offset")
			//	continue;
			std::string jname = joint->getName();
			std::string jPosName = joint->getName();
			//if (joint->getParent() && joint->getParent()->getParent()) 
			//	jname = joint->getParent()->getName();
			
			int channelId = _context->channels().search(jname, SkChannel::Quat);
			if (channelId < 0)	continue;
			int bufferId = frame.toBufferIndex(channelId);
			if (bufferId < 0)	continue;			
			SrQuat quat = SrQuat(tran);
			//if (joint->getParent()) quat = SrQuat();
			frame.buffer()[bufferId + 0] = quat.w;
			frame.buffer()[bufferId + 1] = quat.x;
			frame.buffer()[bufferId + 2] = quat.y;
			frame.buffer()[bufferId + 3] = quat.z;
			
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
