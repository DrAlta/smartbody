#include "BoneMap.h"
#include "sk/sk_channel_array.h"

BoneMap::BoneMap()
{
}

BoneMap::~BoneMap()
{
}

void BoneMap::apply(SkMotion* motion)
{
	if (!motion)
		return;
	
	SkChannelArray& channels = motion->channels();
	for (std::vector<std::pair<std::string, std::string> >::iterator iter = map.begin();
		iter != map.end();
		iter++)
	{
		std::string from = (*iter).first;
		std::string to = (*iter).second;
		channels.changeChannelName(from, to);
	}

}

void BoneMap::apply(SkSkeleton* skeleton)
{
	if (!skeleton)
		return;
	
	std::vector<SkJoint*> joints = skeleton->joints();
	for (size_t j = 0; j < joints.size(); j++)
	{
		for (std::vector<std::pair<std::string, std::string> >::iterator iter = map.begin();
			 iter != map.end();
			 iter++)
		{
			std::string from = (*iter).first;
			std::string to = (*iter).second;
			if (joints[j]->name() == from.c_str())
			{
				joints[j]->name(to);
			}
		}
	}
	SkChannelArray& channels = skeleton->channels();
	for (std::vector<std::pair<std::string, std::string> >::iterator iter = map.begin();
		iter != map.end();
		iter++)
	{
		std::string from = (*iter).first;
		std::string to = (*iter).second;
		channels.changeChannelName(from, to);
	}
}
