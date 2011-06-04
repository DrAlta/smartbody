#include "SteeringAgent.h"
#include <sbm/mcontrol_util.h>
#include <sbm/me_ct_param_animation_data.h>
#define DebugInfo 0
#define UseTransition 0

SteeringAgent::SteeringAgent(SbmCharacter* c) : character(c)
{
	agent = NULL;
	target = NULL;
	scootCurve = new srLinearCurve();

	scootAccel = 5.0f;	// should be exposed
	locoSpdGain = 70.0f;
	paLocoAngleGain = 2.0f;
	scootThreshold = 0.02f;	
	paLocoScootGain = 1.0f;
	locoScootGain = 2.0f;
	distThreshold = 150.0f;	// exposed, centimeter
	transition = 0.20f;
	desiredSpeed = 1.0f;	// exposed, meter
	facingAngle = -200.0f;
	facingAngleThreshold = 10;	//angle
	facingAdjustPhase = false;
}

SteeringAgent::~SteeringAgent()
{
}

void SteeringAgent::evaluate()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (!agent)
		return;
	PPRAgent* pprAgent = dynamic_cast<PPRAgent*>(agent);
	if (!pprAgent)
		return;

	character->_lastReachStatus = character->_reachTarget;
	// get current world offset position
	float x, y, z;
	float yaw, pitch, roll;
	character->get_world_offset(x, y, z, yaw, pitch, roll);
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
			goal.targetLocation = Util::Point(x1 / 100.0f, character->getHeight() / 200.0f, z1 / 100.0f);
			agent->addGoal(goal);
		}
	}

	//---given goal, check if reach already
	bool reachTarget = false;
	
	const std::queue<SteerLib::AgentGoalInfo>& goalQueue = pprAgent->getLandmarkQueue();
	if (goalQueue.size() > 0)
	{
		float dist = sqrt((x - goalQueue.front().targetLocation.x * 100.0f) * (x - goalQueue.front().targetLocation.x * 100.0f) + 
					 (y - goalQueue.front().targetLocation.y * 100.0f) * (y - goalQueue.front().targetLocation.y * 100.0f) + 
					 (z - goalQueue.front().targetLocation.z * 100.0f) * (z - goalQueue.front().targetLocation.z * 100.0f));

		if (dist < distThreshold)
			character->steeringAgent->getAgent()->clearGoals();
	}
	int numGoals = goalQueue.size();
	if (numGoals == 0 && character->_numSteeringGoal > 0)
		reachTarget = true;

	const SteerLib::SteeringCommand & steeringCommand = pprAgent->getSteeringCommand();
	float angleGlobal = atan2(steeringCommand.targetDirection.x, steeringCommand.targetDirection.z) * (180.0f/float(M_PI));
	normalizeAngle(angleGlobal);
	normalizeAngle(yaw);
	float angleDiff = angleGlobal - yaw;
	normalizeAngle(angleDiff);
	float newSpeed = desiredSpeed;

	//printf("num Goals = %d\n",numGoals);

	//---update for locomotion_ct
	if (mcu.steering_use_procedural)
	{
		if (!character->locomotion_ct)
		{
//			LOG("Locomotion engine not enabled!");
			return;
		}
		if (!character->locomotion_ct->is_enabled())
		{
			return;
		}

		float speed = steeringCommand.targetSpeed * locoSpdGain;
		if (numGoals == 0)
		{
			if (character->_reachTarget)
				return;
			std::stringstream strstr;
			strstr << "test loco char ";
			strstr << character->name;
			strstr << " stop";
			mcu.execute((char*)strstr.str().c_str());
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
					strstr << character->name;
					strstr << " leftward spd 0.0 rps " << turn;
					mcu.execute((char*)strstr.str().c_str());	
				}
				else
					character->_reachTarget = true;
			}
			else
				character->_reachTarget = true;
		}
		else
		{
			character->_reachTarget = false;
			if (fabs(steeringCommand.scoot) > scootThreshold)
			{
#if DebugInfo
				LOG("Character: %s, Scoot value: %f, Speed: %f", character->name, steeringCommand.scoot, steeringCommand.targetSpeed);				
#endif	
				float turn = -2.0f;
				if (steeringCommand.scoot > 0)
					turn *= -1.0f;
				std::stringstream strstr;
				strstr << "test loco char ";
				strstr << character->name;
				strstr << " forward spd 0.0 rps " << turn;
				mcu.execute((char*)strstr.str().c_str());			
			}
			else
			{
#if 0
				if (speed == 0.0f)
				{
					std::stringstream strstr;
					strstr << "test loco char ";
					strstr << character->name;
					strstr << " stop";
					mcu.execute((char*)strstr.str().c_str());
				}
				else
#endif
				{
					if (steeringCommand.aimForTargetDirection)
					{
						float spd = 1.0f;
						std::stringstream strstr;
						strstr << "test loco char ";
						strstr << character->name;

						strstr << " forward spd " << speed << " rps ";
						if (angleDiff < 0)
						 strstr << spd << " ";
						else
						 strstr << -spd << " ";
						strstr << "angle " << angleDiff * M_PI/180.0;
						mcu.execute((char*)strstr.str().c_str());
					}
					else
					{
						float turningScale = float(M_PI);
						float turningAmount = float(M_PI);
						std::stringstream strstr;
						strstr << "test loco char ";
						strstr << character->name;
						strstr << " forward spd " << speed;
						//<< " rps ";						
						//strstr << turningScale * steeringCommand.turningAmount;
						//strstr << " angle ";
						//if (steeringCommand.turningAmount < 0)
						//	strstr << -1.0f * turningAmount;
						//else
						//	strstr << turningAmount;
						mcu.execute((char*)strstr.str().c_str());
					}
				}
			}
		}
	}
	//---update for param_animation_ct
	else
	{
		if (!character->param_animation_ct)
		{
//			LOG("Locomotion engine not enabled!");
			return;
		}
		PAStateData* curState =  character->param_animation_ct->getCurrentPAStateData();
		std::string curStateName = character->param_animation_ct->getCurrentStateName();
		std::string nextStateName = character->param_animation_ct->getNextStateName();

		if (reachTarget)
		{
			if (curStateName == "UtahLocomotion" || nextStateName == "UtahLocomotion"
				|| nextStateName == "UtahStopToWalk" || nextStateName == "UtahStartingLeft" || nextStateName == "UtahStartingRight"
				)
			{
				PAStateData* state = mcu.lookUpPAState("UtahWalkToStop");
				character->param_animation_ct->schedule(state, false);
				facingAdjustPhase = true;
			}
			if (nextStateName == "" && (curStateName == "UtahStopToWalk" || curStateName == "UtahStartingLeft" || curStateName == "UtahStartingRight"))
			{
				PAStateData* state = mcu.lookUpPAState("UtahLocomotion");
				character->param_animation_ct->schedule(state, true);
				state = mcu.lookUpPAState("UtahWalkToStop");
				character->param_animation_ct->schedule(state, false);
				facingAdjustPhase = true;			
			}
//			character->param_animation_ct->dumpScheduling();
		}
		// start locomotion
		if (curState)
			if (curState->stateName == PseudoIdleState && numGoals != 0 && nextStateName == "")
			{
				character->_reachTarget = false;
				float targetAngle = atan2(goalQueue.front().targetLocation.x * 100.0f - x, goalQueue.front().targetLocation.z * 100.0f - z) * (180.0f/float(M_PI));
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
						command << "panim schedule char " << character->name;			
						command << " state UtahStartingLeft loop false playnow true " << " 0 " << 1 - w << " " << w;
						mcu.execute((char*) command.str().c_str());
					}
					else
					{
						w = diff / 90;
						std::stringstream command;
						command << "panim schedule char " << character->name;					
						command << " state UtahStartingLeft loop false playnow true " << 1 - w << " " << w << " " << " 0 ";
						mcu.execute((char*) command.str().c_str());
					}
				}
				else
				{
					if (diff < -90)
					{
						w = (diff + 180) / 90;
						std::stringstream command;
						command << "panim schedule char " << character->name;
						command << " state UtahStartingRight loop false playnow true " << " 0 " << w << " " << 1 - w;
						mcu.execute((char*) command.str().c_str());
					}
					else
					{
						w = -diff / 90;
						std::stringstream command;
						command << "panim schedule char " << character->name;
						command << " state UtahStartingRight loop false playnow true " << 1 - w << " " << w << " 0 ";
						mcu.execute((char*) command.str().c_str());
					}				
				}
				std::stringstream command1;
				command1 << "panim schedule char " << character->name;
				command1 << " state UtahLocomotion loop true playnow false";
				mcu.execute((char*) command1.str().c_str());
			}	
		float addOnTurning = (float)scootCurve->evaluate(mcu.time);
		if (fabs(steeringCommand.scoot) > scootThreshold)
		{
			float scootValue = atan2(steeringCommand.scoot, steeringCommand.targetSpeed) * (180.0f / float(M_PI));
			normalizeAngle(scootValue);
			scootValue *= -paLocoScootGain;
#if UseTransition
			scootCurve->insert(mcu.time + transition, scootValue);
#else
		//	addOnTurning = scootValue;
			addOnTurning = steeringCommand.scoot * 3.0f;
#endif
		}
