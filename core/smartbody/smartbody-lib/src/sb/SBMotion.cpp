#include "SBMotion.h"
#include <sbm/mcontrol_util.h>
#include <sb/SBScene.h>
#include <sbm/me_utilities.hpp>
#include <sr/sr_euler.h>
#include <sb/SBSkeleton.h>
#include <sb/SBJoint.h>

namespace SmartBody {

FootStepRecord::FootStepRecord()
{

}

FootStepRecord::~FootStepRecord()
{

}

FootStepRecord& FootStepRecord::operator=( const FootStepRecord& rt )
{
	jointNames = rt.jointNames;
	posVec    = rt.posVec;
	startTime = rt.startTime;
	endTime   = rt.endTime;
	return *this;
}

void FootStepRecord::updateJointAveargePosition( SBSkeleton* skel, SBMotion* motion )
{
	int steps = 0;
	posVec.resize(jointNames.size());
	for (unsigned int i=0;i<posVec.size();i++) posVec[i] = SrVec();

	float frameRate = (float)motion->getFrameRate();
	motion->connect(skel);
	//for (float t = startTime; t <= endTime; t += frameRate)
	float t = startTime;
	{
		motion->apply(t);
		skel->update_global_matrices();
		for (unsigned int k=0;k<jointNames.size();k++)
		{
			SBJoint* joint = skel->getJointByName(jointNames[k]);
			if (joint)
			{
				posVec[k] += joint->gmat().get_translation();				
			}
		}		
		steps++;
	}
	for (unsigned int i=0;i<posVec.size();i++)
		posVec[i] /= (float)steps;
	motion->disconnect();			
}

SBMotion::SBMotion() : SkMotion()
{
	_motionFile = "";

	alignIndex = 0; 
}

SBMotion::SBMotion(const SBMotion& motion)
{
	//...

	alignIndex = 0;
}

SBMotion::SBMotion(std::string file) : SkMotion()
{
	_motionFile = file;

	alignIndex = 0;
}

SBMotion::~SBMotion()
{
	_motionFile = "";
}

const std::string& SBMotion::getMotionFileName()
{
	return filename();
}

int SBMotion::getNumFrames()
{
	return frames();
}

std::vector<float> SBMotion::getFrameData(int frameId)
{
	std::vector<float> ret;
	for (int i = 0; i < getFrameSize(); i++)
		ret.push_back(posture(frameId)[i]);
	return ret;
}

int SBMotion::getFrameSize()
{
	return posture_size();
}

int SBMotion::getNumChannels()
{
	return channels().size();
}

std::vector<std::string> SBMotion::getChannels()
{
	std::vector<std::string> ret;
	for (int i = 0; i < channels().size(); i++)
	{
		std::string chanName = channels().name(i);
		//if (channels()[i].joint)
		//	chanName = channels()[i].joint->name().c_str();
		int	chanType = channels()[i].type;
		std::string chanTypeString;
		switch (chanType)
		{
			case 0:
				chanTypeString = "XPos";
				break;
			case 1:	
				chanTypeString = "YPos";
				break;
			case 2:
				chanTypeString = "ZPos";
				break;
			case 6:
				chanTypeString = "Quat";
				break;
			default:
				chanTypeString = "Others";
		}
		std::string name = chanName + "_" + chanTypeString;
		ret.push_back(name);
	}
	return ret;
}

void SBMotion::checkSkeleton(std::string skel)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	int chanSize;
	SkChannel chan;

	SkMotion* motion;
	std::map<std::string, SkMotion*>::iterator motionIter = mcu.motion_map.find(getName().c_str());
	if (motionIter != mcu.motion_map.end())
		motion = motionIter->second;
	else
	{
		LOG("checkSkeleton ERR: Motion %s NOT EXIST!", getName().c_str());
		return;
	}

	SkSkeleton* skSkel = load_skeleton(skel.c_str(), mcu.me_paths, mcu.resource_manager, mcu.skScale);
	if (skSkel)
	{
		int numValidChannels = motion->connect(skSkel);	// connect and check for the joints
		SkChannelArray& mChanArray = motion->channels();
		int mChanSize = mChanArray.size();
		SkChannelArray& skelChanArray = skSkel->channels();
		int skelChanSize = skelChanArray.size();
		chanSize = mChanSize;
		LOG("Channels in skeleton %s's channel matching motion %s's channel are preceeded with '+'", skel.c_str(), getName().c_str());
		LOG("motion %s's Channel Info:", getName().c_str());
		LOG("Channel Size: %d", chanSize);
		for (int i = 0; i < chanSize; i++)
		{				
			std::stringstream outputInfo;
			chan = mChanArray[i];
			std::string jointName = chan.joint->name().c_str();
			int	chanType = chan.type;
			std::string chanTypeString;
			switch (chanType)
			{
				case 0:
					chanTypeString = "XPos";
					break;
				case 1:	
					chanTypeString = "YPos";
					break;
				case 2:
					chanTypeString = "ZPos";
					break;
				case 6:
					chanTypeString = "Quat";
					break;
				default:
					chanTypeString = "Others";
			}
			int pos;
			pos = skelChanArray.linear_search(chan.joint->name(), chan.type);
			if (pos != -1)
				outputInfo << "+ ";
			if (pos == -1)	
				outputInfo << "  ";
			outputInfo << i << ": " << jointName.c_str() << " (" << chanTypeString << ")";
			LOG("%s", outputInfo.str().c_str());
		}
	}
	else
		LOG("Skeleton %s NOT EXIST!", skel.c_str());
}

void SBMotion::connect(SBSkeleton* skel)
{
	SkMotion::connect(skel);
}

void SBMotion::disconnect()
{
	SkMotion::disconnect();
}


void SBMotion::alignToBegin(int numFrames)
{
	alignToSide(numFrames, 1);
}

void SBMotion::alignToEnd(int numFrames)
{
	alignToSide(numFrames, 0);
}

int SBMotion::getAlignIndex()
{
	return alignIndex;
}

void SBMotion::recoverAlign()
{
	if (alignIndex > 0)
		alignToEnd(alignIndex);
	else if (alignIndex < 0)
		alignToBegin(-alignIndex);
}

