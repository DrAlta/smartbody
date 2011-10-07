/*
 *  SteeringAgent.cpp - part of Motion Engine and SmartBody-lib
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


#include "SteeringAgent.h"
#include <sbm/mcontrol_util.h>
#include <sbm/me_ct_param_animation_data.h>
#define DebugInfo 0
#define FastStart 0

SteeringAgent::SteeringAgent(SbmCharacter* c) : character(c)
{
	agent = NULL;
	target = NULL;
	dt = 1.0f / 60.0f;
	// There parameters are ad-hoc
	basicLocoAngleGain = 2.0f;
	basicLocoScootGain = 10.0f;

	locoSpdGain = 70.0f;
	locoScootGain = 2.0f;

	paLocoAngleGain = 2.0f;
	paLocoScootGain = 9.0f;

	scootThreshold = 0.1f;	
	speedThreshold = 0.1f;
	angleSpeedThreshold = 10.0f;
	distThreshold = 180.0f;			// exposed, unit: centimeter
	distDownThreshold = 40.0f;

	desiredSpeed = 1.0f;			// exposed, unit: meter/sec
	facingAngle = -200.0f;			// exposed, unit: deg
	facingAngleThreshold = 10;
	acceleration = 2.0f;			// exposed, unit: meter/s^2
	scootAcceleration = 200.0f;		// exposed, unit: unknown
	angleAcceleration = 400.0f;		// exposed, unit: unknown
	stepAdjust = false;

	forward = Util::Vector(-1.0f, 0.0f, 0.0f);
	rightSide = rightSideInXZPlane(forward);
	currentSpeed = 1.0f;
	velocity = forward * currentSpeed;

	stepTargetX = 0.0f;
	stepTargetZ = 0.0f;
	steppingMode = false;
	
	paramTestDur = 2.0f;
	paramTestStartTime = 0.0f;
	paramTestFlag = false;
	paramTestAngle = 0.0f;
	paramTestDistance = 0.0f;
	prevX = 0.0f;
	prevZ = 0.0f;
	prevYaw = 0.0f;

	speedWindowSize = 10;
	angleWindowSize = 3;
	scootWindowSize = 3;

	inControl = true;
}

SteeringAgent::~SteeringAgent()
{
}

void SteeringAgent::evaluate()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	//---get current world offset position
	float x, y, z;
	float yaw, pitch, roll;
	character->get_world_offset(x, y, z, yaw, pitch, roll);

	// parameter testing
	if (paramTestFlag)
		parameterTesting();

	// TODO: better place for below code (step approach the target)
	if (steppingMode)
		evaluateSteppingLoco(x, y, z, yaw);

	if (!agent)
		return;
	PPRAgent* pprAgent = dynamic_cast<PPRAgent*>(agent);
	if (!pprAgent)
		return;

	const std::queue<SteerLib::AgentGoalInfo>& goalQueue = pprAgent->getLandmarkQueue();
	int numGoals = goalQueue.size();
	if (numGoals == 0) {
		newSpeed = 0.0f;
	}
	// Update Steering Engine (position, orientation, scalar speed)
	Util::Point newPosition(x / 100.0f, 0.0f, z / 100.0f);
	Util::Vector newOrientation = Util::rotateInXZPlane(Util::Vector(0.0f, 0.0f, 1.0f), yaw * float(M_PI) / 180.0f);
	pprAgent->updateAgentState(newPosition, newOrientation, newSpeed);
	pprAgent->updateAI((float)mcu.time, dt, (unsigned int)(mcu.time / dt));

	// Prepare Data
	//---if there is a target, update the goal
	if (target)
	{
		float x1, y1, z1;
		float yaw1, pitch1, roll1;
		target->get_world_offset(x1, y1, z1, yaw1, pitch1, roll1);
		float dist = sqrt((x - x1) * (x - x1) + (y - y1) * (y - y1) + (z - z1) * (z - z1));
		if (dist > distThreshold)
		{
			agent->clearGoals();
			SteerLib::AgentGoalInfo goal;
			goal.desiredSpeed = desiredSpeed;
			goal.goalType = SteerLib::GOAL_TYPE_SEEK_STATIC_TARGET;
			goal.targetIsRandom = false;
			goal.targetLocation = Util::Point(cmToM(x1), 0.0f, cmToM(z1 - 100.0f));
			agent->addGoal(goal);
		}
	}
	character->_lastReachStatus = character->_reachTarget;
	character->_reachTarget = false;

	// Evaluate
	//float newSpeed = desiredSpeed;
	// Meat Hook Locomotion Evaluation
	if (mcu.locomotion_type == mcu.Basic)
	{
		if (!character->basic_locomotion_ct)
			return;
		newSpeed = evaluateBasicLoco(x, y, z, yaw);
	}
	// Procedural Locomotion Evaluation
	if (mcu.locomotion_type == mcu.Procedural)
	{
		if (!character->locomotion_ct)
			return;
		if (!character->locomotion_ct->is_enabled())
			return;
		evaluateProceduralLoco(x, y, z, yaw);
	}

	// Example-Based Locomotion Evaluation
	if (mcu.locomotion_type == mcu.Example)
	{
		if (!character->param_animation_ct)
			return;
		newSpeed = evaluateExampleLoco(x, y, z, yaw);
	}

	// Event Handler	
	if (!character->_lastReachStatus && character->_reachTarget)
	{		
		std::string eventType = "locomotion";		
		MotionEvent motionEvent;
		motionEvent.setType(eventType);			
		std::string param = std::string(character->getName()) + " success";
		motionEvent.setParameters(param);
		EventManager* manager = EventManager::getEventManager();		
		manager->handleEvent(&motionEvent, mcu.time);
	}
	character->_numSteeringGoal = goalQueue.size();
}

void SteeringAgent::setAgent(SteerLib::AgentInterface* a)
{
	agent = a;
}

SteerLib::AgentInterface* SteeringAgent::getAgent()
{
	return agent;
}

void SteeringAgent::setTargetAgent(SbmCharacter* tChar)
{
	target = tChar;
}

SbmCharacter* SteeringAgent::getTargetAgent()
{
	return target;
}

void SteeringAgent::normalizeAngle(float& angle)
{
	while (angle > 180.0f)
		angle -= 360.0f;
	while (angle < -180.0f)
		angle += 360.0f;	
}

/*
	Notes:
	- The proximity is decided by Steering Suite
	- Facing not supported
*/
float SteeringAgent::evaluateBasicLoco(float x, float y, float z, float yaw)
{
 	PPRAgent* pprAgent = dynamic_cast<PPRAgent*>(agent);
	const std::queue<SteerLib::AgentGoalInfo>& goalQueue = pprAgent->getLandmarkQueue();
	const SteerLib::SteeringCommand & steeringCommand = pprAgent->getSteeringCommand();


	//--------------------------------------------------------
	// WJ added start
	// TODO: define/initialize these vars properly:
	Util::Vector totalSteeringForce;
	Util::Vector newForward;

	//
	// choose the new orientation of the agent
	//
	if (!steeringCommand.aimForTargetDirection) {
		// simple turning case "turn left" or "turn right"
		newForward = forward + PED_MAX_TURNING_RATE * steeringCommand.turningAmount * rightSide;
	}
	else {
		// turn to face "targetDirection" - magnitude of targetDirection doesn't matter
		float initialDot = dot(steeringCommand.targetDirection, rightSide);
		float turningRate = (initialDot > 0.0f) ? PED_MAX_TURNING_RATE : -PED_MAX_TURNING_RATE;  // positive rate is right-turn
		newForward = forward + turningRate * fabsf(steeringCommand.turningAmount) * rightSide;
		float newDot = dot(steeringCommand.targetDirection, rightSideInXZPlane(newForward)); // dot with the new side vector
		if (initialDot*newDot <= 0.0f) {
			// if the two dot products are different signs, that means we turned too much, so just set the new forward to the goal vector.
			// NOTE that above condition is less than **OR EQUALS TO** - that is because initialDot will be zero when the agent is 
			// pointing already in the exact correct direction.  If we used strictly less than, the pedestrian oscillates between a 
			// small offset direction and the actual target direction.

			//
			// TODO: known bug here: if agent is facing exactly opposite its goal, it completely flips around because of this condition.
			//   ironically, because of the equals sign above...
			//   proper solution is to add extra conditions that verify the original direction of forward was not opposite of targetDirection.
			//
			// WJ: need to change here
			newForward = Util::Vector(steeringCommand.targetDirection.x, 0.0f, steeringCommand.targetDirection.z);

		}
	}

	//
	// set the orientation
	//
	newForward = normalize(newForward);
	forward = newForward;
	rightSide = rightSideInXZPlane(newForward);

	// This next line is specific to command-based steering, but is not physically based.
	// everything else in command-based steering, however, is physcially based.
	velocity = newForward * currentSpeed;

	//
	// choose the force of the agent.  In command-based mode, the force is always aligned 
	// with the agent's forward facing direction, so we can use scalars until we add 
	// side-to-side scoot at the end.
	//
	assert(fabsf(steeringCommand.acceleration) <= 1.0f); // -1.0f <= acceleration <= 1.0f;
	if (!steeringCommand.aimForTargetSpeed) {
		// simple "speed up" or "slow down"
		totalSteeringForce = PED_MAX_FORCE * steeringCommand.acceleration * forward;
	}
	else {
		// accelerate towards a target speed
		// do it the naive greedy way;
		//
		// the most force you can apply without making velocity direction flip:
		// (force / mass) * time-step = delta-speed
		// if delta-speed == -speed
		// force * mass * time-step = -speed
		//
		float maxBackwardsForce = (-PED_BRAKING_RATE * fabsf(currentSpeed) * 60.0f/* * _mass / _dt*/);
		float scalarForce = (steeringCommand.targetSpeed - currentSpeed) * 8.0f; // crudely trying to make accelerations quicker...
		if (scalarForce > PED_MAX_FORCE) scalarForce = PED_MAX_FORCE;
		if (scalarForce < maxBackwardsForce) scalarForce = maxBackwardsForce;
		totalSteeringForce = scalarForce * forward; // forward is a unit vector, normalized during turning just above.
	}

	// TODO: should we clamp scoot?
	// add the side-to-side motion to the planned steering force.
	totalSteeringForce = totalSteeringForce + PED_SCOOT_RATE * steeringCommand.scoot * rightSide;

	// WJ added end
	//---------------------------------------------------------------------------


	float angleGlobal = radToDeg(atan2(steeringCommand.targetDirection.x, steeringCommand.targetDirection.z));
	normalizeAngle(angleGlobal);
	normalizeAngle(yaw);
	float angleDiff = angleGlobal - yaw;
	normalizeAngle(angleDiff);

	float newSpeed = desiredSpeed;

	int numGoals = goalQueue.size();
	if (numGoals == 0)
	{
		character->_reachTarget = true;
		character->basic_locomotion_ct->setValid(false);
		character->basic_locomotion_ct->setMovingSpd(0.0f);
		character->basic_locomotion_ct->setTurningSpd(0.0f);
		character->basic_locomotion_ct->setScootSpd(0.0f);
		newSpeed = 0.0f;
	}
	else
	{
		character->basic_locomotion_ct->setValid(true);
		float curSpeed = cmToM(character->basic_locomotion_ct->getMovingSpd());
		if (steeringCommand.aimForTargetSpeed)
		{
			if (curSpeed < steeringCommand.targetSpeed)
			{
				curSpeed += acceleration * dt;
				if (curSpeed > steeringCommand.targetSpeed)
					curSpeed = steeringCommand.targetSpeed;
			}
			else
			{
				curSpeed -= acceleration * dt;
				if (curSpeed < steeringCommand.targetSpeed)
					curSpeed = steeringCommand.targetSpeed;
			}
		}
		else
			curSpeed += acceleration * dt;
		newSpeed = curSpeed;
		curSpeed = mToCm(curSpeed);
		character->basic_locomotion_ct->setMovingSpd(curSpeed);	


		//-------------------------------
		// WJ added start

		// do euler step with force
		// compute acceleration and velocity by a simple Euler step
		const Util::Vector clippedForce = clamp(totalSteeringForce, PED_MAX_FORCE);
		Util::Vector acceleration = (clippedForce/* / _mass*/);
		velocity = velocity + (dt*acceleration);
		velocity = clamp(velocity, PED_MAX_SPEED);  // clamp _velocity to the max speed
		currentSpeed = velocity.length();
		forward = Util::normalize(velocity);
		angleGlobal = radToDeg(atan2(forward.x, forward.z));
		normalizeAngle(angleGlobal);

		curSpeed = mToCm(currentSpeed);
		newSpeed = currentSpeed;
		character->basic_locomotion_ct->setMovingSpd(curSpeed);
		character->basic_locomotion_ct->setDesiredHeading(angleGlobal); // affective setting

		// WJ added end
		//------------------------------------


		character->basic_locomotion_ct->setTurningSpd(angleDiff * basicLocoAngleGain);
		float curScoot = character->basic_locomotion_ct->getScootSpd() / basicLocoScootGain;
		if (steeringCommand.scoot != 0.0)
		{
			if (curScoot < steeringCommand.scoot)
				curScoot += scootAcceleration * dt;
			else
				curScoot -= scootAcceleration * dt;	
		}
		else
		{
			if (fabs(curScoot) < scootThreshold)
				curScoot = 0.0f;
			else
			{
				if (curScoot > 0.0f)
				{
					curScoot -= scootAcceleration * dt;
					if (curScoot < 0.0)	curScoot = 0.0;
				}
				else
				{
					curScoot += scootAcceleration * dt;
					if (curScoot > 0.0)	curScoot = 0.0;
				}
			}
		}
		character->basic_locomotion_ct->setScootSpd(curScoot * basicLocoScootGain);
	}
	return newSpeed;
}


