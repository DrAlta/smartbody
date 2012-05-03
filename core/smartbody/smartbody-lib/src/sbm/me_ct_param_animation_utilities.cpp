/*
 *  me_ct_param_animation_utilities.cpp - part of Motion Engine and SmartBody-lib
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

#include "me_ct_param_animation_utilities.h"
#include <sbm/gwiz_math.h>
#include <sbm/mcontrol_util.h>
#include <sr/sr_euler.h>
#include <sbm/SBAnimationState.h>

const double timeThreshold = 0.05;

PATimeManager::PATimeManager()
{
}

PATimeManager::PATimeManager(PAStateData* data)
{
	stateData = data;

	if (stateData->state->keys.size() > 0)
	{
		if (stateData->state->keys[0].size() > 0)
			for (size_t i = 0; i < stateData->state->keys.size(); i++)
				localTimes.push_back(stateData->state->keys[i][0]);
	}
	else
	{
		SmartBody::SBAnimationState0D* state0D = dynamic_cast<SmartBody::SBAnimationState0D*>(stateData->state);
		if (state0D)
		{
			localTimes.push_back(0);
		}
		else
		{
			LOG("State %s has no keys, setting all local times to zero.", stateData->state->stateName.c_str());
			for (int x = 0; x < stateData->state->getNumMotions(); x++)
			{
				localTimes.push_back(0);
			}

		}
	}
	if (stateData->weights.size() > localTimes.size())
	{
		LOG("Problem: fewer keys (%d) than weights (%d)", stateData->state->keys.size(), stateData->weights.size());
	}
	setMotionTimes();
	setKey();
	setLocalTime();
	loadEvents();
}

PATimeManager::~PATimeManager()
{
}

int PATimeManager::getNumKeys()
{
	return key.size();
}

void PATimeManager::updateLocalTimes(double time)
{
	localTime = time;
	prevLocalTime = localTime;
	getParallelTimes(localTime, localTimes);
	setMotionTimes();
}


bool PATimeManager::step(double timeStep)
{
	bool notReachDuration = true;
	std::vector<double> prevMotionTimes;
	prevMotionTimes.resize(motionTimes.size());
	std::copy(motionTimes.begin(),motionTimes.end(),prevMotionTimes.begin());

	prevLocalTime = localTime;
	double newLocalTime = localTime + timeStep;
	int loopcounter = 0;
	double lastKey = 0;
	if (stateData->state->keys.size() > 0)
		lastKey = key[key.size() - 1];
	if (stateData->state->keys.size() > 0 && 
		(newLocalTime > lastKey && lastKey >= 0.0))
	{
		double d = key[key.size() - 1] - key[0];
		int times = (int) (newLocalTime / d);
		if (times > 0)
		{
			newLocalTime = newLocalTime - float(times) * d;
			loadEvents();
		}
		notReachDuration = false;
	}

	localTime = newLocalTime;
	getParallelTimes(localTime, localTimes);
	if (localTimes.size() == 0)
	{
		LOG("PATimeManager::step ERROR: Miss the timing.");
		return notReachDuration;
	}

	int numMotionTimes = motionTimes.size();
	setMotionTimes();
	checkEvents();
	if (motionTimes.size() == numMotionTimes)
	{
		timeDiffs.clear();
		for (size_t i = 0; i < stateData->weights.size(); i++)
			timeDiffs.push_back(motionTimes[i] - prevMotionTimes[i]);
	}
	else
	{
		LOG("Motion times do not match! Please check!");
	}

	return notReachDuration;
}


void PATimeManager::loadEvents()
{
	// add the event instances to this controller
	while (!_events.empty())
		_events.pop();


	std::vector<std::pair<SmartBody::MotionEvent*, int> >& events = stateData->state->getEvents();
	for (size_t x = 0; x < events.size(); x++)
	{
		_events.push(events[x]);
	}
}

void PATimeManager::checkEvents()
{
	int motionIndex = 0;
	if (motionIndex >= (int) localTimes.size())
		return;

	while (!_events.empty())
	{
		std::pair<SmartBody::MotionEvent*, int>& event = _events.front();
		// localTime is the parameterized time, determine the local time of the event
		if (event.first->isEnabled() && localTimes[motionIndex] >= event.first->getTime())
		{
			SmartBody::EventManager* manager = EventManager::getEventManager();
			manager->handleEvent(event.first, localTimes[motionIndex]);
			std::string type = event.first->getType();
			std::string params = event.first->getParameters();
			//LOG("EVENT: %f %s %s", time, type.c_str(), params.c_str());
			_events.pop();
		}
		else
		{
			return;
		}
	}
}


void PATimeManager::updateWeights()
{
	setKey();
	setLocalTime();
}

double PATimeManager::getDuration()
{
	return (key[getNumKeys() - 1] - key[0]);
}

void PATimeManager::setKey()
{
	key.clear();
	int numKeys = 0;
	if (stateData->weights.size() > 0) numKeys = stateData->state->keys[0].size();
	for (int i = 0; i < numKeys; i++)
	{
		double tempK = 0.0;
		for (size_t j = 0; j < stateData->weights.size(); j++)
			tempK += stateData->weights[j] * stateData->state->keys[j][i];
		key.push_back(tempK);
	}
}

void PATimeManager::setLocalTime()
{
	if (localTimes.size() < stateData->weights.size())
	{
		LOG("Problem with number of local times (%d) versus number of weights (%d)", localTimes.size(), stateData->weights.size());
		return;
	}
	localTime = 0.0;
	for (size_t i = 0; i < stateData->weights.size(); i++)
		localTime += stateData->weights[i] * localTimes[i];
	prevLocalTime = localTime;
}

void PATimeManager::setMotionTimes()
{
	motionTimes.clear();
	if (localTimes.size() > 0)
	{
		for (size_t i = 0; i < stateData->weights.size(); i++)
		{
			int d = (int) (localTimes[i] / stateData->state->motions[i]->duration());
			motionTimes.push_back(localTimes[i] - d * stateData->state->motions[i]->duration());
		}
	}
	
}

int PATimeManager::getSection(double time)
{
	for (int i = 0; i < getNumKeys() - 1; i++)
	{
		if (key[i] <= time && key[i + 1] > time)
			return i;
	}
	return -1;
}

void PATimeManager::getParallelTimes(double time, std::vector<double>& times)
{
	times.clear();
	int section = getSection(time);
	if (section < 0)
	{
		for (size_t i = 0; i < stateData->weights.size(); i++)
		{
			times.push_back(0);
		}
		return;
	}
	for (size_t i = 0; i < stateData->weights.size(); i++)
	{
		double t = stateData->state->keys[i][section] + (stateData->state->keys[i][section + 1] - stateData->state->keys[i][section]) * (time - key[section]) / (key[section + 1] - key[section]);
		times.push_back(t);
	}
}

std::vector<double>& PATimeManager::getKey()
{
	return key;
}

PAMotions::PAMotions()
{
}

PAMotions::PAMotions(PAStateData* data)
{
	stateData = data;
}

PAMotions::~PAMotions()
{
}


void PAMotions::setMotionContextMaps(MeControllerContext* context)
{
	SkChannelArray& cChannels = context->channels();
	for (size_t mId = 0; mId < stateData->state->motions.size(); mId++)
	{
		SkChannelArray& mChannels = stateData->state->motions[mId]->channels();
		const int size = mChannels.size();
		SrBuffer<int> map;
		map.size(size);
		for (int i = 0; i < size; i++)
		{
			int chanIndex = cChannels.search(mChannels.name(i), mChannels.type(i));
			map[i] = context->toBufferIndex(chanIndex);
		}
		motionContextMaps.push_back(map);
	}
	_context = context;
}

int PAMotions::getNumMotions()
{
	return stateData->state->motions.size();
}


void PAMotions::initChanId(MeControllerContext* context, std::string baseJointName)
{
	SkChannelArray& cChannels = context->channels();

	// base joint
	baseChanId.x = cChannels.search(baseJointName.c_str(), SkChannel::XPos);
	baseChanId.y = cChannels.search(baseJointName.c_str(), SkChannel::YPos);
	baseChanId.z = cChannels.search(baseJointName.c_str(), SkChannel::ZPos);
	baseChanId.q = cChannels.search(baseJointName.c_str(), SkChannel::Quat);
	baseBuffId.x = context->toBufferIndex(baseChanId.x);
	baseBuffId.y = context->toBufferIndex(baseChanId.y);
	baseBuffId.z = context->toBufferIndex(baseChanId.z);
	baseBuffId.q = context->toBufferIndex(baseChanId.q);
}

void PAMotions::initPreRotation(const SrQuat& q)
{
	basePrerot.w = q.w;
	basePrerot.x = q.x;
	basePrerot.y = q.y;
	basePrerot.z = q.z;
}


void PAMotions::getBuffer(SkMotion* motion, double t, SrBuffer<int>& map, SrBuffer<float>& buff)
{
	double deltaT = motion->duration() / double(motion->frames() - 1);
	int lastFrame = int (t/deltaT);
	motion->apply(float(t), &buff[0], &map, SkMotion::Linear, &lastFrame);
}


SrMat PAMotions::getBaseMatFromBuffer(SrBuffer<float>& buffer)
{
	SrQuat quat;
	quat.w = buffer[baseBuffId.q + 0];
	quat.x = buffer[baseBuffId.q + 1];
	quat.y = buffer[baseBuffId.q + 2];
	quat.z = buffer[baseBuffId.q + 3];
	SrMat mat;
	quat.get_mat(mat);
	mat.set(12, buffer[baseBuffId.x]);
	mat.set(13, buffer[baseBuffId.y]);
	mat.set(14, buffer[baseBuffId.z]);
	return mat;
}

void PAMotions::setBufferByBaseMat(SrMat& mat, SrBuffer<float>& buffer)
{
	SrQuat quat = SrQuat(mat);
	buffer[baseBuffId.x] = mat.get(12);
	buffer[baseBuffId.y] = mat.get(13);
	buffer[baseBuffId.z] = mat.get(14);
	buffer[baseBuffId.q + 0] = quat.w;
	buffer[baseBuffId.q + 1] = quat.x;
	buffer[baseBuffId.q + 2] = quat.y;
	buffer[baseBuffId.q + 3] = quat.z;	
}


void PAMotions::getUpdateMat(SrMat& dest, SrMat& src)
{
	SrQuat quat = SrQuat(src);

	SrMat prerotMat;
//	prerot.get_mat(prerotMat);
	basePrerot.get_mat(prerotMat);
/*
	SrMat src0 = src * prerotMat.inverse();
	SrMat mat0;
	float rx, ry, rz;
	sr_euler_angles(rotType, src0, rx, ry, rz);
	rx = 0.0;
	rz = 0.0;
	sr_euler_mat(rotType, mat0, rx, ry, rz);
	SrMat mat;
	mat = mat0;// * prerotMat;
	SrQuat quatP = SrQuat(mat);
*/
/*
	SrMat mat;
	float rx, ry, rz;
	sr_euler_angles(rotType, src, rx, ry, rz);
	rx = 0.0;
	rz = 0.0;
	sr_euler_mat(rotType, mat, rx, ry, rz);
	SrQuat quatP = SrQuat(mat);
*/
	SrVec vec = quat.axis() * quat.angle();
	SrVec vec1 = vec * basePrerot;//prerotMat.inverse();
