/*
 *  me_ct_saccade.h - part of Motion Engine and SmartBody-lib
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

#ifndef _ME_CT_SACCADE_H_
#define _ME_CT_SACCADE_H_

#include <sbm/SBController.h>
#include <sk/sk_skeleton.h>
#include "gwiz_math.h"
#include "sr_linear_curve.h"

class MeCtSaccade : public SmartBody::SBController
{
	public:
		static std::string CONTROLLER_TYPE;
		
		enum IntervalMode {Mutual, Away};
		enum BehaviorMode {Talking, Listening, Thinking};

	private:
		double			_prevTime;
		double			_dt;
		float 			_duration;
		SkChannelArray	_channels;
		SkSkeleton* 	_skeleton;
		int				_idL;
		int				_idR;
		bool			_initialized;
		bool			_valid;
		bool			_useModel;

		// saccade information
		float			_time;
		float			_dur;
		float			_magnitude;
		float			_direction;
		IntervalMode	_intervalMode;
		BehaviorMode	_behaviorMode;
		float			_rotation;
		SrVec			_axis;
		SrQuat			_lastFixedRotation;
		SrQuat			_fixedRotation;

		//--- saccade statistics
		// direction stat
		float			_percentBin0;
		float			_percentBin45;
		float			_percentBin90;
		float			_percentBin135;
		float			_percentBin180;
		float			_percentBin225;
		float			_percentBin270;
		float			_percentBin315;
		float			_thinkingPercentBin0;
		float			_thinkingPercentBin45;
		float			_thinkingPercentBin90;
		float			_thinkingPercentBin135;
		float			_thinkingPercentBin180;
		float			_thinkingPercentBin225;
		float			_thinkingPercentBin270;
		float			_thinkingPercentBin315;
		// magnitude stat
		float			_highestFrequency;
		float			_talkingLimit;
		float			_listeningLimit;
		float			_thinkingLimit;
		// interval stat
		float			_listeningPercentMutual;
		float			_talkingPercentMutual;
		float			_thinkingPercentMutual;
		float			_talkingMutualMean;
		float			_talkingMutualVariant;
		float			_talkingAwayMean;
		float			_talkingAwayVariant;
		float			_listeningMutualMean;
		float			_listeningMutualVariant;
		float			_listeningAwayMean;
		float			_listeningAwayVariant;
		float			_thinkingMean;
		float			_thinkingVariant;
		float			_minInterval;
		// duration stat
		float			_intercept;
		float			_slope;

	public:
		MeCtSaccade(SkSkeleton* skel);
		~MeCtSaccade();

		// set and get
		bool getValid()						{return _valid;}
		void setValid(bool v)				{_valid = v;}
		void setTalkingAngleLimit(float v)	{_talkingLimit = v;}
		float getTalkingAngleLimit()		{return _talkingLimit;}
		void setListeningAngleLimit(float v){_listeningLimit = v;}
		float getListeningAngleLimit()		{return _listeningLimit;}
		void setThinkingAngleLimit(float v)	{_thinkingLimit = v;}
		float getThinkingAngleLimit()		{return _thinkingLimit;}
		void setBehaviorMode(BehaviorMode m){_behaviorMode = m;}
		BehaviorMode getBehaviorMode()		{return _behaviorMode;}
		bool getUseModel()					{return _useModel;}
		void setUseModel(bool v)			{_useModel = v;}
		void spawnOnce(float dir, float amplitude, float dur);

	private:
		void spawning(double t);
		void processing(double t, MeFrameData& frame);

		float floatRandom(float min, float max);
		float gaussianRandom(float mean, float variant);
		float directionRandom();			// unit: degree
		float magnitudeRandom();			// unit: degree
		float intervalRandom();				// unit: sec
		float duration(float amplitude);	// unit: sec

		void initSaccade(MeFrameData& frame);
		virtual bool controller_evaluate(double t, MeFrameData& frame);
		virtual SkChannelArray& controller_channels()	{ return(_channels); }
		virtual double controller_duration()			{ return((double)_duration); }
		virtual const std::string& controller_type() const		{ return(CONTROLLER_TYPE); }
};

#endif
