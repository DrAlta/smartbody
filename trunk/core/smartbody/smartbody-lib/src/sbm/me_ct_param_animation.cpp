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
#include <sr/sr_euler.h>

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

	// auto scheduling
	autoScheduling(t);
	
	// make sure there is always a pseudo idle state running in the system
	if ((!curStateData && !nextStateData) ||
		!curStateData)
	{
		if (!hasPAState(PseudoIdleState))
		{
			std::vector<double> weights;
			schedule(NULL, weights);
			mcu.mark("locomotion");
			return true;
		}
	}	
	if (curStateData)
	{
		if (curStateData->state &&
			curStateData->state->stateName != PseudoIdleState && 
			nextStateData == NULL && (curStateData->wrapMode == PAStateData::Once))
		{
			if (!hasPAState(PseudoIdleState))
			{
				std::vector<double> weights;
				schedule(NULL, weights);
				return true;
			}
		}
			
	}
	//------

	// dealing with transition
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

				SrMat curBaseMat;
				SrMat nextBaseMat;

				bool pseudoTransition = (curStateData->getStateName() == PseudoIdleState || nextStateData->getStateName() == PseudoIdleState);								
				if (pseudoTransition && (!curStateData->isPartialBlending() && !nextStateData->isPartialBlending())) // first time && not partial blending case
				{
					bool transitionIn = (curStateData->getStateName() == PseudoIdleState);
					curStateData->evaluateTransition(timeStep * transitionManager->getSlope(), buffer1, transitionIn);
					nextStateData->evaluateTransition(timeStep, buffer2, transitionIn);
					curBaseMat = curStateData->woManager->getBaseTransformMat();
					nextBaseMat = nextStateData->woManager->getBaseTransformMat();

					if (!transitionIn) // update base rotation & translation
					{
						SrMat origBase = curStateData->woManager->getBaseMatFromBuffer(buffer1);//combineMat(curStateData->woManager->getBaseMatFromBuffer(buffer1),curBaseMat);
						SrMat newBuffBase = origBase;
						newBuffBase = origBase*nextStateData->woManager->getCurrentBaseTransformMat();
						curStateData->woManager->setBufferByBaseMat(newBuffBase,buffer1);	
						curBaseMat = SrMat();//nextBaseMat.inverse();//SrMat();//curBaseMat*nextStateData->woManager->getBaseTransformMat().inverse();
						nextBaseMat = SrMat();//nextBaseMat.inverse();//SrMat();//nextBaseMat.inverse();
					}

					if (!transitionManager->startTransition)
					{
						SrVec pos;
						if (transitionIn)
						{
							// update base transform mat
							curBaseMat = curStateData->woManager->getCurrentBaseTransformMat();
							pos = curBaseMat.get_translation();
							//LOG("startTransistion In, curBase = %f %f %f",pos[0],pos[1],pos[2]);
						}
						else
						{							
							curBaseMat = nextStateData->woManager->getCurrentBaseTransformMat().inverse();//SrMat();//nbMat.inverse();							
							pos = curBaseMat.get_translation();
							SrVec origPos = nextStateData->woManager->getCurrentBaseTransformMat().get_translation();
						}
						transitionManager->startTransition = true;
					}					
				}
				else // proceed as usual
				{
					curStateData->evaluate(timeStep * transitionManager->getSlope(), buffer1);
					nextStateData->evaluate(timeStep, buffer2);
					curBaseMat = curStateData->woManager->getBaseTransformMat();
					nextBaseMat = nextStateData->woManager->getBaseTransformMat();
				}			
				transitionManager->blending(frame.buffer(), buffer1, buffer2, transformMat, curBaseMat, nextBaseMat, timeStep, _context);
				if (!curStateData->isPartialBlending() && !nextStateData->isPartialBlending())
					updateWo(transformMat, woWriter, frame.buffer());
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

	// if there's only one state, update current state
	if (curStateData && curStateData->state->stateName != PseudoIdleState)
	{
		if (curStateData->active)
		{
			curStateData->evaluate(timeStep, frame.buffer());
			if (!curStateData->isPartialBlending())
				updateWo(curStateData->woManager->getBaseTransformMat(), woWriter, frame.buffer());
			return true;
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

void MeCtParamAnimation::schedule(PAState* state, double x, double y, double z, PAStateData::WrapMode wrap, PAStateData::ScheduleMode schedule, PAStateData::BlendMode blend, std::string jName, double timeOffset)
{
	std::vector<double> weights;
	weights.resize(state->getNumMotions());
	SmartBody::SBAnimationState0D* state0D = dynamic_cast<SmartBody::SBAnimationState0D*>(state);
	if (state0D)
	{
		if (state->getNumMotions() > 0)
			weights[0] = 1.0f;
	}
	else
	{
		SmartBody::SBAnimationState1D* state1D = dynamic_cast<SmartBody::SBAnimationState1D*>(state);
		if (state1D)
		{
			state->getWeightsFromParameters(x, weights);
		}
		else
		{
			SmartBody::SBAnimationState2D* state2D = dynamic_cast<SmartBody::SBAnimationState2D*>(state);
			if (state2D)
			{
				state->getWeightsFromParameters(x, y, weights);
			}
			else
			{
				SmartBody::SBAnimationState3D* state3D = dynamic_cast<SmartBody::SBAnimationState3D*>(state);
				if (state3D)
				{
					state->getWeightsFromParameters(x, y, z, weights);
				}
				else
				{
					LOG("Unknown state type. What is this?");
				}
			}
		}
	}
	this->schedule(state, weights, wrap, schedule, blend, jName, timeOffset);
}

void MeCtParamAnimation::schedule(PAState* stateData, const std::vector<double>& weights, PAStateData::WrapMode wrap, PAStateData::ScheduleMode schedule, PAStateData::BlendMode blend, std::string jName, double timeOffset)
{
	ScheduleUnit unit;
	SmartBody::SBAnimationState* animState = dynamic_cast<SmartBody::SBAnimationState*>(stateData);
	if (animState)
	{
		animState->validateState(); // to make sure the animaion state is valid before schedule it
	}

	// schedule
	unit.weights = weights;
	unit.data = stateData;
	unit.wrap = wrap;
	unit.schedule = schedule;
	unit.blend = blend;
	unit.partialJoint = jName;
	unit.time = mcuCBHandle::singleton().time + timeOffset;

	// make sure the weights are valid for non-pseudoidle state
	if (unit.weights.size() == 0 && unit.data != NULL)
	{
		if (unit.data->stateName != PseudoIdleState)
		{
			LOG("MeCtParamAnimation::schedule Warning: state %s has no weights assigned.", unit.data->stateName.c_str());
			unit.weights.resize(unit.data->getNumMotions());
			for (int i = 0; i < unit.data->getNumMotions(); i++)
			{
				if (i == 0)
					unit.weights[i] = 1.0;
				else
					unit.weights[i] = 0.0;
			}
		}
	}

	// make sure there's motion for non-pseudoidle state
	if (unit.data != NULL)
	{
		if (unit.data->stateName != PseudoIdleState && unit.data->getNumMotions() == 0)
		{
			LOG("MeCtParamAnimation::schedule ERR: state %s has no motions attached.", unit.data->stateName.c_str());
			return;
		}
	}

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
		{
			if (iter->data->stateName == name)
				return true;
		}
		else
		{
			if (name == PseudoIdleState)
				return true;
		}

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

	// if current state is pseudo idle & next state is also pseudo idle, ignore
	if (curStateData != NULL)
	{
		if (curStateData->getStateName() == PseudoIdleState)
		{
			if (nextUnit.data == NULL)
				return;
			else
				if (nextUnit.data->stateName == PseudoIdleState)
					return;
		}
	}

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
		waitingList.pop_front();
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
			if (curStateData->state->stateName == PseudoIdleState)
				transitionManager = new PATransitionManager();
			else
			{
				if (nextStateData->scheduleMode == PAStateData::Now)
					transitionManager = new PATransitionManager();
				else
				{
					// check to see if the current local time cannot afford the defaultTransition time
					double actualTransitionTime = defaultTransition;
					if (curStateData->timeManager->localTime >= (curStateData->timeManager->getDuration() - defaultTransition))
						actualTransitionTime = curStateData->timeManager->getDuration() - curStateData->timeManager->localTime;
					transitionManager = new PATransitionManager(curStateData->timeManager->getDuration() - actualTransitionTime, actualTransitionTime);	
				}
			}
		}
		else
		{
			transitionManager = new PATransitionManager(data, curStateData, nextStateData);
			nextStateData->timeManager->updateLocalTimes(transitionManager->s2);
		}
		waitingList.pop_front();
	}
}