// 	if (quat.angle() > 0.01)
// 	{		
// 		sr_out << "prerot = " << basePrerot << srnl;	
// 		sr_out << "vec = " << vec << srnl;
// 		sr_out << "vec1 = " << vec1 << srnl;
// 	}
	quat = SrQuat(vec1);
	SrMat mat;
	quat.get_mat(mat);
	float rx, ry, rz;
	sr_euler_angles(rotType, mat, rx, ry, rz);
	rx = 0.0;
	rz = 0.0;
	sr_euler_mat(rotType, mat, rx, ry, rz);
	SrQuat quatP = SrQuat(mat);

//	SrVec vec = quat.axis() * quat.angle();
//	SrVec vec1 = vec * prerotMat.inverse();
//	vec1.x = 0;
//	vec1.z = 0;
//	SrQuat quatP = SrQuat(vec1);

/*
	quat_t q = quat_t(quat.w, quat.x, quat.y, quat.z);
	euler_t e = euler_t(q);	
	e.p(0.0);
	e.r(0.0);
	q = quat_t(e);
	SrQuat quatP = SrQuat((float)q.w(), (float)q.x(), (float)q.y(), (float)q.z());
*/

	quatP.get_mat(dest);
	dest.set(12, src.get(12));
	dest.set(14, src.get(14));
}

