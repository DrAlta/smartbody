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

const char* MeCtParamAnimation::Context::CONTEXT_TYPE = "MeCtParamAnimation::Context";
const char* MeCtParamAnimation::CONTROLLER_TYPE = "MeCtParamAnimation";

void MeCtParamAnimation::Context::child_channels_updated( MeController* child )
{
}

MeCtParamAnimation::MeCtParamAnimation(SbmCharacter* c, MeCtChannelWriter* wo) : MeCtContainer(new MeCtParamAnimation::Context(this)), character(c), woWriter(wo)
{
	baseJointName = "base";
	curStateModule = NULL;
	nextStateModule = NULL;
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
	double timeStep = t - prevGlobalTime;
	prevGlobalTime = t;
	autoScheduling(t);
	controllerBlending->updateBuffer(frame.buffer());

	//--make sure there is always a pseudo idle state running in the system
	if (!curStateModule && !nextStateModule)
		schedule(NULL, true, true);
	if (curStateModule)
	{
		if (curStateModule->data->stateName != PseudoIdleState && nextStateModule == NULL && !curStateModule->loop)
			schedule(NULL, true);
	}
	//------

	if (transitionManager)
	{
		if (curStateModule == NULL || nextStateModule == NULL)
		{
			LOG("MeCtParamAnimation::controller_evaluate ERR!");
			reset();
			return false;
		}

		if (!transitionManager->blendingMode)
			transitionManager->align(curStateModule, nextStateModule);
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
				curStateModule->evaluate(timeStep * transitionManager->getSlope(), buffer1);
				nextStateModule->evaluate(timeStep, buffer2);
				transitionManager->blending(frame.buffer(), buffer1, buffer2, transformMat, curStateModule->woManager->getBaseTransformMat(), nextStateModule->woManager->getBaseTransformMat(), timeStep, _context);
				updateWo(transformMat, woWriter, frame.buffer());
				return true;
			}
			else
			{
				delete transitionManager;
				transitionManager = NULL;
				delete curStateModule;
				curStateModule = nextStateModule;
				nextStateModule = NULL;
			}			
		}
	}

	if (curStateModule)
	{
		if (curStateModule->active)
		{
			curStateModule->evaluate(timeStep, frame.buffer());
			updateWo(curStateModule->woManager->getBaseTransformMat(), woWriter, frame.buffer());
			if (controllerBlending->getKey(t) > 0.0)
				PATransitionManager::bufferBlending(frame.buffer(), controllerBlending->getBuffer(), frame.buffer(), controllerBlending->getKey(t), _context);
			return true;
		}
		else
		{
			delete curStateModule;
			curStateModule = NULL;
		}
	}
	return true;
}

void MeCtParamAnimation::setBaseJointName(std::string name)
{
	baseJointName = name;
}

std::string MeCtParamAnimation::getBaseJointName()
{
	return baseJointName;
}

void MeCtParamAnimation::dumpScheduling()
{
	if (curStateModule)
		LOG("Current State: %s", curStateModule->data->stateName.c_str());
	if (nextStateModule)
		LOG("Next State: %s", nextStateModule->data->stateName.c_str());
	LOG("Number of states to be scheduled: %d", waitingList.size());
	std::list<ScheduleUnit>::iterator iter = waitingList.begin();
	for (; iter != waitingList.end(); iter++)
		LOG("* %s", iter->data->stateName.c_str());
}

void MeCtParamAnimation::schedule(PAStateData* stateData, bool l, bool pn)
{
	ScheduleUnit unit;
	unit.data = stateData;
	unit.loop = l;
	unit.playNow = pn;
	unit.time = mcuCBHandle::singleton().time;
	waitingList.push_back(unit);
}

void MeCtParamAnimation::unschedule()
{
	reset();
}

void MeCtParamAnimation::updateWeights(std::vector<double> w)
{
	if (curStateModule == NULL)
		return;
	if (curStateModule->data->getNumMotions() != w.size())
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

	curStateModule->timeManager->updateWeights(w);
	curStateModule->interpolator->weights = w;
	curStateModule->woManager->weights = w;
	curStateModule->data->weights = w;
	if (transitionManager)
		transitionManager->update();
}

void MeCtParamAnimation::updateWeights()
{
	if (curStateModule == NULL)
		return;
	std::vector<double> w = curStateModule->data->weights;
	updateWeights(w);
}

