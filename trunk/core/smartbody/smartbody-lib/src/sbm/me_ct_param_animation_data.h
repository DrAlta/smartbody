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
#include <sr/sr_triangle.h>

const std::string PseudoIdleState = "PseudoIdle";
class ParameterManager;

//There are PAStateData stored inside mcu.
//Everytime PAStateModule is created, it would copy the data from mcu.param_animation_states. 
//toStates and fromStates are just pointers and they are not that useful
class PAStateData
{
	public:
		PAStateData();
		PAStateData(PAStateData* data);
		PAStateData(const std::string& name);
		~PAStateData();

		std::string stateName;
		std::vector<SkMotion*> motions;
		std::vector<std::vector<double> > keys;
		std::vector<double> weights;
		std::vector<PAStateData*> toStates;				
		std::vector<PAStateData*> fromStates;
		bool cycle;
		ParameterManager* paramManager;


		virtual int getNumMotions();
		virtual int getNumKeys();
		int getMotionId(const std::string& motion);
};

//There are PATransitionData stored inside mcu
//Everytime PATransitionManager is created, it would copy the data from mcu.param_animation_transitions.
//fromState and toState are just pointers and they have to be changed when PATransitionManager is created.
class PATransitionData
{
	public:
		PATransitionData();
		PATransitionData(PATransitionData* data, PAStateData* from, PAStateData* to);
		~PATransitionData();

	public:
		PAStateData* fromState;
		PAStateData* toState;
		std::string fromMotionName;
		std::string toMotionName;
		std::vector<double> easeOutStart;
		std::vector<double> easeOutEnd;
		double easeInStart;
		double easeInEnd;
		
	public:
		virtual int getNumEaseOut();
};

struct TriangleInfo
{
	SrTriangle triangle;
	std::string motion1;
	std::string motion2;
	std::string motion3;
};

struct TetrahedronInfo
{
	SrVec v1;
	SrVec v2;
	SrVec v3;
	SrVec v4;
	std::string motion1;
	std::string motion2;
	std::string motion3;
	std::string motion4;
};

class ParameterManager
{
	public:
		ParameterManager(ParameterManager* pm, PAStateData* s);
		ParameterManager(PAStateData* s);
		~ParameterManager();

		bool setWeight(double x);
		bool setWeight(double x, double y);
		bool setWeight(double x, double y, double z);
		void getParameter(float& x);
		void getParameter(float& x, float& y);
		void getParameter(float& x, float& y, float& z);
		void setParameter(const std::string& motion, double x);
		void setParameter(const std::string& motion, double x, double y);
		void setParameter(const std::string& motion, double x, double y, double z);
		void addTriangle(const std::string& motion1, const std::string& motion2, const std::string& motion3);
		void addTetrahedron(const std::string& motion1, const std::string& motion2, const std::string& motion3, const std::string& motion4);
		void buildTetrahedron();
		int getType();
		void setType(int typ);

		int getNumParameters();
		int getMinVecX();
		int getMinVecY();
		int getMaxVecX();
		int getMaxVecY();
		SrVec getVec(const std::string& motion);
		SrVec getVec(int id);
		SrVec getPrevVec();
		void setPrevVec(SrVec& vec);
		const std::string& getMotionName(int id);
		int getMotionId(const std::string& name);

		int getNumTriangles();
		SrTriangle& getTriangle(int id);
		float getMinimumDist(SrVec& pt, SrVec& a, SrVec& b, SrVec& minimumPt);

		// access data
		PAStateData* getState() {return state;}
		std::vector<std::string>& getMotionNames() {return motionNames;}
		std::vector<SrVec>& getParameters() {return parameters;}
		std::vector<TriangleInfo>& getTriangles() {return triangles;}
		std::vector<TetrahedronInfo> & getTetrahedrons() {return tetrahedrons;}

	private:
		bool insideTriangle(SrVec& pt, SrVec& v1, SrVec& v2, SrVec& v3);
		void getWeight(SrVec& pt, SrVec& v1, SrVec& v2, SrVec& v3, double& w1, double& w2, double& w3);
		void getWeight(SrVec& pt, SrVec& v1, SrVec& v2, SrVec& v3, SrVec& v4, double& w1, double& w2, double& w3, double& w4);
		SrVec closestPtPointTriangle(SrVec& pt, SrVec& v1, SrVec& v2, SrVec& v3);
		int PointOutsideOfPlane(SrVec p, SrVec a, SrVec b, SrVec c);

	private:
		int type;
		PAStateData* state;
		std::vector<std::string> motionNames;
		std::vector<SrVec> parameters;
		SrVec previousParam;
		std::vector<TriangleInfo> triangles;
		std::vector<TetrahedronInfo> tetrahedrons;
		std::string emptyString;
};

class MotionParameters
{
	public:
		MotionParameters(SkMotion* m, SkSkeleton* skel, std::string jName = "");
		~MotionParameters();

		void setFrameId(int min, int max);
		void setFrameId(double min, double max);
		double getParameter(int type);

	private:
		double getAvgSpeed();
		double getAccSpeed();
		double getAvgAngularSpeed();
		double getAccAngularSpeed();
		double getTransitionX();
		double getTransitionY();
		double getTransitionZ();
		double getAvgRootJointY();

	private:
		int minFrameId;
		int maxFrameId;
		SkMotion* motion;
		SkSkeleton* skeleton;
		SkJoint* joint;
};

#endif