/*
	Notes:
	- The proximity is decided by Steering Suite
	- bml "manner" tag is not supported
*/
float SteeringAgent::evaluateProceduralLoco(float x, float y, float z, float yaw)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	PPRAgent* pprAgent = dynamic_cast<PPRAgent*>(agent);
	const std::queue<SteerLib::AgentGoalInfo>& goalQueue = pprAgent->getLandmarkQueue();
	const SteerLib::SteeringCommand & steeringCommand = pprAgent->getSteeringCommand();

	float angleGlobal = radToDeg(atan2(steeringCommand.targetDirection.x, steeringCommand.targetDirection.z));
	normalizeAngle(angleGlobal);
	normalizeAngle(yaw);
	float angleDiff = angleGlobal - yaw;
	normalizeAngle(angleDiff);

	float speed = steeringCommand.targetSpeed * locoSpdGain;
	int numGoals = goalQueue.size();
	if (numGoals == 0)
	{				
		std::stringstream strstr;
		strstr << "test loco char ";
		strstr << character->getName();
		strstr << " stop";
		mcu.execute((char*)strstr.str().c_str());
		MeCtLocomotionNavigator* nav = character->locomotion_ct->get_navigator();
		if (fabs(facingAngle) <= 180)
		{
			float diff = facingAngle - yaw;
			normalizeAngle(diff);
			if (fabs(diff) > facingAngleThreshold)
			{
				float turn = 1.0f;
				if (diff > 0)
					turn *= -1.0f;
				std::stringstream strstr;
				strstr << "test loco char ";
				strstr << character->getName();
				strstr << " leftward spd 0.0 rps " << turn;
				mcu.execute((char*)strstr.str().c_str());	
			}
			else if (nav->limb_blending_factor < 0.1)
				character->_reachTarget = true;
		}
		else if (nav->limb_blending_factor < 0.1)
			character->_reachTarget = true;
	}
	else
	{
		if (fabs(steeringCommand.scoot) > scootThreshold)
		{
			float turn = -2.0f;
			if (steeringCommand.scoot > 0)
				turn *= -1.0f;
			std::stringstream strstr;
			strstr << "test loco char ";
			strstr << character->getName();
			strstr << " forward spd 0.0 rps " << turn;
			mcu.execute((char*)strstr.str().c_str());			
		}
		else
		{
			if (steeringCommand.aimForTargetDirection)
			{
				float spd = 1.0f;
				std::stringstream strstr;
				strstr << "test loco char ";
				strstr << character->getName();
				strstr << " forward spd " << speed << " rps ";
				if (angleDiff < 0)
				 strstr << spd << " ";
				else
				 strstr << -spd << " ";
				strstr << "angle " << degToRad(angleDiff);
				mcu.execute((char*)strstr.str().c_str());
			}
			else
			{
				std::stringstream strstr;
				strstr << "test loco char ";
				strstr << character->getName();
				strstr << " forward spd " << speed;
				mcu.execute((char*)strstr.str().c_str());
			}
		}
	}
	return desiredSpeed;
}