void SBMotion::alignToSide(int numFrames, int direction)
{
	if (numFrames >= getNumFrames())
	{
		LOG("SBMotion::alignToSide WARNING: number of frames %d exceed the motion cycle.", numFrames);
		return;
	}

	if (direction != 0 && direction != 1)
	{
		LOG("SBMotion::alignToSide WARNING: direction %d not valid.", direction);
		return;
	}

	if (getNumFrames() == 1)
	{
		LOG("SBMotion::alignToSide WARNING: motion %s only has one frame, no need to align.", this->getName().c_str());
		return;
	}

	// register previous motion
	registerAnimation();
	
	// handle base joint
	// get adjust matrix
	SrMat adjustBaseMat;
	if (isRegistered())
	{
		int startFrameId = 0;
		int endFrameId = getNumFrames() - 1;

		SrMat matStart;
		_frameOrientation[startFrameId].get_mat(matStart);
		matStart.setl4(_frameOffset[startFrameId]);

		SrMat matEnd;
		_frameOrientation[endFrameId].get_mat(matEnd);
		matEnd.setl4(_frameOffset[endFrameId]);

		if (direction == 0)
		{
			SrMat mat0;
			_frameOrientation[0].get_mat(mat0);
			mat0.setl4(_frameOffset[0]);
			SrMat mat1;
			_frameOrientation[1].get_mat(mat1);
			mat1.setl4(_frameOffset[1]);
			SrMat mat01 = mat0.inverse() * mat1;

			adjustBaseMat = mat01 * (matStart.inverse() * matEnd);
		}
		else if (direction == 1)
		{
			SrMat mat1;
			_frameOrientation[getNumFrames() - 1].get_mat(mat1);
			mat1.setl4(_frameOffset[getNumFrames() - 1]);
			SrMat mat0;
			_frameOrientation[getNumFrames() - 2].get_mat(mat0);
			mat0.setl4(_frameOffset[getNumFrames() - 2]);
			SrMat mat10 = mat1.inverse() * mat0;

			adjustBaseMat = mat10 * (matEnd.inverse() * matStart);
		}
	}

	// calculate new matrix vector
	std::vector<SrMat> copyMatVec;
	copyMatVec.resize(getNumFrames());
	for (int i = 0; i < getNumFrames(); i++)
	{
		int origMotionId = 0;
		bool cycle = false;
		if (direction == 0)
		{
			if (i < (getNumFrames() - numFrames))
				origMotionId = i + numFrames;
			else
			{
				origMotionId = i - (getNumFrames() - numFrames);
				cycle = true;
			}
		}
		else if (direction == 1)
		{
			if (i < numFrames)
			{
				origMotionId = i + (getNumFrames() - numFrames);
				cycle = true;
			}
			else
				origMotionId = i - numFrames;
		}

		_frameOrientation[origMotionId].get_mat(copyMatVec[i]);
		copyMatVec[i].setl4(_frameOffset[origMotionId]);
		if (cycle)
		{
			SrMat temp = copyMatVec[i] * adjustBaseMat;
			copyMatVec[i] = temp;
		}
	}

	// copy it back
	for (int i = 0; i < getNumFrames(); i++)
	{
		/*
		gwiz::matrix_t actualMatrix;
		for (int r = 0; r < 4; r++)
			for (int c = 0; c < 4; c++)
			{
				actualMatrix.set(r, c, copyMatVec[i].get(r, c));
			}
		gwiz::quat_t retQuat = actualMatrix.quat();

		_frameOrientation[i] = SrQuat((float)retQuat.w(), (float)retQuat.x(), (float)retQuat.y(), (float)retQuat.z());
		*/
		_frameOrientation[i] = SrQuat(copyMatVec[i]);
		_frameOffset[i] = copyMatVec[i].get_translation();
	}

	// create a new motion
	SkChannelArray& mchan_arr = this->channels();
	SkMotion* copyMotion = new SmartBody::SBMotion();
	srSynchPoints sp(synch_points);
	copyMotion->synch_points = sp;
	copyMotion->init(mchan_arr);

	// insert frames into new motion
	for (int i = 0; i < getNumFrames(); i++)
	{
		int origMotionId = 0;
		if (direction == 0)
		{
			if (i < (getNumFrames() - numFrames))
				origMotionId = i + numFrames;
			else
				origMotionId = i - (getNumFrames() - numFrames);
		}
		else if (direction == 1)
		{
			if (i < numFrames)
				origMotionId = i + (getNumFrames() - numFrames);
			else
				origMotionId = i - numFrames;
		}
		copyMotion->insert_frame(i, this->keytime(i));
		float *ref_p = this->posture(origMotionId);
		float *new_p = copyMotion->posture(i);
		for (int k = 0; k < mchan_arr.size(); k++)
		{
			SkChannel& chan = mchan_arr[k];
			int index = mchan_arr.float_position(k);			
			for (int n = 0; n < chan.size(); n++)
				new_p[index + n] = ref_p[index + n];
		}
	}

	// assign back to the old motion
	for (int i = 0; i < getNumFrames(); i++)
	{
		float* orig_p = this->posture(i);
		float* copy_p = copyMotion->posture(i);
		for (int k = 0; k < mchan_arr.size(); k++)
		{
			SkChannel& chan = mchan_arr[k];
			int index = mchan_arr.float_position(k);			
			for (int n = 0; n < chan.size(); n++)
				orig_p[index + n] = copy_p[index + n];
		}
	}

	// unregister animation
	unregisterAnimation();

	delete copyMotion;
	copyMotion = NULL;

	if (direction == 0)
		alignIndex -= numFrames;	
	else if (direction == 1)
		alignIndex += numFrames;
}


SBMotion* SBMotion::duplicateCycle(int num, std::string newName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	registerAnimation();

	num++;

	// create a new motion
	SkChannelArray& mchan_arr = this->channels();
	SBMotion* copyMotion = new SmartBody::SBMotion();
	std::string copyMotionName = newName;
	if (newName == "")
	{
		std::stringstream ss;
		ss << this->getName() << "_duplicate" << (num - 1);
		copyMotionName = ss.str();
	}

	copyMotion->setName(copyMotionName);
	mcu.motion_map.insert(std::pair<std::string, SkMotion*>(copyMotionName, copyMotion));
	srSynchPoints sp(synch_points);
	copyMotion->synch_points = sp;
	copyMotion->init(mchan_arr);

	std::vector<SrVec>& frameOffset = getFrameOffset();
	std::vector<SrQuat>& frameOrientation = getFrameOrientation();
	SrMat adjustBaseMat;
	if (isRegistered())
	{
		int startFrameId = 0;
		int endFrameId = getNumFrames() - 1;

		SrMat matStart;
		frameOrientation[startFrameId].get_mat(matStart);
		matStart.setl4(frameOffset[startFrameId]);

		SrMat matEnd;
		frameOrientation[endFrameId].get_mat(matEnd);
		matEnd.setl4(frameOffset[endFrameId]);

		SrMat mat0;
		frameOrientation[0].get_mat(mat0);
		mat0.setl4(frameOffset[0]);
		SrMat mat1;
		frameOrientation[1].get_mat(mat1);
		mat1.setl4(frameOffset[1]);
		SrMat mat01 = mat0.inverse() * mat1;

		adjustBaseMat = mat01 * (matStart.inverse() * matEnd);
	}

	std::vector<SrQuat> copyFrameOrientation;
	std::vector<SrVec> copyFrameOffset;
	copyFrameOrientation.resize(getNumFrames() * num);
	copyFrameOffset.resize(getNumFrames() * num);
	for (int cycleId = 0; cycleId < num; cycleId++)
	{
		// handle base joints
		SrMat adjustMats;
		SrVec adjustVec;
		SrQuat adjustQuat;
		for (int i = 0; i < cycleId; i++)
		{
			SrMat temp = adjustMats * adjustBaseMat;
			adjustMats = temp;
		}

		for (int i = 0; i < getNumFrames(); i++)
		{
			// copy frame data without base joints
			copyMotion->insert_frame(i + cycleId * getNumFrames(), this->keytime(i) + float(cycleId * getDuration()));
			float* orig_p = posture(i);
			float* copy_p = copyMotion->posture(i + cycleId * getNumFrames());
			for (int k = 0; k < mchan_arr.size(); k++)
			{
				SkChannel& chan = mchan_arr[k];
				int index = mchan_arr.float_position(k);			
				for (int n = 0; n < chan.size(); n++)
					copy_p[index + n] = orig_p[index + n];
			}
			SrMat curMat;
			frameOrientation[i].get_mat(curMat);
			curMat.setl4(frameOffset[i]);
			SrMat newMat = curMat * adjustMats;
			copyFrameOffset[i + cycleId * getNumFrames()] = newMat.get_translation();
			copyFrameOrientation[i + cycleId * getNumFrames()] = SrQuat(newMat);
		}	
	}
	unregisterAnimation();
	copyMotion->setFrameOffset(copyFrameOffset);
	copyMotion->setFrameOrientation(copyFrameOrientation);
	copyMotion->unregisterAnimation();

	return copyMotion;
}

SBMotion* SBMotion::retarget( std::string name, std::string srcSkeletonName, std::string dstSkeletonName, std::vector<std::string>& endJoints, std::map<std::string, SrVec>& offsetJointMap)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	SBSkeleton* srcSkeleton = mcu._scene->getSkeleton(srcSkeletonName);
	SBSkeleton* dstSkeleton = mcu._scene->getSkeleton(dstSkeletonName);
	if (!srcSkeleton || !dstSkeleton)
	{
		LOG("Skeleton %s or %s not found. Retargeted motion %s not built.",srcSkeletonName.c_str(),dstSkeletonName.c_str(),name.c_str());
		return NULL;
	}
	
	SkMotion* motion = buildRetargetMotion(srcSkeleton,dstSkeleton, endJoints, offsetJointMap);
	SBMotion* sbmotion = dynamic_cast<SBMotion*>(motion);
	if (sbmotion)
	{
		std::string motionName = "";
		if (name == "")
		{
			motionName = sbmotion->getName();
			if (motionName == EMPTY_STRING)
				motionName = getName() + "_retarget";
		}
		else
			motionName = name;
		sbmotion->setName(motionName.c_str());


		//mcu.motion_map.insert(std::pair<std::string, SkMotion*>(motionName, motion));
		mcu.motion_map[motionName] = motion;
	}
	return sbmotion;
}