void PAMotions::getProcessedMat(SrMat& dest, SrMat& src)
{
	SrQuat quat = SrQuat(src);

	SrMat prerotMat;
//	prerot.get_mat(prerotMat);
	basePrerot.get_mat(prerotMat);
/*
	SrMat src0 = src * prerotMat.inverse();
	SrMat mat0;
	float rx, ry, rz;
	sr_euler_angles(rotType, src0, rx, ry, rz);
	ry = 0.0;
	sr_euler_mat(rotType, mat0, rx, ry, rz);
	SrMat mat;
	mat = mat0 * prerotMat;
	SrQuat quatP = SrQuat(mat);
*/
	/*
	SrMat mat;
	float rx, ry, rz;
	sr_euler_angles(rotType, src, rx, ry, rz);
	ry = 0.0;
	sr_euler_mat(rotType, mat, rx, ry, rz);
	SrQuat quatP = SrQuat(mat);
	*/
	
	SrVec vec = quat.axis() * quat.angle();
	SrVec vec1 = vec * prerotMat;//prerotMat.inverse();
	quat = SrQuat(vec1);
	SrMat mat;
	quat.get_mat(mat);
	float rx, ry, rz;
	sr_euler_angles(rotType, mat, rx, ry, rz);
	ry = 0.0;
	sr_euler_mat(rotType, mat, rx, ry, rz);
	SrQuat quatP = SrQuat(mat);
	vec1 = quatP.axis() * quatP.angle();
	SrVec vec2 = vec1 * prerotMat.inverse();
	quatP = SrQuat(vec2);

/*
	quat_t q = quat_t(quat.w, quat.x, quat.y, quat.z);
	euler_t e = euler_t(q);
	e.h(0.0);
	q = quat_t(e);
	SrQuat quatP = SrQuat((float)q.w(), (float)q.x(), (float)q.y(), (float)q.z());
*/
	quatP.get_mat(dest);
	dest.set(13, src.get(13));
}

