#include "me_ct_motion_profile.h"

/************************************************************************/
/* Piece-wise Linear Curve                                              */
/************************************************************************/
void LinearProfileCurve::init()
{
	curve.clear();
}

void LinearProfileCurve::addPt( float t, float val )
{
	curve.insert(t,val);
}

float LinearProfileCurve::evaluate( float t )
{
	return (float)curve.evaluate(t);
}

/************************************************************************/
/* Motion Profile                                                       */
/************************************************************************/

MotionProfile::MotionProfile( SkMotion* m )
{
	motion = m;
}

MotionProfile::~MotionProfile( void )
{

}

void MotionProfile::buildInterpolationProfile( float startTime, float endTime, float timeStep )
{
	cleanUpProfileMap(interpolationProfile);
	ProfileCurveMap::iterator mi;	
	for (mi  = velocityProfile.begin();
		 mi != velocityProfile.end();
		 mi++)
	{
		std::string chanName = mi->first;
		ProfileCurve* velCurve = mi->second;
		interpolationProfile[chanName] = createProfileCurve();
		createNormalizeInterpolationCurve(startTime,endTime,timeStep,velCurve,interpolationProfile[chanName]);		
	}
}

void MotionProfile::buildVelocityProfile( float startTime, float endTime, float timeStep )
{
	float motionLength = motion->duration();
	std::vector<float> motionBuffer[2];
	for (int k=0;k<2;k++)
		motionBuffer[k].resize(motion->channels().count_floats()); 

	// clean up profile curves
	cleanUpProfileMap(velocityProfile);

	float prevTime = startTime - timeStep;
	if (prevTime < 0.f) prevTime = 0.f;
	motion->apply(prevTime,&motionBuffer[0][0],NULL);
	int curBufferIdx = 1;
	int prevBufferIdx = 0;
	for (float t = startTime; t <= endTime; t+= timeStep)
	{
		// get current motion frame
		motion->apply(t,&motionBuffer[curBufferIdx][0],NULL);		
		computeVelocity(motion,t,motionBuffer[prevBufferIdx],motionBuffer[curBufferIdx],velocityProfile);
		curBufferIdx = 1 - curBufferIdx;
		prevBufferIdx = 1 - prevBufferIdx;
	}
}

void MotionProfile::createNormalizeInterpolationCurve( float startTime, float endTime, float timeStep, ProfileCurve* velCurve, ProfileCurve* interpCurve )
{
	float integration = 0.0f;
	float normStep = timeStep/(endTime-startTime);
	for (float t = startTime; t <= endTime; t+= timeStep)
	{		
		float r = velCurve->evaluate(t);
		integration += r*normStep;
	}

	float intSum = 0.f;
	if (integration == 0.f) // just produce a linear curve
	{
		for (float t = startTime; t <= endTime; t+= timeStep)
		{	
			float normT = (t-startTime)/(endTime-startTime);
			intSum += normT;
			interpCurve->addPt(normT,intSum);
		}
		return;
	}
	float intDiv = 1.f/integration;
	
	for (float t = startTime; t <= endTime; t+= timeStep)
	{	
		float normT = (t-startTime)/(endTime-startTime);
		intSum += velCurve->evaluate(t)*normStep*intDiv;
		interpCurve->addPt(normT,intSum);
	}
}

void MotionProfile::computeVelocity( SkMotion* m, float t, std::vector<float>& prevFrame, std::vector<float>& curFrame, ProfileCurveMap& outProfile )
{
	SkChannelArray& channels = m->channels();
	for (int i=0;i<channels.size();i++)
	{
		std::string chanName = channels.name(i);
		int chanType = channels.type(i);
		if (chanType != SkChannel::Quat)
			continue;
		
		int floatIdx = channels.float_position(i);
		SrQuat pq = SrQuat(&prevFrame[floatIdx]);
		SrQuat cq = SrQuat(&curFrame[floatIdx]);
		
		SrQuat diff = cq*pq.inverse();
		diff.normalize();
		ProfileCurveMap::iterator mi = outProfile.find(chanName);
		
		if (mi == outProfile.end())
			outProfile[chanName] = createProfileCurve();

		ProfileCurve* curve = outProfile[chanName];
		float velAngle = diff.angle();	
		curve->addPt(t,diff.angle());
	}
}

void MotionProfile::cleanUpProfileMap( ProfileCurveMap& curveMap )
{
	ProfileCurveMap::iterator mi;	
	for (mi  = curveMap.begin();
		 mi != curveMap.end();
		 mi++)
	{
		ProfileCurve* curve = mi->second;
		delete curve;
	}
	curveMap.clear();
}

ProfileCurve* MotionProfile::createProfileCurve()
{
	return new LinearProfileCurve();
}