SBMotion* SBMotion::buildConstraintMotion( SBSkeleton* sourceSk, SBSkeleton* targetSk, SBMotion* targetMotion, std::vector<std::string>& endJoints, std::vector<std::string>& endJointRoots )
{
	SkChannelArray& mchan_arr = this->channels();
	SmartBody::SBSkeleton* interSk = new SmartBody::SBSkeleton(dynamic_cast<SBSkeleton*>(targetSk)); // copy for an intermediate skeleton
	SmartBody::SBSkeleton* tempSrcSk = new SmartBody::SBSkeleton(dynamic_cast<SBSkeleton*>(sourceSk));

	tempSrcSk->invalidate_global_matrices();
	tempSrcSk->update_global_matrices();
	interSk->invalidate_global_matrices();
	interSk->update_global_matrices();	

	SmartBody::SBJoint* rootJoint = dynamic_cast<SBJoint*>(interSk->root());
	SmartBody::SBMotion* constraintMotion = dynamic_cast<SBMotion*>(targetMotion->copyMotion()); // copy the motion first	
	MeCtIKTreeScenario ikScenario;
	ikScenario.buildIKTreeFromJointRoot(rootJoint);	
	MeCtJacobianIK ikJacobian;
	float sceneScale = (float)1.f/SBScene::getScene()->getScale();
	ikJacobian.maxOffset = 0.05f*sceneScale;
	ikJacobian.dampJ = 1.5f*sceneScale;

	float heightRatio = (interSk->getBaseHeight()/tempSrcSk->getBaseHeight());//*0.99f;

	std::map<std::string, float> jointScaleMap;
	ConstraintMap consMap;
	ConstraintMap noRotConstraint;
	for (unsigned int i=0;i<endJoints.size();i++)
	{
		std::string jname = endJoints[i];
		std::string jrootName = "";
		if (endJointRoots.size() > i)
			jrootName = endJointRoots[i];

		SBJoint* srcJoint = tempSrcSk->getJointByName(jname);
		SBJoint* tgtJoint = interSk->getJointByName(jname);
		if (srcJoint && tgtJoint) // a valid effector
		{
			EffectorConstantConstraint* constraint = new EffectorConstantConstraint();
			constraint->efffectorName = jname;
			//SBJoint* pjoint = dynamic_cast<SBJoint*>(tgtJoint->getParent()->getParent()->getParent());			
			if (interSk->getJointByName(jrootName))
			{
				constraint->rootName = jrootName;
			}
			else
			{
				SBJoint* pjoint = tgtJoint->getParent();
				while (pjoint->getParent() != rootJoint)
					pjoint = pjoint->getParent();
				constraint->rootName = pjoint->name();
			}			
			consMap[jname] = constraint;
		}
	}

	SrVec prevPos;
	SrVec prevOffsetPos;
	for (int iframe=0; iframe<frames(); iframe++)
	{
		int chanID = mchan_arr.search(rootJoint->name(),SkChannel::XPos);
		float* cur_p = constraintMotion->posture(iframe);	
		float* orig_p = this->posture(iframe);
		int index = mchan_arr.float_position(chanID);
		SrVec curPos;
		for (int k=0;k<3;k++)
		{
			cur_p[index+k] = orig_p[index+k]*heightRatio;
			curPos[k] = orig_p[index+k];
		}
		SrVec baseOffset = curPos - prevPos;
		prevPos = curPos;
		SrVec curScalePos = curPos*heightRatio;
		if (iframe > 0)
		{
			curScalePos = prevOffsetPos + baseOffset*heightRatio;
			for (int k=0;k<3;k++)
			{
				cur_p[index+k] = curScalePos[k];
			}
		}
		prevOffsetPos = curScalePos;
	}

	constraintMotion->connect(interSk);
	this->connect(tempSrcSk);
	bool prevConstraint = false;

	constraintMotion->apply_frame(0);
	this->apply_frame(0);
	constraintMotion->connected_skeleton()->update_global_matrices();
	this->connected_skeleton()->update_global_matrices();
	std::map<std::string, SrVec> prevPosMap;
	ConstraintMap::iterator mi;
	SBJoint* srcRoot = tempSrcSk->getJointByName(rootJoint->name());
	SBJoint* tgtRoot = rootJoint;
	for ( mi  = consMap.begin();
		  mi != consMap.end();
		  mi++)
	{
		EffectorConstantConstraint* con = dynamic_cast<EffectorConstantConstraint*>(mi->second);
		SBJoint* srcJoint = tempSrcSk->getJointByName(con->efffectorName);
		SBJoint* tgtJoint = interSk->getJointByName(con->efffectorName);
		if (tgtJoint)
		{
			SrVec offset = srcJoint->getMatrixGlobal().get_translation() - srcRoot->getMatrixGlobal().get_translation();
			con->targetPos = tgtRoot->getMatrixGlobal().get_translation() + offset*heightRatio;//tgtJoint->getMatrixGlobal().get_translation(); // initialize the frame 
		}					
		if (srcJoint)
		{
			prevPosMap[con->efffectorName] = srcJoint->getMatrixGlobal().get_translation();
		}
	}



	for (int iframe=0; iframe<frames(); iframe++)
	{			
		constraintMotion->apply_frame(iframe);
		this->apply_frame(iframe);
		constraintMotion->connected_skeleton()->update_global_matrices();
		this->connected_skeleton()->update_global_matrices();
		float* cur_p = constraintMotion->posture(iframe);		
		if (!prevConstraint)
		{
			ikScenario.setTreeNodeQuat(interSk,QUAT_INIT);
			prevConstraint = true;
		}		
		ikScenario.copyTreeNodeQuat(QUAT_INIT,QUAT_CUR);
		ikScenario.setTreeNodeQuat(interSk,QUAT_REF);
		// update root transformation
		if (rootJoint->pos())
		{
			SkJointPos* jpos = rootJoint->pos();
			ikScenario.ikTreeRootPos = SrVec(jpos->value(0),jpos->value(1),jpos->value(2));
		}		
		if (rootJoint->getParent())
			ikScenario.ikGlobalMat = rootJoint->getParent()->gmat();

		// update joint constraint
		ConstraintMap::iterator mi;
		for ( mi  = consMap.begin();
			  mi != consMap.end();
			  mi++)
		{
			EffectorConstantConstraint* con = dynamic_cast<EffectorConstantConstraint*>(mi->second);
			SBJoint* srcJoint = tempSrcSk->getJointByName(con->efffectorName);
			if (srcJoint)
			{
				//con->targetPos = srcJoint->getMatrixGlobal().get_translation();
				SrVec offset = srcJoint->getMatrixGlobal().get_translation() - prevPosMap[con->efffectorName];
				con->targetPos += offset*heightRatio;
				prevPosMap[con->efffectorName] = srcJoint->getMatrixGlobal().get_translation();
			}			
		}

		// update IK for each constraint at this frame	
		ikScenario.ikPosEffectors = &consMap;
		ikScenario.ikRotEffectors = &noRotConstraint;
		for (int k=0;k<20;k++)
		{
			ikJacobian.update(&ikScenario);
			ikScenario.copyTreeNodeQuat(QUAT_CUR,QUAT_INIT);	
		}
		for (unsigned int k=0;k<ikScenario.ikTreeNodes.size();k++)
		{
			MeCtIKTreeNode* ikNode = ikScenario.ikTreeNodes[k];
			SBJoint* ikJoint = interSk->getJointByName(ikNode->nodeName);
			int chanID = mchan_arr.search(ikNode->nodeName,SkChannel::Quat);
			SrQuat& nq = ikNode->getQuat(QUAT_CUR);
			SrQuat kq; 
			if (ikJoint) kq = ikJoint->quat()->rawValue();			
			if (chanID != -1)
			{
				int floatIdx = mchan_arr.float_position(chanID);
				//LOG("old quat = %f %f %f %f",cur_p[ floatIdx + 0 ],cur_p[ floatIdx + 1 ],cur_p[ floatIdx + 2 ],cur_p[ floatIdx + 3 ] );
				//LOG("new quat = %f %f %f %f",nq.w,nq.x,nq.y,nq.z );
				//LOG("skel quat = %f %f %f %f",kq.w,kq.x,kq.y,kq.z );
				cur_p[ floatIdx + 0 ] = nq.w;
				cur_p[ floatIdx + 1 ] = nq.x;
				cur_p[ floatIdx + 2 ] = nq.y;
				cur_p[ floatIdx + 3 ] = nq.z;
			}
		}				
	}
	constraintMotion->disconnect();
	this->disconnect();
	return constraintMotion;
}


