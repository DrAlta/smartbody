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
#include <sbm/SBScene.h>
#include <sbm/SBPythonClass.h>
#include <sbm/me_ct_param_animation_data.h>
#define DebugInfo 0
#define FastStart 1

SteeringAgent::SteeringAgent(SbmCharacter* c) : character(c)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	agent = NULL;
	target = NULL;
	dt = 1.0f / 60.0f;

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

	inControl = true;

	fastInitial = false;
	smoothing = false;

	lastMessage = "";
	numMessageRepeats = 0;

	updateSteerStateName();

	// add the steering attributes to the character
	addSteeringAttributes();

	setSteerParamsDirty(true);
	initSteerParams();
}

SteeringAgent::~SteeringAgent()
{
}

void SteeringAgent::addSteeringAttributes()
{
	if (!character)
		return;
	
	if (!character->hasAttribute("steering.basicLocoAngleGain"))
		character->createDoubleAttribute("steering.basicLocoAngleGain", 2.0f, true, "steering", 200, false, false, false, ""); 
	
	if (!character->hasAttribute("steering.basicLocoScootGain"))
		character->createDoubleAttribute("steering.basicLocoScootGain", 10.0f, true, "steering", 210, false, false, false, ""); 

	if (!character->hasAttribute("steering.locoSpdGain"))
		character->createDoubleAttribute("steering.locoSpdGain", 70.0f, true, "steering", 220, false, false, false, ""); 

	if (!character->hasAttribute("steering.locoScootGain"))
		character->createDoubleAttribute("steering.locoScootGain", 2.0f, true, "steering", 230, false, false, false, ""); 

	if (!character->hasAttribute("steering.paLocoAngleGain"))
		character->createDoubleAttribute("steering.paLocoAngleGain", 2.0f, true, "steering", 240, false, false, false, ""); 

	if (!character->hasAttribute("steering.paLocoScootGain"))
		character->createDoubleAttribute("steering.paLocoScootGain", 9.0f, true, "steering", 250, false, false, false, ""); 
	
	if (!character->hasAttribute("steering.scootThreshold"))
		character->createDoubleAttribute("steering.scootThreshold", .1f, true, "steering", 260, false, false, false, ""); 
	
	if (!character->hasAttribute("steering.speedThreshold"))
		character->createDoubleAttribute("steering.speedThreshold", .1f, true, "steering", 270, false, false, false, ""); 
	
	if (!character->hasAttribute("steering.angleSpeedThreshold"))
		character->createDoubleAttribute("steering.angleSpeedThreshold", 10.f, true, "steering", 280, false, false, false, ""); 

	if (!character->hasAttribute("steering.distThreshold"))
		character->createDoubleAttribute("steering.distThreshold", 1.80f, true, "steering", 290, false, false, false, ""); 

	if (!character->hasAttribute("steering.distDownThreshold"))
		character->createDoubleAttribute("steering.distDownThreshold", .3f, true, "steering", 300, false, false, false, ""); 

	if (!character->hasAttribute("steering.brakingGain"))
		character->createDoubleAttribute("steering.brakingGain", 1.2f, true, "steering", 310, false, false, false, ""); 
	
	if (!character->hasAttribute("steering.desiredSpeed"))
		character->createDoubleAttribute("steering.desiredSpeed", 1.0f, true, "steering", 310, false, false, false, ""); 	

	if (!character->hasAttribute("steering.pathAngleAcc"))
		character->createDoubleAttribute("steering.pathAngleAcc", 1000.f, true, "steering", 310, false, false, false, "");

	if (!character->hasAttribute("steering.pathMaxSpeed"))
		character->createDoubleAttribute("steering.pathMaxSpeed", 1.5f, true, "steering", 310, false, false, false, "");	

	if (!character->hasAttribute("steering.pathMinSpeed"))
		character->createDoubleAttribute("steering.pathMinSpeed", 0.6f, true, "steering", 310, false, false, false, "");
	
	if (!character->hasAttribute("steering.pathStartStep"))
		character->createBoolAttribute("steering.pathStartStep", true, true, "steering", 330, false, false, false, ""); 
	
	if (!character->hasAttribute("steering.facingAngleThreshold"))
		character->createDoubleAttribute("steering.facingAngleThreshold", 10.f, true, "steering", 340, false, false, false, ""); 

	if (!character->hasAttribute("steering.acceleration"))
		character->createDoubleAttribute("steering.acceleration", 2.f, true, "steering", 350, false, false, false, ""); 

	if (!character->hasAttribute("steering.scootAcceleration"))
		character->createDoubleAttribute("steering.scootAcceleration", 200.f, true, "steering", 360, false, false, false, ""); 
	
	if (!character->hasAttribute("steering.angleAcceleration"))
		character->createDoubleAttribute("steering.angleAcceleration", 400.f, true, "steering", 370, false, false, false, ""); 

	if (!character->hasAttribute("steering.stepAdjust"))
		character->createBoolAttribute("steering.stepAdjust", false, true, "steering", 380, false, false, false, ""); 

	if (!character->hasAttribute("steering.smoothing"))
		character->createBoolAttribute("steering.smoothing", true, true, "steering", 390, false, false, false, ""); 

	if (!character->hasAttribute("steering.pathFollowingMode"))
		character->createBoolAttribute("steering.pathFollowingMode", false, true, "steering", 390, false, false, false, "");

	if (!character->hasAttribute("steering.speedWindowSize"))
		character->createIntAttribute("steering.speedWindowSize", 10, true, "steering", 400, false, false, false, ""); 

	if (!character->hasAttribute("steering.angleWindowSize"))
		character->createIntAttribute("steering.angleWindowSize", 3, true, "steering", 410, false, false, false, ""); 

	if (!character->hasAttribute("steering.scootWindowSize"))
		character->createIntAttribute("steering.scootWindowSize", 3, true, "steering", 420, false, false, false, ""); 
	
	if (!character->hasAttribute("steering.pedMaxTurningRateMultiplier"))
		character->createDoubleAttribute("steering.pedMaxTurningRateMultiplier", 20.f, true, "steering", 430, false, false, false, ""); 

	setSteerParamsDirty(false);
}

