#include "SBMotion.h"
#include <sbm/mcontrol_util.h>
#include <sbm/me_utilities.hpp>

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
		std::string chanName = channels()[i].joint->name().c_str();
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
		std::string name = chanName + " " + chanTypeString;
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

SBMotion* SBMotion::mirror(std::string name)
{
	SkMotion* motion = buildMirrorMotion();
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

		mcuCBHandle& mcu = mcuCBHandle::singleton(); 
		mcu.motion_map.insert(std::pair<std::string, SkMotion*>(motionName, motion));
	}
	return sbmotion;
}

};