int MeCtParamAnimation::getNumWeights()
{
	if (curStateModule)
		return curStateModule->interpolator->getNumMotions();
	else
		return 0;
}

std::string MeCtParamAnimation::getCurrentStateName()
{
	if (curStateModule)
		return curStateModule->data->stateName;
	else
		return "";
}

std::string MeCtParamAnimation::getNextStateName()
{
	if (nextStateModule)
		return nextStateModule->data->stateName;
	else
		return "";
}

PAStateData* MeCtParamAnimation::getCurrentPAStateData()
{
	if (curStateModule)
		return curStateModule->data;
	else
		return NULL;
}

bool MeCtParamAnimation::hasPAState(std::string name)
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
	if (getCurrentStateName() != PseudoIdleState)
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

	if (curStateModule == NULL)
	{
		if (nextStateModule)
			delete nextStateModule;
		nextStateModule = NULL;
		curStateModule = createStateModule(nextUnit.data, nextUnit.loop, nextUnit.playNow);
		curStateModule->active = true;
#if PrintPADebugInfo
		LOG("State %s being scheduled.[ACTIVE]", curStateModule->data->stateName.c_str());
#endif
		waitingList.pop_front();
		controllerBlending->addKey(time, 1.0);
		controllerBlending->addKey(time + defaultTransition, 0.0);
	}
	else
	{
		if (transitionManager)
			delete transitionManager;
		PATransitionData* data = NULL;
		if (nextUnit.data)
			data = mcuCBHandle::singleton().lookUpPATransition(curStateModule->data->stateName, nextUnit.data->stateName);
		nextStateModule = createStateModule(nextUnit.data, nextUnit.loop, nextUnit.playNow);
		nextStateModule->active = false;
		if (!data)
		{
			PseudoPAStateModule* pseudo = dynamic_cast<PseudoPAStateModule*> (curStateModule);
			if (pseudo)
				transitionManager = new PATransitionManager();
			else
			{
				if (nextStateModule->playNow)
					transitionManager = new PATransitionManager();
				else
				{
					// check to see if the current local time cannot afford the defaultTransition time
					double actualTransitionTime = defaultTransition;
					if (curStateModule->timeManager->localTime >= (curStateModule->timeManager->getDuration() - defaultTransition))
						actualTransitionTime = curStateModule->timeManager->getDuration() - curStateModule->timeManager->localTime;
					transitionManager = new PATransitionManager(curStateModule->timeManager->getDuration() - actualTransitionTime, actualTransitionTime);	
//					transitionManager = new PATransitionManager(curStateModule->timeManager->getDuration() - defaultTransition);					
				}
			}
#if PrintPADebugInfo
		LOG("State %s being scheduled.[ACTIVE]", curStateModule->data->stateName.c_str());
#endif
		}
		else
		{
			transitionManager = new PATransitionManager(data, curStateModule->data, nextStateModule->data);
			nextStateModule->timeManager->updateLocalTimes(transitionManager->s2);
#if PrintPADebugInfo
		LOG("State %s being scheduled.[NOT ACTIVE]", nextStateModule->data->stateName.c_str());
#endif
		}
		waitingList.pop_front();
	}
}

PAStateModule* MeCtParamAnimation::createStateModule(PAStateData* stateData, bool l, bool pn)
{
	PAStateModule* module = NULL;
	if (stateData)
		module = new PAStateModule(stateData, l, pn);
	else
		module = new PseudoPAStateModule();
	if (_context)
	{
		module->interpolator->setMotionContextMaps(_context);
		module->interpolator->initChanId(_context, baseJointName);
		module->woManager->setMotionContextMaps(_context);
		module->woManager->initChanId(_context, baseJointName);
	}
	else
		return NULL;
	return module;
}

void MeCtParamAnimation::reset()
{
	if (curStateModule)
		delete curStateModule;
	curStateModule = NULL;
	if (nextStateModule)
		delete nextStateModule;
	nextStateModule = NULL;
	transitionManager = NULL;
	waitingList.clear();	
}

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
	if (curStateModule->data->cycle)
		return;

	if (controllerBlending->getKey(t) > 0.0)
		return;

	double toStateEnd = curStateModule->timeManager->getDuration() - curStateModule->timeManager->localTime;
	if (toStateEnd < defaultTransition)
	{
		controllerBlending->addKey(t, 1.0);
		controllerBlending->addKey(t + toStateEnd, 0.0);
	}
	
}