#include "SBMotion.h"
#include <sbm/mcontrol_util.h>
#include <sbm/SBScene.h>
#include <sbm/me_utilities.hpp>
#include <sr/sr_euler.h>
#include <sbm/SBSkeleton.h>
#include <sbm/SBJoint.h>

namespace SmartBody {


SBMotion::SBMotion() : SkMotion()
{
	_motionFile = "";
}

SBMotion::SBMotion(const SBMotion& motion)
{
	//...
}

SBMotion::SBMotion(std::string file) : SkMotion()
{
	_motionFile = file;
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
	for (int i = minFrameId; i < maxFrameId - 1; i++)
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
	for (int i = minFrameId; i < maxFrameId - 1; i++)
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
