#ifndef _SBMOTION_H
#define _SBMOTION_H

#include <vector>
#include <sbm/SBSkeleton.h>
#include <sk/sk_motion.h>
#include <sbm/SBJoint.h>

namespace SmartBody {

class SBMotion : public SkMotion
{
	public:
		SBMotion();
		SBMotion(const SBMotion& motion);
		SBMotion(std::string motionFile);
		~SBMotion();

		const std::string& getMotionFileName();
		int getNumFrames();
		std::vector<float> getFrameData(int i);
		int getFrameSize();

		int getNumChannels();
		std::vector<std::string> getChannels();
		void checkSkeleton(std::string skel);
		
		virtual void connect(SBSkeleton* skel);
		virtual void disconnect();

		SBMotion* mirror(std::string name, std::string skeletonName);

		float getJointSpeed(SBJoint* joint, float startTime, float endTime);
		float getJointAngularSpeed(SBJoint* joint, float startTime, float endTime);
		std::vector<float> getJointTransition(SBJoint* joint, float startTime, float endTime);

	protected:
		std::string _motionFile;
		std::string _emptyString;
};

};

#endif