void SteeringAgent::initSteerParams()
{
	if (character && character->hasAttribute("steering.basicLocoAngleGain"))
		basicLocoAngleGain = (float) character->getDoubleAttribute("steering.basicLocoAngleGain");
	else
		basicLocoAngleGain = 2.0f;

	if (character && character->hasAttribute("steering.basicLocoScootGain"))
		basicLocoScootGain = (float) character->getDoubleAttribute("steering.basicLocoScootGain");
	else
		basicLocoScootGain = 10.0f;

	if (character && character->hasAttribute("steering.locoSpdGain"))
		locoSpdGain = (float) character->getDoubleAttribute("steering.locoSpdGain");
	else
		locoSpdGain = 70.0f;

	if (character && character->hasAttribute("steering.locoScootGain"))
		locoScootGain = (float) character->getDoubleAttribute("steering.locoScootGain");
	else
		locoScootGain =  2.0f;
	
	if (character && character->hasAttribute("steering.paLocoAngleGain"))
		paLocoAngleGain = (float) character->getDoubleAttribute("steering.paLocoAngleGain");
	else
		paLocoAngleGain =  2.0f;
	
	if (character && character->hasAttribute("steering.paLocoScootGain"))
		paLocoScootGain = (float) character->getDoubleAttribute("steering.paLocoScootGain");
	else
		paLocoScootGain =  9.0f;

	if (character && character->hasAttribute("steering.scootThreshold"))
		scootThreshold = (float) character->getDoubleAttribute("steering.scootThreshold");
	else
		scootThreshold = 0.1f;

	if (character && character->hasAttribute("steering.speedThreshold"))
		speedThreshold = (float) character->getDoubleAttribute("steering.speedThreshold");
	else
		speedThreshold = 0.1f;
	
	if (character && character->hasAttribute("steering.angleSpeedThreshold"))
		angleSpeedThreshold = (float) character->getDoubleAttribute("steering.angleSpeedThreshold");
	else
		angleSpeedThreshold = 10.0f;

	if (character && character->hasAttribute("steering.distThreshold"))
		distThreshold = (float) character->getDoubleAttribute("steering.distThreshold");
	else
		distThreshold = 1.80f;	

	if (character && character->hasAttribute("steering.distDownThreshold"))
		distDownThreshold = (float) character->getDoubleAttribute("steering.distDownThreshold");
	else
		distDownThreshold = 0.3f;

	if (character && character->hasAttribute("steering.brakingGain"))
		brakingGain = (float) character->getDoubleAttribute("steering.brakingGain");
	else
		brakingGain = 1.2f;
	
	if (character && character->hasAttribute("steering.desiredSpeed"))
		desiredSpeed = (float) character->getDoubleAttribute("steering.desiredSpeed");
	else
		desiredSpeed = 1.0f;	

	if (character && character->hasAttribute("steering.facingAngle"))
		facingAngle = (float) character->getDoubleAttribute("steering.facingAngle");
	else
		facingAngle = -200.0f;

	if (character && character->hasAttribute("steering.facingAngleThreshold"))
		facingAngleThreshold = (float) character->getDoubleAttribute("steering.facingAngleThreshold");
	else
		facingAngleThreshold = 10;
	
	if (character && character->hasAttribute("steering.acceleration"))
		acceleration = (float) character->getDoubleAttribute("steering.acceleration");
	else
		acceleration = 2.0f;	
	
	if (character && character->hasAttribute("steering.scootAcceleration"))
		scootAcceleration = (float) character->getDoubleAttribute("steering.scootAcceleration");
	else
		scootAcceleration = 200.f;	

	if (character && character->hasAttribute("steering.angleAcceleration"))
		angleAcceleration = (float) character->getDoubleAttribute("steering.angleAcceleration");
	else
		angleAcceleration = 400.f;

	if (character && character->hasAttribute("steering.stepAdjust"))
		stepAdjust = character->getBoolAttribute("steering.stepAdjust");
	else
		stepAdjust = false;

	if (character && character->hasAttribute("steering.smoothing"))
		smoothing = character->getBoolAttribute("steering.smoothing");
	else
		smoothing = true;

	if (character && character->hasAttribute("steering.pathFollowingMode"))
		pathFollowing = character->getBoolAttribute("steering.pathFollowingMode");
	else
		pathFollowing = false;
	
	if (character && character->hasAttribute("steering.speedWindowSize"))
		speedWindowSize = character->getIntAttribute("steering.speedWindowSize");
	else
		speedWindowSize = 10;
	
	if (character && character->hasAttribute("steering.angleWindowSize"))
		angleWindowSize = character->getIntAttribute("steering.angleWindowSize");
	else
		angleWindowSize = 3;

	if (character && character->hasAttribute("steering.scootWindowSize"))
		scootWindowSize = character->getIntAttribute("steering.scootWindowSize");
	else
		scootWindowSize = 3;

	if (character && character->hasAttribute("steering.pedMaxTurningRateMultiplier"))
		pedMaxTurningRateMultiplier = (float) character->getDoubleAttribute("steering.pedMaxTurningRateMultiplier");
	else
		pedMaxTurningRateMultiplier = 20.f;


	setSteerParamsDirty(false);

}

