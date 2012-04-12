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

class PAStateData;

class PATimeManager
{
	public:
		PATimeManager();
		PATimeManager(PAStateData* data);
		~PATimeManager();

		int getNumKeys();
		void updateLocalTimes(double time);
		bool step(double timeStep);
		void updateWeights();
		double getDuration();
		double getLocalTime(double motionTime, int motionIndex);
		void getParallelTimes(double time, std::vector<double>& times);
	
		std::vector<double> localTimes;			//always in ascending order
		std::vector<double> motionTimes;		//actual motion times get from localTimes
		std::vector<double> timeDiffs;			//time steps from last evaluation to this evaluation, get from motionTimes
		std::vector<double> key;				//key get from keys and current weights
		double localTime;
		double prevLocalTime;

		PAStateData* stateData;

	public:
	protected:
		void setKey();
		void setLocalTime();
		void setMotionTimes();
		int getSection(double time);
};

class PAMotions
{
	public:
		MeControllerContext* _context;
	
	protected:
		std::vector<SrBuffer<int> > motionContextMaps;
		JointChannelId baseChanId;
		JointChannelId baseBuffId;
		SrQuat basePrerot;

	public:
		PAMotions();
		PAMotions(PAStateData* data);
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
		PAStateData* stateData;
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
		PAWoManager(PAStateData* data);
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
		PAInterpolator(PAStateData* data);
		~PAInterpolator();

		std::vector<std::string> joints;	// joints to be blended, if this is defined which means partial, world offset would be ignored
		bool additive;

	public:
		void blending(std::vector<double>& times, SrBuffer<float>& buff);
		void setAdditiveMode(bool flag);
		void clearBlendingJoints();
		void setBlendingJoints(std::vector<std::string>& j);

	private:
		void handleBaseMatForBuffer(SrBuffer<float>& buff);
};

class PAState;
class PAStateData
{
	public:
		PAStateData();
		PAStateData(const std::string& stateName, std::vector<double>& w, bool l = true, bool pn = false);
		PAStateData(PAState* state, std::vector<double>& w, bool l = true, bool pn = false);
		~PAStateData();
		virtual void evaluate(double timeStep, SrBuffer<float>& buffer);

		std::vector<double> weights;

		PATimeManager* timeManager;
		PAWoManager* woManager;
		PAInterpolator* interpolator;

		bool loop;
		bool active;
		bool playNow;
		PAState* state;




	
};
/*
class PseudoPAStateData : public PAStateData
{
	public:
		PseudoPAStateData();
		~PseudoPAStateData();

		void evaluate(double timeStep, SrBuffer<float>& buffer);
};
*/
class PATransition;
class PATransitionManager
{
	public:
		
		PATransitionManager();
		PATransitionManager(double easeOutStart, double duration);
		PATransitionManager(PATransition* transition, PAStateData* from, PAStateData* to);
		~PATransitionManager();

		void align(PAStateData* current, PAStateData* next);
		void blending(SrBuffer<float>& buffer, SrBuffer<float>&buffer1, SrBuffer<float>&buffer2, SrMat& mat, SrMat& mat1, SrMat& mat2, double timeStep, MeControllerContext* context);
		void update();
		double getSlope();
		int getNumEaseOut();
		bool blendingMode;
		bool active;

		PAStateData* from;
		PAStateData* to;
		PATransition* transition;
		srLinearCurve* curve;
		double duration;
		double localTime;
		std::vector<double> easeOutStarts;
		std::vector<double> easeOutEnds;
		double s1;
		double e1;
		double s2;
		double e2;
		


		static void bufferBlending(SrBuffer<float>& buffer, SrBuffer<float>& buffer1, SrBuffer<float>& buffer2, double w, MeControllerContext* context);

	private:
		double getTime(double time, const std::vector<double>& key, const std::vector<std::vector<double> >& keys, const std::vector<double>& w);
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
