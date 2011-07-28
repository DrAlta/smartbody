#ifndef _KINECT_PROCESSOR_H_
#define _KINECT_PROCESSOR_H_

#include <string>
#include <vector>
#include <list>
#include <sr/sr_quat.h>
class KinectProcessor
{
	public:
		KinectProcessor();
		~KinectProcessor();
		int getNumBones();
		const char* getSBJointName(int i);
		void setSBJointName(int i, const char* jName);
		static void processGlobalRotation(std::vector<SrQuat>& quats);
		void filterRotation(std::vector<SrQuat>& quats);
	
	private:
		std::vector<std::string>			boneMapping;
		std::vector<std::list<SrQuat> >		rotationBuffer;
		int									filterSize;
};
#endif