void SteeringAgent::updateSteerStateName()
{
	if (!character)
		return;

	std::string prefix = character->getName();
	if (character->statePrefix != "")
		prefix = character->statePrefix;
	stepStateName = prefix + "Step";
	locomotionName = prefix + "Locomotion";
	startingLName = prefix + "StartingLeft";
	startingRName = prefix + "StartingRight";
	idleTurnName = prefix + "IdleTurn";
	jumpName = prefix + "Jump";
}

void SteeringAgent::evaluate()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	//dt = (float) mcu.time_dt;

	if (!character)
		return;

	if (isSteerParamsDirty())
		initSteerParams();

	//---get current world offset position
	float x, y, z;
	float yaw, pitch, roll;
	character->get_world_offset(x, y, z, yaw, pitch, roll);

	//LOG("Character world offset : x = %f, y = %f, z = %f",x,y,z);

	// parameter testing
	if (paramTestFlag)
		parameterTesting();

	if (!agent)
		return;
	PPRAgent* pprAgent = dynamic_cast<PPRAgent*>(agent);
	if (!pprAgent)
		return;

	//mcu.mark("Steering",0,"Evaluate");
	const std::queue<SteerLib::AgentGoalInfo>& goalQueue = pprAgent->getLandmarkQueue();
	int numGoals = goalQueue.size();

	// make sure the character is within the grid
	if (mcu._scene->getSteerManager()->getEngineDriver()->_engine->getSpatialDatabase()->getCellIndexFromLocation(x * mcu.steeringScale, z * mcu.steeringScale) == -1)
	{
		if (numGoals > 0)
		{
			LOG("Character %s is out of range of grid (%f, %f). All goals will be cancelled.", character->getName().c_str(), x * mcu.steeringScale, z * mcu.steeringScale);
			agent->clearGoals();
		}
	}

	if (numGoals == 0) {
		newSpeed = 0.0f;
	}

	if (goalList.size() != 0 && numGoals == 0)
	{
		float goalx = goalList.front();
		goalList.pop_front();
		float goaly = goalList.front();
		goalList.pop_front();
		float goalz = goalList.front();
		goalList.pop_front();
		agent->clearGoals();
		SteerLib::AgentGoalInfo goal;
		goal.desiredSpeed = desiredSpeed;
		goal.goalType = SteerLib::GOAL_TYPE_SEEK_STATIC_TARGET;
		goal.targetIsRandom = false;
		goal.targetLocation = Util::Point(goalx * mcu.steeringScale, 0.0f, goalz * mcu.steeringScale);
		// make sure that the desired goal is within the bounds of the steering grid
		if (mcu._scene->getSteerManager()->getEngineDriver()->_engine->getSpatialDatabase()->getCellIndexFromLocation(goal.targetLocation.x, goal.targetLocation.z) == -1)
		{
			LOG("Goal (%f, %f) for character %s is out of range of grid.", goal.targetLocation.x, goal.targetLocation.z, character->getName().c_str());
		}
		else
		{
			agent->addGoal(goal);
		}
	}

	// Update Steering Engine (position, orientation, scalar speed)
	Util::Point newPosition(x * mcu.steeringScale, 0.0f, z * mcu.steeringScale);
	Util::Vector newOrientation = Util::rotateInXZPlane(Util::Vector(0.0f, 0.0f, 1.0f), yaw * float(M_PI) / 180.0f);
	try {
		pprAgent->updateAgentState(newPosition, newOrientation, newSpeed);
		pprAgent->updateAI((float)mcu.time, dt, (unsigned int)(mcu.time / dt));
	} catch (Util::GenericException& ge) {
		std::string message = ge.what();
		if (lastMessage == message)
		{
			numMessageRepeats++;
			if (numMessageRepeats % 100 == 0)
			{
				LOG("Message repeated %d times", numMessageRepeats);
			}
			else
			{
				numMessageRepeats = 0;
				LOG("Problem updating agent state: %s", ge.what());
			}
		}
	}

	
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
			goal.targetLocation = Util::Point(x1 * mcu.steeringScale, 0.0f, (z1 * mcu.steeringScale - 100.0f * mcu.steeringScale));
			agent->addGoal(goal);
		}
	}
	character->_lastReachStatus = character->_reachTarget;
	character->_reachTarget = false;

	// Evaluate
	//float newSpeed = desiredSpeed;
	// Meat Hook Locomotion Evaluation
	if (character->locomotion_type == character->Basic)
	{
		if (!character->basic_locomotion_ct)
			return;
		newSpeed = evaluateBasicLoco(x, y, z, yaw);
	}
	// Procedural Locomotion Evaluation
	if (character->locomotion_type == character->Procedural)
	{
		if (!character->locomotion_ct)
			return;
		if (!character->locomotion_ct->is_enabled())
			return;
		evaluateProceduralLoco(x, y, z, yaw);
	}

	// Example-Based Locomotion Evaluation
	if (character->locomotion_type == character->Example)
	{
		if (!character->param_animation_ct)
			return;

		if (!pathFollowing)
		//if (1)
		{
			newSpeed = evaluateExampleLoco(x, y, z, yaw);
		}
		else
		{
			evaluatePathFollowing(x, y, z, yaw);
		}
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

	//mcu.mark("Steering");
}