PAInterpolator::PAInterpolator()
{
}

PAInterpolator::PAInterpolator(PAStateData* data) : PAMotions(data)
{
}

PAInterpolator::~PAInterpolator()
{
}

void PAInterpolator::blending(std::vector<double>& times, SrBuffer<float>& buff)
{
	std::vector<int> indices;
	int numMotions = getNumMotions();
	for (int i = 0; i < numMotions; i++)
	{
		if (stateData->weights[i] != 0.0)
			indices.push_back(i);
	}

	SrBuffer<float> buffer;
	buffer.size(buff.size());

	if (indices.size() == 0 && stateData->getStateName() == PseudoIdleState)
	{
		handleBaseMatForBuffer(buff);
		return;
	}
	else if (indices.size() == 1)
	{
		buffer = buff;
		int id = indices[0];
		double time = times[id];
		getBuffer(stateData->state->motions[id], time, motionContextMaps[id], buffer);
		handleBaseMatForBuffer(buffer);
	}
	else
	{
		int numMotions = indices.size();
		if (numMotions == 0)
			return;
		std::vector<SrBuffer<float> > buffers;
		for (int i = 0; i < numMotions; i++)
		{
			buffers.push_back(SrBuffer<float>());
			SrBuffer<float>& b = buffers[buffers.size() - 1];
			b.size(buff.size());
			b = buff;
			getBuffer(stateData->state->motions[indices[i]], times[indices[i]], motionContextMaps[indices[i]], b);
			handleBaseMatForBuffer(b);
		}
		buffer = buffers[0];
		SkChannelArray& motionChan = stateData->state->motions[indices[0]]->channels();
		int chanSize = motionChan.size();
		for (int i = 0; i < chanSize; i++)
		{
			const std::string& chanName = motionChan.name(i);
			for (int j = 1; j < numMotions; j++)
			{
				if (stateData->motionIndex.size() != stateData->state->motions.size())
				{
					stateData->updateMotionIndices();
				}
				if (stateData->motionIndex.size() == 0)
					continue;

				if ((int)stateData->motionIndex[indices[j]].size() <= i)
					continue;

				int id = stateData->motionIndex[indices[j]][i];
				//int id = stateData->state->motions[indices[j]]->channels().search(chanName, motionChan[i].type);
				if (id < 0)
					continue;
				int buffId = motionContextMaps[indices[0]].get(i);
				if (buffId >= 0)
				{
					double prevWeight = 0.0;
					for (int k = 0; k < j; k++)
						prevWeight += stateData->weights[indices[k]];
					double w = prevWeight / (stateData->weights[indices[j]] + prevWeight);
					stateData->state->motions[indices[0]]->channels()[i].interp(&buffer[buffId], &buffer[buffId], &buffers[j][buffId], (float)(1 - w));
				}
			}
		}
	}

	if (joints.size() > 0)
	{
		for (size_t i = 0; i < joints.size(); i++)
		{
			JointChannelId chanId;
			JointChannelId buffId;
			chanId.q = _context->channels().search(joints[i].c_str(), SkChannel::Quat);
			if (chanId.q < 0)
				continue;

			buffId.q = _context->toBufferIndex(chanId.q);
			if (buffId.q < 0)
				continue;
			
			SrQuat origQ;
			origQ.w = buff[buffId.q + 0];
			origQ.x = buff[buffId.q + 1];
			origQ.y = buff[buffId.q + 2];
			origQ.z = buff[buffId.q + 3];
			SrQuat currQ;
			currQ.w = buffer[buffId.q + 0];
			currQ.x = buffer[buffId.q + 1];
			currQ.y = buffer[buffId.q + 2];
			currQ.z = buffer[buffId.q + 3];
			SrQuat finalQ;
			if (stateData->blendMode == PAStateData::Additive)
				finalQ = origQ * currQ;
			else
				finalQ = currQ;
			buff[buffId.q + 0] = finalQ.w;
			buff[buffId.q + 1] = finalQ.x;
			buff[buffId.q + 2] = finalQ.y;
			buff[buffId.q + 3] = finalQ.z;
		}
	}
	else
		buff = buffer;
}

