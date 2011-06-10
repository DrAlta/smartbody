#ifndef _STEERINGAGENT_H_
#define _STEERINGAGENT_H_

#include <sbm/sbm_character.hpp>
#include <sbm/SteerSuiteEngineDriver.h>
#include <sbm/sr_linear_curve.h>

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

	private:
		SteerLib::AgentInterface* agent;
		SbmCharacter* character;
		SbmCharacter* target;
		srLinearCurve* scootCurve;

	public:
		float scootAccel;
		float locoSpdGain;
		float paLocoAngleGain;
		float scootThreshold;	
		float paLocoScootGain;
		float locoScootGain;
		float distThreshold;	// centimeter
		float transition;
		float desiredSpeed;
		float facingAngle;
		float facingAngleThreshold;
		float acceleration;
		float scootAcceleration;
		float angleAcceleration;
		bool facingAdjustPhase;
};

#endif