void SteeringAgent::evaluatePathFollowing(float x, float y, float z, float yaw)
{
	PAStateData* curState = character->param_animation_ct->getCurrentPAStateData();
	const std::string& curStateName = character->param_animation_ct->getCurrentStateName();
	mcuCBHandle& mcu = mcuCBHandle::singleton();	
	bool locomotionEnd = false;
	static int counter = 0;		
	
	if (character->param_animation_ct->isIdle() && steerPath.pathLength() > 0)    // need to define when you want to start the locomotion
	{
		PAStateData* locoState = mcu.lookUpPAState(locomotionName.c_str());
		SrVec pathDir;
		float pathDist;
		SrVec targetPos = steerPath.closestPointOnPath(SrVec(x,0,z),pathDir,pathDist);
		float distOnPath = steerPath.pathDistance(targetPos);
		float sceneScale = 1.f/getScene()->getScale();
		float maxSpeed = (float)character->getDoubleAttribute("steering.pathMaxSpeed")*sceneScale;
		nextPtOnPath = steerPath.pathPoint(distOnPath+maxSpeed);		
		float targetAngle = radToDeg(atan2(nextPtOnPath.x - x, nextPtOnPath.z - z));
		normalizeAngle(targetAngle);
		normalizeAngle(yaw);
		float diff = targetAngle - yaw;
		normalizeAngle(diff);
		
		// using the transition motion with different direction
		if (character->getBoolAttribute("steering.pathStartStep"))
		//if (0)
		{
			startLocomotion(diff);
		}
		else
		{
			locoState->paramManager->setWeight(0, 0, 0);
			std::stringstream command;
			command << "panim schedule char " << character->getName();
			command << " state " << locomotionName << " loop true playnow true";
			mcu.execute((char*) command.str().c_str());  
		}		
		counter = 0;	
	}

	if (curStateName == locomotionName)
	{
		//locomotionEnd = true;
		float curSpeed;
		float curTurningAngle;
		float curScoot;
		curState->paramManager->getParameter(curSpeed, curTurningAngle, curScoot);
		curSteerPos = SrVec(x,0,z);
		curSteerDir = SrVec(sin(degToRad(yaw)), 0, cos(degToRad(yaw)));
		// predict next position
		//LOG("curSpeed = %f, curTurningAngle = %f, curScoot = %f",curSpeed,curTurningAngle,curScoot);
		SrMat rotMat; rotMat.roty(curTurningAngle*dt);
		nextSteerDir = curSteerDir*rotMat;
		float radius = fabs(curSpeed/curTurningAngle);
		nextSteerPos = curSteerPos;// + curSteerDir*curSpeed*dt;//curSteerPos - curSteerDir*radius + nextSteerDir*radius; //
		//SrVec nextPos = curPos + (curDir+nextDir)*0.5f*curSpeed*dt;
		SrVec pathDir;
		float pathDist;
		nextPtOnPath = steerPath.closestPointOnPath(nextSteerPos,pathDir,pathDist);
		float distOnPath = steerPath.pathDistance(nextPtOnPath);
		nextPtOnPath = steerPath.pathPoint(distOnPath+curSpeed);
		SrVec ptDir = nextPtOnPath - curSteerPos; ptDir.normalize();
		
		float sceneScale = 1.f/getScene()->getScale();
		float maxSpeed = (float)character->getDoubleAttribute("steering.pathMaxSpeed")*sceneScale;
		float minSpeed = (float)character->getDoubleAttribute("steering.pathMinSpeed")*sceneScale;
		float angAcc = (float)character->getDoubleAttribute("steering.pathAngleAcc");
		float detectSeg = maxSpeed*1.5f;
		const float maxAcc = (maxSpeed)/2.f;//-minSpeed);///2.f;
		const float maxDcc = -(maxSpeed)/2.f;

		float pathCurvature = steerPath.pathCurvature(distOnPath-detectSeg*0.2f,distOnPath+detectSeg*0.8f)*2.0f;
		if (pathCurvature > M_PI) pathCurvature = (float)M_PI;
		static int counter = 0;		
		float curvSpeed = (1.f - pathCurvature/(float)M_PI)*(maxSpeed-minSpeed) + minSpeed;
		float acc = (curSpeed > curvSpeed) ? maxDcc : maxAcc;
		// do whatever you need to calculate
 		float newSpeed = curSpeed + acc*dt; 

		float distToTarget = (curSteerPos - steerPath.pathPoint(steerPath.pathLength()-0.01f)).len();		
		if (distToTarget < curSpeed * brakingGain ) // if close to target, slow down directly
			newSpeed = distToTarget / brakingGain;
		
		float distThreshold = 0.05f*sceneScale;
		float speedThreshold = 0.05f*sceneScale;
		if (distToTarget < distThreshold && newSpeed < speedThreshold)
			locomotionEnd = true;

		float newTurningAngle = radToDeg(asin(cross(curSteerDir,ptDir).y));
		normalizeAngle(newTurningAngle);
		newTurningAngle = newTurningAngle * (float)character->getDoubleAttribute("steering.paLocoAngleGain");
		float nextTurningAngle;		
		if (newTurningAngle > curTurningAngle)
		{
			nextTurningAngle = curTurningAngle + angAcc*dt;
			if (nextTurningAngle > newTurningAngle) nextTurningAngle = newTurningAngle;
		}
		else
		{
			nextTurningAngle = curTurningAngle - angAcc*dt;
			if (nextTurningAngle < newTurningAngle) nextTurningAngle = newTurningAngle;
		}
				
		//float newTurningAngle = normalizeAngle();//dt;
		//float newTurningAngle = (asin(cross(curSteerDir,ptDir).y))/dt;
		float newScoot =0.f;

#if 0 // output debug info
		if (counter > 100)
		{
			counter = 0;
			LOG("path curvature = %f, curveSpeed = %f, newSpeed = %f, distToTarget = %f, newTurningAngle = %f",pathCurvature,curvSpeed,newSpeed,distToTarget, newTurningAngle);			
		}
		counter++;
#endif
		// update locomotion state
		curState->paramManager->setWeight(newSpeed, nextTurningAngle, newScoot);
		character->param_animation_ct->updateWeights();	
		
	}
	if (locomotionEnd)      // need to define when you want to end the locomotion
	{
		character->param_animation_ct->schedule(NULL, true, true);

		// adjust facing angle 
		if (fabs(facingAngle) <= 180)
		{
			float diff = facingAngle - yaw;
			normalizeAngle(diff);
			adjustFacingAngle(diff);
		}
		
		//LOG("path following end");

		character->trajectoryGoalList.clear();
		agent->clearGoals();
		goalList.clear();
		steerPath.clearPath();		
	}
}


