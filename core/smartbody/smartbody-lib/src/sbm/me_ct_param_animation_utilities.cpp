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

const double timeThreshold = 0.05;

PATimeManager::PATimeManager()
{
}

PATimeManager::PATimeManager(std::vector<SkMotion*> m, std::vector<std::vector<double> > k, std::vector<double> w)
{
	motions = m;
	keys = k;
	weights = w;
	if (keys.size() > 0)
		if (keys[0].size() > 0)
			for (size_t i = 0; i < keys.size(); i++)
				localTimes.push_back(keys[i][0]);
	setMotionTimes();
	setKey();
	setLocalTime();
}

PATimeManager::~PATimeManager()
{
}

int PATimeManager::getNumKeys()
{
	return key.size();
}

int PATimeManager::getNumWeights()
{
	return weights.size();
}

int PATimeManager::getNumMotions()
{
	return motions.size();
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
	std::vector<double> prevMotionTimes = motionTimes;

	prevLocalTime = localTime;
	double newLocalTime = localTime + timeStep;
	int loopcounter = 0;
	while (newLocalTime > key[key.size() - 1])
	{
		loopcounter ++;
		if (loopcounter > 10)
		{
			newLocalTime = localTime;
			LOG("time step %f, local time %f, motion last key %f, motion key size: %d", timeStep, localTime, key[key.size() - 1], key.size());
			return true;
		}
		newLocalTime -= (key[key.size() - 1] - key[0]);
		notReachDuration = false;
	}
	localTime = newLocalTime;
	getParallelTimes(localTime, localTimes);
	if (localTimes.size() == 0)
	{
		LOG("PATimeManager::step ERROR: Miss the timing.");
		return notReachDuration;
	}
	setMotionTimes();

	timeDiffs.clear();
	for (int i = 0; i < getNumWeights(); i++)
		timeDiffs.push_back(motionTimes[i] - prevMotionTimes[i]);

	return notReachDuration;
}

void PATimeManager::updateWeights(std::vector<double>& w)
{
	weights = w;
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
	if (getNumWeights() > 0) numKeys = keys[0].size();
	for (int i = 0; i < numKeys; i++)
	{
		double tempK = 0.0;
		for (int j = 0; j < getNumWeights(); j++)
			tempK += weights[j] * keys[j][i];
		key.push_back(tempK);
	}
}

void PATimeManager::setLocalTime()
{
	localTime = 0.0;
	for (int i = 0; i < getNumWeights(); i++)
		localTime += weights[i] * localTimes[i];
	prevLocalTime = localTime;
}

