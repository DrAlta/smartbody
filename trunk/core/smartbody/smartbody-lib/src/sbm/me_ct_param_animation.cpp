/*
 *  me_ct_param_animation.cpp - part of Motion Engine and SmartBody-lib
 *  Copyright (C) 2008  University of Southern California
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

#include "me_ct_param_animation.h"
#include <sbm/mcontrol_util.h>
#include <sbm/SBAnimationState.h>
#include <sbm/SBSkeleton.h>

std::string MeCtParamAnimation::Context::CONTEXT_TYPE = "MeCtParamAnimation::Context";
std::string MeCtParamAnimation::CONTROLLER_TYPE = "MeCtParamAnimation";

#define debug 0

void MeCtParamAnimation::Context::child_channels_updated( MeController* child )
{
}

MeCtParamAnimation::MeCtParamAnimation() : MeCtContainer(new MeCtParamAnimation::Context(this))
{
	character = NULL;
	woWriter = NULL;
}

MeCtParamAnimation::MeCtParamAnimation(SbmCharacter* c, MeCtChannelWriter* wo) : MeCtContainer(new MeCtParamAnimation::Context(this)), character(c), woWriter(wo)
{
	baseJointName = "base";
	curStateData = NULL;
	nextStateData = NULL;
	transitionManager = NULL;
	controllerBlending = new PAControllerBlending();
	waitingList.clear();
	prevGlobalTime = mcuCBHandle::singleton().time;
}

MeCtParamAnimation::~MeCtParamAnimation()
{
	reset();
}

void MeCtParamAnimation::controller_map_updated()
{
}

SkChannelArray& MeCtParamAnimation::controller_channels()
{
	return channels;
}

double MeCtParamAnimation::controller_duration()
{
	return -1;
}

bool MeCtParamAnimation::controller_evaluate(double t, MeFrameData& frame)
{	
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.mark("locomotion",0,"controller_evaluate");
	double timeStep = t - prevGlobalTime;
	prevGlobalTime = t;
	autoScheduling(t);
	if (controllerBlending)
		controllerBlending->updateBuffer(frame.buffer());
	//--make sure there is always a pseudo idle state running in the system
	if ((!curStateData && !nextStateData) ||
		!curStateData)
	{
		std::vector<double> weights;
		schedule(NULL, weights, true, true);
		mcu.mark("locomotion");
		return true;
	}
		
	if (curStateData)
	{
		if (curStateData->state &&
			curStateData->state->stateName != PseudoIdleState && 
			nextStateData == NULL && !curStateData->loop)
		{
			std::vector<double> weights;
			schedule(NULL, weights, true);
		}
			
	}
	//------

	if (transitionManager)
	{
		if (curStateData == NULL || nextStateData == NULL)
		{
			std::string errorInfo;
			errorInfo = character->getName() + "'s animation state transition warning. ";
			if (curStateData)
				if (curStateData->state)
					errorInfo += "current state: " + curStateData->state->stateName;
			else
				errorInfo += "current state: null ";
			if (nextStateData)
				if (nextStateData->state)
					errorInfo += "next state: " + nextStateData->state->stateName;
			else 
				errorInfo += "next state: null ";
			LOG("%s", errorInfo.c_str());
			if (curStateData == NULL && nextStateData != NULL)
			{
				if (nextStateData->state)
					LOG("would start state %s now", nextStateData->state->stateName.c_str());
				// should not delete, just swap the pointers
				curStateData = nextStateData;
				curStateData->active = true;
				nextStateData = NULL;
				delete transitionManager;
				transitionManager = NULL;
				return true;
			}
			if (curStateData != NULL && nextStateData == NULL)
			{
				LOG("scheduling problem, please check the corresponding time marks for two states.");
				reset();
				return false;
			}
		}

		if (!transitionManager->blendingMode)
			transitionManager->align(curStateData, nextStateData);
		else
		{
			if (transitionManager->active)
			{
				SrBuffer<float> buffer1;
				buffer1.size(frame.buffer().size());
				buffer1 = frame.buffer();
				SrBuffer<float> buffer2;
				buffer2.size(frame.buffer().size());
				buffer2 = frame.buffer();
				SrMat transformMat;
				curStateData->evaluate(timeStep * transitionManager->getSlope(), buffer1);
				nextStateData->evaluate(timeStep, buffer2);
				transitionManager->blending(frame.buffer(), buffer1, buffer2, transformMat, curStateData->woManager->getBaseTransformMat(), nextStateData->woManager->getBaseTransformMat(), timeStep, _context);
				updateWo(transformMat, woWriter, frame.buffer());

#if debug
				std::cout << "current state " << curStateData->data->stateName << " " << curStateData->timeManager->localTime << std::endl;
				std::cout << "next state " << nextStateData->data->stateName << " " << nextStateData->timeManager->localTime << std::endl;

				// creating two extra characters just to check out the blending
				std::string name1 = std::string(character->getName()) + "1";
				SbmCharacter* c1 = mcu.getCharacter(name1);
				if (!c1)
				{
					std::string command = "char " + name1 + " init common.sk";
					mcu.execute((char *)command.c_str());
					std::string command1 = "char " + name1 + " mesh brad";
					mcu.execute((char *)command1.c_str());
				}
				std::string name2 = std::string(character->getName()) + "2";
				SbmCharacter* c2 = mcu.getCharacter(name2);
				if (!c2)
				{
					std::string command = "char " + name2 + " init common.sk";
					mcu.execute((char *)command.c_str());
					std::string command1 = "char " + name2 + " mesh brad";
					mcu.execute((char *)command1.c_str());
				}
				if (c1 && c2)
				{
					// world offset translation
					int woPosChanId = _context->channels().search("world_offset", SkChannel::XPos);
					int woPosBufferId = _context->toBufferIndex(woPosChanId);

					std::stringstream command1;
					command1 << "set character " << c1->getName() << " world_offset x " << woWriter->get_data()[0] + 100 << " y " << woWriter->get_data()[1] << " z " << woWriter->get_data()[2];
					std::stringstream command2;
					command2 << "set character " << c2->getName() << " world_offset x " << woWriter->get_data()[0] + 200 << " y " << woWriter->get_data()[1] << " z " << woWriter->get_data()[2];
					mcu.execute((char *)command1.str().c_str());
					mcu.execute((char *)command2.str().c_str());				

					// other joints
					for (int i = 0; i < (int)character->getSkeleton()->joints().size(); i++)
					{
						int chanId = _context->channels().search(character->getSkeleton()->joints()[i]->name(), SkChannel::Quat);
						int bufferId = _context->toBufferIndex(chanId);
						std::stringstream command1;
						command1 << "receiver skeleton " << c1->getName() << " other rotation " << character->getSkeleton()->joints()[i]->name() << " " 
								 << buffer1[bufferId + 0] << " " << buffer1[bufferId + 1] << " " << buffer1[bufferId + 2] << " " << buffer1[bufferId + 3];
						std::stringstream command2;
						command2 << "receiver skeleton " << c2->getName() << " other rotation " << character->getSkeleton()->joints()[i]->name() << " " 
								 << buffer2[bufferId + 0] << " " << buffer2[bufferId + 1] << " " << buffer2[bufferId + 2] << " " << buffer2[bufferId + 3];
						mcu.execute((char *)command1.str().c_str());
						mcu.execute((char *)command2.str().c_str());
					}
				}
#endif
				mcu.mark("locomotion");
				return true;
			}
			else
			{
				delete transitionManager;
				transitionManager = NULL;
				delete curStateData;
				curStateData = nextStateData;
				nextStateData = NULL;
			}			
		}
	}

	if (curStateData && curStateData->state->stateName != "PseudoIdle")
	{
		if (curStateData->active)
		{
//			if (curStateData->data->stateName != PseudoIdleState)
//				std::cout << "current state " << curStateData->data->stateName << " " << curStateData->timeManager->localTime << std::endl;
			curStateData->evaluate(timeStep, frame.buffer());
			updateWo(curStateData->woManager->getBaseTransformMat(), woWriter, frame.buffer());
			if (controllerBlending)
				if (controllerBlending->getKey(t) > 0.0)					PATransitionManager::bufferBlending(frame.buffer(), controllerBlending->getBuffer(), frame.buffer(), controllerBlending->getKey(t), _context);			return true;
		}
		else
		{
			delete curStateData;
			curStateData = NULL;
		}
	}
	mcu.mark("locomotion");
	return true;
}

void MeCtParamAnimation::setBaseJointName(const std::string& name)
{
	baseJointName = name;
}

const std::string& MeCtParamAnimation::getBaseJointName()
{
	return baseJointName;
}

void MeCtParamAnimation::dumpScheduling()
{
	if (curStateData)
		LOG("Current State: %s", curStateData->state->stateName.c_str());
	if (nextStateData)
		LOG("Next State: %s", nextStateData->state->stateName.c_str());
	LOG("Number of states to be scheduled: %d", waitingList.size());
	std::list<ScheduleUnit>::iterator iter = waitingList.begin();
	for (; iter != waitingList.end(); iter++)
	{
		if (iter->data)
			LOG("* %s", iter->data->stateName.c_str());
		else
			LOG("* Idle State");
	}
}

void MeCtParamAnimation::schedule(PAState* stateData, const std::vector<double>& weights, bool l, bool pn, bool a, std::string name)
{
	ScheduleUnit unit;
	SmartBody::SBAnimationState* animState = dynamic_cast<SmartBody::SBAnimationState*>(stateData);
	if (animState)
	{
		animState->validateState(); // to make sure the animaion state is valid before schedule it
	}
	unit.weights = weights;
	unit.data = stateData;
	unit.loop = l;
	unit.playNow = pn;
	unit.time = mcuCBHandle::singleton().time;
	unit.additive = a;
	unit.partialJoint = name;
	waitingList.push_back(unit);
}

void MeCtParamAnimation::unschedule()
{
	reset();
}

void MeCtParamAnimation::updateWeights(std::vector<double>& w)
{
	if (curStateData == NULL)
		return;
	if (curStateData->state->getNumMotions() != w.size())
		return;

	double wCheck = 0.0;
	for (size_t i = 0; i < w.size(); i++)
		wCheck += w[i];
	if (fabs(wCheck - 1.0) > 0.1)
	{
		for (size_t i = 0; i < w.size(); i++)
			if (i == 0) w[i] = 1.0;
			else		w[i] = 0.0;
	}

	curStateData->weights = w;
	curStateData->timeManager->updateWeights();
	
	if (transitionManager)
		transitionManager->update();
}

void MeCtParamAnimation::updateWeights()
{
	if (curStateData == NULL)
		return;
	std::vector<double>& w = curStateData->weights;
	updateWeights(w);
}

int MeCtParamAnimation::getNumWeights()
{
	if (curStateData)
		return curStateData->interpolator->getNumMotions();
	else
		return 0;
}

const std::string& MeCtParamAnimation::getNextStateName()
{
	if (nextStateData)
		return nextStateData->state->stateName;
	else
		return m_emptyString;
}

const std::string& MeCtParamAnimation::getCurrentStateName()
{
	if (curStateData)
		if (curStateData->state)
			return curStateData->state->stateName;
		else
			return m_emptyString;
	else
		return m_emptyString;
}

PAStateData* MeCtParamAnimation::getCurrentPAStateData()
{
	if (curStateData)
		return curStateData;
	else
		return NULL;
}

bool MeCtParamAnimation::hasPAState(const std::string& name)
{
	if (getCurrentStateName() == name)
		return true;
	if (getNextStateName() == name)
		return true;
	std::list<ScheduleUnit>::iterator iter = waitingList.begin();
	for (; iter!= waitingList.end(); iter++)
	{
		if (iter->data)
			if (iter->data->stateName == name)
				return true;
	}
	return false;
}

bool MeCtParamAnimation::isIdle()
{
	if (getCurrentPAStateData() && 
		getCurrentPAStateData()->state &&
		getCurrentPAStateData()->state->stateName != PseudoIdleState)
		return false;
	if (getNextStateName() != "")
		return false;
	if (waitingList.size() != 0)
		return false;
	return true;
}

void MeCtParamAnimation::autoScheduling(double time)
{
	if (waitingList.size() == 0)
		return;

	if (transitionManager)
		return;

	ScheduleUnit nextUnit = waitingList.front();
	if (time < nextUnit.time)
		return;

	if (curStateData == NULL)
	{
		if (nextStateData)
			delete nextStateData;
		nextStateData = NULL;
		curStateData = createStateModule(nextUnit);
		if (!curStateData)
		{
			return;
		}
		curStateData->active = true;
#if PrintPADebugInfo
		LOG("State %s being scheduled.[ACTIVE]", curStateData->data->stateName.c_str());
#endif
		waitingList.pop_front();
		if (controllerBlending)
		{
			controllerBlending->addKey(time, 1.0);			
			controllerBlending->addKey(time + defaultTransition, 0.0);		
		}	
	}
	else
	{
		if (transitionManager)
			delete transitionManager;
		PATransition* data = NULL;
		if (nextUnit.data)
			data = mcuCBHandle::singleton().lookUpPATransition(curStateData->state->stateName, nextUnit.data->stateName);
		nextStateData = createStateModule(nextUnit);
		nextStateData->active = false;
		if (!data)
		{
			if (curStateData->state->stateName == "PseudoIdle")
				transitionManager = new PATransitionManager();
			else
			{
				if (nextStateData->playNow)
					transitionManager = new PATransitionManager();
				else
				{
					// check to see if the current local time cannot afford the defaultTransition time
					double actualTransitionTime = defaultTransition;
					if (curStateData->timeManager->localTime >= (curStateData->timeManager->getDuration() - defaultTransition))
						actualTransitionTime = curStateData->timeManager->getDuration() - curStateData->timeManager->localTime;
					transitionManager = new PATransitionManager(curStateData->timeManager->getDuration() - actualTransitionTime, actualTransitionTime);	
//					transitionManager = new PATransitionManager(curStateData->timeManager->getDuration() - defaultTransition);					
				}
			}
#if PrintPADebugInfo
		LOG("State %s being scheduled.[ACTIVE]", curStateData->data->stateName.c_str());
#endif
		}
		else
		{
			transitionManager = new PATransitionManager(data, curStateData, nextStateData);
			nextStateData->timeManager->updateLocalTimes(transitionManager->s2);
#if PrintPADebugInfo
		LOG("State %s being scheduled.[NOT ACTIVE]", nextStateData->data->stateName.c_str());
#endif
		}
		waitingList.pop_front();
	}
}

PAStateData* MeCtParamAnimation::createStateModule(ScheduleUnit su)
{
	PAStateData* module = NULL;
	if (su.data)
	{
		module = new PAStateData(su.data,  su.weights, su.loop, su.playNow);
		module->interpolator->setAdditiveMode(su.additive);
		std::vector<std::string> joints;
		SkJoint* j = character->getSkeleton()->search_joint(su.partialJoint.c_str());
		if (j)
		{
			std::vector<SkJoint*> jVec;
			SkJoint::recursive_children(jVec, j);
			for (size_t i = 0; i < jVec.size(); i++)
			{
				joints.push_back(jVec[i]->name());
			}
		}

		module->interpolator->setBlendingJoints(joints);
	}
	else
	{
		module = new PAStateData("PseudoIdle",  su.weights, su.loop, su.playNow);
		module->playNow = su.playNow;
	}
	if (_context)
	{
		module->interpolator->setMotionContextMaps(_context);
		module->interpolator->initChanId(_context, baseJointName);
		SkJoint* baseJoint = character->getSkeleton()->search_joint(baseJointName.c_str());
		if (!baseJoint)
			return NULL;
		module->interpolator->initPreRotation(baseJoint->quat()->prerot());
		module->woManager->setMotionContextMaps(_context);
		module->woManager->initChanId(_context, baseJointName);
		SrQuat preRot = character->getSkeleton()->search_joint(baseJointName.c_str())->quat()->prerot();
		//LOG("prerot axis = %f %f %f, angle = %f",preRot.axis()[0],preRot.axis()[1],preRot.axis()[2], preRot.angle());
		module->woManager->initPreRotation(preRot);
	}
	else
		return NULL;
	return module;
}

void MeCtParamAnimation::reset()
{
	if (curStateData)
		delete curStateData;
	curStateData = NULL;
	if (nextStateData)
		delete nextStateData;
	nextStateData = NULL;
	transitionManager = NULL;
	waitingList.clear();	
	delete controllerBlending;
	controllerBlending = NULL;}

void MeCtParamAnimation::updateWo(SrMat&mat, MeCtChannelWriter* woWriter, SrBuffer<float>& buffer)
{
	// get current woMat
	SrBuffer<float>& woValue = woWriter->get_data();
	SrMat currentWoMat;
	SrQuat quat;
	quat.w = woValue[3];
	quat.x = woValue[4];
	quat.y = woValue[5];
	quat.z = woValue[6];
	quat.get_mat(currentWoMat);
	currentWoMat.set(12, woValue[0]);
	currentWoMat.set(13, woValue[1]);
	currentWoMat.set(14, woValue[2]);	

	// separate transform matrix to rotation and transltion matrix
	SrQuat q = SrQuat(mat);
	SrMat rot;
	q.get_mat(rot);
	SrMat translate;
	translate.set(12, mat.get(12));
	translate.set(14, mat.get(14));

	// apply rotation and transition matrix perspectively
	SrQuat woQuat = SrQuat(currentWoMat);
	SrMat currentRot;
	woQuat.get_mat(currentRot);
	SrMat newTranslate = translate * currentRot;
	SrMat newWoMat =  rot * currentRot;

	// set new wo mat back to skeleton and woWriter
	quat = SrQuat(newWoMat);
	woValue[3] = quat.w;
	woValue[4] = quat.x;
	woValue[5] = quat.y;
	woValue[6] = quat.z;
	woValue[0] = newTranslate.get(12) + currentWoMat.get(12);
	woValue[1] = newTranslate.get(13) + currentWoMat.get(13);
	woValue[2] = newTranslate.get(14) + currentWoMat.get(14);
	gwiz::quat_t skelQ = gwiz::quat_t(woValue[3], woValue[4], woValue[5], woValue[6]);
	gwiz::euler_t skelE = gwiz::euler_t(skelQ);
	character->set_world_offset(woValue[0], woValue[1], woValue[2], (float)skelE.h(), (float)skelE.p(), (float)skelE.r());
}

void MeCtParamAnimation::controllerEaseOut(double t)
{
	if (curStateData->state->cycle)
		return;

	if (controllerBlending->getKey(t) > 0.0)
		return;

	double toStateEnd = curStateData->timeManager->getDuration() - curStateData->timeManager->localTime;
	if (toStateEnd < defaultTransition)
	{
		controllerBlending->addKey(t, 1.0);
		controllerBlending->addKey(t + toStateEnd, 0.0);
	}
	
}