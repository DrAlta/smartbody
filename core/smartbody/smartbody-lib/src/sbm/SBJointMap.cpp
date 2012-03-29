#include <sbm/SBJointMap.h>
#include "sk/sk_channel_array.h"
#include <sbm/SBMotion.h>
#include <sbm/SBSkeleton.h>

namespace SmartBody {

SBJointMap::SBJointMap()
{
}

SBJointMap::~SBJointMap()
{
}

void SBJointMap::applyMotion(SmartBody::SBMotion* motion)
{
	if (!motion)
		return;
	
	SkChannelArray& channels = motion->channels();
	for (std::vector<std::pair<std::string, std::string> >::iterator iter = _map.begin();
		iter != _map.end();
		iter++)
	{
		std::string from = (*iter).first;
		std::string to = (*iter).second;
		channels.changeChannelName(from, to);
	}

}

void SBJointMap::applySkeleton(SmartBody::SBSkeleton* skeleton)
{
	if (!skeleton)
		return;
	
	std::vector<SkJoint*> joints = skeleton->joints();
	for (size_t j = 0; j < joints.size(); j++)
	{
		for (std::vector<std::pair<std::string, std::string> >::iterator iter = _map.begin();
			 iter != _map.end();
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
	for (std::vector<std::pair<std::string, std::string> >::iterator iter = _map.begin();
		iter != _map.end();
		iter++)
	{
		std::string from = (*iter).first;
		std::string to = (*iter).second;
		channels.changeChannelName(from, to);
	}
}

void SBJointMap::setMapping(const std::string& from, const std::string& to)
{
	for (std::vector<std::pair<std::string, std::string> >::iterator iter = _map.begin();
		iter != _map.end();
		iter++)
	{
		std::string f = (*iter).first;
		if (from == f)
		{
			(*iter).second = to;
			return;
		}
	}

	_map.push_back(std::pair<std::string, std::string>(from, to));
}

void SBJointMap::removeMapping(const std::string& from)
{
}

std::string SBJointMap::getMapSource(const std::string& to)
{
	for (std::vector<std::pair<std::string, std::string> >::iterator iter = _map.begin();
		iter != _map.end();
		iter++)
	{
		std::string t = (*iter).second;
		if (to == t)
		{
			std::string f = (*iter).first;
			return f;
		}
	}
	return "";
}

std::string SBJointMap::getMapTarget(const std::string& from)
{
	for (std::vector<std::pair<std::string, std::string> >::iterator iter = _map.begin();
		iter != _map.end();
		iter++)
	{
		std::string f = (*iter).first;
		if (from == f)
		{
			std::string t = (*iter).second;
			return t;
		}
	}
	return "";
}

int SBJointMap::getNumMappings()
{
	return _map.size();
}

std::string SBJointMap::getTarget(int num)
{
	if (_map.size() > (size_t) num)
	{
		return _map[num].first;
	}
	return "";
}

std::string SBJointMap::getSource(int num)
{
	if (_map.size() > (size_t) num)
	{
		return _map[num].second;
	}
	return "";
}

}
