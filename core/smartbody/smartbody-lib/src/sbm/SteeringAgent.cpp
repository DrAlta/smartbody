#include "SteeringAgent.h"
#include <sbm/mcontrol_util.h>
#include <sbm/me_ct_param_animation_data.h>
#define DebugInfo 0

SteeringAgent::SteeringAgent(SbmCharacter* c) : character(c)
{
	agent = NULL;
	target = NULL;

	// There parameters are ad-hoc
	basicLocoAngleGain = 2.0f;
	basicLocoScootGain = 10.0f;

	locoSpdGain = 70.0f;
	locoScootGain = 2.0f;

	paLocoAngleGain = 2.0f;
	paLocoScootGain = 9.0f;

	scootThreshold = 0.02f;	
	distThreshold = 150.0f;			// exposed, unit: centimeter

	desiredSpeed = 1.0f;			// exposed, unit: meter/sec
	facingAngle = -200.0f;			// exposed, unit: deg
	facingAngleThreshold = 10;
	facingAdjustPhase = false;
	acceleration = 2.0f;			// exposed, unit: meter/s^2
	scootAcceleration = 200.0f;		// exposed, unit: unknown
	angleAcceleration = 450.0f;		// exposed, unit: unknown
}

SteeringAgent::~SteeringAgent()
{
}

void SteeringAgent::evaluate()
{
	float dt = 1.0f / 60.0f;
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
			goal.targetLocation = Util::Point(cmToM(x1), 0.0f, cmToM(z1));
			agent->addGoal(goal);
		}
	}

	//---given goal, check if reach already
	bool reachTarget = false;
	character->_reachTarget = false;
	const std::queue<SteerLib::AgentGoalInfo>& goalQueue = pprAgent->getLandmarkQueue();
	if (goalQueue.size() > 0)
	{
		float dist = sqrt((x - mToCm(goalQueue.front().targetLocation.x)) * (x - mToCm(goalQueue.front().targetLocation.x)) + 
						  (y - mToCm(goalQueue.front().targetLocation.y)) * (y - mToCm(goalQueue.front().targetLocation.y)) + 
						  (z - mToCm(goalQueue.front().targetLocation.z)) * (z - mToCm(goalQueue.front().targetLocation.z)));

		if (dist < distThreshold)
			character->steeringAgent->getAgent()->clearGoals();
	}
	int numGoals = goalQueue.size();
	if (numGoals == 0 && character->_numSteeringGoal > 0)
		reachTarget = true;

	const SteerLib::SteeringCommand & steeringCommand = pprAgent->getSteeringCommand();
	float angleGlobal = radToDeg(atan2(steeringCommand.targetDirection.x, steeringCommand.targetDirection.z));
	normalizeAngle(angleGlobal);
	normalizeAngle(yaw);
	float angleDiff = angleGlobal - yaw;
	normalizeAngle(angleDiff);
	float newSpeed = desiredSpeed;			//angle diff is used for turning speed

	// Meat Hook Locomotion Evaluation
	if (mcu.locomotion_type == mcu.Basic)
	{
		if (!character->basic_locomotion_ct)
			return;
		if (numGoals == 0)
		{
			character->_reachTarget = true;
			character->basic_locomotion_ct->setValid(false);
			character->basic_locomotion_ct->setMovingSpd(0.0f);
			character->basic_locomotion_ct->setTurningSpd(0.0f);
			character->basic_locomotion_ct->setScootSpd(0.0f);
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
	}

	// Procedural Locomotion Evaluation
	if (mcu.locomotion_type == mcu.Procedural)
	{
		if (!character->locomotion_ct)
			return;
		if (!character->locomotion_ct->is_enabled())
			return;
		float speed = steeringCommand.targetSpeed * locoSpdGain;
		if (numGoals == 0)
		{				
			std::stringstream strstr;
			strstr << "test loco char ";
			strstr << character->name;
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
					strstr << character->name;
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
			character->_reachTarget = false;
			if (fabs(steeringCommand.scoot) > scootThreshold)
			{
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
					strstr << "angle " << degToRad(angleDiff);
					mcu.execute((char*)strstr.str().c_str());
				}
				else
				{
					std::stringstream strstr;
					strstr << "test loco char ";
					strstr << character->name;
					strstr << " forward spd " << speed;
					mcu.execute((char*)strstr.str().c_str());
				}
			}
		}
	}

	// Example-Based Locomotion Evaluation
	if (mcu.locomotion_type == mcu.Example)
	{
		if (!character->param_animation_ct)
		{
			return;
		}
		PAStateData* curState =  character->param_animation_ct->getCurrentPAStateData();
		std::string curStateName = character->param_animation_ct->getCurrentStateName();
		std::string nextStateName = character->param_animation_ct->getNextStateName();

		//---end locomotion
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
			//character->param_animation_ct->dumpScheduling();
		}

		//---start locomotion
		if (curState)
			if (curState->stateName == PseudoIdleState && numGoals != 0 && nextStateName == "")
			{
				float targetAngle = radToDeg(atan2(mToCm(goalQueue.front().targetLocation.x) - x, mToCm(goalQueue.front().targetLocation.z) - z));
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
				PAStateData* locoState = mcu.lookUpPAState("UtahLocomotion");
				if (!locoState)
					return;
				for (int i = 0; i < locoState->getNumMotions(); i++)
				{
					if (i == 0)
						locoState->weights[i] = 1.0;
					else
						locoState->weights[i] = 0.0;
				}
				std::stringstream command1;
				command1 << "panim schedule char " << character->name;
				command1 << " state UtahLocomotion loop true playnow false";
				mcu.execute((char*) command1.str().c_str());
			}	

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
				if (steeringCommand.aimForTargetDirection)
				{
					float addOnTurning = angleDiff * paLocoAngleGain;
					if (curTurningAngle < addOnTurning)
						curTurningAngle += angleAcceleration * dt;
					else if (curTurningAngle > addOnTurning)
						curTurningAngle -= angleAcceleration * dt;
				}
				else
					curTurningAngle = steeringCommand.turningAmount * dt;
				newSpeed = curSpeed;
				curSpeed = mToCm(curSpeed);
				curState->paramManager->setWeight(curSpeed, curTurningAngle, curScoot);
				character->param_animation_ct->updateWeights();
			}

		//---adjust for facing
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

	// Update Steering Engine (position, orientation, scalar speed)
	Util::Point newPosition(x / 100.0f, y / 100.0f, z / 100.0f);
	Util::Vector newOrientation = Util::rotateInXZPlane(Util::Vector(0.0f, 0.0f, 1.0f), yaw * float(M_PI) / 180.0f);
	pprAgent->updateAgentState(newPosition, newOrientation, newSpeed);
	pprAgent->updateAI((float)mcu.time, (float)mcu.time_dt, int(mcu.time / mcu.time_dt));
	character->_numSteeringGoal = numGoals;

	// Event Handler
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