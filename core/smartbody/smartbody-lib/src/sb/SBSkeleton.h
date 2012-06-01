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
};


};


#endif