void SteeringAgent::setAgent(SteerLib::AgentInterface* a)
{
	agent = a;
}

SteerLib::AgentInterface* SteeringAgent::getAgent()
{
	return agent;
}

void SteeringAgent::setCharacter(SbmCharacter* c)
{
	character = c;
	addSteeringAttributes();
}

SbmCharacter* SteeringAgent::getCharacter()
{
	return character;
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
	mcuCBHandle& mcu = mcuCBHandle::singleton();
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
	//LOG("turning Rate= %f\n",angleDiff/dt);
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
		float curSpeed = character->basic_locomotion_ct->getMovingSpd() * mcu.steeringScale;
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
		curSpeed = curSpeed / mcu.steeringScale;
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

		curSpeed = currentSpeed / mcu.steeringScale;
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
	float ped_max_turning_rate = PED_MAX_TURNING_RATE * pedMaxTurningRateMultiplier;

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
		targetLoc.x = goalQueue.front().targetLocation.x;
		targetLoc.y = goalQueue.front().targetLocation.y;
		targetLoc.z = goalQueue.front().targetLocation.z;
		distToTarget = sqrt((x * mcu.steeringScale - targetLoc.x) * (x * mcu.steeringScale - targetLoc.x) + 
							(z * mcu.steeringScale - targetLoc.z) * (z * mcu.steeringScale - targetLoc.z));

		if (distToTarget < distDownThreshold)
			character->steeringAgent->getAgent()->clearGoals();
	}
	int numGoals = goalQueue.size();
	if (numGoals == 0)
	{
		reachTarget = true;
		character->_reachTarget = reachTarget;
	}
	if (character->_numSteeringGoal == 0 && numGoals != 0 && distToTarget < distThreshold)
		stepAdjust = true;
	if (distToTarget > distThreshold)
		stepAdjust = false;

	// slow down mechanism when close to the target
	float targetSpeed = steeringCommand.targetSpeed;
	if (distToTarget < targetSpeed * brakingGain && goalList.size() == 0)
		targetSpeed = distToTarget / brakingGain;

	if (stepAdjust)
		if (!character->param_animation_ct->hasPAState(stepStateName.c_str()))
		{
			agentToTargetDist = distToTarget / mcu.steeringScale;
			agentToTargetVec.x = targetLoc.x - x * mcu.steeringScale;
			agentToTargetVec.y = targetLoc.y - y * mcu.steeringScale;
			agentToTargetVec.z = targetLoc.z - z * mcu.steeringScale;
			agentToTargetVec /= mcu.steeringScale;
		}

	PAStateData* curState =  character->param_animation_ct->getCurrentPAStateData();
	const std::string& curStateName = character->param_animation_ct->getCurrentStateName();
	const std::string& nextStateName = character->param_animation_ct->getNextStateName();

	//---If you are close enough to the target when starting locomotion, use step adjust
	if (character->param_animation_ct->isIdle() && (agentToTargetDist > distDownThreshold))
	{
		SrVec heading = SrVec(sin(degToRad(yaw)), 0, cos(degToRad(yaw)));
		float y = dot(agentToTargetVec, heading);
		SrVec verticalHeading = SrVec(sin(degToRad(yaw - 90)), 0, cos(degToRad(yaw - 90)));
		float x = dot(agentToTargetVec, verticalHeading);
		if (!character->param_animation_ct->hasPAState(stepStateName.c_str()))
		{
			PAStateData* stepState = mcu.lookUpPAState(stepStateName.c_str());
			stepState->paramManager->setWeight(x, y);
			std::stringstream command;
			command << "panim schedule char " << character->getName();			
			command << " state " << stepStateName << " loop false playnow false additive false joint null ";
			for (int i = 0; i < stepState->getNumMotions(); i++)
				command << stepState->weights[i] << " ";
			mcu.execute((char*) command.str().c_str());
		}		
		return 0;
	}

	//---start locomotion
	if (character->param_animation_ct->isIdle() && numGoals != 0 && nextStateName == "" && distToTarget > distDownThreshold)
	{
		// check to see if there's anything obstacles around it
		float targetAngle = radToDeg(atan2(pprAgent->getStartTargetPosition().x - x * mcu.steeringScale, pprAgent->getStartTargetPosition().z - z * mcu.steeringScale));
		normalizeAngle(targetAngle);
		normalizeAngle(yaw);
		float diff = targetAngle - yaw;
		normalizeAngle(diff);

		// Improve on the starting angle by examining whether there's obstacles around
		std::map<std::string, SbmPawn*>& cMap = mcu.getPawnMap();			
		std::map<std::string, SbmPawn*>::iterator iter = cMap.begin();
		std::vector<float> neigbors;
		for (; iter != cMap.end(); iter++)
		{
			if (iter->second->getName() != character->getName())
			{
				float cX, cY, cZ, cYaw, cRoll, cPitch;
				iter->second->get_world_offset(cX, cY, cZ, cYaw, cPitch, cRoll);
				float cDist = sqrt((x - cX) * (x - cX) + (z - cZ) * (z - cZ));
				if (cDist < (1.5f / mcu.steeringScale))
				{
					float cAngle = radToDeg(atan2(pprAgent->getStartTargetPosition().x - x, pprAgent->getStartTargetPosition().z - z));
					normalizeAngle(cAngle);
					float cDiff = cAngle - yaw;
					normalizeAngle(cDiff);
					if (diff * cDiff > 0)
						neigbors.push_back(cAngle);
				}
			}
		}
		if (neigbors.size() > 0)
			fastInitial = true;
		else
			fastInitial = false;
		if (!fastInitial)
		{
			startLocomotion(diff);	
		}
		else
		{
	//		if (character->steeringConfig == character->MINIMAL)
			{
				PAStateData* locoState = mcu.lookUpPAState(locomotionName.c_str());
				locoState->paramManager->setWeight(0, 0, 0);
				std::stringstream command;
				command << "panim schedule char " << character->getName();
				command << " state " << locomotionName << " loop true playnow true additive false joint null";
				mcu.execute((char*) command.str().c_str());
			}
		}
		return 0;
	}	

	//---end locomotion
	if (character->_numSteeringGoal != 0 && numGoals == 0)
	{
		if (goalList.size() == 0)
		{
			character->param_animation_ct->schedule(NULL, true, true);
		}
		else
		{
			float goalx = goalList.front();
			goalList.pop_front();
			float goaly = goalList.front();
			goalList.pop_front();
			float goalz = goalList.front();
			goalList.pop_front();
			agent->clearGoals();
			SteerLib::AgentGoalInfo goal;
			goal.desiredSpeed = desiredSpeed;
			goal.goalType = SteerLib::GOAL_TYPE_SEEK_STATIC_TARGET;
			goal.targetIsRandom = false;
			goal.targetLocation = Util::Point(goalx * mcu.steeringScale, 0.0f, goalz * mcu.steeringScale);
			agent->addGoal(goal);
		}
		return 0;
	}

	//---If the facing angle is not correct, use idle turning

	if (character->param_animation_ct->isIdle() && fabs(facingAngle) <= 180 && character->steeringConfig == character->STANDARD)
	{
		float diff = facingAngle - yaw;
		normalizeAngle(diff);
		adjustFacingAngle(diff);
		
		return 0;
	}


	//---Need a better way to handle the control between steering and Parameterized Animation Controller
	if (character->param_animation_ct->hasPAState(jumpName))
		inControl = false;
	else
		inControl = true;

	//---update locomotion
	float curSpeed = 0.0f;
	float curTurningAngle = 0.0f;
	float curScoot = 0.0f;
	if (curStateName == locomotionName && numGoals != 0)
	{
		curState->paramManager->getParameter(curSpeed, curTurningAngle, curScoot);
		if (smoothing)
		{
			float addOnScoot = steeringCommand.scoot * paLocoScootGain;
			if (steeringCommand.scoot != 0.0)
			{
				if (curScoot < addOnScoot)
				{
					curScoot += scootAcceleration * dt;
					if (curScoot > addOnScoot)
						curScoot = addOnScoot;
				}
				else
				{
					curScoot -= scootAcceleration * dt;
					if (curScoot < addOnScoot)
						curScoot = addOnScoot;
				}
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
			curSpeed = curSpeed * mcu.steeringScale;
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

			float addOnTurning = angleDiff * paLocoAngleGain;
			if (fabs(curTurningAngle - addOnTurning) > angleSpeedThreshold)
			{
				if (curTurningAngle < addOnTurning)
				{
					curTurningAngle += angleAcceleration * dt;
					if (curTurningAngle > addOnTurning)
						curTurningAngle = addOnTurning;
				}
				else if (curTurningAngle > addOnTurning)
				{					
					curTurningAngle -= angleAcceleration * dt;
					if (curTurningAngle < addOnTurning)
						curTurningAngle = addOnTurning;
				}
			}
			// update locomotion state
			newSpeed = curSpeed;
			curSpeed = curSpeed / mcu.steeringScale;
		}
		else	// direct gaining
		{
			float angleGlobal = radToDeg(atan2(forward.x, forward.z));
			normalizeAngle(angleGlobal);
			normalizeAngle(yaw);
			float angleDiff = angleGlobal - yaw;
			normalizeAngle(angleDiff);

			curSpeed = targetSpeed / mcu.steeringScale;
			curTurningAngle = angleDiff * paLocoAngleGain;
			curScoot = steeringCommand.scoot * paLocoScootGain;
			
			newSpeed = targetSpeed;
		}

		if (inControl)
		{
			curState->paramManager->setWeight(curSpeed, curTurningAngle, curScoot);
			character->param_animation_ct->updateWeights();
		}
	}
#endif
	return newSpeed;
}