SBMotion* SBMotion::autoFootSkateCleanUp( std::string name, std::string srcSkeletonName, std::string rootName, std::vector<FootStepRecord>& footStepRecords )
{
	SBSkeleton* origSkel = SBScene::getScene()->getSkeleton(srcSkeletonName);
	if (!origSkel) return NULL;
	SBSkeleton* skel = new SBSkeleton(origSkel);
	if (!skel) return NULL;
	SBJoint* rootJoint = skel->getJointByName(rootName);
	if (!rootJoint) return NULL;

	SBMotion* cleanMotion = dynamic_cast<SBMotion*>(this->copyMotion()); // copy the motion first	
	MeCtIKTreeScenario ikScenario;
	ikScenario.buildIKTreeFromJointRoot(rootJoint);
	MeCtCCDIK ikCCD;
	MeCtJacobianIK ikJacobian;
	float sceneScale = (float)1.f/SBScene::getScene()->getScale();
	ikJacobian.maxOffset = 0.1f*sceneScale;
	ikJacobian.dampJ = 1.5f*sceneScale;
	std::map<int, std::vector<int> > frameConstraintMap;

	//ConstraintMap cons;
	std::vector<ConstraintMap> constrainMapList;
	constrainMapList.resize(footStepRecords.size());

	SkChannelArray& mchan_arr = cleanMotion->channels();
	ConstraintMap noRotConstraint;
	for (unsigned int i=0;i<footStepRecords.size();i++)
	{
		FootStepRecord& rec = footStepRecords[i];
		if (rec.jointNames.size() == 1)
		{
			SBJoint* joint = skel->getJointByName(rec.jointNames[0]);
			if (joint && joint->child(0))
				rec.jointNames.push_back(joint->child(0)->name());
// 			if (joint && joint->child(0) && joint->child(0)->child(0))
// 				rec.jointNames.push_back(joint->child(0)->child(0)->name());
		}

	}

	cleanMotion->connect(skel);
	for (unsigned int i=0;i<footStepRecords.size();i++)
	{
		FootStepRecord& rec = footStepRecords[i];
		rec.updateJointAveargePosition(skel, this);
		ConstraintMap& cons = constrainMapList[i];		
		// setup constraint
		for (unsigned int k=0;k<rec.jointNames.size();k++)
		{
			SBJoint* conJoint = skel->getJointByName(rec.jointNames[k]);
			if (conJoint)
			{
				EffectorConstantConstraint* constraint = new EffectorConstantConstraint();
				constraint->efffectorName = rec.jointNames[k];
				constraint->targetPos = rec.posVec[k];
				SBJoint* pjoint = dynamic_cast<SBJoint*>(conJoint->getParent()->getParent()->getParent());
				constraint->rootName = pjoint->name();
				//LOG("effector = %s, root = %s, target pos = %f %f %f",rec.jointNames[k].c_str(),constraint->rootName.c_str(), rec.posVec[k][0],rec.posVec[k][1],rec.posVec[k][2]);
				cons[rec.jointNames[k]] = constraint;
			}				
		}
		int curFrame = -1;
		for (float t=rec.startTime; t<=rec.endTime; t+= (float)getFrameRate())
		{
			int nextFrame = (int)(t/(float)getFrameRate());
			if (nextFrame == curFrame)
				continue;
			curFrame = nextFrame;
			if (frameConstraintMap.find(curFrame) == frameConstraintMap.end())
				frameConstraintMap[curFrame] = std::vector<int>();
			frameConstraintMap[curFrame].push_back(i);
		}
	}

	bool prevConstraint = false;
	for (int iframe=0; iframe<frames(); iframe++)
	{
 		if (frameConstraintMap.find(iframe) == frameConstraintMap.end())
		{
			//prevConstraint = false;
 			continue;
		}

		std::vector<int>& recIdxList = frameConstraintMap[iframe];
		cleanMotion->apply_frame(iframe);
		float* cur_p = cleanMotion->posture(iframe);		
		if (!prevConstraint)
		{
			ikScenario.setTreeNodeQuat(skel,QUAT_INIT);
			//prevConstraint = true;
		}		
		ikScenario.copyTreeNodeQuat(QUAT_INIT,QUAT_CUR);
		ikScenario.setTreeNodeQuat(skel,QUAT_REF);
		if (rootJoint->pos())
		{
			SkJointPos* jpos = rootJoint->pos();
			ikScenario.ikTreeRootPos = SrVec(jpos->value(0),jpos->value(1),jpos->value(2));
		}		
		if (rootJoint->getParent())
			ikScenario.ikGlobalMat = rootJoint->getParent()->gmat();

		// update IK for each constraint at this frame
		
// 		for (unsigned int k=0; k<recIdxList.size(); k++)
// 		{
// 			FootStepRecord& rec = footStepRecords[recIdxList[k]];
// 			ConstraintMap& cons = constrainMapList[recIdxList[k]];
// 			ikScenario.ikPosEffectors = &cons;
// 			ikScenario.ikRotEffectors = &noRotConstraint;
// 			ikCCD.update(&ikScenario);
// 		}
		
// 		ConstraintMap combineCons;
// 		for (unsigned int k=0; k<recIdxList.size(); k++)
// 		{
// 			ConstraintMap& cons = constrainMapList[recIdxList[k]];
// 			combineCons.insert(cons.begin(),cons.end());
// 		}
// 		ikScenario.ikPosEffectors = &combineCons;
// 		ikScenario.ikRotEffectors = &noRotConstraint;
// 		for (int k=0;k<10;k++)
// 		{
// 			ikJacobian.update(&ikScenario);
// 			ikScenario.copyTreeNodeQuat(QUAT_CUR,QUAT_INIT);	
// 		}
		

			
		for (unsigned int k=0;k<ikScenario.ikTreeNodes.size();k++)
		{
			MeCtIKTreeNode* ikNode = ikScenario.ikTreeNodes[k];
			SBJoint* ikJoint = skel->getJointByName(ikNode->nodeName);
			int chanID = mchan_arr.search(ikNode->nodeName,SkChannel::Quat);
			SrQuat& nq = ikNode->getQuat(QUAT_CUR);
			SrQuat kq; 
			if (ikJoint) kq = ikJoint->quat()->rawValue();
			//SrQuat& kq = 
			if (chanID != -1)
			{
				int floatIdx = mchan_arr.float_position(chanID);
				//LOG("old quat = %f %f %f %f",cur_p[ floatIdx + 0 ],cur_p[ floatIdx + 1 ],cur_p[ floatIdx + 2 ],cur_p[ floatIdx + 3 ] );
				//LOG("new quat = %f %f %f %f",nq.w,nq.x,nq.y,nq.z );
				//LOG("skel quat = %f %f %f %f",kq.w,kq.x,kq.y,kq.z );
				cur_p[ floatIdx + 0 ] = nq.w;
				cur_p[ floatIdx + 1 ] = nq.x;
				cur_p[ floatIdx + 2 ] = nq.y;
				cur_p[ floatIdx + 3 ] = nq.z;
			}
		}				
	}
	cleanMotion->disconnect();

	delete skel; // delete the copy
	return cleanMotion;	
}