/*
	Notes:
	- Proximity controller by user
*/
float SteeringAgent::evaluateExampleLoco(float x, float y, float z, float yaw)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	PPRAgent* pprAgent = dynamic_cast<PPRAgent*>(agent);
	const std::queue<SteerLib::AgentGoalInfo>& goalQueue = pprAgent->getLandmarkQueue();
	const SteerLib::SteeringCommand & steeringCommand = pprAgent->getSteeringCommand();

	//*** IMPORTANT: use the example-based animation to update the steering agent
	forward = pprAgent->forward();
	rightSide = rightSideInXZPlane(forward);

	//--------------------------------------------------------
	// WJ added start
	// TODO: define/initialize these vars properly:
	float ped_max_turning_rate = PED_MAX_TURNING_RATE * 20.0f;

	//Util::Vector totalSteeringForce;
	Util::Vector newForward;

	//
	// choose the new orientation of the agent
	//
	if (!steeringCommand.aimForTargetDirection) {
		// simple turning case "turn left" or "turn right"
		newForward = forward + ped_max_turning_rate * steeringCommand.turningAmount * rightSide;
	}
	else {
		// turn to face "targetDirection" - magnitude of targetDirection doesn't matter
		float initialDot = dot(steeringCommand.targetDirection, rightSide);
		float turningRate = (initialDot > 0.0f) ? ped_max_turning_rate : -ped_max_turning_rate;  // positive rate is right-turn
		newForward = forward + turningRate * fabsf(steeringCommand.turningAmount) * rightSide;
		float newDot = dot(steeringCommand.targetDirection, rightSideInXZPlane(newForward)); // dot with the new side vector
		if (initialDot*newDot <= 0.0f) {
			// if the two dot products are different signs, that means we turned too much, so just set the new forward to the goal vector.
			// NOTE that above condition is less than **OR EQUALS TO** - that is because initialDot will be zero when the agent is 
			// pointing already in the exact correct direction.  If we used strictly less than, the pedestrian oscillates between a 
			// small offset direction and the actual target direction.

			//
			// TODO: known bug here: if agent is facing exactly opposite its goal, it completely flips around because of this condition.
			//   ironically, because of the equals sign above...
			//   proper solution is to add extra conditions that verify the original direction of forward was not opposite of targetDirection.
			//
			// WJ: need to change here
			newForward = Util::Vector(steeringCommand.targetDirection.x, 0.0f, steeringCommand.targetDirection.z);

		}
		//*** remove the gradually change of turning
		//*** let the animation system try its best to achieve this turning angle
		//newForward = Util::Vector(steeringCommand.targetDirection.x, 0.0f, steeringCommand.targetDirection.z);
	}

	//
	// set the orientation
	//
	newForward = normalize(newForward);
	forward = newForward;
	rightSide = rightSideInXZPlane(newForward);

	pprAgent->updateDesiredForward(forward);
	// WJ added end
	//---------------------------------------------------------------------------
