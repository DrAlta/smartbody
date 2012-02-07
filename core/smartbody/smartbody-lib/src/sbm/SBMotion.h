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
		bool translate(float x, float y, float z, const std::string& baseJointName);
		bool rotate(float xaxis, float yaxis, float zaxis, const std::string& baseJointName);
		bool scale(float factor);
		bool retime(float factor);
	//	bool trim(int numFramesFromFront, int numFramesFromBack);
	//	bool move(int startFrame, int endFrame, int position);

		float getJointSpeed(SBJoint* joint, float startTime, float endTime);
		float getJointAngularSpeed(SBJoint* joint, float startTime, float endTime);
		std::vector<float> getJointTransition(SBJoint* joint, float startTime, float endTime);

		double getTimeStart();
		double getTimeReady();
		double getTimeStrokeStart();
		double getTimeStroke();
		double getTimeStrokeEnd();
		double getTimeRelax();
		double getTimeStop();

	protected:
		std::string _motionFile;
		std::string _emptyString;
};

};

#endif