SBMotion* SBMotion::mirror(std::string name, std::string skeletonName)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	SBSkeleton* skeleton = mcu._scene->getSkeleton(skeletonName);
	if (!skeleton)
	{
		LOG("Skeleton %s not found. Mirror motion %s not built.",skeletonName.c_str(),name.c_str());
		return NULL;
	}
	SkMotion* motion = buildMirrorMotion(skeleton);
	SBMotion* sbmotion = dynamic_cast<SBMotion*>(motion);
	if (sbmotion)
	{
		std::string motionName = "";
		if (name == "")
		{
			motionName = sbmotion->getName();
			if (motionName == EMPTY_STRING)
				motionName = getName() + "_mirror";
		}
		else
			motionName = name;
		sbmotion->setName(motionName.c_str());

		
		mcu.motion_map.insert(std::pair<std::string, SkMotion*>(motionName, motion));
	}
	return sbmotion;
}

SBMotion* SBMotion::smoothCycle( std::string name, float timeInterval )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 	
	SkMotion* motion = buildSmoothMotionCycle(timeInterval);
	SBMotion* sbmotion = dynamic_cast<SBMotion*>(motion);
	if (sbmotion)
	{
		std::string motionName = "";
		if (name == "")
		{
			motionName = sbmotion->getName();
			if (motionName == EMPTY_STRING)
				motionName = getName() + "_smoothCycle";
		}
		else
			motionName = name;
		sbmotion->setName(motionName.c_str());
		mcu.motion_map.insert(std::pair<std::string, SkMotion*>(motionName, motion));
	}
	return sbmotion;
}

float SBMotion::getJointSpeed(SBJoint* joint, float startTime, float endTime)
{
	if (!joint)
		return 0.f;
	if (connected_skeleton() == NULL)
	{
		LOG("Motion %s is not connected to any skeleton, cannot retrieve parameter speed.", getName().c_str());
		return 0;
	}

	float dt = duration() / float(frames() - 1);
	int minFrameId = int(startTime / dt);
	int maxFrameId = int(endTime / dt);
	float distance = 0;
	for (int i = minFrameId; i < maxFrameId + 1; i++)
	{
		apply_frame(i);
		connected_skeleton()->update_global_matrices();
		const SrMat& srcMat = joint->gmat();
		SrVec srcPt = SrVec(srcMat.get(12), srcMat.get(13), srcMat.get(14));
		apply_frame(i + 1);
		connected_skeleton()->update_global_matrices();
		const SrMat& destMat = joint->gmat();
		SrVec destPt = SrVec(destMat.get(12), destMat.get(13), destMat.get(14));
		distance += dist(srcPt, destPt);
	}
	float accSpd = distance / (endTime - startTime);
	connected_skeleton()->clearJointValues(); // reset the joint quat/pos
	return accSpd;
}

float SBMotion::getJointSpeedAxis(SBJoint* joint, const std::string& axis, float startTime, float endTime)
{
	int axisIndex = 0;
	if (axis == "X" || axis == "x")
		axisIndex = 0;
	else if (axis == "Y" || axis == "y")
		axisIndex = 1;
	else if (axis == "Z" || axis == "z")
		axisIndex = 2;
	else
	{
		LOG("Bad axis specified '%s', defaulting to use the X-axis.", axis.c_str());
	}

	if (!joint)
		return 0.f;
	if (connected_skeleton() == NULL)
	{
		LOG("Motion %s is not connected to any skeleton, cannot retrieve parameter speed.", getName().c_str());
		return 0;
	}

	float dt = duration() / float(frames() - 1);
	int minFrameId = int(startTime / dt);
	int maxFrameId = int(endTime / dt);
	float distance = 0;
	for (int i = minFrameId; i < maxFrameId + 1; i++)
	{
		apply_frame(i);
		connected_skeleton()->update_global_matrices();
		const SrMat& srcMat = joint->gmat();
		SrVec srcPt = SrVec(srcMat.get(12), srcMat.get(13), srcMat.get(14));
		apply_frame(i + 1);
		connected_skeleton()->update_global_matrices();
		const SrMat& destMat = joint->gmat();
		SrVec destPt = SrVec(destMat.get(12), destMat.get(13), destMat.get(14));
		distance += destPt[axisIndex] - srcPt[axisIndex];
	}
	float accSpd = distance / (endTime - startTime);
	connected_skeleton()->clearJointValues(); // reset the joint quat/pos
	return accSpd;
}

float SBMotion::getJointAngularSpeed(SBJoint* joint, float startTime, float endTime)
{
	if (!joint)
	{
		return 0.f;
	}
	if (connected_skeleton() == NULL)
	{
		LOG("Motion %s is not connected to any skeleton, cannot retrieve parameter angular speed.", getName().c_str());
		return 0;
	}
	float dt = duration() / float(frames() - 1);
	int minFrameId = int(startTime / dt);
	int maxFrameId = int(endTime / dt);
	float diffRotY = 0.0f;
	for (int i = minFrameId; i < maxFrameId + 1; i++)
	{
		apply_frame(i);
		connected_skeleton()->update_global_matrices();
		const SrMat& srcMat = joint->gmat();
		float rx, ry, rz;
		sr_euler_angles(rotType, srcMat, rx, ry, rz);
		float srcRotY = ry;
		apply_frame(i + 1);
		connected_skeleton()->update_global_matrices();
		const SrMat& destMat = joint->gmat();
		sr_euler_angles(rotType, destMat, rx, ry, rz);
		float destRotY = ry;
		float diff;
		if (destRotY * srcRotY < 0 && fabs(destRotY) > 1.0f)
			diff = - destRotY - srcRotY;
		else
			diff = destRotY - srcRotY;
		diffRotY += diff;
	}
	float accAngularSpd = diffRotY / (endTime - startTime);
	accAngularSpd *= (180.0f/ float(M_PI));
	connected_skeleton()->clearJointValues(); // reset the joint quat/pos
	return accAngularSpd;
}

float SBMotion::getJointAngularSpeedAxis(SBJoint* joint, const std::string& axis, float startTime, float endTime)
{	
	int axisIndex = 0;
	if (axis == "X" || axis == "x")
		axisIndex = 0;
	else if (axis == "Y" || axis == "y")
		axisIndex = 1;
	else if (axis == "Z" || axis == "z")
		axisIndex = 2;
	else
	{
		LOG("Bad axis specified '%s', defaulting to use the X-axis.", axis.c_str());
	}

	if (!joint)
	{
		return 0.f;
	}
	if (connected_skeleton() == NULL)
	{
		LOG("Motion %s is not connected to any skeleton, cannot retrieve parameter angular speed.", getName().c_str());
		return 0;
	}
	float dt = duration() / float(frames() - 1);
	int minFrameId = int(startTime / dt);
	int maxFrameId = int(endTime / dt);
	float diffRot = 0.0f;
	for (int i = minFrameId; i < maxFrameId + 1; i++)
	{
		apply_frame(i);
		connected_skeleton()->update_global_matrices();
		const SrMat& srcMat = joint->gmat();
		float rx, ry, rz;
		sr_euler_angles(rotType, srcMat, rx, ry, rz);
		float srcRot = rx;
		if (axisIndex == 0)
			srcRot = rx;
		else if (axisIndex == 1)
			srcRot = ry;
		else if (axisIndex == 2)
			srcRot = rz;
		
		apply_frame(i + 1);
		connected_skeleton()->update_global_matrices();
		const SrMat& destMat = joint->gmat();
		sr_euler_angles(rotType, destMat, rx, ry, rz);

		float destRot = rx;
		if (axisIndex == 0)
			destRot = rx;
		else if (axisIndex == 1)
			destRot = ry;
		else if (axisIndex == 2)
			destRot = rz;
		float diff;
		if (destRot * srcRot < 0 && fabs(destRot) > 1.0f)
			diff = - destRot - srcRot;
		else
			diff = destRot - srcRot;
		diffRot += diff;
	}
	float accAngularSpd = diffRot / (endTime - startTime);
	accAngularSpd *= (180.0f/ float(M_PI));
	connected_skeleton()->clearJointValues(); // reset the joint quat/pos
	return accAngularSpd;
}

