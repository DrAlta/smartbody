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
	float invDt = 1.f/0.016;
	if (_dt > 1e-6)
		invDt = 1.f/(float)_dt;	
#if 0
	if (_valid && _context )
	{
		SbmPhysicsCharacter* phyChar = _character->getPhysicsCharacter();
		if (!phyChar) return true;

		bool hasPhy = (mcu.physicsEngine->getBoolAttribute("enable") && phyChar->getBoolAttribute("enable"));		
		std::vector<SbmJointObj*> jointObjList = phyChar->getJointObjList();

		// recompute joint torque 
		// update physics simulation results to character channel arrays
		for (unsigned int i=0;i<jointObjList.size();i++)
		{
			SbmJointObj* obj = jointObjList[i];
			SbmPhysicsJoint* phyJoint = obj->getPhyJoint();
			SBJoint* joint = obj->getPhyJoint()->getSBJoint();

			bool kinematicRoot = (joint->getName() == "base" || joint->getName() == "JtPelvis") && phyChar->getBoolAttribute("kinematicRoot");
			if (kinematicRoot)
				continue;

			if (!joint)	continue;

			//obj->updateSbmObj();
			SrMat tran = obj->getRelativeOrientation();					
			std::string jname = joint->getName();
			std::string jPosName = joint->getName();
			
			int channelId = _context->channels().search(jname, SkChannel::Quat);
			if (channelId < 0)	continue;
			int bufferId = frame.toBufferIndex(channelId);
			if (bufferId < 0)	continue;	

			
			SrMat preRot;
			joint->quat()->prerot().inverse().get_mat(preRot);

			SrQuat oldPhyQuat = SrQuat(tran);
			SrQuat phyQuat = SrQuat(tran*preRot);
			SrQuat inQuat;
			// input reference pose
			inQuat.w = frame.buffer()[bufferId + 0];
			inQuat.x = frame.buffer()[bufferId + 1];
			inQuat.y = frame.buffer()[bufferId + 2];
			inQuat.z = frame.buffer()[bufferId + 3];
			
			SrQuat newQuat = (joint->quat()->prerot()*inQuat);

			
			SrMat newMat; newQuat.get_mat(newMat);						
			SrQuat pQuat;			
			if (joint->getParent())
			{
				pQuat = SrQuat(joint->getParent()->getMatrixGlobal());
				newMat = newMat*joint->getParent()->getMatrixGlobal();
			}
			
			SrQuat prevQuat = phyJoint->getRefQuat();
 			SrQuat quatDiff = (inQuat*prevQuat.inverse());
			quatDiff.normalize();
			SrVec angDiff = quatDiff.axisAngle();
 			angDiff = angDiff*prevQuat*pQuat;
 			angDiff = angDiff*invDt;	


			SrQuat pQuatPhy;			
			if (obj->getParentObj())
				pQuatPhy = obj->getParentObj()->getGlobalTransform().rot;

			SrQuat newGQuat = SrQuat(newMat);			
			//LOG("joint name = %s",joint->getName().c_str());
			//sr_out << "gmat Quat = " << SrQuat(joint->getMatrixGlobal()) << srnl;
			//sr_out << "old phyQuat = " << pQuatPhy*oldPhyQuat << "  , newQuat = " << newQuat*pQuat << srnl;
			//sr_out << "new GQuat = " << newGQuat << srnl;
			
			//LOG("joint name = %s",joint->getName().c_str());
			//sr_out << "angVel = " << angDiff << srnl;
 			phyJoint->setRefAngularVel(angDiff);			
			phyJoint->setRefQuat(inQuat);
			if (hasPhy)
			{
				frame.buffer()[bufferId + 0] = phyQuat.w;
				frame.buffer()[bufferId + 1] = phyQuat.x;
				frame.buffer()[bufferId + 2] = phyQuat.y;
				frame.buffer()[bufferId + 3] = phyQuat.z;

				int positionChannelID = _context->channels().search(jPosName, SkChannel::XPos);
				if (positionChannelID < 0) continue;
				int posBufferID = frame.toBufferIndex(positionChannelID);
				if (posBufferID < 0)	continue;
				SrVec pos = tran.get_translation() - joint->offset(); // remove the joint offset in local space to get actual Pos channel values
				//LOG("joint name = %s",joint->getName().c_str());
				//sr_out << "Pos = " << pos << srnl;
				frame.buffer()[posBufferID + 0] = pos.x;
				frame.buffer()[posBufferID + 1] = pos.y;
				frame.buffer()[posBufferID + 2] = pos.z;	
			}
			else
			{
				SbmTransform diffSRT = SbmTransform::diff(obj->getGlobalTransform(),obj->getRefTransform());
				SrVec refAngVel = diffSRT.rot.axisAngle()*obj->getGlobalTransform().rot;
				refAngVel = refAngVel*invDt;
				SrVec refLinearVel = diffSRT.tran*invDt;
				obj->setVec3Attribute("refLinearVelocity",refLinearVel[0],refLinearVel[1],refLinearVel[2]);
				obj->setVec3Attribute("refAngularVelocity",refAngVel[0],refAngVel[1],refAngVel[2]);
				obj->setLinearVel(refLinearVel);
				obj->setAngularVel(refAngVel);
			}
		}				
	}
#endif
	return true;
}

