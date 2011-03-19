/*
 *  me_ct_param_animation_data.h - part of Motion Engine and SmartBody-lib
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


#ifndef _ME_CT_PARAM_ANIMATION_DATA_H_
#define _ME_CT_PARAM_ANIMATION_DATA_H_
#include "me_ct_param_animation_utilities.h"
#include <sbm/sbm_character.hpp>

class ParameterManager;
class PAStateData
{
	public:
		std::string stateName;
		std::vector<SkMotion*> motions;
		std::vector<std::vector<double>> keys;
		std::vector<double> weights;
		std::vector<PAStateData*> toStates;
		std::vector<PAStateData*> fromStates;
		bool cycle;
		ParameterManager* paramManager;

	public:
		PAStateData(std::string name);
		~PAStateData();
		int getNumMotions();
		int getNumKeys();
		int getMotionId(std::string motion);
};

class PATransitionData
{
	public:
		PAStateData* fromState;
		PAStateData* toState;
		std::string fromMotionName;
		std::string toMotionName;
		double easeOutStart;
		double easeOutEnd;
		double easeInStart;
		double easeInEnd;
};

class ParameterManager
{
	public:
		ParameterManager(PAStateData* s);
		~ParameterManager();

		void setWeight(double x);
		void setWeight(double x, double y);
		void addParameter(std::string motion, double x);
		void addParameter(std::string motion, double x, double y);
		int getType();
		void setType(int typ);

		int getNumParameters();
		std::string getMinVec(int type);
		std::string getMaxVec(int type);
		SrVec getVec(std::string motion);
		SrVec getVec(int id);
		SrVec getPrevVec();
		void setPrevVec(SrVec& vec);
		std::string getMotionName(int id);

	private:
		bool insideTriangle(SrVec& pt, SrVec& v1, SrVec& v2, SrVec& v3);
		void getWeight(SrVec& pt, SrVec& v1, SrVec& v2, SrVec& v3, double& w1, double& w2, double& w3);

	private:
		int type;
		PAStateData* state;
		std::map<std::string, SrVec> parameterMaps;
		SrVec previousParam;
};

class MotionParameters
{
	public:
		MotionParameters(SkMotion* m, SbmCharacter* c, std::string jName = "");
		~MotionParameters();

		void setFrameId(int min, int max);
		void setFrameId(double min, double max);
		double getParameter(int type);

	private:
		double getAvgSpeed();
		double getAccSpeed();
		double getAvgAngularSpeed();
		double getAccAngularSpeed();

	private:
		int minFrameId;
		int maxFrameId;
		SkMotion* motion;
		SkSkeleton* skeleton;
		SkJoint* joint;
};

#endif