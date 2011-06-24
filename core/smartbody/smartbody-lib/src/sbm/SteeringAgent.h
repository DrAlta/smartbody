#ifndef _STEERINGAGENT_H_
#define _STEERINGAGENT_H_

#include <sbm/sbm_character.hpp>
#include <sbm/SteerSuiteEngineDriver.h>

class SteeringAgent
{
	public:
		SteeringAgent(SbmCharacter* c);
		~SteeringAgent();

		void evaluate();
		void setAgent(SteerLib::AgentInterface* a);
		SteerLib::AgentInterface* getAgent();
		void setTargetAgent(SbmCharacter* tChar);
		SbmCharacter* getTargetAgent();

	private:
		void normalizeAngle(float& angle);
		inline float cmToM(float v)		{return (v / 100.0f);}
		inline float mToCm(float v)		{return (v * 100.0f);}
		inline float degToRad(float v)	{return (v * float(M_PI) / 180.0f);}
		inline float radToDeg(float v)	{return (v * 180.0f / float(M_PI));}

	private:
		SteerLib::AgentInterface* agent;
		SbmCharacter* character;
		SbmCharacter* target;

	public:
		// basic param
		float basicLocoAngleGain;
		float basicLocoScootGain;
		// procedural param
		float locoSpdGain;
		float locoScootGain;
		// example param
		float paLocoAngleGain;	
		float paLocoScootGain;
		// global param
		float scootThreshold;
		float distThreshold;	
		float desiredSpeed;
		float facingAngle;
		float facingAngleThreshold;
		float acceleration;
		float scootAcceleration;
		float angleAcceleration;
		bool facingAdjustPhase;
};

#endif