#if 0
	bool reachTarget = false;
	float agentToTargetDist = 0.0f;
	SrVec agentToTargetVec;
	if (goalQueue.size() > 0)
	{
		stepAdjust = false; 
		targetLoc.x = mToCm(goalQueue.front().targetLocation.x);
		targetLoc.y = mToCm(goalQueue.front().targetLocation.y);
		targetLoc.z = mToCm(goalQueue.front().targetLocation.z);
		float dist = sqrt((x - mToCm(goalQueue.front().targetLocation.x)) * (x - mToCm(goalQueue.front().targetLocation.x)) + 
					  	//  (y - mToCm(goalQueue.front().targetLocation.y)) * (y - mToCm(goalQueue.front().targetLocation.y)) + 
						  (z - mToCm(goalQueue.front().targetLocation.z)) * (z - mToCm(goalQueue.front().targetLocation.z)));
		if (dist < distThreshold)
		{
			stepAdjust = true;
			character->steeringAgent->getAgent()->clearGoals();
		}
	}
	int numGoals = goalQueue.size();
	if (numGoals == 0 && character->_numSteeringGoal > 0)
		reachTarget = true;

	if (stepAdjust)
		if (!character->param_animation_ct->hasPAState("UtahStep"))
		{
			agentToTargetDist = sqrt((x - targetLoc.x) * (x - targetLoc.x) + 
				  					 //(y - targetLoc.y) * (y - targetLoc.y) + 
									 (z - targetLoc.z) * (z - targetLoc.z));
			agentToTargetVec.x = targetLoc.x - x;
			agentToTargetVec.y = targetLoc.y - y;
			agentToTargetVec.z = targetLoc.z - z;
		}

	PAStateData* curState =  character->param_animation_ct->getCurrentPAStateData();
	std::string curStateName = character->param_animation_ct->getCurrentStateName();
	std::string nextStateName = character->param_animation_ct->getNextStateName();
	//---end locomotion and macro control and step control
	if (reachTarget)
	{
		if (!character->param_animation_ct->hasPAState("UtahWalkToStop"))
		{
			if (curStateName == "UtahLocomotion" || nextStateName == "UtahLocomotion"
				|| nextStateName == "UtahStopToWalk" || nextStateName == "UtahStartingLeft" || nextStateName == "UtahStartingRight"
				)
			{
				PAStateData* state = mcu.lookUpPAState("UtahWalkToStop");
				character->param_animation_ct->schedule(state, false);
			}
			if (nextStateName == "" && (curStateName == "UtahStopToWalk" || curStateName == "UtahStartingLeft" || curStateName == "UtahStartingRight"))
			{
				std::cout << character->getName() << " schedule stop" << std::endl;
				PAStateData* state = mcu.lookUpPAState("UtahLocomotion");
				character->param_animation_ct->schedule(state, true);
				state = mcu.lookUpPAState("UtahWalkToStop");
				character->param_animation_ct->schedule(state, false);
			}
		}
	}
	else if (character->param_animation_ct->isIdle() && (agentToTargetDist > distDownThreshold))// || fabs(facingAngle) <= 180))
	{
		SrVec heading = SrVec(sin(degToRad(yaw)), 0, cos(degToRad(yaw)));
		float y = dot(agentToTargetVec, heading);
		SrVec verticalHeading = SrVec(sin(degToRad(yaw - 90)), 0, cos(degToRad(yaw - 90)));
		float x = dot(agentToTargetVec, verticalHeading);
/*		float z = 0.0f;
		if (fabs(facingAngle) <= 180)
		{
			float diff = yaw - facingAngle;
			normalizeAngle(diff);
			if (fabs(diff) > facingAngleThreshold)
				z = diff;
			else
				facingAngle = -200.0f;
		}
		PAStateData* stepState = mcu.lookUpPAState("UtahStep");
		stepState->paramManager->setWeight(x, y, z);
*/
		if (!character->param_animation_ct->hasPAState("UtahStep"))
		{
			PAStateData* stepState = mcu.lookUpPAState("UtahStep");
			stepState->paramManager->setWeight(x, y);
			std::stringstream command;
			command << "panim schedule char " << character->getName();			
			command << " state UtahStep loop false playnow false ";
			for (int i = 0; i < stepState->getNumMotions(); i++)
				command << stepState->weights[i] << " ";
			mcu.execute((char*) command.str().c_str());
		}
	}
	else if (character->param_animation_ct->isIdle() && fabs(facingAngle) <= 180)
	{
		float diff = facingAngle - yaw;
		normalizeAngle(diff);
		std::string playNow;
		if (fabs(diff) > facingAngleThreshold)
		{
			double w = 0;
			playNow = "false";
			if (diff <= -90)
			{
				w = (diff + 180) / 180;
				std::stringstream command1;
				command1 << "panim schedule char " << character->getName();
				command1 << " state UtahIdleTurnRight loop false playnow " << playNow << " 0 " << w << " " << 1 - w;
				mcu.execute((char*) command1.str().c_str());						
			}
			else if (diff >= 90)
			{
				w = (diff - 90) / 90;
				std::stringstream command1;
				command1 << "panim schedule char " << character->getName();
				command1 << " state UtahIdleTurnLeft loop false playnow " << playNow << " 0 " << 1 - w << " " << w;
				mcu.execute((char*) command1.str().c_str());												
			}
			else if (diff <= 0)
			{
				w = fabs(diff / 90);
				std::stringstream command1;
				command1 << "panim schedule char " << character->getName();
				command1 << " state UtahIdleTurnRight loop false playnow " << playNow << " " << 1 - w << " " << w << " 0 ";
				mcu.execute((char*) command1.str().c_str());
			}
			else if (diff >= 0)
			{
				w = diff / 90;
				std::stringstream command1;
				command1 << "panim schedule char " << character->getName();
				command1 << " state UtahIdleTurnLeft loop false playnow " << playNow << " " << 1 - w << " " << w << " 0 ";
				mcu.execute((char*) command1.str().c_str());	
			}
		}
		else
		{
			character->_reachTarget = true;
			facingAngle = -200;
		}
	}
	else if (character->param_animation_ct->isIdle())
	{
		character->_reachTarget = true;
		speedCache.clear();
		angleCache.clear();
		scootCache.clear();
	}

	if (character->param_animation_ct->isIdle() && (agentToTargetDist < distDownThreshold))
		stepAdjust = false;

	//---start locomotion
