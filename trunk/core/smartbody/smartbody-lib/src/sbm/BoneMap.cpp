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
	for (std::map<std::string, std::string>::iterator iter = map.begin();
		iter != map.end();
		iter++)
	{
		std::string from = (*iter).first;
		std::string to = (*iter).first;
		channels.changeChannelName(from, to);
	}

}

void BoneMap::apply(SkSkeleton* skeleton)
{
}