void SteeringAgent::startLocomotion( float angleDiff )
{
	//		if (character->steeringConfig == character->STANDARD)
	{
		/*			
		float angleGlobal = radToDeg(atan2(forward.x, forward.z));
		normalizeAngle(angleGlobal);
		normalizeAngle(yaw);
		float diff = angleGlobal - yaw;
		normalizeAngle(diff);
		*/
		mcuCBHandle& mcu = mcuCBHandle::singleton();
		double w;
		if (angleDiff > 0)
		{
			if (angleDiff > 90)
			{
				w = (angleDiff - 90) / 90;
				std::stringstream command;
				command << "panim schedule char " << character->getName();			
				command << " state " << startingLName << " loop false playnow false additive false joint null " << " 0 " << 1 - w << " " << w;
				mcu.execute((char*) command.str().c_str());
			}
			else
			{
				w = angleDiff / 90;
				std::stringstream command;
				command << "panim schedule char " << character->getName();					
				command << " state " << startingLName << " loop false playnow false additive false joint null " << 1 - w << " " << w << " " << " 0 ";
				mcu.execute((char*) command.str().c_str());
			}
		}
		else
		{
			if (angleDiff < -90)
			{
				w = (angleDiff + 180) / 90;
				std::stringstream command;
				command << "panim schedule char " << character->getName();
				command << " state " << startingRName << " loop false playnow false additive false joint null " << " 0 " << w << " " << 1 - w;
				mcu.execute((char*) command.str().c_str());
			}
			else
			{
				w = -angleDiff / 90;
				std::stringstream command;
				command << "panim schedule char " << character->getName();
				command << " state " << startingRName << " loop false playnow true additive false joint null " << 1 - w << " " << w << " 0 ";
				mcu.execute((char*) command.str().c_str());
			}				
		}
		PAStateData* locoState = mcu.lookUpPAState(locomotionName.c_str());
		for (int i = 0; i < locoState->getNumMotions(); i++)
		{
			if (i == 0)
				locoState->weights[i] = 1.0;
			else
				locoState->weights[i] = 0.0;
		}
		std::stringstream command1;
		command1 << "panim schedule char " << character->getName();
		command1 << " state " << locomotionName << " loop true playnow false additive false joint null";
		mcu.execute((char*) command1.str().c_str());
	}
}

