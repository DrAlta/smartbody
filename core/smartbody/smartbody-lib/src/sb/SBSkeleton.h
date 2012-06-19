#ifndef _SBSKELETON_H_
#define _SBSKELETON_H_

#include <sk/sk_skeleton.h>
#include <string>


namespace SmartBody {

class SBJoint;
class SBCharacter;

class SBSkeleton : public SkSkeleton
{
public:
	SBSkeleton();
	SBSkeleton(std::string skelFile);
	SBSkeleton(SBSkeleton* copySkel);

	virtual bool load(std::string skeletonFile);
	virtual bool save(std::string skeletonFile);
		std::string saveToString();

	const std::string& getName();
	int getNumJoints();
	SBJoint* getJoint(int index);		
	SBJoint* getJointByName(const std::string& jointName);
	std::vector<std::string> getJointNames();

	int getNumChannels();
	std::string getChannelType(int index);
	int getChannelSize(int index);

	SBCharacter* getCharacter();

	void update();

	/* the following are designed to re-orient joints local axes. added by David Huang Jun 2012 */
	// Orient skeleton joints local axes to match world coordinate axes (Y-up Z-front)
	void orientJointsLocalAxesToWorld(void);
	/* Create a new standard T-pose skel from source (TposeSk) with no pre-rotations
	// put TposeSk (source skel) into T-pose before running this! */
	void _createSkelWithoutPreRot(SBSkeleton* TposeSk, SBSkeleton* newSk, const char* new_name=0);
	// same as above but for Python interface
	SBSkeleton* createSkelWithoutPreRot(const char* new_name);
};


};


#endif