#if !FastStart	// starting with angle transition
	if (curState)
		if (curState->stateName == PseudoIdleState && numGoals != 0 && nextStateName == "")
//		if (character->_numSteeringGoal == 0 && numGoals != 0)
		{
			//float targetAngle = radToDeg(atan2(mToCm(goalQueue.front().targetLocation.x) - x, mToCm(goalQueue.front().targetLocation.z) - z));
			float targetAngle = radToDeg(atan2(mToCm(pprAgent->getStartTargetPosition().x) - x, mToCm(pprAgent->getStartTargetPosition().z) - z));
			normalizeAngle(targetAngle);
			float diff = targetAngle - yaw;
			normalizeAngle(diff);
			double w;
			if (diff > 0)
			{
				if (diff > 90)
				{
					w = (diff - 90) / 90;
					std::stringstream command;
					command << "panim schedule char " << character->getName();			
					command << " state UtahStartingLeft loop false playnow false " << " 0 " << 1 - w << " " << w;
					mcu.execute((char*) command.str().c_str());
				}
				else
				{
					w = diff / 90;
					std::stringstream command;
					command << "panim schedule char " << character->getName();					
					command << " state UtahStartingLeft loop false playnow false " << 1 - w << " " << w << " " << " 0 ";
					mcu.execute((char*) command.str().c_str());
				}
			}
			else
			{
				if (diff < -90)
				{
					w = (diff + 180) / 90;
					std::stringstream command;
					command << "panim schedule char " << character->getName();
					command << " state UtahStartingRight loop false playnow false " << " 0 " << w << " " << 1 - w;
					mcu.execute((char*) command.str().c_str());
				}
				else
				{
					w = -diff / 90;
					std::stringstream command;
					command << "panim schedule char " << character->getName();
					command << " state UtahStartingRight loop false playnow true " << 1 - w << " " << w << " 0 ";
					mcu.execute((char*) command.str().c_str());
				}				
			}
			PAStateData* locoState = mcu.lookUpPAState("UtahLocomotion");

			for (int i = 0; i < locoState->getNumMotions(); i++)
			{
				if (i == 0)
					locoState->weights[i] = 1.0;
				else
					locoState->weights[i] = 0.0;
			}
			std::stringstream command1;
			command1 << "panim schedule char " << character->getName();
			command1 << " state UtahLocomotion loop true playnow false";
			mcu.execute((char*) command1.str().c_str());
		}	
#else	// starting without transition
	if (curState)
		if (curState->stateName == PseudoIdleState && numGoals != 0 && nextStateName == "")
		{
			PAStateData* locoState = mcu.lookUpPAState("UtahLocomotion");
			locoState->paramManager->setWeight(0, 0, 0);
			std::stringstream command;
			command << "panim schedule char " << character->name;
			command << " state UtahLocomotion loop true playnow true";
			mcu.execute((char*) command.str().c_str());
		}