void PAInterpolator::clearBlendingJoints()
{
	joints.clear();
}

void PAInterpolator::setBlendingJoints(std::vector<std::string>& j)
{
	joints = j;
}


void PAInterpolator::handleBaseMatForBuffer(SrBuffer<float>& buffer)
{
	SrMat baseMat = getBaseMatFromBuffer(buffer);
	SrMat processedBaseMat;
	getProcessedMat(processedBaseMat, baseMat);
	setBufferByBaseMat(processedBaseMat, buffer);
}

PAWoManager::PAWoManager()
{
}

PAWoManager::PAWoManager(PAStateData* data) : PAMotions(data)
{
	firstTime = true;
	for (int i = 0; i < getNumMotions(); i++)
	{
		SrMat mat;
		baseMats.push_back(mat);
	}
	intializeTransition = false;
}

PAWoManager::~PAWoManager()
{
}

void PAWoManager::apply(std::vector<double>& times, std::vector<double>& timeDiffs, SrBuffer<float>& buffer)
{
	std::vector<SrMat> currentBaseMats;
	getBaseMats(currentBaseMats, times, timeDiffs, buffer.size(), buffer);
	if (!firstTime)
	{
		std::vector<int> indices;
		for (int i = 0; i < getNumMotions(); i++)
			if (stateData->weights[i] != 0.0)
				indices.push_back(i);

		if (indices.size() == 0 && stateData->state->stateName == PseudoIdleState)
		{
			//baseTransformMat = currentBaseMats[0] * baseMats[0].inverse();
			currentBaseTransformMat = currentBaseMats[0];
		}
		else if (indices.size() == 1)
		{
			int id = indices[0];
			if (timeDiffs[id] > 0)
				baseTransformMat = currentBaseMats[id] * baseMats[id].inverse();
	#if LoopHandle
			else
			{
				SrMat newCurrentBase = currentBaseMats[id] * baseTransitionMats[id];
				baseTransformMat = newCurrentBase * baseMats[id].inverse();				
			}
	#endif
		}
		else
		{
			std::vector<SrMat> mats;
			int numMotions = indices.size();
			if (numMotions == 0)
				return;
			for (int i = 0; i < numMotions; i++)
			{
				SrMat mat;
				if (timeDiffs[indices[i]] > 0)
					mat = currentBaseMats[indices[i]] * baseMats[indices[i]].inverse();
	#if LoopHandle
				else
				{
					SrMat newCurrentBase = currentBaseMats[indices[i]] * baseTransitionMats[indices[i]];
					mat = newCurrentBase * baseMats[indices[i]].inverse();
				}
	#endif
				mats.push_back(mat);
			}
			SrMat tempMat = mats[0];
			for (int i = 1; i < numMotions; i++)
			{
				double prevWeight = 0.0;
				for (int j = 0; j < i; j++)
					prevWeight += stateData->weights[indices[j]];
				double w = prevWeight / (stateData->weights[indices[i]] + prevWeight);
				SrMat mat = tempMat;
				matInterp(tempMat, mat, mats[i], (float)(w));
			}
			baseTransformMat = tempMat;
		}
	}
	else
	{
		firstBaseTransformMat = currentBaseMats[0];
		currentBaseTransformMat = currentBaseMats[0];
		firstTime = false;
	}
	baseMats = currentBaseMats;
}

SrMat& PAWoManager::getBaseTransformMat()
{
	return baseTransformMat;
}


SrMat& PAWoManager::getFirstBaseTransformMat()
{
	return firstBaseTransformMat;
}

SrMat& PAWoManager::getCurrentBaseTransformMat()
{
	return currentBaseTransformMat;
}

void PAWoManager::matInterp(SrMat& ret, SrMat& mat1, SrMat& mat2, float w)
{
	SrQuat quat1 = SrQuat(mat1);
	SrQuat quat2 = SrQuat(mat2);
	SrQuat quat = slerp(quat1, quat2, 1 - w);

	quat.get_mat(ret);
	float posX = mat1.get(12) * w + mat2.get(12) * (1 - w);
	float posY = mat1.get(13) * w + mat2.get(13) * (1 - w);
	float posZ = mat1.get(14) * w + mat2.get(14) * (1 - w);
	ret.set(12, posX);
	ret.set(13, posY);
	ret.set(14, posZ);
}

