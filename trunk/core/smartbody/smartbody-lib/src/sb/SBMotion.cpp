#include "SBMotion.h"
#include <sbm/mcontrol_util.h>
#include <sb/SBScene.h>
#include <sbm/me_utilities.hpp>
#include <sr/sr_euler.h>
#include <sb/SBSkeleton.h>
#include <sb/SBJoint.h>

namespace SmartBody {


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

	alignIndex -= numFrames;	
}


SBMotion* SBMotion::retarget( std::string name, std::string srcSkeletonName, std::string dstSkeletonName )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton(); 
	SBSkeleton* srcSkeleton = mcu._scene->getSkeleton(srcSkeletonName);
	SBSkeleton* dstSkeleton = mcu._scene->getSkeleton(dstSkeletonName);
	if (!srcSkeleton || !dstSkeleton)
	{
		LOG("Skeleton %s or %s not found. Retargeted motion %s not built.",srcSkeletonName.c_str(),dstSkeletonName.c_str(),name.c_str());
		return NULL;
	}
	
	SkMotion* motion = buildRetargetMotion(srcSkeleton,dstSkeleton);
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


		mcu.motion_map.insert(std::pair<std::string, SkMotion*>(motionName, motion));
	}
	return sbmotion;

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

/*
bool SBMotion::trim(int numFramesFromFront, int numFramesFromBack)
{
	return true;
}

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

};
