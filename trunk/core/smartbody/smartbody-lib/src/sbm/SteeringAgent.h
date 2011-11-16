/*
 *  SteeringAgent.h - part of Motion Engine and SmartBody-lib
 *  Copyright (C) 2011  University of Southern California
 *
 *  SmartBody-lib is free software: you can redistribute it and/or
 *  modify it under the terms of the Lesser GNU General Public License
 *  as published by the Free Software Foundation, version 3 of the
 *  license.
 *
 *  SmartBody-lib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  Lesser GNU General Public License for more details.
 *
 *  You should have received a copy of the Lesser GNU General Public
 *  License along with SmartBody-lib.  If not, see:
 *      http://www.gnu.org/licenses/lgpl-3.0.txt
 *
 *  CONTRIBUTORS:
 *      Yuyu Xu, USC
 */


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
		void startParameterTesting();
		void updateSteerStateName();

	private:
		void normalizeAngle(float& angle);
		inline float cmToM(float v)		{return (v / 100.0f);}
		inline float mToCm(float v)		{return (v * 100.0f);}
		inline float degToRad(float v)	{return (v * float(M_PI) / 180.0f);}
		inline float radToDeg(float v)	{return (v * 180.0f / float(M_PI));}

		float evaluateBasicLoco(float x, float y, float z, float yaw);
		float evaluateProceduralLoco(float x, float y, float z, float yaw);
		float evaluateExampleLoco(float x, float y, float z, float yaw);

		float evaluateSteppingLoco(float x, float y, float z, float yaw);

		void parameterTesting();
		void cacheParameter(std::list<float>& sampleData, float data, int size);
		float getFilteredParameter(std::list<float>& sampleData);

	private:
		SteerLib::AgentInterface* agent;
		SbmCharacter* character;
		SbmCharacter* target;
		float dt;

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
		float speedThreshold;
		float angleSpeedThreshold;
		float distThreshold;	
		float distDownThreshold;
		float desiredSpeed;
		float facingAngle;
		float facingAngleThreshold;
		float acceleration;
		float scootAcceleration;
		float angleAcceleration;
		SrVec targetLoc;
		bool stepAdjust;

		float newSpeed;
		//----------------------------
		// WJ added start
		// basic param
		Util::Vector forward;
		Util::Vector rightSide;
		float currentSpeed;
		Util::Vector velocity;
		// WJ added end
		//----------------------------

		// stepping param
		float stepTargetX;
		float stepTargetZ;
		bool steppingMode;

		// parameter testing
		float paramTestDur;
		float paramTestStartTime;
		bool paramTestFlag;
		float paramTestAngle;
		float paramTestDistance;
		float prevX;
		float prevZ;
		float prevYaw;

		// low pass filter
		int speedWindowSize;
		int angleWindowSize;
		int scootWindowSize;
		std::list<float> speedCache;
		std::list<float> angleCache;
		std::list<float> scootCache;

		// heading over control to parameterized animation engine
		bool inControl;

		std::string stepStateName;
		std::string locomotionName;
		std::string startingLName;
		std::string startingRName;
		std::string idleTurnName;
};

#endif