#if UseTransition
		else
			scootCurve->insert(mcu.time + transition, 0.0);
#endif
		// update locomotion
		float curSpeed = 0.0f;
		float curTurningAngle = 0.0f;
		if (curState)
			if (curState->stateName == "UtahLocomotion" && numGoals != 0)
			{
				curState->paramManager->getParameter(curSpeed, curTurningAngle);
				curSpeed = curSpeed / 100.0f;
				if (steeringCommand.aimForTargetSpeed)
				{
					// target speed is the desired speed for the character
					if (curSpeed < steeringCommand.targetSpeed)
					{
						curSpeed += steeringCommand.acceleration * 100.0f/ 60.0f;
						if (curSpeed > steeringCommand.targetSpeed)
							curSpeed = steeringCommand.targetSpeed;
					}
					else
					{
						curSpeed -= steeringCommand.acceleration * 100.0f/ 60.0f;
						if (curSpeed < steeringCommand.targetSpeed)
							curSpeed = steeringCommand.targetSpeed;
					}
				}
				else
					curSpeed += steeringCommand.acceleration / 60.0f;
				newSpeed = curSpeed;
				if (steeringCommand.aimForTargetDirection)
				{
			//		angleDiff += addOnTurning;
					curTurningAngle = angleDiff * paLocoAngleGain;
				}
				else
					curTurningAngle = steeringCommand.turningAmount / 60.0f;
				curSpeed *= 100.0f;
			//	curState->paramManager->setWeight(curSpeed, curTurningAngle);
				curState->paramManager->setWeight(curSpeed, curTurningAngle, addOnTurning);
				character->param_animation_ct->updateWeights();
			}
		if (curState)
		{
			if (curState->stateName == PseudoIdleState && numGoals == 0 && nextStateName == "")
			{
				if (fabs(facingAngle) <= 180)
				{
					float diff = facingAngle - yaw;
					normalizeAngle(diff);
					std::string playNow;
					if (fabs(diff) > facingAngleThreshold)
					{
						double w = 0;
						if (facingAdjustPhase)
							playNow = "true";
						else
							playNow = "false";
						if (diff <= -90)
						{
							w = (diff + 180) / 180;
							std::stringstream command1;
							command1 << "panim schedule char " << character->name;
							command1 << " state UtahIdleTurnRight loop false playnow " << playNow << " 0 " << w << " " << 1 - w;
							mcu.execute((char*) command1.str().c_str());						
							facingAdjustPhase = false;
						}
						else if (diff >= 90)
						{
							w = (diff - 90) / 90;
							std::stringstream command1;
							command1 << "panim schedule char " << character->name;
							command1 << " state UtahIdleTurnLeft loop false playnow " << playNow << " 0 " << 1 - w << " " << w;
							mcu.execute((char*) command1.str().c_str());												
							facingAdjustPhase = false;
						}
						else if (diff <= 0)
						{
							w = fabs(diff / 90);
							std::stringstream command1;
							command1 << "panim schedule char " << character->name;
							command1 << " state UtahIdleTurnRight loop false playnow " << playNow << " " << 1 - w << " " << w << " 0 ";
							mcu.execute((char*) command1.str().c_str());
							facingAdjustPhase = true;
						}
						else if (diff >= 0)
						{
							w = diff / 90;
							std::stringstream command1;
							command1 << "panim schedule char " << character->name;
							command1 << " state UtahIdleTurnLeft loop false playnow " << playNow << " " << 1 - w << " " << w << " 0 ";
							mcu.execute((char*) command1.str().c_str());	
							facingAdjustPhase = true;
						}
					}
					else
						character->_reachTarget = true;
				}
				else
					character->_reachTarget = true;
			}
		}
	}

	//---update the steering engine with character state (position, orientation, scalar speed)
	Util::Point newPosition(x / 100.0f, y / 100.0f, z / 100.0f);
	Util::Vector newOrientation = Util::rotateInXZPlane(Util::Vector(0.0f, 0.0f, 1.0f), yaw * float(M_PI) / 180.0f);
	pprAgent->updateAgentState(newPosition, newOrientation, newSpeed);
	pprAgent->updateAI((float)mcu.time, (float)mcu.time_dt, int(mcu.time / mcu.time_dt));
	character->_numSteeringGoal = numGoals;

	//printf("Reach target = %d, num of goals = %d\n",character->_reachTarget,character->_numSteeringGoal);
	//---
	if (!character->_lastReachStatus && character->_reachTarget)
	{
		std::string eventType = "locomotion";		
		MotionEvent motionEvent;
		motionEvent.setType(eventType);			
		std::string param = std::string(character->name) + " success";
		motionEvent.setParameters(param);
		EventManager* manager = EventManager::getEventManager();		
		manager->handleEvent(&motionEvent, mcu.time);
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