#include "SBSkeleton.h"
#include "SBJoint.h"
#include "SBCharacter.h"
#include <sbm/mcontrol_util.h>

#include <sr/sr_string.h>

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

bool SBSkeleton::save(std::string skeletonFile)
{
	SkSkeleton* myskel = dynamic_cast<SkSkeleton*>(this);
	FILE *fp = NULL;
	fp = fopen(skeletonFile.c_str(), "w");
	if (fp == NULL )
	{
		LOG("Can't create skeleton file %s.", skeletonFile.c_str());
		return false;
	}
	SrOutput output(fp);
	if (output.valid())
	{
		output.filename(skeletonFile.c_str());
		if (!SkSkeleton::save(output))
		{
			LOG("Problem saving skeleton to file %s.", skeletonFile.c_str());
			return false;
		} 
		else
		{
			LOG("Skeleton saved to file %s.", skeletonFile.c_str());
			return true;
		}
	}
	else
	{
		LOG("Problem saving skeleton to file %s.", skeletonFile.c_str());
		return false;
	}
}

std::string SBSkeleton::saveToString()
{
	std::string retString = "";
	SkSkeleton* myskel = dynamic_cast<SkSkeleton*>(this);
	SrString outputString;
	SrOutput output(outputString);
	if (myskel->save(output))
	{
		retString = (const char*)outputString;
	}
	return retString;
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
		jointNames.push_back(alljoints[i]->name());
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


/* the following are designed to re-orient joints local axes. added by David Huang Jun 2012*/
/* Orient skeleton joints local axes to match world coordinate axes (Y-up Z-front)
// put skel into T-pose before running this! */
void SBSkeleton::orientJointsLocalAxesToWorld(void)
{
	const std::vector<SkJoint*>& jnts = SkSkeleton::joints();
	update_global_matrices();
	for(size_t i=0; i<jnts.size(); i++)
	{
		SkJoint* j = jnts[i];
		j->quat()->orientation(/*j->prerot() * */j->quat()->value());
		const SrMat& gmat = j->gmat();
		SrQuat grot; grot.set(gmat);
		j->quat()->prerot(grot.inverse());
		j->quat()->postrot(grot.inverse());
	}
	
	// now we can remove local rotations
	for(size_t i=0; i<jnts.size(); i++)
		jnts[i]->init_rot();
	update_global_matrices();
}

/* Create a new standard T-pose skel with no pre-rotations from source (TposeSk)
// put source skel(TposeSk) into T-pose before running this!   */
void SBSkeleton::_createSkelWithoutPreRot(SBSkeleton* TposeSk, SBSkeleton* newSk, const char* new_name)
{
	TposeSk->invalidate_global_matrices();
	TposeSk->update_global_matrices();
	newSk->copy(TposeSk); // first copy
	if(new_name || strlen(new_name)<1)
	{
		newSk->name(string(new_name));
		SrString fname(TposeSk->skfilename().c_str());
		fname.remove_file_name();
		fname.append(new_name); 
		//fname.append(".sk");
		newSk->skfilename(fname);
	}
	
	const std::vector<SkJoint*>& new_jnts = newSk->joints();
	for(size_t i=1; i<new_jnts.size(); i++)
	{
		SkJoint* new_j = new_jnts[i];
		if(!new_j->parent()) continue;
		SkJoint* src_j = TposeSk->search_joint(new_j->name().c_str());
		if(!src_j) continue;
		if(!src_j->parent()) continue;
		SrVec new_offset = src_j->gmat().get_translation() -
			src_j->parent()->gmat().get_translation();		
		new_j->offset(new_offset);
	}
	
	for(size_t i=1; i<new_jnts.size(); i++)
	{
		SkJoint* new_j = new_jnts[i];
		new_j->init_values();
		new_j->quat()->prerot(SrQuat::null);
		new_j->quat()->postrot(SrQuat::null);
	}
}
// same as above but for Python interface
SBSkeleton* SBSkeleton::createSkelWithoutPreRot(const char* new_name)
{
	SBSkeleton* newSk = new SBSkeleton;
	_createSkelWithoutPreRot(this, newSk, new_name);
	return newSk;
}

void SBSkeleton::rescale( float scaleRatio )
{
	for (unsigned int i=0;i<joints().size();i++)
	{
		SBJoint* joint = dynamic_cast<SBJoint*>(joints()[i]);
		if (joint)
			joint->setOffset(joint->getOffset()*(float)scaleRatio);
	}
}

}; //namespace