std::vector<float> SBMotion::getJointTransition(SBJoint* joint, float startTime, float endTime)
{
	if (!joint)
		return std::vector<float>();
	std::vector<float> transitions;
	if (connected_skeleton() == NULL)
	{
		LOG("Motion %s is not connected to any skeleton, cannot retrieve parameter angular speed.", getName().c_str());
		transitions.push_back(0);
		transitions.push_back(0);
		transitions.push_back(0);
		return transitions;
	}
	float dt = duration() / float(frames() - 1);
	int minFrameId = int(startTime / dt);
	int maxFrameId = int(endTime / dt);
	apply_frame(minFrameId);
	connected_skeleton()->update_global_matrices();
	const SrMat& srcMat = joint->gmat();
	const SrMat& srcMat0 = joint->gmatZero();
	SrVec srcPnt = SrVec(srcMat.get(12), srcMat.get(13), srcMat.get(14));
	float rx, ry, rz;
	float rx0, ry0, rz0;
	sr_euler_angles(rotType, srcMat, rx, ry, rz);
	sr_euler_angles(rotType, srcMat0, rx0, ry0, rz0);
	apply_frame(maxFrameId);
	connected_skeleton()->update_global_matrices();
	const SrMat& destMat = joint->gmat();
	SrVec destPnt = SrVec(destMat.get(12), destMat.get(13), destMat.get(14));
	SrVec transitionVec = destPnt - srcPnt;
	SrVec heading = SrVec(sin(ry - ry0 - 1.57f), 0, cos(ry - ry0 - 1.57f));
	float x = dot(transitionVec, heading);
	transitions.push_back(x);
	float y = destMat.get(14) - srcMat.get(14);
	transitions.push_back(y);
	heading = SrVec(sin(ry - ry0), 0, cos(ry - ry0));
	float z = dot(transitionVec, heading);
	transitions.push_back(z);
	connected_skeleton()->clearJointValues(); // reset the joint quat/pos
	return transitions;
}


bool SBMotion::translate(float x, float y, float z, const std::string& baseJointName)
{
	SrVec offset(x, y, z);
	// get the base x, y, z
	SkChannelArray& ch = channels();
	int pos[3];
	pos[0] = ch.search(baseJointName, SkChannel::XPos);
	pos[1] = ch.search(baseJointName, SkChannel::YPos);
	pos[2] = ch.search(baseJointName, SkChannel::ZPos);
	if (pos[0] == -1 || pos[1] == -1 || pos[2] == -1)
	{
		LOG("No joint named '%s' found in motion %s, cannot translate.", baseJointName.c_str(), getName().c_str());
		return false;
	}

	int numFrames = frames();
	for (int f = 0; f < numFrames; f++)
	{
		float* curFrame = posture(f);
		for (int p = 0; p < 3; p++)
		{
			curFrame[pos[p]] = curFrame[pos[p]] + offset[p];
		}

	}

	LOG("Motion %s with %d frames offset by (%f, %f, %f)", getName().c_str(), frames(), offset[0], offset[1], offset[2]);
	return true;
}

bool SBMotion::rotate(float xaxis, float yaxis, float zaxis, const std::string& baseJointName)
{
	SrVec rotation;
	rotation[0] = xaxis;
	rotation[1] = yaxis;
	rotation[2] = zaxis;

	// get the base quaternion
	SkChannelArray& ch = channels();
	int pos = -1;
	pos = ch.search(baseJointName.c_str(), SkChannel::Quat);
	if (pos == -1)
	{
		LOG("No joint named '%s' found in motion %s, cannot rotate.", baseJointName.c_str(), getName().c_str());
		return false;
	}

	SrMat xRot;
	xRot.rotx(rotation[0] * (float) M_PI / 180.0f);
	SrMat yRot;
	yRot.roty(rotation[1] * (float) M_PI / 180.0f);
	SrMat zRot;
	zRot.rotz(rotation[2] * (float) M_PI / 180.0f);
	SrMat finalMat = xRot * yRot * zRot;

	int numFrames = frames();
	for (int f = 0; f < numFrames; f++)
	{
		float* curFrame = posture(f);
		SrQuat curQuat(curFrame[pos], curFrame[pos + 1], curFrame[pos + 2], curFrame[pos + 3]); 
		SrMat currentMat;
		curQuat.get_mat(currentMat);
		currentMat *= finalMat;
		SrQuat newQuat(currentMat);
		curFrame[pos + 0] = newQuat.w;
		curFrame[pos + 1] = newQuat.x;
		curFrame[pos + 2] = newQuat.y;
		curFrame[pos + 3] = newQuat.z;
	}

	LOG("Motion %s with %d frames rotated by (%f, %f, %f)", getName().c_str(), frames(), rotation[0], rotation[1], rotation[2]);
	return true;
}

bool SBMotion::scale(float factor)
{
	SkChannelArray& ch = channels();
	int numFrames = frames();
	for (int f = 0; f < numFrames; f++)
	{
		float* p = posture(f);
		int index = 0;
		for (int c = 0; c < ch.size(); c++)
		{
			SkChannel& channel = ch[c];
			// only scale the positions
			if (channel.type == SkChannel::XPos ||
				channel.type == SkChannel::YPos ||
				channel.type == SkChannel::ZPos)
			{
				p[index] *= factor;
			}
			index += channel.size();
		}
	}
	LOG("Motion %s with %d frames scaled by a factor of %f", getName().c_str(), frames(), factor);
	return true;
}

bool SBMotion::retime(float factor)
{
	for (int f = 0; f < frames(); f++)
	{
		keytime(f, keytime(f) * factor);
	}

	LOG("Motion %s with %d frames retimed by a factor of %f", getName().c_str(), frames(), factor);
	return true;
}


bool SBMotion::trim(int numFramesFromFront, int numFramesFromBack)
{
	int newFrames = frames() - numFramesFromFront - numFramesFromBack;
	if (numFramesFromFront < 0 || numFramesFromBack < 0)
	{
		LOG("trim frames can not be negative");
		return false;
	}
	if (newFrames < 1)
	{
		LOG("Motion %s has only %d frames, can not be trimmed by %d frames at front and %d frames at back",getName().c_str(), frames(), numFramesFromFront, numFramesFromBack);
		return false;
	}
	for (int i=0;i<newFrames;i++)
	{
		int idx = i + numFramesFromFront;
		Frame& srcFrame = _frames[idx];
		Frame& tgtFrame = _frames[i];
		// copy over the posture at that frame, but reserved the key time
		memcpy(tgtFrame.posture,srcFrame.posture,sizeof(float)*posture_size());
	}
	// remove all frames from the back
	for (int i=frames()-1; i>= newFrames; i--)
	{
		Frame& delFrame = _frames[i];
		delete [] delFrame.posture;
		_frames.pop_back();
	}

	return true;
}

/*
bool SBMotion::move(int startFrame, int endFrame, int position)
{
	return true;
}
*/

double SBMotion::getTimeStart()
{
	return time_start();
}

double SBMotion::getTimeReady()
{
	return time_ready();
}

double SBMotion::getTimeStrokeStart()
{
	return time_stroke_start();
}

double SBMotion::getTimeStroke()
{
	return time_stroke_emphasis();
}

double SBMotion::getTimeStrokeEnd()
{
	return time_stroke_end();
}

double SBMotion::getTimeRelax()
{
	return time_relax();
}

double SBMotion::getTimeStop()
{
	return time_stop();
}

double SBMotion::getDuration()
{
	return duration();
}


double SBMotion::getFrameRate()
{
	return getDuration() / double(getNumFrames() - 1);
}

