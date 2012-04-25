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
#include <sr/sr_triangle.h>

namespace SmartBody {
	class MotionEvent;
}


const std::string PseudoIdleState = "PseudoIdle";

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

class PAState
{
	public:
		PAState();
		PAState(PAState* data);
		PAState(const std::string& name);
		~PAState();

		bool getWeightsFromParameters(double x, std::vector<double>& weights);
		bool getWeightsFromParameters(double x, double y, std::vector<double>& weights);
		bool getWeightsFromParameters(double x, double y, double z, std::vector<double>& weights);
		void getParametersFromWeights(float& x, std::vector<double>& weights);
		void getParametersFromWeights(float& x, float& y, std::vector<double>& weights);
		void getParametersFromWeights(float& x, float& y, float& z, std::vector<double>& weights);
		void setParameter(const std::string& motion, double x);
		void setParameter(const std::string& motion, double x, double y);
		void setParameter(const std::string& motion, double x, double y, double z);
		void getParameter(const std::string& motion, double& x);
		void getParameter(const std::string& motion, double& x, double& y);
		void getParameter(const std::string& motion, double& x, double& y, double& z);
		void removeParameter(const std::string& motion);
		void addTriangle(const std::string& motion1, const std::string& motion2, const std::string& motion3);
		int getTriangleIndex(const std::string& motion1, const std::string& motion2, const std::string& motion3);
		void removeTriangle(const std::string& motion1, const std::string& motion2, const std::string& motion3);
		void removeTriangles(const std::string& motion);
		void addTetrahedron(const std::string& motion1, const std::string& motion2, const std::string& motion3, const std::string& motion4);
		void removeTetrahedron(const std::string& motion1, const std::string& motion2, const std::string& motion3, const std::string& motion4);
		void removeTetrahedrons(const std::string& motion);
		int getTetrahedronIndex(const std::string& motion1, const std::string& motion2, const std::string& motion3, const std::string& motion4);
		void buildTetrahedron();
		int getType();
		void setType(int typ);
		double getLocalTime(double motionTime, int motionIndex);
		double getMotionTime(double localTime, int motionIndex);

		int getNumParameters();
		int getMinVecX();
		int getMinVecY();
		int getMaxVecX();
		int getMaxVecY();
		SrVec getVec(const std::string& motion);
		SrVec getVec(int id);
		const std::string& getMotionName(int id);
		int getMotionId(const std::string& name);

		int getNumTriangles();
		SrTriangle& getTriangle(int id);
		float getMinimumDist(SrVec& pt, SrVec& a, SrVec& b, SrVec& minimumPt);

		std::vector<SrVec>& getParameters() {return parameters;}
		std::vector<TriangleInfo>& getTriangles() {return triangles;}
		std::vector<TetrahedronInfo> & getTetrahedrons() {return tetrahedrons;}

		std::vector<std::pair<SmartBody::MotionEvent*, int> >& getEvents();
		void addEventToMotion(const std::string& motion, SmartBody::MotionEvent* motionEvent);

		std::string stateName;
		std::vector<SkMotion*> motions;
		std::vector<std::vector<double> > keys;

		bool cycle;

		virtual int getNumMotions();
		virtual int getNumKeys();

	protected:
		void updateParameterScale();
		bool insideTriangle(SrVec& pt, SrVec& v1, SrVec& v2, SrVec& v3);
		void getWeight(SrVec& pt, SrVec& v1, SrVec& v2, SrVec& v3, double& w1, double& w2, double& w3);
		void getWeight(SrVec& pt, SrVec& v1, SrVec& v2, SrVec& v3, SrVec& v4, double& w1, double& w2, double& w3, double& w4);
		SrVec closestPtPointTriangle(SrVec& pt, SrVec& v1, SrVec& v2, SrVec& v3);
		int PointOutsideOfPlane(SrVec p, SrVec a, SrVec b, SrVec c);
		SrVec vecMultiply(SrVec& vec1, SrVec& vec2);

		int type;
		//std::vector<std::string> motionNames;
		std::vector<SrVec> parameters;
		SrVec parameterScale;
		std::vector<TriangleInfo> triangles;
		std::vector<TetrahedronInfo> tetrahedrons;
		std::string emptyString;
		std::vector<std::pair<SmartBody::MotionEvent*, int> > _events;
};

//There are PATransition stored inside mcu
//Everytime PATransitionManager is created, it would copy the data from mcu.param_animation_transitions.
//fromState and toState are just pointers and they have to be changed when PATransitionManager is created.
class PATransition
{
	public:
		PATransition();
		PATransition(PATransition* data, PAState* from, PAState* to);
		~PATransition();

	public:
		PAState* fromState;
		PAState* toState;
		std::string fromMotionName;
		std::string toMotionName;
		std::vector<double> easeOutStart;
		std::vector<double> easeOutEnd;
		double easeInStart;
		double easeInEnd;
		
	public:
		virtual int getNumEaseOut();
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
