/*
 *  me_ct_param_animation_utilities.h - part of Motion Engine and SmartBody-lib
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

#ifndef _ME_CT_PARAM_ANIMATION_UTILITIES_H_
#define _ME_CT_PARAM_ANIMATION_UTILITIES_H_

#include <sk/sk_motion.h>
#include <me/me_controller.h>
#include <me/me_ct_channel_writer.hpp>
#include <sbm/sr_linear_curve.h>
#include <sbm/sbm_pawn.hpp>
#include <sbm/me_ct_param_animation_data.h>

#define PrintPADebugInfo 0
#define MultiBlending 1
#define LoopHandle 0
const int rotType = 132;
const double defaultTransition = 0.3;

struct JointChannelId 
{
	int x, y, z, q;
};

class PATimeManager
{
	public:
		std::vector<SkMotion*> motions;
		std::vector<std::vector<double> > keys;	//always in ascending order
		std::vector<double> weights;		
		std::vector<double> localTimes;			//always in ascending order
		std::vector<double> motionTimes;		//actual motion times get from localTimes
		std::vector<double> timeDiffs;			//time steps from last evaluation to this evaluation, get from motionTimes
		std::vector<double> key;				//key get from keys and current weights
		double localTime;
		double prevLocalTime;

	public:
		PATimeManager();
		PATimeManager(std::vector<SkMotion*> m, std::vector<std::vector<double> > k, std::vector<double> w);
		~PATimeManager();

		int getNumKeys();
		int getNumWeights();
		int getNumMotions();
		void updateLocalTimes(double time);
		bool step(double timeStep);
		void updateWeights(std::vector<double>& w);
		double getDuration();

	private:
		void setKey();
		void setLocalTime();
		void setMotionTimes();
		int getSection(double time);
		void getParallelTimes(double time, std::vector<double>& times);
};

class PAMotions
{
	public:
		std::vector<SkMotion*> motions;
		std::vector<double> weights;
	
	protected:
		std::vector<SrBuffer<int> > motionContextMaps;
		JointChannelId baseChanId;
		JointChannelId baseBuffId;
		SrQuat basePrerot;

	public:
		PAMotions();
		PAMotions(std::vector<SkMotion*> m, std::vector<double> w);
		~PAMotions();

		int getNumMotions();
		void setMotionContextMaps(MeControllerContext* context);
		void initChanId(MeControllerContext* context, std::string baseJointName);
		void initPreRotation(const SrQuat& q);

	protected:
		void getBuffer(SkMotion* motion, double t, SrBuffer<int>& map, SrBuffer<float>& buff);
		SrMat getBaseMatFromBuffer(SrBuffer<float>& buffer);
		void setBufferByBaseMat(SrMat& mat, SrBuffer<float>& buffer);
		void getUpdateMat(SrMat& dest, SrMat& src);
		void getProcessedMat(SrMat& dest, SrMat& src);
};

class PAWoManager : public PAMotions
{
	private:
		bool firstTime;
		std::vector<SrMat>	baseMats;
		std::vector<SrMat>	baseTransitionMats;
		bool intializeTransition;
		SrMat baseTransformMat;

	public:
		PAWoManager();
		PAWoManager(std::vector<SkMotion*> m, std::vector<double> w);
		~PAWoManager();

		void apply(std::vector<double>& times, std::vector<double>& timeDiffs, SrBuffer<float>& buffer);
		SrMat& getBaseTransformMat();
		static void matInterp(SrMat& ret, SrMat& mat1, SrMat& mat2, float w);

	private:
		void getBaseMats(std::vector<SrMat>& mats, std::vector<double>& times, std::vector<double>& timeDiffs, int bufferSize);
};	

class PAInterpolator : public PAMotions
{
	public:
		PAInterpolator();
		PAInterpolator(std::vector<SkMotion*> m, std::vector<double> w);
		~PAInterpolator();

	public:
		void blending(std::vector<double> times, SrBuffer<float>& buff);

	private:
		void handleBaseMatForBuffer(SrBuffer<float>& buff);
};

class PAStateData;
class PAStateModule
{
	public:
		PATimeManager* timeManager;
		PAWoManager* woManager;
		PAInterpolator* interpolator;
		
		bool loop;
		bool active;
		bool playNow;
		PAStateData* data;
		
	public:
		PAStateModule(PAStateData* stateData, bool l = true, bool pn = false);
		~PAStateModule();
		virtual void evaluate(double timeStep, SrBuffer<float>& buffer);
};

class PseudoPAStateModule : public PAStateModule
{
	public:
		PseudoPAStateModule();
		~PseudoPAStateModule();

		void evaluate(double timeStep, SrBuffer<float>& buffer);
};

class PATransitionData;
class PATransitionManager
{
	public:
		bool blendingMode;
		bool active;

		PATransitionData* data;
		srLinearCurve* curve;
		double duration;
		double localTime;
		std::vector<double> easeOutStarts;
		std::vector<double> easeOutEnds;
		double s1;
		double e1;
		double s2;
		double e2;
		
	public:
		PATransitionManager();
		PATransitionManager(double easeOutStart, double duration);
		PATransitionManager(PATransitionData* transitionData, PAStateData* from, PAStateData* to);
		~PATransitionManager();

		void align(PAStateModule* current, PAStateModule* next);
		void blending(SrBuffer<float>& buffer, SrBuffer<float>&buffer1, SrBuffer<float>&buffer2, SrMat& mat, SrMat& mat1, SrMat& mat2, double timeStep, MeControllerContext* context);
		void update();
		double getSlope();
		int getNumEaseOut();

		static void bufferBlending(SrBuffer<float>& buffer, SrBuffer<float>& buffer1, SrBuffer<float>& buffer2, double w, MeControllerContext* context);

	private:
		double getTime(double time, std::vector<double> key, std::vector<std::vector<double> > keys, std::vector<double> w);
};

class PAControllerBlending
{
	public:
		PAControllerBlending();
		~PAControllerBlending();

		double getKey(double t);
		void addKey(double t, double weight);
		void updateBuffer(SrBuffer<float>& buff);
		SrBuffer<float>& getBuffer();

	private:
		SrBuffer<float> buffer;
		srLinearCurve* curve;
};

#endif