#endif
	//---Need a better way to handle the control between steering and Parameterized Animation Controller
	if (character->param_animation_ct->hasPAState("UtahJump"))
		inControl = false;
	else
		inControl = true;

	//---update locomotion
	float curSpeed = 0.0f;
	float curTurningAngle = 0.0f;
	float curScoot = 0.0f;
	if (curState)
		if (curState->stateName == "UtahLocomotion" && numGoals != 0)
		{
			curState->paramManager->getParameter(curSpeed, curTurningAngle, curScoot);
			float addOnScoot = steeringCommand.scoot * paLocoScootGain;
			if (steeringCommand.scoot != 0.0)
			{
				if (curScoot < addOnScoot)
					curScoot += scootAcceleration * dt;
				else
					curScoot -= scootAcceleration * dt;
			}
			else
			{
				if (fabs(curScoot) < scootThreshold)
					curScoot = 0.0f;
				else
				{
					if (curScoot > 0.0f)
					{
						curScoot -= scootAcceleration * dt;
						if (curScoot < 0.0)	curScoot = 0.0;
					}
					else
					{
						curScoot += scootAcceleration * dt;
						if (curScoot > 0.0)	curScoot = 0.0;
					}
				}
			}
			curSpeed = cmToM(curSpeed);
			if (steeringCommand.aimForTargetSpeed)
			{
				if (fabs(curSpeed - steeringCommand.targetSpeed) > speedThreshold)
				{
					if (curSpeed < steeringCommand.targetSpeed)
					{
						curSpeed += acceleration * dt;
						if (curSpeed > steeringCommand.targetSpeed)
							curSpeed = steeringCommand.targetSpeed;
					}
					else
					{
						curSpeed -= acceleration * dt;
						if (curSpeed < steeringCommand.targetSpeed)
							curSpeed = steeringCommand.targetSpeed;
					}
				}
			}
			else
				curSpeed += acceleration * dt;

			float angleGlobal = radToDeg(atan2(forward.x, forward.z));
			normalizeAngle(angleGlobal);
			normalizeAngle(yaw);
			float angleDiff = angleGlobal - yaw;
			normalizeAngle(angleDiff);

			// address the problem of obstacle avoiding failure during high speed movement
			//if (curSpeed > 200.0f)				
			//	paLocoAngleGain = 4.0f;
			//else if (curSpeed > 300.0f)			
			//	paLocoAngleGain = 7.0f;
			//else
			//	paLocoAngleGain = 2.5f;

			paLocoAngleGain = 2.0f;
			float addOnTurning = angleDiff * paLocoAngleGain;
			if (fabs(curTurningAngle - addOnTurning) > angleSpeedThreshold)
			{
				if (curTurningAngle < addOnTurning)
					curTurningAngle += angleAcceleration * dt;
				else if (curTurningAngle > addOnTurning)
					curTurningAngle -= angleAcceleration * dt;
			}
			// update locomotion state
			newSpeed = curSpeed;
			curSpeed = mToCm(curSpeed);

			//std::cout << curSpeed << " " << curTurningAngle << " " << curScoot << "   ";
			//cacheParameter(speedCache, curSpeed, speedWindowSize);
			//curSpeed = getFilteredParameter(speedCache);
			//cacheParameter(angleCache, curTurningAngle, angleWindowSize);
			//curTurningAngle = getFilteredParameter(angleCache);
			//cacheParameter(scootCache, curScoot, scootWindowSize);
			//curScoot = getFilteredParameter(scootCache);
			//std::cout << curSpeed << " " << curTurningAngle << " " << curScoot << std::endl;
			if (inControl)
			{
				curState->paramManager->setWeight(curSpeed, curTurningAngle, curScoot);
				character->param_animation_ct->updateWeights();
			}
		}
