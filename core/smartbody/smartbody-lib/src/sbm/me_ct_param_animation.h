/*
 *  me_ct_param_animation.h - part of Motion Engine and SmartBody-lib
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

#ifndef _ME_CT_PARAM_ANIMATION_H_
#define _ME_CT_PARAM_ANIMATION_H_

#include <me/me_ct_container.hpp>
#include <sbm/sbm_character.hpp>
#include <sbm/me_ct_param_animation_utilities.h>
#include <sbm/me_ct_param_animation_data.h>


class PAState;
class PATransition;
class PAStateData;
class PATransitionManager;
class PAControllerBlending;

struct ScheduleUnit
{
	PAState* data;
	std::vector<double> weights;
	double time;
	bool loop;
	bool playNow;
	bool additive;
	std::string partialJoint;
};
/*
	MeCtParamAnimation's task:	schedule in states
								controller the transitions between states
								ease in and ease out the states
*/
class MeCtParamAnimation : public MeCtContainer
{
	public:
		static std::string CONTROLLER_TYPE;

		class Context : public MeCtContainer::Context 
		{
		protected:
			static std::string CONTEXT_TYPE;
		public:
			Context( MeCtParamAnimation* container, MeControllerContext* context = NULL )
				:	MeCtContainer::Context( container, context )
			{}

			const std::string& context_type() const {	return CONTEXT_TYPE; }
			void child_channels_updated( MeController* child );
		};

	public:
		MeCtParamAnimation();
		MeCtParamAnimation(SbmCharacter* c, MeCtChannelWriter* wo);
		~MeCtParamAnimation();

		virtual void controller_map_updated();
		virtual SkChannelArray& controller_channels();
		virtual double controller_duration();
		virtual const std::string& controller_type() const {return CONTROLLER_TYPE;}
		virtual bool controller_evaluate( double t, MeFrameData& frame );

		void setBaseJointName(const std::string& name);
		const std::string& getBaseJointName();
		
		void dumpScheduling();
		void schedule(PAState* state, const std::vector<double>& weights, bool l, bool pn = false, bool additive = false, std::string jName = "");
		void unschedule();
		void updateWeights(std::vector<double>& w);
		void updateWeights();
		
		int getNumWeights();
		
		const std::string& getNextStateName();
		const std::string& getCurrentStateName();
		PAStateData* getCurrentPAStateData();
		bool hasPAState(const std::string& stateName);
		bool isIdle();

	private:
		void autoScheduling(double time);
		PAStateData* createStateModule(ScheduleUnit su);
		void reset();
		void updateWo(SrMat&mat, MeCtChannelWriter* wo, SrBuffer<float>& buffer);
		void controllerEaseOut(double t);

	private:

		SkChannelArray	channels;
		SbmCharacter*	character;
		std::string baseJointName;
		MeCtChannelWriter* woWriter;
		PATransitionManager* transitionManager;
		PAControllerBlending* controllerBlending;
		double			prevGlobalTime;

		PAStateData*	curStateData;
		PAStateData*	nextStateData;
		std::list<ScheduleUnit> waitingList;
};
#endif