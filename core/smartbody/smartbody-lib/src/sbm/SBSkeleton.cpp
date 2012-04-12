#include "SBSkeleton.h"
#include "SBJoint.h"
#include "SBCharacter.h"
#include <sbm/mcontrol_util.h>

namespace SmartBody {

SBSkeleton::SBSkeleton() : SkSkeleton()
{
}

SBSkeleton::SBSkeleton(std::string skelFile) : SkSkeleton()
{
	load(skelFile);
}

SBSkeleton::SBSkeleton(SBSkeleton* copySkel) : SkSkeleton(copySkel)
{
}

bool SBSkeleton::load(std::string skeletonFile)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	std::map<std::string, SkSkeleton*>::iterator iter = mcu.skeleton_map.find(std::string(skeletonFile.c_str()));
	if (iter != mcu.skeleton_map.end())
	{
		SkSkeleton* existingSkel = iter->second;
		SmartBody::SBSkeleton* existingSBSkel = dynamic_cast<SmartBody::SBSkeleton*>(existingSkel);
		copy(existingSBSkel);
		update();
		return true;
	}
	else
	{
		FILE *fp = NULL;
		fp = fopen(skeletonFile.c_str(), "rt");
		if (fp == NULL )
		{
			LOG("No skeleton found for file %s.", skeletonFile.c_str());
			return false;
		}
		SrInput input(fp);
		if (input.valid())
		{
			input.filename(skeletonFile.c_str());
			if (!SkSkeleton::load(input, 1.0f))
			{
				LOG("Problem loading skeleton from file %s.", skeletonFile.c_str());
				return false;
			} 
			else
			{
				update();
				return true;
			}
		}
		else
		{
			LOG("No skeleton found for file %s.", skeletonFile.c_str());
			return false;
		}
	}
}

const std::string& SBSkeleton::getName()
{
	return name();
}

int SBSkeleton::getNumJoints()
{
	return joints().size();
}

SBJoint* SBSkeleton::getJointByName(const std::string& jointName)
{
	SkJoint* j = search_joint(jointName.c_str());
	if (j)
	{
		SBJoint* sbJoint = dynamic_cast<SBJoint*>(j);
		return sbJoint;
	}
	else
	{
		return NULL;
	}
}

std::vector<std::string> SBSkeleton::getJointNames()
{
	std::vector<std::string> jointNames;
	const std::vector<SkJoint*>& alljoints = joints();
	for (size_t i = 0; i < alljoints.size(); i++)
	{
		jointNames.push_back(alljoints[i]->getName());
	}
	return jointNames;
}


SBJoint* SBSkeleton::getJoint(int index)
{
	const std::vector<SkJoint*>& alljoints = joints();
	if (size_t(index) >=0 && size_t(index) < alljoints.size())
	{
		SBJoint* sbJoint = dynamic_cast<SBJoint*>(alljoints[index]);
		return sbJoint;
	}
	else
	{
		return NULL;
	}
}

int SBSkeleton::getNumChannels()
{
	return channels().size();
}

std::string SBSkeleton::getChannelType(int index)
{
	if (index > 0 && index < channels().size())
	{
		return SkChannel::type_name(channels()[index].type);
	}
	else
	{
		return "";
	}
}

int SBSkeleton::getChannelSize(int index)
{
	if (index > 0 && index < channels().size())
	{
		return channels()[index].size();
	}
	else
	{
		return 0;
	}

}

SBCharacter* SBSkeleton::getCharacter()
{
	// determine which character uses this skeleton
	// NOTE: there should be back pointer between the skeleton and the pawn/character
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	for (std::map<std::string, SbmCharacter*>::iterator iter = mcu.getCharacterMap().begin();
		 iter != mcu.getCharacterMap().end();
		 iter++)
	{
		SmartBody::SBCharacter* character = dynamic_cast<SmartBody::SBCharacter*>((*iter).second);
		if (character->getSkeleton() == this)
			return character;
	
	}

	return NULL;
}

void SBSkeleton::update()
{
	refresh_joints();
	make_active_channels();
		
	SBCharacter* character = getCharacter();
	if (character)
	{
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		if ( mcuCBHandle::singleton().sbm_character_listener )
			mcuCBHandle::singleton().sbm_character_listener->OnCharacterUpdate( character->getName().c_str(), character->getClassType() );
	}
}

};
