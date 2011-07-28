#pragma once
#include "me_ct_motion_example.hpp"
#include <sbm/sr_linear_curve.h>
#include <sbm/sr_spline_curve.h>

class ProfileCurve
{
public:
	float startTime, endTime;
	ProfileCurve() {}
	~ProfileCurve() {}
public:
	virtual void init() = 0;
	virtual void addPt(float t, float val) = 0;
	virtual float evaluate(float t) = 0;		
};

class LinearProfileCurve : public ProfileCurve
{
protected:
	srLinearCurve curve;	
public:
	LinearProfileCurve() {}
	~LinearProfileCurve() {}
public:
	virtual void init();
	virtual void addPt(float t, float val);
	virtual float evaluate(float t);	
};

typedef std::map<std::string,ProfileCurve*> ProfileCurveMap;

class MotionProfile
{
public:
	enum { PROFILE_VELOCITY = 0, NUM_PROFILE_TYPES };
public:
	SkMotion* motion;			
	ProfileCurveMap velocityProfile; 
	ProfileCurveMap interpolationProfile;
public:
	MotionProfile(SkMotion* m);
	virtual ~MotionProfile(void);	
	void buildVelocityProfile(float startTime, float endTime, float timeStep);
	void buildInterpolationProfile(float startTime, float endTime, float timeStep);
protected:
	void computeVelocity(SkMotion* m, float t, std::vector<float>& prevFrame, std::vector<float>& curFrame, ProfileCurveMap& outProfile);
	void createNormalizeInterpolationCurve(float startTime, float endTime, float timeStep, ProfileCurve* velCurve, ProfileCurve* interpCurve);
	static void cleanUpProfileMap(ProfileCurveMap& curveMap);
	ProfileCurve* createProfileCurve();
};