void SBMotion::addEvent(double time, const std::string& type, const std::string& parameters, bool onceOnly)
{
	MotionEvent* motionEvent = new MotionEvent();
	motionEvent->setIsOnceOnly(onceOnly);
	motionEvent->setTime(time);
	motionEvent->setType(type);
	motionEvent->setParameters(parameters);
	addMotionEvent(motionEvent);
}

bool SBMotion::kMeansClustering1D(int num, std::vector<double>& inputPoints, std::vector<double>& outMeans)
{
	if ((int)inputPoints.size() < num)
	{
		LOG("PAAutoFootStepsEditor::kMeansClustering1D Warning: Input points are less than number of means");
		return false;
	}

	// pick initial point
	int step = inputPoints.size() / num;
	for (int i = 0; i < num; i++)
		outMeans.push_back(inputPoints[i * step]);

	double convergence = 0.1;
	calculateMeans(inputPoints, outMeans, convergence);

	outMeans.resize(num);
	return true;
}


void SBMotion::calculateMeans(std::vector<double>&inputPoints, std::vector<double>& means, double convergentValue)
{
	std::vector<std::vector<double> > partitionBin;
	partitionBin.resize(means.size());

	// partition
	for (size_t i = 0; i < inputPoints.size(); i++)
	{
		double minDist = 9999;
		int minDistId = -1;
		for (size_t j = 0; j < means.size(); j++)
		{
			double dist = fabs(inputPoints[i] - means[j]);
			if (dist < minDist)
			{
				minDist = dist;
				minDistId = j;
			}
		}
		if (minDistId >= 0)
		{
			partitionBin[minDistId].push_back(inputPoints[i]);
		}
	}

	// get new means
	std::vector<double> newMeans;
	for (size_t i = 0; i < means.size(); i++)
	{
		double newMean = 0;
		for (size_t j = 0; j < partitionBin[i].size(); j++)
			newMean += partitionBin[i][j];
		newMean /= double(partitionBin[i].size());
		newMeans.push_back(newMean);
	}

	double diff = 0.0f;
	for (size_t i = 0; i < means.size(); i++)
	{
		diff = diff + (means[i] - newMeans[i]) * (means[i] - newMeans[i]);
	}
	diff = sqrt(diff);

	if (diff < convergentValue)
		return;
	else
	{
		means = newMeans;
		calculateMeans(inputPoints, means, convergentValue);
	}
}


SBMotion* SBMotion::constrain( std::string name, std::string srcSkeletonName, std::string tgtSkeletonName, std::string tgtMotionName, std::vector<std::string>& endJoints, std::vector<std::string>& endJointRoots )
{
	SBSkeleton* origSkel = SBScene::getScene()->getSkeleton(srcSkeletonName);
	if (!origSkel) return NULL;
	SBMotion* tgtMotion = SBScene::getScene()->getMotion(tgtMotionName);
	if (!tgtMotion) return NULL;
	SBSkeleton* tgtSkel  = SBScene::getScene()->getSkeleton(tgtSkeletonName);
	if (!tgtSkel) return NULL;

	SBMotion* constraintMotion = buildConstraintMotion(origSkel, tgtSkel, tgtMotion, endJoints, endJointRoots);

	if (constraintMotion)
	{
		std::string motionName = "";
		if (name == "")
		{
			motionName = constraintMotion->getName();
			if (motionName == EMPTY_STRING)
				motionName = getName() + "_cleanup";
		}
		else
			motionName = name;
		constraintMotion->setName(motionName.c_str());
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		//mcu.motion_map.insert(std::pair<std::string, SkMotion*>(motionName, cleanUpMotion));
		mcu.motion_map[motionName] = constraintMotion;
	}
	return constraintMotion;

}


SBMotion* SBMotion::footSkateCleanUp( std::string name, std::vector<std::string>& footJoints, std::string srcSkeletonName, 
									  std::string srcMotionName, std::string tgtSkeletonName, std::string tgtRootName, 
									  float floorHeight, float heightThreshold, float speedThreshold )
{
	LOG("foot skate cleanup for motion %s.", this->getName().c_str());

	SBSkeleton* origSkel = SBScene::getScene()->getSkeleton(srcSkeletonName);
	if (!origSkel) return NULL;
	SBMotion* origMotion = SBScene::getScene()->getMotion(srcMotionName);
	if (!origMotion) return NULL;

	LOG("foot skate cleanup for motion %s.", this->getName().c_str());

	SBSkeleton* skelCopy = new SBSkeleton(origSkel);
	std::vector<FootStepRecord> footStepRecords;
	bool hasFootPlant = origMotion->autoFootPlantDetection(origSkel, footJoints, floorHeight, heightThreshold, speedThreshold,footStepRecords);

	for (unsigned int i=0;i<footStepRecords.size();i++)
	{
		FootStepRecord& record = footStepRecords[i];
		LOG("Footstep joint = %s, start frame = %f, end frame = %f",record.jointNames[0].c_str(), record.startTime, record.endTime);
	}

	SBMotion* cleanUpMotion = autoFootSkateCleanUp(name,tgtSkeletonName, tgtRootName, footStepRecords);
	
	if (cleanUpMotion)
	{
		std::string motionName = "";
		if (name == "")
		{
			motionName = cleanUpMotion->getName();
			if (motionName == EMPTY_STRING)
				motionName = getName() + "_cleanup";
		}
		else
			motionName = name;
		cleanUpMotion->setName(motionName.c_str());
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		//mcu.motion_map.insert(std::pair<std::string, SkMotion*>(motionName, cleanUpMotion));
		mcu.motion_map[motionName] = cleanUpMotion;
	}
	return cleanUpMotion;
}

bool SBMotion::autoFootPlantDetection( SBSkeleton* srcSk, std::vector<std::string>& footJoints, float floorHeight, float heightThreshold, float speedThreshold, std::vector<FootStepRecord>& footStepRecords )
{
	int speedWindow = 3;
	footStepRecords.clear();	
	std::vector<std::vector<float> > vecTiming;
	vecTiming.resize(footJoints.size());	
	std::vector<std::vector<SrVec> > vecPos;
	vecPos.resize(footJoints.size());
	this->connect(srcSk);
	for(int f = 0; f < frames(); f++)
	{
		this->apply_frame(f);
		this->connected_skeleton()->update_global_matrices();
		for (size_t jointId = 0; jointId < footJoints.size(); jointId ++)
		{
			std::string jointName = footJoints[jointId];
			SBJoint* joint = srcSk->getJointByName(jointName);
			if (!joint)
				continue;			
			// get height
			const SrMat& gMat = joint->gmat();
			SrVec gPos = SrVec(gMat.get(12), gMat.get(13), gMat.get(14));			
			// get speed
			int startFrame = f - speedWindow / 2;
			int endFrame = f + speedWindow / 2;
			float startTime = startFrame * (float)getFrameRate();
			float endTime = endFrame * (float)getFrameRate();
			float speed = getJointSpeed(joint, startTime, endTime);		

			std::vector<SBJoint*> descendants = joint->getDescendants();
			for (size_t k = 0; k < descendants.size(); k++)
			{
				SBJoint* des = descendants[k];
				gPos += des->gmat().get_translation();
				speed += getJointSpeed(des, startTime, endTime);
			}
			gPos /= (float)(descendants.size()+1);
			speed /= (float)(descendants.size()+1);
			// filter for height
			if (gPos.y < floorHeight || gPos.y > (floorHeight + heightThreshold))
				continue;
			// filter speed
			if (speed <= speedThreshold)
			{
				vecTiming[jointId].push_back((float)f);	
				vecPos[jointId].push_back(gPos); // also push back the joint's current position
			}
		}
	}	

	// merging the constraints
	float sceneScale = 1.f/(float)SBScene::getScene()->getScale();
	float Fmax = 10.f;
	float dmax = 0.1f * sceneScale;
	for (unsigned int i=0;i<footJoints.size();i++)
	{
		std::string jointName = footJoints[i];
		std::vector<float>& plantFrames = vecTiming[i];
		std::vector<SrVec>& plantPos = vecPos[i];
		if (plantFrames.size() == 0)
			continue;
		std::vector<float> mergeFrames;
		float curFrame = plantFrames[0];
		SrVec curPos = plantPos[0];
		mergeFrames.push_back(curFrame); // init the constraint
		for (unsigned int k=1;k<plantFrames.size();k++)
		{
			float nextFrame = plantFrames[k];
			SrVec nextPos = plantPos[k];
			float curDist = (nextPos - curPos).norm();
			float ftol = Fmax*exp(-curDist*log(Fmax)/dmax);
			if (ftol > fabs(nextFrame-curFrame)) // merge constraint
			{
				mergeFrames.push_back(nextFrame);
				curFrame = nextFrame;
				curPos = nextPos;
			}
			else // finalize the current constraint
			{
				FootStepRecord footPlant;
				footPlant.jointNames.push_back(jointName);
				SBJoint* joint = srcSk->getJointByName(jointName);				
				footPlant.startTime = mergeFrames[0]*(float)getFrameRate();
				footPlant.endTime   = mergeFrames[mergeFrames.size()-1]*(float)getFrameRate();	
				footStepRecords.push_back(footPlant);

				curFrame = nextFrame;
				curPos = nextPos;
				mergeFrames.clear();
				mergeFrames.push_back(curFrame);
			}
		}

		// push again if there are values in mergeFrames
		if (mergeFrames.size() > 0)
		{
			FootStepRecord footPlant;
			footPlant.jointNames.push_back(jointName);
			footPlant.startTime = mergeFrames[0]*(float)getFrameRate();
			footPlant.endTime   = mergeFrames[mergeFrames.size()-1]*(float)getFrameRate();	
			footStepRecords.push_back(footPlant);
		}		
	}
	this->disconnect();
	return true;
}