void SteeringAgent::adjustFacingAngle( float angleDiff )
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();	
	std::string playNow;
	if (fabs(angleDiff) > facingAngleThreshold && !character->param_animation_ct->hasPAState(idleTurnName.c_str()))
	{
		PAStateData* idleTurnState = mcu.lookUpPAState(idleTurnName.c_str());
		idleTurnState->paramManager->setWeight(-angleDiff);
		std::stringstream command;
		command << "panim schedule char " << character->getName();			
		command << " state " << idleTurnName << " loop false playnow false additive false joint null ";
		for (int i = 0; i < idleTurnState->getNumMotions(); i++)
			command << idleTurnState->weights[i] << " ";
		mcu.execute((char*) command.str().c_str());
	}
	else
	{
		facingAngle = -200;
	}
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
	if (character->param_animation_ct->isIdle() && (dist > 10.f))
	{
		SrVec heading = SrVec(sin(degToRad(yaw)), 0, cos(degToRad(yaw)));
		float offsety = dot(agentToTargetVec, heading);
		SrVec verticalHeading = SrVec(sin(degToRad(yaw - 90)), 0, cos(degToRad(yaw - 90)));
		float offsetx = dot(agentToTargetVec, verticalHeading);
		if (!character->param_animation_ct->hasPAState(stepStateName.c_str()))
		{
			PAStateData* stepState = mcu.lookUpPAState(stepStateName.c_str());
			stepState->paramManager->setWeight(x, y);
			std::stringstream command;
			command << "panim schedule char " << character->getName();			
			command << " state " << stepStateName << " loop false playnow false additive false joint null ";
			for (int i = 0; i < stepState->getNumMotions(); i++)
				command << stepState->weights[i] << " ";
			mcu.execute((char*) command.str().c_str());
		}	
	}
	if (dist < 10.0f)
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

void SteeringAgent::setSteerParamsDirty(bool val)
{
	_dirty = val;
}

bool SteeringAgent::isSteerParamsDirty()
{
	return _dirty;
}