void PAWoManager::getBaseMats(std::vector<SrMat>& mats, std::vector<double>& times, std::vector<double>& timeDiffs, int bufferSize, SrBuffer<float>& inBuff)
{
	if (!intializeTransition)
	{
		SrBuffer<float> buffer;
		buffer.size(bufferSize);
		for (int i = 0; i < getNumMotions(); i++)
		{	
			SrMat src;
			getBuffer(stateData->state->motions[i], 0.0, motionContextMaps[i], buffer);
			src = getBaseMatFromBuffer(buffer);
			SrMat dest;
			getBuffer(stateData->state->motions[i], stateData->state->motions[i]->duration(), motionContextMaps[i], buffer);
			dest = getBaseMatFromBuffer(buffer);
			SrMat transition = src.inverse() * dest;
			baseTransitionMats.push_back(SrMat());
			SrMat& updateTransitionMat = baseTransitionMats[baseTransitionMats.size() - 1];
			getUpdateMat(updateTransitionMat, transition);
		}
		intializeTransition = true;
	}

	SrBuffer<float> buffer;
	buffer.size(bufferSize);
	int numMotions = getNumMotions();
	if ( getNumMotions() == 0 && stateData->state->stateName == PseudoIdleState)
	{
		SrMat baseMat;			
		baseMat = getBaseMatFromBuffer(inBuff);
		mats.push_back(SrMat());
		SrMat& updateBaseMat = mats[mats.size() - 1];
		getUpdateMat(updateBaseMat, baseMat);		
	}	
	else if (numMotions == times.size())
	{
		for (int i = 0; i < getNumMotions(); i++)
		{
			
			double time = times[i];
			SrMat baseMat;
			getBuffer(stateData->state->motions[i], time, motionContextMaps[i], buffer);
			baseMat = getBaseMatFromBuffer(buffer);
			mats.push_back(SrMat());
			SrMat& updateBaseMat = mats[mats.size() - 1];
			getUpdateMat(updateBaseMat, baseMat);
		}
	}
	
}

PAStateData::PAStateData(const std::string& stateName, std::vector<double>& w, BlendMode blend, WrapMode wrap, ScheduleMode schedule)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	PAState* s = mcu.lookUpPAState(stateName);
	state = s;
	if (state)
	{
		weights.resize(s->getNumMotions());
		for (size_t x = 0; x < w.size(); x++)
			weights[x] = w[x];
		timeManager = new PATimeManager(this);
		interpolator = new PAInterpolator(this);
		woManager = new PAWoManager(this);
	}
	else
	{
		std::vector<SkMotion*> motions;
		std::vector<std::vector<double> > keys;
		timeManager = new PATimeManager(this);
		interpolator = new PAInterpolator(this);
		woManager = new PAWoManager(this);
	}
	
	blendMode = blend;
	wrapMode = wrap;
	scheduleMode = schedule;

	active = false;
}

PAStateData::PAStateData(PAState* s, std::vector<double>& w, BlendMode blend, WrapMode wrap, ScheduleMode schedule)
{
	state = s;
	weights.resize(s->getNumMotions());
	for (size_t x = 0; x < w.size(); x++)
		weights[x] = w[x];
	timeManager = new PATimeManager(this);
	interpolator = new PAInterpolator(this);
	woManager = new PAWoManager(this);

	blendMode = blend;
	wrapMode = wrap;
	scheduleMode = schedule;

	active = false;
}

PAStateData::~PAStateData()
{
	if (timeManager)
		delete timeManager;
	timeManager = NULL;
	if (interpolator)
		delete interpolator;
	interpolator = NULL;
	if (woManager)
		delete woManager;
	woManager = NULL;
}

void PAStateData::evaluateTransition( double timeStep, SrBuffer<float>& buffer, bool tranIn )
{
	if (state && getStateName() == PseudoIdleState) // transition 
	{
		SrBuffer<float> buffCopy = buffer;		
		if (tranIn) // PseudoIdle ----> Animation State
		{
			interpolator->blending(timeManager->motionTimes, buffer); // remove (x,z) & ry from buffer
			woManager->apply(timeManager->motionTimes,timeManager->timeDiffs, buffCopy); // add (x,z) & ry to world offset
			active = true;
		}
		else // Animation State ----> PseudoIdle		
		{				
			woManager->apply(timeManager->motionTimes,timeManager->timeDiffs,buffCopy); // add (x,z) & ry to world offset
			active = true;
		}
	}
	else // if not Pseudo Idle, just proceed as usual
	{
		evaluate(timeStep,buffer);
	}
}