#else
	bool reachTarget = false;
	float agentToTargetDist = 0.0f;
	SrVec agentToTargetVec;
	float distToTarget = -1;
	if (goalQueue.size() > 0)
	{
		targetLoc.x = mToCm(goalQueue.front().targetLocation.x);
		targetLoc.y = mToCm(goalQueue.front().targetLocation.y);
		targetLoc.z = mToCm(goalQueue.front().targetLocation.z);
		distToTarget = sqrt((x - mToCm(goalQueue.front().targetLocation.x)) * (x - mToCm(goalQueue.front().targetLocation.x)) + 
						  (z - mToCm(goalQueue.front().targetLocation.z)) * (z - mToCm(goalQueue.front().targetLocation.z)));

		if (distToTarget < distDownThreshold)
			character->steeringAgent->getAgent()->clearGoals();
	}
	int numGoals = goalQueue.size();
	if (numGoals == 0)
	{
		reachTarget = true;
	}
	if (character->_numSteeringGoal == 0 && numGoals != 0 && distToTarget < distThreshold)
		stepAdjust = true;
	if (distToTarget > distThreshold)
		stepAdjust = false;

	float targetSpeed = steeringCommand.targetSpeed;
	float gain = 80;
	if (targetSpeed > 3)
		gain = 110;
	if (distToTarget < targetSpeed * gain)
		targetSpeed = distToTarget / gain;

	if (stepAdjust)
		if (!character->param_animation_ct->hasPAState("UtahStep"))
		{
			agentToTargetDist = distToTarget;
			agentToTargetVec.x = targetLoc.x - x;
			agentToTargetVec.y = targetLoc.y - y;
			agentToTargetVec.z = targetLoc.z - z;
		}

	PAStateData* curState =  character->param_animation_ct->getCurrentPAStateData();
	std::string curStateName = character->param_animation_ct->getCurrentStateName();
	std::string nextStateName = character->param_animation_ct->getNextStateName();

	//---If you are close enough to the target when starting locomotion, use step adjust
	if (character->param_animation_ct->isIdle() && (agentToTargetDist > distDownThreshold))
	{
		SrVec heading = SrVec(sin(degToRad(yaw)), 0, cos(degToRad(yaw)));
		float y = dot(agentToTargetVec, heading);
		SrVec verticalHeading = SrVec(sin(degToRad(yaw - 90)), 0, cos(degToRad(yaw - 90)));
		float x = dot(agentToTargetVec, verticalHeading);
		if (!character->param_animation_ct->hasPAState("UtahStep"))
		{
			PAStateData* stepState = mcu.lookUpPAState("UtahStep");
			stepState->paramManager->setWeight(x, y);
			std::stringstream command;
			command << "panim schedule char " << character->getName();			
			command << " state UtahStep loop false playnow false ";
			for (int i = 0; i < stepState->getNumMotions(); i++)
				command << stepState->weights[i] << " ";
			mcu.execute((char*) command.str().c_str());
		}		
		return 0;
	}

	//---start locomotion
	if (character->param_animation_ct->isIdle() && numGoals != 0 && nextStateName == "" && distToTarget > distDownThreshold)
	{
		float targetAngle = radToDeg(atan2(mToCm(pprAgent->getStartTargetPosition().x) - x, mToCm(pprAgent->getStartTargetPosition().z) - z));
		normalizeAngle(targetAngle);
		float diff = targetAngle - yaw;
		normalizeAngle(diff);
		double w;
		if (diff > 0)
		{
			if (diff > 90)
			{
				w = (diff - 90) / 90;
				std::stringstream command;
				command << "panim schedule char " << character->getName();			
				command << " state UtahStartingLeft loop false playnow false " << " 0 " << 1 - w << " " << w;
				mcu.execute((char*) command.str().c_str());
			}
			else
			{
				w = diff / 90;
				std::stringstream command;
				command << "panim schedule char " << character->getName();					
				command << " state UtahStartingLeft loop false playnow false " << 1 - w << " " << w << " " << " 0 ";
				mcu.execute((char*) command.str().c_str());
			}
		}
		else
		{
			if (diff < -90)
			{
				w = (diff + 180) / 90;
				std::stringstream command;
				command << "panim schedule char " << character->getName();
				command << " state UtahStartingRight loop false playnow false " << " 0 " << w << " " << 1 - w;
				mcu.execute((char*) command.str().c_str());
			}
			else
			{
				w = -diff / 90;
				std::stringstream command;
				command << "panim schedule char " << character->getName();
				command << " state UtahStartingRight loop false playnow true " << 1 - w << " " << w << " 0 ";
				mcu.execute((char*) command.str().c_str());
			}				
		}
		PAStateData* locoState = mcu.lookUpPAState("UtahLocomotion");

		for (int i = 0; i < locoState->getNumMotions(); i++)
		{
			if (i == 0)
				locoState->weights[i] = 1.0;
			else
				locoState->weights[i] = 0.0;
		}
		std::stringstream command1;
		command1 << "panim schedule char " << character->getName();
		command1 << " state UtahLocomotion loop true playnow false";
		mcu.execute((char*) command1.str().c_str());
		return 0;
	}	

	//---end locomotion
	if (character->_numSteeringGoal != 0 && numGoals == 0)
	{
		character->param_animation_ct->schedule(NULL, true, true);
		return 0;
	}

	//---If the facing angle is not correct, use idle turning
	if (character->param_animation_ct->isIdle() && fabs(facingAngle) <= 180)
	{
		float diff = facingAngle - yaw;
		normalizeAngle(diff);
		std::string playNow;
		if (fabs(diff) > facingAngleThreshold)
		{
			double w = 0;
			playNow = "false";
			if (diff <= -90 && !character->param_animation_ct->hasPAState("UtahIdleTurnRight"))
			{
				w = (diff + 180) / 180;
				std::stringstream command1;
				command1 << "panim schedule char " << character->getName();
				command1 << " state UtahIdleTurnRight loop false playnow " << playNow << " 0 " << w << " " << 1 - w;
				mcu.execute((char*) command1.str().c_str());						
			}
			else if (diff >= 90 && !character->param_animation_ct->hasPAState("UtahIdleTurnLeft"))
			{
				w = (diff - 90) / 90;
				std::stringstream command1;
				command1 << "panim schedule char " << character->getName();
				command1 << " state UtahIdleTurnLeft loop false playnow " << playNow << " 0 " << 1 - w << " " << w;
				mcu.execute((char*) command1.str().c_str());												
			}
			else if (diff <= 0 && !character->param_animation_ct->hasPAState("UtahIdleTurnRight"))
			{
				w = fabs(diff / 90);
				std::stringstream command1;
				command1 << "panim schedule char " << character->getName();
				command1 << " state UtahIdleTurnRight loop false playnow " << playNow << " " << 1 - w << " " << w << " 0 ";
				mcu.execute((char*) command1.str().c_str());
			}
			else if (diff >= 0 && !character->param_animation_ct->hasPAState("UtahIdleTurnLeft"))
			{
				w = diff / 90;
				std::stringstream command1;
				command1 << "panim schedule char " << character->getName();
				command1 << " state UtahIdleTurnLeft loop false playnow " << playNow << " " << 1 - w << " " << w << " 0 ";
				mcu.execute((char*) command1.str().c_str());	
			}
		}
		else
		{
			facingAngle = -200;
		}
		return 0;
	}


	//---Need a better way to handle the control between steering and Parameterized Animation Controller
	if (character->param_animation_ct->hasPAState("UtahJump"))
		inControl = false;
	else
		inControl = true;

	//---update locomotion
	float curSpeed = 0.0f;
	float curTurningAngle = 0.0f;
	float curScoot = 0.0f;
	if (curStateName == "UtahLocomotion" && numGoals != 0)
	{
		curState->paramManager->getParameter(curSpeed, curTurningAngle, curScoot);
		float addOnScoot = steeringCommand.scoot * paLocoScootGain;
		if (steeringCommand.scoot != 0.0)
		{
			if (curScoot < addOnScoot)
				curScoot += scootAcceleration * dt;
			else
				curScoot -= scootAcceleration * dt;
		}
		else
		{
			if (fabs(curScoot) < scootThreshold)
				curScoot = 0.0f;
			else
			{
				if (curScoot > 0.0f)
				{
					curScoot -= scootAcceleration * dt;
					if (curScoot < 0.0)	curScoot = 0.0;
				}
				else
				{
					curScoot += scootAcceleration * dt;
					if (curScoot > 0.0)	curScoot = 0.0;
				}
			}
		}
		curSpeed = cmToM(curSpeed);
		if (steeringCommand.aimForTargetSpeed)
		{
			if (fabs(curSpeed - targetSpeed) > speedThreshold)
			{
				if (curSpeed < targetSpeed)
				{
					curSpeed += acceleration * dt;
					if (curSpeed > targetSpeed)
						curSpeed = targetSpeed;
				}
				else
				{
					curSpeed -= acceleration * dt;
					if (curSpeed < targetSpeed)
						curSpeed = targetSpeed;
				}
			}
		}
		else
			curSpeed += acceleration * dt;

		float angleGlobal = radToDeg(atan2(forward.x, forward.z));
		normalizeAngle(angleGlobal);
		normalizeAngle(yaw);
		float angleDiff = angleGlobal - yaw;
		normalizeAngle(angleDiff);

		paLocoAngleGain = 2.0f;
		float addOnTurning = angleDiff * paLocoAngleGain;
		if (fabs(curTurningAngle - addOnTurning) > angleSpeedThreshold)
		{
			if (curTurningAngle < addOnTurning)
				curTurningAngle += angleAcceleration * dt;
			else if (curTurningAngle > addOnTurning)
				curTurningAngle -= angleAcceleration * dt;
		}
		// update locomotion state
		newSpeed = curSpeed;
		curSpeed = mToCm(curSpeed);

		if (inControl)
		{
			curState->paramManager->setWeight(curSpeed, curTurningAngle, curScoot);
			character->param_animation_ct->updateWeights();
		}
	}