void PATimeManager::setMotionTimes()
{
	motionTimes.clear();
	for (int i = 0; i < getNumWeights(); i++)
	{
		if (localTimes[i] >= motions[i]->duration())
			motionTimes.push_back(localTimes[i] - motions[i]->duration());
		else
			motionTimes.push_back(localTimes[i]);
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
		return;
	for (int i = 0; i < getNumWeights(); i++)
	{
		double t = keys[i][section] + (keys[i][section + 1] - keys[i][section]) * (time - key[section]) / (key[section + 1] - key[section]);
		times.push_back(t);
	}
}

PAMotions::PAMotions()
{
}

PAMotions::PAMotions(std::vector<SkMotion*> m, std::vector<double> w)
{
	motions = m;
	weights = w;
}

PAMotions::~PAMotions()
{
}


void PAMotions::setMotionContextMaps(MeControllerContext* context)
{
	SkChannelArray& cChannels = context->channels();
	for (size_t mId = 0; mId < motions.size(); mId++)
	{
		SkChannelArray& mChannels = motions[mId]->channels();
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
}

int PAMotions::getNumMotions()
{
	return motions.size();
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
	SrVec vec1 = vec * prerotMat.inverse();
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
	SrVec vec1 = vec * prerotMat.inverse();
	quat = SrQuat(vec1);
	SrMat mat;
	quat.get_mat(mat);
	float rx, ry, rz;
	sr_euler_angles(rotType, mat, rx, ry, rz);
	ry = 0.0;
	sr_euler_mat(rotType, mat, rx, ry, rz);
	SrQuat quatP = SrQuat(mat);
	vec1 = quatP.axis() * quatP.angle();
	SrVec vec2 = vec1 * prerotMat;
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

PAInterpolator::PAInterpolator(std::vector<SkMotion*> m, std::vector<double> w) : PAMotions(m, w)
{
}

PAInterpolator::~PAInterpolator()
{
}

void PAInterpolator::blending(std::vector<double>& times, SrBuffer<float>& buff)
{
	std::vector<int> indices;
	for (int i = 0; i < getNumMotions(); i++)
		if (weights[i] != 0.0)
			indices.push_back(i);

	if (indices.size() == 1)
	{
		SrBuffer<float> buffer;
		buffer.size(buff.size());
		buffer = buff;
		int id = indices[0];
		double time = times[id];
		getBuffer(motions[id], time, motionContextMaps[id], buffer);
		handleBaseMatForBuffer(buffer);
		buff = buffer;
	}

#if MultiBlending
	else
	{
		int numMotions = indices.size();
		if (numMotions == 0)
			return;
		std::vector<SrBuffer<float> > buffers;
		for (int i = 0; i < numMotions; i++)
		{
			buffers.push_back(SrBuffer<float>());
			SrBuffer<float>& buffer = buffers[buffers.size() - 1];
			buffer.size(buff.size());
			buffer = buff;
			getBuffer(motions[indices[i]], times[indices[i]], motionContextMaps[indices[i]], buffer);
			handleBaseMatForBuffer(buffer);
		}
		SrBuffer<float> tempBuffer;
		tempBuffer.size(buff.size());
		tempBuffer = buffers[0];
		SkChannelArray& motionChan = motions[indices[0]]->channels();
		int chanSize = motionChan.size();
		for (int i = 0; i < chanSize; i++)
		{
			const std::string& chanName = motionChan.name(i);
			for (int j = 1; j < numMotions; j++)
			{
				int id = motions[indices[j]]->channels().search(chanName, motionChan[i].type);
				if (id < 0)
					continue;
				int buffId = motionContextMaps[indices[0]].get(i);
				if (buffId >= 0)
				{
					double prevWeight = 0.0;
					for (int k = 0; k < j; k++)
						prevWeight += weights[indices[k]];
					double w = prevWeight / (weights[indices[j]] + prevWeight);
					motions[indices[0]]->channels()[i].interp(&tempBuffer[buffId], &tempBuffer[buffId], &buffers[j][buffId], (float)(1 - w));
				}
			}
		}
		buff = tempBuffer;
	}
#else
	else if (indices.size() == 2)
	{
		SrBuffer<float> buffer1;
		SrBuffer<float> buffer2;
		buffer1.size(buff.size());
		buffer2.size(buff.size());
		buffer1 = buff;
		buffer2 = buff;
		int motionId1 = indices[0];
		int motionId2 = indices[1];
		double time1 = times[motionId1];
		double time2 = times[motionId2];
		getBuffer(motions[motionId1], time1, motionContextMaps[motionId1], buffer1);
		handleBaseMatForBuffer(buffer1);
		getBuffer(motions[motionId2], time2, motionContextMaps[motionId2], buffer2);
		handleBaseMatForBuffer(buffer2);
		SkChannelArray& motion1Chan = motions[motionId1]->channels();
		SkChannelArray& motion2Chan = motions[motionId2]->channels();
		int chanSize = motion1Chan.size();
		for (int i = 0; i < chanSize; i++)
		{
			SkJointName chanName = motion1Chan.name(i);
			int idMotion2 = motion2Chan.search(chanName, motion1Chan[i].type);
			if (idMotion2 < 0)
				continue;

			int index1 = motionContextMaps[indices[0]].get(i);
			int index2 = motionContextMaps[indices[1]].get(idMotion2);
			if (index1 >= 0 && index2 >= 0)
				motions[motionId2]->channels()[i].interp(&buff[index1], &buffer1[index1], &buffer2[index2], (float)(1 - weights[motionId1]));
		}
	}
	else
	{
		LOG("more than three motions, not valid");
		return;
	}
#endif
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

PAWoManager::PAWoManager(std::vector<SkMotion*> m, std::vector<double> w) : PAMotions(m, w)
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
	getBaseMats(currentBaseMats, times, timeDiffs, buffer.size());
	if (!firstTime)
	{
		std::vector<int> indices;
		for (int i = 0; i < getNumMotions(); i++)
			if (weights[i] != 0.0)
				indices.push_back(i);

		if (indices.size() == 1)
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
#if MultiBlending
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
					prevWeight += weights[indices[j]];
				double w = prevWeight / (weights[indices[i]] + prevWeight);
				SrMat mat = tempMat;
				matInterp(tempMat, mat, mats[i], (float)(w));
			}
			baseTransformMat = tempMat;
		}
#else
		else if (indices.size() == 2)
		{
			int id1 = indices[0];
			int id2 = indices[1];
			SrMat mat1;
			SrMat mat2;
			if (timeDiffs[id1] > 0)
				mat1 = currentBaseMats[id1] * baseMats[id1].inverse();
	#if LoopHandle
			else
			{
				SrMat newCurrentBase = currentBaseMats[id1] * baseTransitionMats[id1];
				mat1 = newCurrentBase * baseMats[id1].inverse();				
			}
	#endif
			double time2 = times[id2];
			if (timeDiffs[id2] > 0)
				mat2 = currentBaseMats[id2] * baseMats[id2].inverse();
	#if LoopHandle
			else
			{
				SrMat newCurrentBase = currentBaseMats[id2] * baseTransitionMats[id2];
				mat2 = newCurrentBase * baseMats[id2].inverse();				
			}
	#endif
			matInterp(baseTransformMat, mat1, mat2, (float)(weights[id1]));
		}
		else
		{
			LOG("Error! Not supporting more than two motions");
			return;
		}
#endif
	}
	else
		firstTime = false;
	baseMats = currentBaseMats;
}

SrMat& PAWoManager::getBaseTransformMat()
{
	return baseTransformMat;
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

void PAWoManager::getBaseMats(std::vector<SrMat>& mats, std::vector<double>& times, std::vector<double>& timeDiffs, int bufferSize)
{
	if (!intializeTransition)
	{
		for (int i = 0; i < getNumMotions(); i++)
		{
			SrBuffer<float> buffer;
			buffer.size(bufferSize);
			SrMat src;
			getBuffer(motions[i], 0.0, motionContextMaps[i], buffer);
			src = getBaseMatFromBuffer(buffer);
			SrMat dest;
			getBuffer(motions[i], motions[i]->duration(), motionContextMaps[i], buffer);
			dest = getBaseMatFromBuffer(buffer);
			SrMat transition = src.inverse() * dest;
			baseTransitionMats.push_back(SrMat());
			SrMat& updateTransitionMat = baseTransitionMats[baseTransitionMats.size() - 1];
			getUpdateMat(updateTransitionMat, transition);
		}
		intializeTransition = true;
	}

	for (int i = 0; i < getNumMotions(); i++)
	{
		SrBuffer<float> buffer;
		buffer.size(bufferSize);
		double time = times[i];
		SrMat baseMat;
		getBuffer(motions[i], time, motionContextMaps[i], buffer);
		baseMat = getBaseMatFromBuffer(buffer);
		mats.push_back(SrMat());
		SrMat& updateBaseMat = mats[mats.size() - 1];
		getUpdateMat(updateBaseMat, baseMat);
	}
}

PAStateModule::PAStateModule(PAStateData* stateData, bool l, bool pn)
{
	timeManager = new PATimeManager(stateData->motions, stateData->keys, stateData->weights);
	interpolator = new PAInterpolator(stateData->motions, stateData->weights);
	woManager = new PAWoManager(stateData->motions, stateData->weights);

//	data = stateData;
	data = new PAStateData(stateData);
	loop = l;
	active = false;
	playNow = pn;
}

PAStateModule::~PAStateModule()
{
//	data = NULL;
	if (data)
		delete data;
	data = NULL;
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

void PAStateModule::evaluate(double timeStep, SrBuffer<float>& buffer)
{
	bool notReachCycle = true;
	notReachCycle = timeManager->step(timeStep);
	if (loop || (!loop && notReachCycle))
	{
		interpolator->blending(timeManager->motionTimes, buffer);
		woManager->apply(timeManager->motionTimes, timeManager->timeDiffs, buffer);
		active = true;
	}
	else
		active = false;
}


PseudoPAStateModule::PseudoPAStateModule() : PAStateModule(new PAStateData(PseudoIdleState))
{
}

PseudoPAStateModule::~PseudoPAStateModule()
{
}

void PseudoPAStateModule::evaluate(double timeStep, SrBuffer<float>& buffer)
{
	if (!active) active = true;
}

PATransitionManager::PATransitionManager()
{
	blendingMode = true;
	active = true;
	data = NULL;
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
	blendingMode = false;
	active = true;
	data = NULL;
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

PATransitionManager::PATransitionManager(PATransitionData* transitionData, PAStateData* from, PAStateData* to)
{
	data = new PATransitionData(transitionData, from, to);
	update();
	localTime = 0.0;
	blendingMode = false;
	active = true;
	curve = new srLinearCurve();
	s1 = -1.0;
	e1 = -1.0;
}

PATransitionManager::~PATransitionManager()
{
	if (data)
		delete data;
	data = NULL;
}


/* 
	This function activate next state module according to current module.
	PATransitionManager has two mode: align mode and blending mode, it starts with align mode, after aligning, switch to blending mode
*/
void PATransitionManager::align(PAStateModule* current, PAStateModule* next)
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
//		std::cout << "transition align" << i << " " << current->timeManager->prevLocalTime << " " << easeOutStarts[i] << " " << current->timeManager->localTime << std::endl;
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
#if PrintPADebugInfo
		LOG("State %s being scheduled.[ACTIVE]", next->data->stateName.c_str());
#endif
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
	if (data == NULL)
		return;
	std::vector<double> fromKey;
	int id;
	if (!data)
		return;

	for (int i = 0; i < data->fromState->getNumMotions(); i++)
	{
		std::string motionName = data->fromState->motions[i]->getName();
		if (motionName == data->fromMotionName)
		{
			fromKey = data->fromState->keys[i];
			id = i;
		}
	}
	
	for (int i = 0; i < data->getNumEaseOut(); i++)
	{
		if (data->easeOutStart[i] < fromKey[0])
		{
			data->easeOutStart[i] += data->fromState->motions[id]->duration();
			data->easeOutEnd[i] += data->fromState->motions[id]->duration();
		}
		easeOutStarts.push_back(getTime(data->easeOutStart[i], fromKey, data->fromState->keys, data->fromState->weights));
		easeOutEnds.push_back(getTime(data->easeOutEnd[i], fromKey, data->fromState->keys, data->fromState->weights));
	}

	std::vector<double> toKey;
	for (int i = 0; i < data->toState->getNumMotions(); i++)
	{
		std::string motionName = data->toState->motions[i]->getName();
		if (motionName == data->toMotionName)
		{
			toKey = data->toState->keys[i];
			id = i;
		}
	}
	if (data->easeInStart < toKey[0])
	{
		data->easeInStart += data->toState->motions[id]->duration();
		data->easeInEnd += data->toState->motions[id]->duration();
	}
	s2 = getTime(data->easeInStart, toKey, data->toState->keys, data->toState->weights);
	e2 = getTime(data->easeInEnd, toKey, data->toState->keys, data->toState->weights);

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


double PATransitionManager::getTime(double time, std::vector<double> key, std::vector<std::vector<double> > keys, std::vector<double> w)
{
	double ret = 0.0;
	int section = -1;
	for (size_t i = 0; i < key.size() - 1; i++)
	{
		if (key[i] <= time && key[i + 1] >= time)
			section = i;
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

PAControllerBlending::PAControllerBlending()
{
	curve = new srLinearCurve();
}

PAControllerBlending::~PAControllerBlending()
{
}

double PAControllerBlending::getKey(double t)
{
	return curve->evaluate(t);
}

void PAControllerBlending::addKey(double t, double weight)
{
	curve->insert(t, weight);
}

void PAControllerBlending::updateBuffer(SrBuffer<float>& buff)
{
	if (buffer.size() != buff.size())
		buffer.size(buff.size());
	buffer = buff;
}

SrBuffer<float>& PAControllerBlending::getBuffer()
{
	return buffer;
}