void PAStateData::evaluate(double timeStep, SrBuffer<float>& buffer)
{
	if (state && state->stateName == PseudoIdleState)
	{
		if (!active)
			active = true;		
		return;
	}

	bool notReachCycle = true;
	notReachCycle = timeManager->step(timeStep);
	SrBuffer<float> buffCopy = buffer;
	if (wrapMode == Loop || ((wrapMode != Loop) && notReachCycle))
	{
		interpolator->blending(timeManager->motionTimes, buffer);
		woManager->apply(timeManager->motionTimes, timeManager->timeDiffs, buffCopy);

		active = true;
	}
	else
		active = false;
}

std::string PAStateData::getStateName()
{
	return state->stateName;	
}

bool PAStateData::isPartialBlending()
{
	if (interpolator->joints.size() != 0)
		return true;
	else
		return false;
}

void PAStateData::updateMotionIndices()
{
	motionIndex.clear();

	if (!state)
		return;

	for (size_t m = 0; m < state->motions.size(); m++)
	{
		motionIndex.push_back(std::vector<int>());
		SkMotion* motion = state->motions[m];
		SkChannelArray& motionChan = state->motions[m]->channels();
		int chanSize = motionChan.size();
		for (int c = 0; c < chanSize; c++)
		{
			const std::string& chanName = motionChan.name(c);
			motionIndex[m].push_back(motionChan.search(chanName, motionChan[c].type));
		}
	}
}

PATransitionManager::PATransitionManager()
{
	from = NULL;
	to = NULL;
	startTransition = false;
	blendingMode = true;
	active = true;
	transition = NULL;
	curve = new srLinearCurve();
	curve->insert(0.0, 1.0);
	curve->insert(defaultTransition, 0.0);
	duration = defaultTransition;
	localTime = 0.0;
	s1 = 0.0;
	e1 += defaultTransition;
	s2 = 0.0;
	e2 += defaultTransition;
}

PATransitionManager::PATransitionManager(double easeOutStart, double dur)
{
	startTransition = false;
	blendingMode = false;
	active = true;
	transition = NULL;
	curve = new srLinearCurve();
	duration = dur;
	localTime = 0.0;
	s1 = easeOutStart;
	e1 = s1 + dur;
	s2 = 0.0;
	// possible bug spot, although for now, the result seems better
#if 1
	e2 = s2 + dur;
#else
	e2 += dur;
#endif
}

PATransitionManager::PATransitionManager(PATransition* trans, PAStateData* f, PAStateData* t)
{
	from = f;
	to = t;
	//transition = trans;
	transition = new PATransition(trans, f->state, to->state);
	update();
	localTime = 0.0;
	startTransition = false;
	blendingMode = false;
	active = true;
	curve = new srLinearCurve();
	s1 = -1.0;
	e1 = -1.0;
}

PATransitionManager::~PATransitionManager()
{
}


/* 
	This function activate next state module according to current module.
	PATransitionManager has two mode: align mode and blending mode, it starts with align mode, after aligning, switch to blending mode
*/
void PATransitionManager::align(PAStateData* current, PAStateData* next)
{

	// possible bug spot, although for now, the result seems better
#if 0
	int numEaseOut = getNumEaseOut();
	for (int i = 0; i < numEaseOut; i++)
	{
		if (fabs(current->timeManager->localTime - easeOutStarts[i]) < timeThreshold)
		{
			s1 = easeOutStarts[i];
			e1 = easeOutEnds[i];
		}
	}

	if (fabs(current->timeManager->localTime - s1) < timeThreshold)
	{
		next->active = true;
		blendingMode = true;
		curve->insert(0.0, 1.0);
		curve->insert(duration, 0.0);
#if PrintPADebugInfo
		LOG("State %s being scheduled.[ACTIVE]", next->data->stateName.c_str());
#endif
	}
#else

	int numEaseOut = getNumEaseOut();
	for (int i = 0; i < numEaseOut; i++)
	{
		if (current->timeManager->prevLocalTime <= easeOutStarts[i] && current->timeManager->localTime >= easeOutStarts[i]) 
		{
			s1 = easeOutStarts[i];
			e1 = easeOutEnds[i];
			break;
		}
	}

	if (current->timeManager->prevLocalTime <= s1 && current->timeManager->localTime >= s1)
	{
		next->active = true;
		blendingMode = true;
		// Important: adjust for the duration (is this enough?)
		duration += (current->timeManager->prevLocalTime - current->timeManager->localTime);
		curve->insert(0.0, 1.0);
		curve->insert(duration, 0.0);
	}
#endif
}

