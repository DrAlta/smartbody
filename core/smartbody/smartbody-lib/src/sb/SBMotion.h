#ifndef _SBMOTION_H
#define _SBMOTION_H

#include <vector>
#include <string>
#include <sk/sk_motion.h>

namespace SmartBody {

class SBJoint;
class SBSkeleton;
class SBMotion;

class FootStepRecord
{
public:
	std::vector<std::string> jointNames; // all joints related to a footstep
	std::vector<SrVec> posVec; // desired positions for these joints
	float startTime, endTime;	
	FootStepRecord();
	~FootStepRecord();
	FootStepRecord& operator= ( const FootStepRecord& rt);

	void updateJointAveargePosition( SBSkeleton* skel, SBMotion* motion);
};

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

		void alignToBegin(int numFrames);
		void alignToEnd(int numFrames);
		int getAlignIndex();
		void recoverAlign();
		SBMotion* duplicateCycle(int num, std::string name);

		SBMotion* mirror(std::string name, std::string skeletonName);
		SBMotion* smoothCycle(std::string name, float timeInterval);
		SBMotion* retarget(std::string name, std::string srcSkeletonName, std::string dstSkeletonName, std::vector<std::string>& endJoints, std::map<std::string, SrVec>& offsetJointMap);
		bool translate(float x, float y, float z, const std::string& baseJointName);
		bool rotate(float xaxis, float yaxis, float zaxis, const std::string& baseJointName);
		bool scale(float factor);
		bool retime(float factor);
		bool trim(int numFramesFromFront, int numFramesFromBack);
	//	bool move(int startFrame, int endFrame, int position);

		float getJointSpeed(SBJoint* joint, float startTime, float endTime);
		float getJointSpeedAxis(SBJoint* joint, const std::string& axis, float startTime, float endTime);
		float getJointAngularSpeed(SBJoint* joint, float startTime, float endTime);
		float getJointAngularSpeedAxis(SBJoint* joint, const std::string& axis, float startTime, float endTime);
		std::vector<float> getJointTransition(SBJoint* joint, float startTime, float endTime);

		bool autoFootStepDetection(std::vector<double>& outMeans, int numStepsPerJoint, int maxNumSteps, SBSkeleton* skeleton, 
								   std::vector<std::string>& selectedJoints, float floorHeight, float floorThreshold, float speedThreshold, 
								   int speedWindow, bool isPrintDebugInfo = false);

		bool autoFootPlantDetection(SBSkeleton* srcSk, std::vector<std::string>& footJoints, float floorHeight, float heightThreshold, float speedThreshold, std::vector<FootStepRecord>& footStepRecords);		
		SBMotion* autoFootSkateCleanUp(std::string name, std::string srcSkeletonName, std::string rootName, std::vector<FootStepRecord>& footStepRecords);
		// API wrapper
		SBMotion* footSkateCleanUp(std::string name, std::vector<std::string>& footJoints, std::string srcSkeletonName, std::string srcMotionName, 
								   std::string tgtSkeletonName, std::string tgtRootName, float floorHeight, float heightThreshold, float speedThreshold);

		double getFrameRate();
		double getDuration();
		double getTimeStart();
		double getTimeReady();
		double getTimeStrokeStart();
		double getTimeStroke();
		double getTimeStrokeEnd();
		double getTimeRelax();
		double getTimeStop();

		bool addTagMetaData(const std::string& tagName, const std::string& strValue);
		bool removeTagMetaData(const std::string& tagName);
		int  getTagMetaDataSize(const std::string& tagName);
		std::string getTagMetaDataString(const std::string& tagName);
		std::string getTagMetaDataStringWithIndex(const std::string& tagName, int index);
		double      getTagMetaDataDouble(const std::string& tagName);
		double      getTagMetaDataDoubleWithIndex(const std::string& tagName, int index);
		std::vector<std::string> getTagMetaDataStringList(const std::string& tagName);	
		std::vector<std::string> getMetaDataTagList();
		

		void addEvent(double time, const std::string& type, const std::string& parameters, bool onceOnly);

	protected:
		void alignToSide(int numFrames, int direction = 0);

		static bool kMeansClustering1D(int num, std::vector<double>& inputPoints, std::vector<double>& outMeans);
		static void calculateMeans(std::vector<double>&inputPoints, std::vector<double>& means, double convergentValue);
		

	protected:
		std::string _motionFile;
		std::string _emptyString;
		int alignIndex;
		std::map<std::string, std::vector<std::string> > tagAttrMap; // store the tagged attributes in a map
};

};

#endif
