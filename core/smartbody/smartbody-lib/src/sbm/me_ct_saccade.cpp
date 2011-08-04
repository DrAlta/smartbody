/*
 *  me_ct_saccade.cpp - part of Motion Engine and SmartBody-lib
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

#include "me_ct_saccade.h"
#include <cstdlib>
#include <ctime> 
#include <math.h>
#include <sbm/mcontrol_util.h>

using namespace gwiz;
const char* MeCtSaccade::CONTROLLER_TYPE = "Saccade";
const char* eyeballL = "eyeball_left";
const char* eyeballR = "eyeball_right";

MeCtSaccade::MeCtSaccade(SkSkeleton* skel) : SmartBody::SBController()
{
	_skeleton = skel;
	if (skel)
		skel->ref();

	_useModel = true;
	_valid = false;
	_initialized = false;
	_idL = -1;
	_idR = -1;
	_prevTime = 0.0;

	_dur = 0.0f;
	_time = -1.0f;
	_direction = 0.0f;
	_intervalMode = Mutual;
	_behaviorMode = Listening;//											expose


	//--- saccade statistics
	// direction stat
	// percentage for listening and talking
	_percentBin0 = 15.54f;				// unit: percentage
	_percentBin45 = 6.46f;
	_percentBin90 = 17.69f;
	_percentBin135 = 7.44f;
	_percentBin180 = 16.80f;
	_percentBin225 = 7.89f;
	_percentBin270 = 20.38f;
	_percentBin315 = 7.79f;
	// percentage for thinking
	_thinkingPercentBin0 = 5.46f;		// unit: percentage
	_thinkingPercentBin45 = 10.54f;
	_thinkingPercentBin90 = 24.69f;
	_thinkingPercentBin135 = 6.44f;
	_thinkingPercentBin180 = 6.89f;
	_thinkingPercentBin225 = 12.80f;
	_thinkingPercentBin270 = 26.38f;
	_thinkingPercentBin315 = 6.79f;

	// magnitude stat
	_highestFrequency = 15.0f;	// unit: percentage
//	_talkingLimit =	27.5f;		// unit: degree
//	_listeningLimit = 22.7f;
	_talkingLimit =	5.0f;			// unit: degree							expose
	_listeningLimit = 4.0f;			//										expose
	_thinkingLimit = _talkingLimit;	// make up data

	// inter-saccadic interval stat
	_listeningPercentMutual = 75.0f;		// mutual gaze or gaze away
	_talkingPercentMutual = 41.0f;
	_thinkingPercentMutual = _talkingPercentMutual;
	_minInterval = 0.001f;
	
	// duration stat
	_intercept = 0.025f;		// unit: sec
	_slope = 0.0024f;		// unit: sec/degree

	srand((unsigned int)time(0));

	//DoubleAttribute* hf = new DoubleAttribute("saccade.talkingLimit");
	//hf->setDefaultValue(5.0);
	//hf->setValue(5.0);
	//_defaultAttributes.push_back(hf);
	//addDefaultAttributeDouble("saccade.talkingLimit", 5, &_talkingLimit);	
	addDefaultAttributeFloat("saccade.talkingLimit", 5.f, &_talkingLimit);
	addDefaultAttributeFloat("saccade.talkingPercentMutual", 41.0f, &_talkingPercentMutual);

}

MeCtSaccade::~MeCtSaccade()
{
	_skeleton->unref();
}

void MeCtSaccade::spawnOnce(float dir, float amplitude, float dur)
{
	if ((float)mcuCBHandle::singleton().time < (_time + _dur))
		return;
	_direction = dir;
	_magnitude = amplitude;
	_dur = dur;

	SrVec vec1 = SrVec(0, 0, 1);
	float direction = (_direction - 90.0f) * (float)M_PI / 180.0f;		// 90.0f here is for adjustment
	SrVec vec2 = SrVec(sin(direction), cos(direction), 0);
	_axis = cross(vec1, vec2);
	_lastFixedRotation = _fixedRotation;
	_fixedRotation = SrQuat(_axis, _magnitude * (float)M_PI / 180.0f);

	_time = (float)mcuCBHandle::singleton().time;
}

void MeCtSaccade::spawning(double t)
{
	float time = float(t);
	if (_time == -1.0f || t > (_time + _dur))
	{
		_rotation = 0.0f;
		_direction = directionRandom();			// degree
		_magnitude = magnitudeRandom();			// degree

		SrVec vec1 = SrVec(0, 0, 1);
		float direction = (_direction - 90.0f) * (float)M_PI / 180.0f;		// 90.0f here is for adjustment
		SrVec vec2 = SrVec(sin(direction), cos(direction), 0);
		_axis = cross(vec1, vec2);
		_lastFixedRotation = _fixedRotation;
		_fixedRotation = SrQuat(_axis, _magnitude * (float)M_PI / 180.0f);

		SrQuat actualRotation = _fixedRotation * _lastFixedRotation.inverse();
		float angle = actualRotation.angle() * 180.0f / (float)M_PI;
	

	//	_dur = duration(_magnitude);			// sec
		_dur = duration(angle);
		float interval = intervalRandom();		// sec
		_time = time + interval;

#if 0
		LOG("--Next Saccade happens at %f, turning angle=%f, duration=%f, interval=%f, direction=%f", _time, actualRotation.angle(), _dur, interval, _direction);
#endif
	}
	if (_time == -1.0f)
		LOG("MeCtSaccade::update ERR: this is not suppose to happen.");
}


void MeCtSaccade::processing(double t, MeFrameData& frame)
{
	float time = float(t);
	float dt = (float)mcuCBHandle::singleton().time_dt;
	if (_time == -1.0f)
		return;

	if ((t > (_time + _dur)) && !_useModel)
	{
		_lastFixedRotation = SrQuat();
		_fixedRotation = SrQuat();
	}
	SrQuat rotation = _lastFixedRotation;
	if (t >= _time && t <= (_time + _dur))
	{
		float r = (time - _time) / _dur;
	//	float y = 1 - r;
		float y = 1 - sqrt(1 - (r - 1) * (r - 1));
		rotation = slerp(_lastFixedRotation, _fixedRotation, y);
	}
	//---
	SrQuat QL = SrQuat( frame.buffer()[_idL + 0],
						frame.buffer()[_idL + 1],
						frame.buffer()[_idL + 2],
						frame.buffer()[_idL + 3] );
	
	SrQuat QR = SrQuat( frame.buffer()[_idR + 0],
						frame.buffer()[_idR + 1],
						frame.buffer()[_idR + 2],
						frame.buffer()[_idR + 3] );

	//--- process
	SrQuat temp(_lastFixedRotation);
	SrQuat actualRot = temp * rotation;
	SrQuat outQL = QL * actualRot;
	SrQuat outQR = outQL;

	//---
	frame.buffer()[_idL + 0] = outQL.w;
	frame.buffer()[_idL + 1] = outQL.x;
	frame.buffer()[_idL + 2] = outQL.y;
	frame.buffer()[_idL + 3] = outQL.z;

	frame.buffer()[_idR + 0] = outQR.w;
	frame.buffer()[_idR + 1] = outQR.x;
	frame.buffer()[_idR + 2] = outQR.y;
	frame.buffer()[_idR + 3] = outQR.z;
}

float MeCtSaccade::floatRandom(float min, float max)
{
	float r = (float)rand() / (float)RAND_MAX;
	return min + r * (max - min);
}

float MeCtSaccade::gaussianRandom(float mean, float variant)
{
	static double V1, V2, S;
	static int phase = 0;
	double X;
	if (phase == 0) 
	{
		do 
		{
			double U1 = (double)rand() / RAND_MAX;
			double U2 = (double)rand() / RAND_MAX;
			V1 = 2 * U1 - 1;
			V2 = 2 * U2 - 1;
			S = V1 * V1 + V2 * V2;
		} 
		while(S >= 1 || S == 0);
		X = V1 * sqrt(-2 * log(S) / S);
	} 
	else
		X = V2 * sqrt(-2 * log(S) / S);
	phase = 1 - phase;
	double Xp = X * sqrt(variant) + mean;	// X is for standard normal distribution
	return (float)Xp;	
}

float MeCtSaccade::directionRandom()
{
	float bound0, bound45, bound90, bound135, bound180, bound225, bound270, bound315;
	if (_behaviorMode == Talking || _behaviorMode == Listening)
	{
		bound0 = _percentBin0;
		bound45 = bound0 + _percentBin45;
		bound90 = bound45 + _percentBin90;
		bound135 = bound90 + _percentBin135;
		bound180 = bound135 + _percentBin180;
		bound225 = bound180 + _percentBin225;
		bound270 = bound225 + _percentBin270;
		bound315 = bound270 + _percentBin315;
	}
	if (_behaviorMode == Thinking)
	{
		bound0 = _thinkingPercentBin0;
		bound45 = bound0 + _thinkingPercentBin45;
		bound90 = bound45 + _thinkingPercentBin90;
		bound135 = bound90 + _thinkingPercentBin135;
		bound180 = bound135 + _thinkingPercentBin180;
		bound225 = bound180 + _thinkingPercentBin225;
		bound270 = bound225 + _thinkingPercentBin270;
		bound315 = bound270 + _thinkingPercentBin315;	
	}

	float dir = 0.0f;
	float binIndex = floatRandom(0.0f, 100.0f);
	if (binIndex >= 0.0f && binIndex <= bound0)
		dir = 0.0f;
	if (binIndex >= bound0 && binIndex <= bound45)
		dir = 45.0f;
	if (binIndex >= bound45 && binIndex <= bound90)
		dir = 90.0f;
	if (binIndex >= bound90 && binIndex <= bound135)
		dir = 135.0f;
	if (binIndex >= bound135 && binIndex <= bound180)
		dir = 180.0f;
	if (binIndex >= bound180 && binIndex <= bound225)
		dir = 225.0f;
	if (binIndex >= bound225 && binIndex <= bound270)
		dir = 270.0f;
	if (binIndex >= bound270 && binIndex <= bound315)
		dir = 315.0f;
	return dir;
}

float MeCtSaccade::magnitudeRandom()
{
	float f = floatRandom(0.0f, 15.0f);
	float a = -6.9f * log(f / 15.7f);
	float limit;
	if (_behaviorMode == Talking)
		limit = _talkingLimit;
	if (_behaviorMode == Listening)
		limit = _listeningLimit;
	if (_behaviorMode == Thinking)
		limit = _thinkingLimit;

	// below is adhoc
	// direction 0 and 180 is moving up and down, it should have a limit
	if (_direction == 90.0f || _direction == 270.0f)		
		limit *= 0.3f;
	if (_direction == 45.0f || _direction == 135.0f || _direction == 225.0f || _direction == 315.0f)
		limit *= 0.35f;

	if (a > limit)
		a = limit;
	return a;
}

float MeCtSaccade::intervalRandom()
{
	float f = floatRandom(0.0f, 100.0f);
	float mutualPercent = 0.0f;
	if (_behaviorMode == Listening)
		mutualPercent = _listeningPercentMutual;
	if (_behaviorMode == Talking)
		mutualPercent = _talkingPercentMutual;
	if (_behaviorMode == Thinking)
		mutualPercent = _thinkingPercentMutual;

	if (f >= 0.0f && f <= mutualPercent)
		_intervalMode = Mutual;
	else
		_intervalMode = Away;
	float interval = -1.0f;
	while (interval < _minInterval)
	{
		if (_intervalMode == Mutual && _behaviorMode == Talking)
			interval = gaussianRandom(_talkingMutualMean, _talkingMutualVariant);
		if (_intervalMode == Away && _behaviorMode == Talking)
			interval = gaussianRandom(_talkingAwayMean, _talkingAwayVariant);
		if (_intervalMode == Mutual && _behaviorMode == Listening)
			interval = gaussianRandom(_listeningMutualMean, _listeningMutualVariant);
		if (_intervalMode == Away && _behaviorMode == Listening)
			interval = gaussianRandom(_listeningAwayMean, _listeningAwayVariant);
		if (_behaviorMode == Thinking)
			interval = gaussianRandom(_thinkingMean, _thinkingVariant);
	}
	return interval;
}

float MeCtSaccade::duration(float amplitude)	// amplitude unit: degree
{
	float slope = _slope;
	float d0 = _intercept;
	float dur = d0 + slope * amplitude;
	return dur;
}

void MeCtSaccade::initSaccade(MeFrameData& frame)
{
	if (!_context)
		return;

	if (!_initialized)
	{
		int idL = _context->channels().search(eyeballL, SkChannel::Quat);
		_idL = frame.toBufferIndex(idL);
		int idR = _context->channels().search(eyeballR, SkChannel::Quat);
		_idR = frame.toBufferIndex(idR);
		if (_idL < 0 || _idR < 0)
			LOG("MeCtSaccade::initBufferIndex ERR: channel id not correct!");

		_initialized = true;
	}

	// saccade statistics that depends on dt
	_talkingMutualMean = 93.9f * (float)_dt;		// unit: second
	_talkingMutualVariant = 94.9f * (float)_dt;	
	_talkingAwayMean = 27.8f * (float)_dt;
	_talkingAwayVariant = 24.0f * (float)_dt;
	_listeningMutualMean = 237.5f * (float)_dt;
	_listeningMutualVariant = 47.1f * (float)_dt;
	_listeningAwayMean = 13.0f * (float)_dt;
	_listeningAwayVariant = 7.1f * (float)_dt;
	// below data is made up
	_thinkingMean = 180.0f * (float)_dt;
	_thinkingVariant = 47.0f * (float)_dt;    
}

bool MeCtSaccade::controller_evaluate(double t, MeFrameData& frame)
{	
	if (_prevTime == 0)
		_dt = 0.016;
	else
	{
		_dt = t - _prevTime;
		_prevTime = t;
	}
	SrBuffer<float>& buff = frame.buffer();
	initSaccade(frame);
	if (_valid)
	{
		if (_useModel)
			spawning(t);
		processing(t, frame);
	}
	return true;
}
