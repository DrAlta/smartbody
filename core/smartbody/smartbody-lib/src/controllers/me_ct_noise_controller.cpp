#include "controllers/me_ct_noise_controller.h"
#include <sr/sr_euler.h>
#include <sbm/mcontrol_util.h>
#include <sb/SBSkeleton.h>

std::string MeCtNoiseController::CONTROLLER_TYPE = "NoiseController";

MeCtNoiseController::MeCtNoiseController(SbmCharacter* character) : SmartBody::SBController()
{
	_character = character;
	_skeletonCopy = new SBSkeleton();
	_skeletonCopy->copy(_character->getSkeleton());
	_valid = true;
	_prevTime = 0.0;
	_duration = 0.0;
	perlinScale = 0.02f;
	perlinFrequency = 0.03f;
	perlinDuration = 1.f/perlinFrequency;
}

MeCtNoiseController::~MeCtNoiseController()
{
	
}

void MeCtNoiseController::setJointChannelQuat( const std::string& jointName, MeFrameData& frame, SrQuat& inQuat )
{
	bool hasRotation = true;
	int channelId = _context->channels().search(jointName, SkChannel::Quat);
	if (channelId < 0)	hasRotation = false;
	int bufferId = frame.toBufferIndex(channelId);
	if (bufferId < 0)	hasRotation = false;	
	
	if (hasRotation)
	{
		frame.buffer()[bufferId + 0] = inQuat.w;
		frame.buffer()[bufferId + 1] = inQuat.x;;
		frame.buffer()[bufferId + 2] = inQuat.y;;
		frame.buffer()[bufferId + 3] = inQuat.z;;
	}
}

void MeCtNoiseController::getJointChannelValues( const std::string& jointName, MeFrameData& frame, SrQuat& outQuat, SrVec& outPos )
{
	bool hasRotation = true;
	int channelId = _context->channels().search(jointName, SkChannel::Quat);
	if (channelId < 0)	hasRotation = false;
	int bufferId = frame.toBufferIndex(channelId);
	if (bufferId < 0)	hasRotation = false;	

	bool hasTranslation = true;
	int positionChannelID = _context->channels().search(jointName, SkChannel::XPos);
	if (positionChannelID < 0) hasTranslation = false;
	int posBufferID = frame.toBufferIndex(positionChannelID);
	if (posBufferID < 0) hasTranslation = false;		
	// input reference pose
	if (hasRotation)
	{
		outQuat.w = frame.buffer()[bufferId + 0];
		outQuat.x = frame.buffer()[bufferId + 1];
		outQuat.y = frame.buffer()[bufferId + 2];
		outQuat.z = frame.buffer()[bufferId + 3];
	}
	if (hasTranslation)
	{
		outPos.x = frame.buffer()[posBufferID + 0];
		outPos.y = frame.buffer()[posBufferID + 1];
		outPos.z = frame.buffer()[posBufferID + 2];				
	}
}

void MeCtNoiseController::setJointNoise( std::vector<std::string>& jointNames, float scale, float frequency )
{
	perlinMap.clear();
	for (unsigned int i=0;i<jointNames.size();i++)
	{
		std::string jname = jointNames[i];
		SBJoint* joint = _skeletonCopy->getJointByName(jname);
		if (joint) // joint actually exist 
		{
			perlinMap[jname] = Perlin();
			perlinMap[jname].init(); // reinitialize perlin noise for the joint
		}
	}
	if (scale > 0.f)
		perlinScale = scale;
	if (frequency > 0.f)
	{
		perlinFrequency = frequency;
		perlinDuration = 1.f/frequency;
	}	
}

float MeCtNoiseController::getNormalizeTime( float t, float offset )
{
	int nT = t/perlinDuration;
	float normalizeTime = (t - perlinDuration*nT)/perlinDuration;
	return normalizeTime;
}

bool MeCtNoiseController::controller_evaluate(double t, MeFrameData& frame)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (_prevTime == 0)
		_dt = 0.016;
	else
	{
		_dt = t - _prevTime;
		_prevTime = t;
	}	
	float invDt = 1.f/0.016;
	if (_dt > 1e-6)
		invDt = 1.f/(float)_dt;	
#if 1
	if (_valid && _context )
	{
		SrVec normalizeTime;
		for (int k=0;k<3;k++)
		{			
			normalizeTime[k] = getNormalizeTime(t, perlinDuration*0.3f*k);
		}
		
		std::map<std::string, Perlin>::iterator mi;
		for ( mi  = perlinMap.begin();
			  mi != perlinMap.end();
			  mi++)
		{
			SrQuat oldQuat, newQuat;
			SrVec oldPos;
			std::string jname = mi->first;
			Perlin& perlinNoise = mi->second;
			getJointChannelValues(jname, frame, oldQuat, oldPos);
			SrVec noiseVec;
			newQuat = oldQuat;
			for (int k=0;k<3;k++)
			{
				noiseVec[k] = perlinNoise.noise1(normalizeTime[k])*perlinScale + newQuat.getData(k+1);
				newQuat.setData(k+1,noiseVec[k]);
			}
			newQuat.normalize();
			setJointChannelQuat(jname, frame, newQuat);
		}
	}	
#endif
	return true;
}