void PATransitionManager::blending(SrBuffer<float>& buffer, SrBuffer<float>&buffer1, SrBuffer<float>&buffer2, SrMat& mat, SrMat& mat1, SrMat& mat2, double timeStep, MeControllerContext* _context)
{
	double w = curve->evaluate(localTime);
	bufferBlending(buffer, buffer1, buffer2, w, _context);
	PAWoManager::matInterp(mat, mat1, mat2, (float)w);

	localTime += timeStep;
	if (localTime > duration) 
		active = false;
}

void PATransitionManager::update()
{
	if (!transition)
		return;
	std::vector<double> fromKey;
	int id;
	

	for (int i = 0; i < transition->fromState->getNumMotions(); i++)
	{
		std::string motionName = transition->fromState->motions[i]->getName();
		if (motionName == transition->fromMotionName)
		{
			fromKey = transition->fromState->keys[i];
			id = i;
		}
	}
	
	if (fromKey.size() > 0)
	{
		for (int i = 0; i < transition->getNumEaseOut(); i++)
		{
			if (transition->easeOutStart[i] < fromKey[0])
			{
				transition->easeOutStart[i] += transition->fromState->motions[id]->duration();
				transition->easeOutEnd[i] += transition->fromState->motions[id]->duration();
			}
			easeOutStarts.push_back(getTime(transition->easeOutStart[i], fromKey, transition->fromState->keys, from->weights));
			easeOutEnds.push_back(getTime(transition->easeOutEnd[i], fromKey, transition->fromState->keys, from->weights));
		}
	}
	

	std::vector<double> toKey;
	for (int i = 0; i < transition->toState->getNumMotions(); i++)
	{
		std::string motionName = transition->toState->motions[i]->getName();
		if (motionName == transition->toMotionName)
		{
			toKey = transition->toState->keys[i];
			id = i;
		}
	}
	if (toKey.size() > 0)
	{
		if (transition->easeInStart < toKey[0])
		{
			transition->easeInStart += transition->toState->motions[id]->duration();
			transition->easeInEnd += transition->toState->motions[id]->duration();
		}
	}
	s2 = getTime(transition->easeInStart, toKey, transition->toState->keys, to->weights);
	e2 = getTime(transition->easeInEnd, toKey, transition->toState->keys, to->weights);

	duration = e2 - s2;
}

double PATransitionManager::getSlope()
{
	double slope = (s1 - e1) / (s2 - e2);
	return slope;
}

int PATransitionManager::getNumEaseOut()
{
	if (easeOutStarts.size() != easeOutEnds.size())
		return -1;
	else
		return easeOutStarts.size();
}

void PATransitionManager::bufferBlending(SrBuffer<float>& buffer, SrBuffer<float>& buffer1, SrBuffer<float>& buffer2, double w, MeControllerContext* _context)
{
	if (!_context)
		return;
	SkChannelArray& channels = _context->channels();
	for (int i = 0; i < channels.size(); i++)
	{
		const std::string& chanName = channels.name(i);
		if (chanName == SbmPawn::WORLD_OFFSET_JOINT_NAME)
			continue;

		if (channels[i].type == SkChannel::XPos || channels[i].type == SkChannel::YPos || channels[i].type == SkChannel::ZPos)
		{
			float v1 = buffer1[_context->toBufferIndex(i)];
			float v2 = buffer2[_context->toBufferIndex(i)];
			float v;
			SkChannel::interp(channels[i].type, &v, &v1, &v2, float(1 - w));
			buffer[_context->toBufferIndex(i)] = v;
		}
		if (channels[i].type == SkChannel::Quat)
		{
			float v1[4];
			float v2[4];
			float v[4];
			for (int j = 0; j < 4; j++)
			{
				v1[j] = buffer1[_context->toBufferIndex(i) + j];
				v2[j] = buffer2[_context->toBufferIndex(i) + j];
			}
			SkChannel::interp(channels[i].type, v, v1, v2, float(1 - w)); 
			for (int j = 0; j < 4; j++)
				buffer[_context->toBufferIndex(i) + j] = v[j];
		}
	}	
}


double PATransitionManager::getTime(double time, const std::vector<double>& key, const std::vector<std::vector<double> >& keys, const std::vector<double>& w)
{
	double ret = 0.0;
	int section = -1;
	if (key.size() > 0)
	{
		for (size_t i = 0; i < key.size() - 1; i++)
		{
			if (key[i] <= time && key[i + 1] >= time)
				section = i;
		}
	}

	if (section < 0)
	{
		LOG("PATransitionManager::getTime ERR!");
		return -1.0;
	}
	for (size_t i = 0; i < w.size(); i++)
	{
		double t = keys[i][section] + (keys[i][section + 1] - keys[i][section]) * (time - key[section]) / (key[section + 1] - key[section]);
		ret += t * w[i];
	}
	return ret;
}