#endif
	return newSpeed;
}

float SteeringAgent::evaluateSteppingLoco(float x, float y, float z, float yaw)
{
	if (!character->param_animation_ct)
		return .0f;
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	float dist = sqrt((x - stepTargetX) * (x - stepTargetX) + (z - stepTargetZ) * (z - stepTargetZ));	
	SrVec agentToTargetVec;
	agentToTargetVec.x = stepTargetX - x;
	agentToTargetVec.y = 0.0f;
	agentToTargetVec.z = stepTargetZ - z;
	if (character->param_animation_ct->isIdle() && (dist > 20.0f))
	{
		SrVec heading = SrVec(sin(degToRad(yaw)), 0, cos(degToRad(yaw)));
		float offsety = dot(agentToTargetVec, heading);
		SrVec verticalHeading = SrVec(sin(degToRad(yaw - 90)), 0, cos(degToRad(yaw - 90)));
		float offsetx = dot(agentToTargetVec, verticalHeading);
		if (!character->param_animation_ct->hasPAState("UtahStep"))
		{
			PAStateData* stepState = mcu.lookUpPAState("UtahStep");
			stepState->paramManager->setWeight(offsetx, offsety);
			std::stringstream command;
			command << "panim schedule char " << character->getName();			
			command << " state UtahStep loop false playnow false ";
			for (int i = 0; i < stepState->getNumMotions(); i++)
				command << stepState->weights[i] << " ";
			mcu.execute((char*) command.str().c_str());
		}	
	}
	if (dist < 20.0f)
		steppingMode = false;

	return 0.0f;
}

void SteeringAgent::startParameterTesting()
{
	LOG("Parameter Testing Start...");
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	paramTestStartTime = (float)mcu.time;
	paramTestFlag = true;
	paramTestAngle = 0.0f;
	paramTestDistance = 0.0f;
	float y, pitch, roll;
	character->get_world_offset(prevX, y, prevZ, prevYaw, pitch, roll);
	normalizeAngle(prevYaw);
}

void SteeringAgent::parameterTesting()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if ((mcu.time - paramTestStartTime) > paramTestDur)
	{
		paramTestFlag = false;
		paramTestAngle *= (float(M_PI) / 1.8f);
		float paramTestVelocity = paramTestDistance / paramTestDur;
		float paramTestAngleVelocity = paramTestAngle / paramTestDur;
		LOG("Parameter Testing Result");
		LOG("Duration: %f", paramTestDur);
		LOG("Velocity: %f", paramTestVelocity);
		LOG("Angle Velocity: %f", paramTestAngleVelocity);
		return;
	}
	// current location
	float x, y, z;
	float yaw, pitch, roll;
	character->get_world_offset(x, y, z, yaw, pitch, roll);
	normalizeAngle(yaw);

	paramTestDistance += sqrt((x - prevX) * (x - prevX) + (z - prevZ) * (z - prevZ));
	if (fabs(yaw - prevYaw) < 300) 
		paramTestAngle += (yaw - prevYaw);

	prevX = x;
	prevZ = z;
	prevYaw = yaw;
	normalizeAngle(prevYaw);
}

void SteeringAgent::cacheParameter(std::list<float>& sampleData, float data, int size)
{
	sampleData.push_back(data);
	while (sampleData.size() > (size_t)size)
		sampleData.pop_front();
}

float SteeringAgent::getFilteredParameter(std::list<float>& sampleData)
{
	float ret = 0.0f;
	std::list<float>::iterator iter = sampleData.begin();
	for (; iter != sampleData.end(); iter++)
		ret += *iter;
	ret /= float(sampleData.size());
	return ret;
}