PAStateData* MeCtParamAnimation::createStateModule(ScheduleUnit su)
{
	PAStateData* module = NULL;
	if (su.data)
	{
		module = new PAStateData(su.data, su.weights, su.blend, su.wrap, su.schedule);
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
		module = new PAStateData(PseudoIdleState, su.weights);
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
}

void MeCtParamAnimation::updateWo(SrMat& mat, MeCtChannelWriter* woWriter, SrBuffer<float>& buffer)
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

	SkJoint* baseJ = character->_skeleton->search_joint(baseJointName.c_str());
	SrVec offset;
	if (baseJ) offset = baseJ->offset(); offset.y = 0.f;
	SrMat negOffset; negOffset.set_translation(-offset);
	SrMat posOffset; posOffset.set_translation(offset);
	//SrMat newWoMat =  rot * currentRot;
	//SrMat newTranslate = translate * currentRot;	
	//SrMat newTranslate = translate * newWoMat;	
	SrMat nextMat = negOffset * mat * posOffset * currentWoMat;
	SrVec newTran = nextMat.get_translation();

	// set new wo mat back to skeleton and woWriter
	SrVec bufferTran = newTran;
	quat = SrQuat(nextMat);
	woValue[3] = quat.w;
	woValue[4] = quat.x;
	woValue[5] = quat.y;
	woValue[6] = quat.z;
	woValue[0] = bufferTran[0];
	woValue[1] = bufferTran[1];
	woValue[2] = bufferTran[2];
	gwiz::quat_t skelQ = gwiz::quat_t(woValue[3], woValue[4], woValue[5], woValue[6]);
	gwiz::euler_t skelE = gwiz::euler_t(skelQ);	
	character->set_world_offset(woValue[0], woValue[1], woValue[2], (float)skelE.h(), (float)skelE.p(), (float)skelE.r());

	JointChannelId baseChanID, baseBuffId;
	baseChanID.x = _context->channels().search(SbmPawn::WORLD_OFFSET_JOINT_NAME, SkChannel::XPos);
	baseChanID.y = _context->channels().search(SbmPawn::WORLD_OFFSET_JOINT_NAME, SkChannel::YPos);
	baseChanID.z = _context->channels().search(SbmPawn::WORLD_OFFSET_JOINT_NAME, SkChannel::ZPos);
	baseChanID.q = _context->channels().search(SbmPawn::WORLD_OFFSET_JOINT_NAME, SkChannel::Quat);

	baseBuffId.x = _context->toBufferIndex(baseChanID.x);
	baseBuffId.y = _context->toBufferIndex(baseChanID.y);
	baseBuffId.z = _context->toBufferIndex(baseChanID.z);	
	baseBuffId.q = _context->toBufferIndex(baseChanID.q);	

	buffer[baseBuffId.x] = bufferTran[0];
	buffer[baseBuffId.y] = bufferTran[1];
	buffer[baseBuffId.z] = bufferTran[2];	
	for (int k = 0; k < 4; k++)
		buffer[baseBuffId.q + k] = quat.getData(k);
}


SrMat MeCtParamAnimation::combineMat( SrMat& mat1, SrMat& mat2 )
{
	SrMat newMat = mat1*mat2;
	SrVec newTranslation = mat1.get_translation() + mat2.get_translation();
	newMat.set_translation(newTranslation);
	return newMat;	
}