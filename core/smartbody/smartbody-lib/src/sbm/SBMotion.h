#ifndef _SBMOTION_H
#define _SBMOTION_H

#include <vector>
#include <string>
#include <sk/sk_motion.h>

namespace SmartBody {

class SBJoint;
class SBSkeleton;

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
		SBMotion* smoothCycle(std::string name, float timeInterval);
		bool translate(float x, float y, float z, const std::string& baseJointName);
		bool rotate(float xaxis, float yaxis, float zaxis, const std::string& baseJointName);
		bool scale(float factor);
		bool retime(float factor);
	//	bool trim(int numFramesFromFront, int numFramesFromBack);
	//	bool move(int startFrame, int endFrame, int position);

		float getJointSpeed(SBJoint* joint, float startTime, float endTime);
		float getJointAngularSpeed(SBJoint* joint, float startTime, float endTime);
		std::vector<float> getJointTransition(SBJoint* joint, float startTime, float endTime);

		double getDuration();
		double getTimeStart();
		double getTimeReady();
		double getTimeStrokeStart();
		double getTimeStroke();
		double getTimeStrokeEnd();
		double getTimeRelax();
		double getTimeStop();

		void addEvent(double time, const std::string& type, const std::string& parameters, bool onceOnly);

	protected:
		std::string _motionFile;
		std::string _emptyString;
};

};

#endif
