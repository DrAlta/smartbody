#include "KinectProcessor.h"

KinectProcessor::KinectProcessor()
{
	//Note: everything is based on below assumptions
	//20 to 24 bones, using kinect sdk standard, the joint order is also kinect sdk order as following
	//index		openNI					kinecSDK				SmartBody
	//0			XN_SKEL_WAIST			HipCenter				base
	//1			XN_SKEL_TORSO			Spine					spine2
	//2			XN_SKEL_NECK			ShoulderCenter			spine3
	//3			XN_SKEL_HEAD			Head					skullbase
	//4			XN_SKEL_LEFT_SHOULDER	ShoulderLeft			l_shoulder
	//5			XN_SKEL_LEFT_ELBOW		ElbowLeft				l_elbow
	//6			XN_SKEL_LEFT_WRIST		WristLeft				l_wrist	
	//7			XN_SKEL_LEFT_HAND		HandLeft				l_index1
	//8			XN_SKEL_RIGHT_SHOULDER	ShoulderRight			r_shoulder
	//9			XN_SKEL_RIGHT_ELBOW		ElbowRight				r_elbow
	//10		XN_SKEL_RIGHT_WRIST		WristRight				r_wrist
	//11		XN_SKEL_RIGHT_HAND		HandRight				r_index1
	//12		XN_SKEL_LEFT_HIP		HipLeft					l_hip		
	//13		XN_SKEL_LEFT_KNEE		KneeLeft				l_knee			
	//14		XN_SKEL_LEFT_ANKLE		AnkleLeft				l_ankle		
	//15		XN_SKEL_LEFT_FOOT		FootLeft				l_forefoot
	//16		XN_SKEL_RIGHT_HIP		HipRight				r_hip
	//17		XN_SKEL_RIGHT_KNEE		KneeRight				r_knee
	//18		XN_SKEL_RIGHT_ANKLE		AnkleRight				r_ankle
	//19		XN_SKEL_RIGHT_FOOT		FootRight				r_forefoot
	//20		XN_SKEL_LEFT_COLLAR								l_acromioclavicular
	//21		XN_SKEL_RIGHT_COLLAR							r_acromioclavicular
	//22		XN_SKEL_LEFT_FINGERTIP									
	//23		XN_SKEL_RIGHT_FINGERTIP							

	// default mapping, can be user defined too
	boneMapping.push_back("base");			
	boneMapping.push_back("spine2");	// should it be spine2 or base
	boneMapping.push_back("spine3");	// should it be spine4?
	boneMapping.push_back("spine5");
	boneMapping.push_back("l_shoulder");
	boneMapping.push_back("l_elbow");
	boneMapping.push_back("l_wrist");
	boneMapping.push_back("l_index1");			// no mapping for hand
	boneMapping.push_back("r_shoulder");
	boneMapping.push_back("r_elbow");
	boneMapping.push_back("r_wrist");
	boneMapping.push_back("r_index1");			// no mapping for hand
	boneMapping.push_back("l_hip");
	boneMapping.push_back("l_knee");
	boneMapping.push_back("l_ankle");
	boneMapping.push_back("l_forefoot");
	boneMapping.push_back("r_hip");
	boneMapping.push_back("r_knee");
	boneMapping.push_back("r_ankle");
	boneMapping.push_back("r_forefoot");

	boneMapping.push_back("l_acromioclavicular");
	boneMapping.push_back("r_acromioclavicular");
	boneMapping.push_back("");
	boneMapping.push_back("");


	filterSize = 10;
	rotationBuffer.resize(24);
	for (int i = 0; i < 24; i++)
		rotationBuffer[i].resize(24);
}

KinectProcessor::~KinectProcessor()
{
	boneMapping.clear();
}


int KinectProcessor::getNumBones()
{
	return boneMapping.size();
}

const char* KinectProcessor::getSBJointName(int i)
{
	if (i >= 0 && i < 24)
		return boneMapping[i].c_str();
	else
		return "";
}

void KinectProcessor::setSBJointName(int i, const char* jName)
{
	if (i >= 0 && i < 24)
		boneMapping[i] = jName;
}

void KinectProcessor::processGlobalRotation(std::vector<SrQuat>& quats)
{
	if (quats.size() != 24)
		return;

	// process: get local, and reverse x-axis
	std::vector<SrQuat> tempQuats(24);
	tempQuats = quats;

#if 0
	tempQuats[1] = quats[1] * quats[0].inverse();
	tempQuats[2] = quats[2] * quats[1].inverse();
	tempQuats[3] = quats[3] * quats[2].inverse();
	tempQuats[4] = quats[4] * quats[2].inverse();
	tempQuats[5] = quats[5] * quats[4].inverse();
	tempQuats[6] = quats[6] * quats[5].inverse();
	tempQuats[7] = quats[7] * quats[6].inverse();
	tempQuats[8] = quats[8] * quats[2].inverse();
	tempQuats[9] = quats[9] * quats[8].inverse();
	tempQuats[10] = quats[10] * quats[9].inverse();
	tempQuats[11] = quats[11] * quats[10].inverse();
	tempQuats[12] = quats[12] * quats[0].inverse();
	tempQuats[13] = quats[13] * quats[12].inverse();
	tempQuats[14] = quats[14] * quats[13].inverse();
	tempQuats[15] = quats[15] * quats[14].inverse();
	tempQuats[16] = quats[16] * quats[0].inverse();
	tempQuats[17] = quats[17] * quats[16].inverse();
	tempQuats[18] = quats[18] * quats[17].inverse();
	tempQuats[19] = quats[19] * quats[18].inverse();
#endif

	// base
	quats[0] = quats[1];

	for (int i = 0; i < 24; i++)
	{
		if (quats[i].w == 0)
		{
			tempQuats[i] = SrQuat(0, 0, 0, 0);
			quats[i] = SrQuat();
		}
		else
		{
			if (i == 0)
				tempQuats[i] = quats[i];
			else if (i == 4)		// l_shoulder
				tempQuats[i] = quats[i] * quats[2].inverse();
			else if (i == 8)		// r_shoulder
				tempQuats[i] = quats[i] * quats[2].inverse();
			else if (i == 12)		// l_hip
				tempQuats[i] = quats[i] * quats[0].inverse();
			else if (i == 16)		// r_hip
				tempQuats[i] = quats[i] * quats[0].inverse();
			else if (i == 20)		// l_acromioclavicular
				tempQuats[i] = quats[i] * quats[0].inverse();
			else if (i == 21)		// r_acromioclavicular
				tempQuats[i] = quats[i] * quats[0].inverse();
			else
				tempQuats[i] = quats[i] * quats[i - 1].inverse();
		}
	}
	quats = tempQuats;

	for (unsigned int i = 0; i < 24; i++)
		quats[i].x *= -1.0f;
}

void KinectProcessor::filterRotation(std::vector<SrQuat>& quats)
{
	while ((int)rotationBuffer[0].size() > filterSize)
	{
		for (int i = 0; i < 24; i++)
			rotationBuffer[i].pop_front();
	}
	for (int i = 0; i < 24; i++)
		rotationBuffer[i].push_back(quats[i]);

	float weight = 1.0f / float(filterSize);
	for (int i = 0; i < 24; i++)
	{
		std::list<SrQuat>::iterator iter = rotationBuffer[i].begin();
		SrQuat temp = *iter;
		for (int j = 1; j < filterSize; j++)
		{
			iter++;
			SrQuat q = *iter;
			SrQuat interpolate = slerp(q, temp, 0.5);
			temp = interpolate;
		}
		quats[i] = temp;
	}
}