bool SBMotion::autoFootStepDetection( std::vector<double>& outMeans, int numStepsPerJoint, int maxNumSteps, SBSkeleton* skeleton, 
									  std::vector<std::string>& selectedJoints, float floorHeight, float floorThreshold, 
									  float speedThreshold, int speedWindow, bool isPrintDebugInfo  )
{
	std::vector<double> possibleTiming;

	// divided
	std::vector<std::vector<double> > vecOutMeans;
	vecOutMeans.resize(selectedJoints.size());
	std::vector<std::vector<double> > vecTiming;
	vecTiming.resize(selectedJoints.size());	
	this->connect(skeleton);
	for(int f = 0; f < frames(); f++)
	{
		this->apply_frame(f);
		this->connected_skeleton()->update_global_matrices();

		for (size_t jointId = 0; jointId < selectedJoints.size(); jointId ++)
		{
			std::string jointName = selectedJoints[jointId];
			SBJoint* joint = skeleton->getJointByName(jointName);
			if (!joint)
				continue;

			// get height
			const SrMat& gMat = joint->gmat();
			SrVec gPos = SrVec(gMat.get(12), gMat.get(13), gMat.get(14));

			// get speed
			int startFrame = f - speedWindow / 2;
			int endFrame = f + speedWindow / 2;
			float startTime = startFrame * (float)getFrameRate();
			float endTime = endFrame * (float)getFrameRate();
			float speed = getJointSpeed(joint, startTime, endTime);

			// print info
			if (isPrintDebugInfo)
				LOG("motion %s at time %f-> speed is %f, height is %f, joint is %s", getName().c_str(), f * getFrameRate(), speed, gPos.y, jointName.c_str());

			// filter for height
			if (gPos.y < floorHeight || gPos.y > (floorHeight + floorThreshold))
				continue;

			// filter speed
			if (speed <= speedThreshold)
			{
				vecTiming[jointId].push_back(f * (float)getFrameRate());
				possibleTiming.push_back(f * (float)getFrameRate());
			}
		}
	}	
	/*
	int numSteps = footStepEditor->stateEditor->getCurrentState()->getNumKeys();
	isConvergent = footStepEditor->kMeansClustering1D(numSteps, possibleTiming, outMeans);
	*/
	int maxSteps = maxNumSteps;
	int stepsPerJoint = numStepsPerJoint;
	bool isConvergent = true;
	for (size_t jointId = 0; jointId < selectedJoints.size(); jointId++)
	{
		if (jointId == (selectedJoints.size() - 1) )
		{
			int mod = maxSteps % selectedJoints.size();
			stepsPerJoint += mod;
		}

		bool retBoolean = kMeansClustering1D(stepsPerJoint, vecTiming[jointId], vecOutMeans[jointId]);
		if (!retBoolean)
		{
			isConvergent = false;
			break;
		}
	}
	if (isConvergent)
	{
		outMeans.clear();
		for (size_t joinId = 0; joinId < selectedJoints.size(); joinId++)
		{
			for (size_t meanId = 0; meanId < vecOutMeans[joinId].size(); meanId++)
				outMeans.push_back(vecOutMeans[joinId][meanId]);
		}
		std::sort(outMeans.begin(), outMeans.end());
	}

	// apply it to corresponding points
	// also appending starting and ending corresponding points
	/*
	int motionIndex = currentState->getMotionId(selectedMotions[m]);
	if (isConvergent)
	{
		std::stringstream ss;
		ss << "[" << motion->getName() << "]detected ";
		for (size_t i = 0; i < outMeans.size(); i++)
			ss << outMeans[i] << " ";
		LOG("%s", ss.str().c_str());
		finalMessage << ss.str() << "\n";
		currentState->keys[motionIndex].clear();
		if (footStepEditor->isProcessAll)
			currentState->keys[motionIndex].push_back(0);
		for (size_t i = 0; i < outMeans.size(); i++)
			currentState->keys[motionIndex].push_back(outMeans[i]);
		if (footStepEditor->isProcessAll)
			currentState->keys[motionIndex].push_back(motion->getDuration());
	}
	else
	{
		motionsNeedManualAdjusting.push_back(motion->getName());
		std::stringstream ss;
		ss << "[" << motion->getName() << "]NOT detected(evenly distrubted): ";
		int actualNum = maxNumSteps;
		currentState->keys[motionIndex].clear();
		if (footStepEditor->isProcessAll)
			actualNum += 2;
		for (int i = 0; i < actualNum; i++)
		{
			double step = motion->getDuration() / double(actualNum - 1);
			currentState->keys[motionIndex].push_back(step * i);
			ss << step * i << " ";
		}
		LOG("%s", ss.str().c_str());
		finalMessage << ss.str() << "\n";
	}
	*/
	disconnect();
	return isConvergent;

}

bool SBMotion::addMetaData( const std::string& tagName, const std::string& strValue )
{
	bool newData = false;
	if (tagAttrMap.find(tagName) == tagAttrMap.end())
	{		
		newData = true;		
	}
	tagAttrMap[tagName] = strValue;	
	return newData;
}

bool SBMotion::removeMetaData( const std::string& tagName )
{
	if (tagAttrMap.find(tagName) != tagAttrMap.end())
	{
		tagAttrMap.erase(tagName);
		return true;
	}
	else
	{
		LOG("Tag %s not found !", tagName.c_str());
	}
	return false;
}

double SBMotion::getMetaDataDouble( const std::string& tagName )
{
	std::string strValue = getMetaDataString(tagName);
	return boost::lexical_cast<double>(strValue);
}

std::string SBMotion::getMetaDataString( const std::string& tagName )
{
	std::string strValue = "";
	if (tagAttrMap.find(tagName) != tagAttrMap.end())
	{
		strValue = tagAttrMap[tagName];		
	}
	else
	{
		LOG("Tag %s not found !", tagName.c_str());
	}
	return strValue;
}

std::vector<std::string> SBMotion::getMetaDataTags()
{
	std::vector<std::string> tagList;
	std::map<std::string, std::string >::iterator mi;
	for ( mi  = tagAttrMap.begin();
		  mi != tagAttrMap.end();
		  mi++)
	{
		tagList.push_back(mi->first);
	}
	return tagList;
}
};
