/*
 *  me_ct_param_animation_data.cpp - part of Motion Engine and SmartBody-lib
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

#include "me_ct_param_animation_data.h"
#include <sr/sr_euler.h>
#include <sbm/me_ct_ublas.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>

#ifdef USE_TETGEN
#include <external/tetgen/tetgen.h>
#endif

const double changeLimit = 20;

PAStateData::PAStateData()
{
	stateName = "unknown";
}

PAStateData::PAStateData(PAStateData* data)
{
	stateName = data->stateName;
	for (unsigned int i = 0; i < data->motions.size(); i++)
	{
		motions.push_back(data->motions[i]);
		data->motions[i]->ref();
	}

	for (unsigned int i = 0; i < data->keys.size(); i++)
	{
		std::vector<double> tempVec;
		for (unsigned int j = 0; j <data->keys[i].size(); j++)
			tempVec.push_back(data->keys[i][j]);
		keys.push_back(tempVec);
	}

	for (unsigned int i = 0; i < data->weights.size(); i++)
		weights.push_back(data->weights[i]);

	for (unsigned int i = 0; i < data->toStates.size(); i++)
		toStates.push_back(data->toStates[i]);

	for (unsigned int i = 0; i < data->fromStates.size(); i++)
		fromStates.push_back(data->fromStates[i]);
	
	cycle = data->cycle;
	paramManager = new ParameterManager(data->paramManager, this);
}


PAStateData::PAStateData(const std::string& name)
{
	stateName = name;
	cycle = false;
	paramManager = new ParameterManager(this);
}

PAStateData::~PAStateData()
{
	for (unsigned int i = 0; i < motions.size(); i++)
	{
		motions[i]->unref();
		motions[i] = NULL;
	}
	motions.clear();
	for (unsigned int i = 0; i < toStates.size(); i++)
		toStates[i] = NULL;
	toStates.clear();
	for (unsigned int i = 0; i < fromStates.size(); i++)
		fromStates[i] = NULL;
	fromStates.clear();

	delete paramManager;
}

int PAStateData::getNumMotions()
{
	return motions.size();
}

int PAStateData::getNumKeys()
{
	if (getNumMotions() > 0)
		return keys[0].size();
	else
		return 0;
}

int PAStateData::getMotionId(const std::string& motion)
{
	for (int i = 0; i < getNumMotions(); i++)
	{
		const std::string& mName = motions[i]->getName();
		if (motion == mName)
			return i;
	}
	return -1;
}


PATransitionData::PATransitionData()
{
}

PATransitionData::PATransitionData(PATransitionData* data, PAStateData* from, PAStateData* to)
{
	this->fromState = from;
	this->toState = to;
	this->fromMotionName = data->fromMotionName;
	this->toMotionName = data->toMotionName;
	for (unsigned int i = 0; i < data->easeOutStart.size(); i++)
		this->easeOutStart.push_back(data->easeOutStart[i]);
	for (unsigned int i = 0; i < data->easeOutEnd.size(); i++)
		this->easeOutEnd.push_back(data->easeOutEnd[i]);
	this->easeInStart = data->easeInStart;
	this->easeInEnd = data->easeInEnd;
}

PATransitionData::~PATransitionData()
{
	fromState = NULL;
	toState = NULL;
}

int PATransitionData::getNumEaseOut()
{
	return easeOutStart.size();
}

ParameterManager::ParameterManager(ParameterManager* pm, PAStateData* s)
{
	state = s;
	type = pm->getType();
	for (unsigned int i = 0; i < pm->getMotionNames().size(); i++)
		motionNames.push_back(pm->getMotionNames()[i]);
	for (unsigned int i = 0; i < pm->getParameters().size(); i++)
		parameters.push_back(pm->getParameters()[i]);

	for (unsigned int i = 0; i < pm->getTriangles().size(); i++)
		triangles.push_back(pm->getTriangles()[i]);
	for (unsigned int i = 0; i < pm->getTetrahedrons().size(); i++)
		tetrahedrons.push_back(pm->getTetrahedrons()[i]);
}

ParameterManager::ParameterManager(PAStateData* s)
{
	state = s;
	type = 0;
}

ParameterManager::~ParameterManager()
{
	state = NULL;
}

bool ParameterManager::setWeight(double x)
{
	if (type != 0)
		return false;

	//double xDiff = fabs(previousParam.x - x);
	//if (xDiff > changeLimit)
	//	x = (previousParam.x + x) * 0.5;
	double left = -9999.0;
	double right = 9999.0;
	std::string leftMotion = "";
	std::string rightMotion = "";
	for (int i = 0; i < getNumParameters(); i++)
	{
		if (parameters[i].x <= x)
			if (parameters[i].x >= left)	
			{
				left = parameters[i].x;
				leftMotion = motionNames[i];
			}
		if (parameters[i].x >= x)
			if (parameters[i].x <= right) 
			{
				right = parameters[i].x;
				rightMotion = motionNames[i];
			}
	}
	for (int i = 0; i < state->getNumMotions(); i++)
		state->weights[i] = 0.0;
	if (right == left)
	{
		int id = state->getMotionId(leftMotion);
		state->weights[id] = 1.0;
		SrVec tmp_vec((float)x, 0.0f, 0.0f);
		setPrevVec( tmp_vec );
	}
	else
	{
		double weight = (x - left) / (right - left);
		int leftId = state->getMotionId(leftMotion);
		int rightId = state->getMotionId(rightMotion);
		if (leftId >= 0 && rightId >= 0)
		{
			state->weights[leftId] = 1 - weight;
			state->weights[rightId] = weight;
			SrVec tmp_vec((float)x, 0.0, 0.0);
			setPrevVec( tmp_vec );
		}
		if (leftId >=0 && rightId < 0)
		{
			state->weights[leftId] = 1.0;
			SrVec tmp_vec((float)left, 0.0, 0.0);
			setPrevVec( tmp_vec );
		}
		if (rightId >=0 && leftId < 0)
		{
			state->weights[rightId] = 1.0;
			SrVec tmp_vec((float)right, 0.0, 0.0);
			setPrevVec( tmp_vec );
		}
	}
	return true;
}

bool ParameterManager::setWeight(double x, double y)
{
	if (type != 1)
		return false;
	
	//double xDiff = fabs(previousParam.x - x);
	//if (xDiff > changeLimit)
	//	x = (previousParam.x + x) * 0.5;
	//double yDiff = fabs(previousParam.y - y);
	//if (yDiff > changeLimit)
	//	y = (previousParam.y + y) * 0.5;

	SrVec pt = SrVec((float)x, (float)y, 0);
	for (int i = 0; i < getNumTriangles(); i++)
	{
		SrVec v1 = triangles[i].triangle.a;
		SrVec v2 = triangles[i].triangle.b;
		SrVec v3 = triangles[i].triangle.c;
		int id1 = state->getMotionId(triangles[i].motion1);
		int id2 = state->getMotionId(triangles[i].motion2);
		int id3 = state->getMotionId(triangles[i].motion3);
		bool in = insideTriangle(pt, v1, v2, v3);
		if (in)
		{
			for (int i = 0; i < state->getNumMotions(); i++)
				state->weights[i] = 0.0;
			getWeight(pt, v1, v2, v3, state->weights[id1], state->weights[id2], state->weights[id3]);
			setPrevVec(pt);
			return true;
		}
	}

	// find the nearest dist
	SrVec param;
	float minDist = 9999;
	int triangleId = -1;
	for (int i = 0; i < getNumTriangles(); i++)
	{
		SrVec v1 = triangles[i].triangle.a;
		SrVec v2 = triangles[i].triangle.b;
		SrVec v3 = triangles[i].triangle.c;
		SrVec vec;
		float dist = getMinimumDist(pt, v1, v2, vec);
		if (dist <= minDist)
		{
			minDist = dist;
			param = vec;
			triangleId = i;
		}
		dist = getMinimumDist(pt, v3, v2, vec);
		if (dist <= minDist)
		{
			minDist = dist;
			param = vec;
			triangleId = i;
		}
		dist = getMinimumDist(pt, v1, v3, vec);
		if (dist <= minDist)
		{
			minDist = dist;
			param = vec;
			triangleId = i;
		}
	}

	if (triangleId >= 0)
	{
		for (int i = 0; i < state->getNumMotions(); i++)
			state->weights[i] = 0.0;

		SrVec v1 = triangles[triangleId].triangle.a;
		SrVec v2 = triangles[triangleId].triangle.b;
		SrVec v3 = triangles[triangleId].triangle.c;
		int id1 = state->getMotionId(triangles[triangleId].motion1);
		int id2 = state->getMotionId(triangles[triangleId].motion2);
		int id3 = state->getMotionId(triangles[triangleId].motion3);
		getWeight(param, v1, v2, v3, state->weights[id1], state->weights[id2], state->weights[id3]);
		setPrevVec(param);
	}
	return true;
}

bool ParameterManager::setWeight(double x, double y, double z)
{
	if (type != 2)
		return false;

	// parameter sudden change detect
	//double zDiff = fabs(previousParam.z - z);
	//if (zDiff > changeLimit)
	//	z = (previousParam.z + z) * 0.5;
	//
	//double xDiff = fabs(previousParam.x - x);
	//if (xDiff > changeLimit)
	//	x = (previousParam.x + x) * 0.5;
	//double yDiff = fabs(previousParam.y - y);
	//if (yDiff > changeLimit)
	//	y = (previousParam.y + y) * 0.5;


	SrVec pt = SrVec((float)x, (float)y, (float)z);
	for (unsigned int i = 0; i < tetrahedrons.size(); i++)
	{
		SrVec v1 = tetrahedrons[i].v1;
		SrVec v2 = tetrahedrons[i].v2;
		SrVec v3 = tetrahedrons[i].v3;
		SrVec v4 = tetrahedrons[i].v4;
		int id1 = state->getMotionId(tetrahedrons[i].motion1);
		int id2 = state->getMotionId(tetrahedrons[i].motion2);
		int id3 = state->getMotionId(tetrahedrons[i].motion3);
		int id4 = state->getMotionId(tetrahedrons[i].motion4);
		double w1 = 0.0;	
		double w2 = 0.0;
		double w3 = 0.0;
		double w4 = 0.0;
		getWeight(pt, v1, v2, v3, v4, w1, w2, w3, w4);
		if (w1 >= 0 && w2 >= 0 && w3 >= 0 && w4 >= 0)
		{
			setPrevVec(pt);
	//		std::cout << state->motions[id1]->name() << " " << state->motions[id2]->name() << " " << state->motions[id3]->name() << " " << state->motions[id4]->name() << std::endl;
			for (int i = 0; i < state->getNumMotions(); i++)
				state->weights[i] = 0.0;

			state->weights[id1] = w1;
			state->weights[id2] = w2;
			state->weights[id3] = w3;
			state->weights[id4] = w4;
			return true;
		} 
	}

	// if it's not inside any tetrahedron, find the most close one
	// refer to <Real-Time Collision Detection>
	int id = -1;
	float min = 99999;
	SrVec param;

	for (unsigned int i = 0; i < tetrahedrons.size(); i++)
	{
		float bestSqDist = 9999;
		SrVec a = tetrahedrons[i].v1;
		SrVec b = tetrahedrons[i].v2;
		SrVec c = tetrahedrons[i].v3;
		SrVec d = tetrahedrons[i].v4;
		SrVec closestPt = a;

		if (PointOutsideOfPlane(pt, a, b, c))
		{
			SrVec q = closestPtPointTriangle(pt, a, b, c);
			float sqDist = dot(q - pt, q - pt);
			if (sqDist < bestSqDist)
			{
				bestSqDist = sqDist;
				closestPt = q;
			}
		}
		if (PointOutsideOfPlane(pt, a, c, d))
		{
			SrVec q = closestPtPointTriangle(pt, a, c, d);
			float sqDist = dot(q - pt, q - pt);
			if (sqDist < bestSqDist)
			{
				bestSqDist = sqDist;
				closestPt = q;
			}
		}
		if (PointOutsideOfPlane(pt, a, d, b))
		{
			SrVec q = closestPtPointTriangle(pt, a, d, b);
			float sqDist = dot(q - pt, q - pt);
			if (sqDist < bestSqDist)
			{
				bestSqDist = sqDist;
				closestPt = q;
			}
		}
		if (PointOutsideOfPlane(pt, b, d, c))
		{
			SrVec q = closestPtPointTriangle(pt, b, d, c);
			float sqDist = dot(q - pt, q - pt);
			if (sqDist < bestSqDist)
			{
				bestSqDist = sqDist;
				closestPt = q;
			}
		}

		if (bestSqDist <= min)
		{
			param = closestPt;
			id = i;
			min = bestSqDist;
		}
	}

	if (id >= 0)
	{
		SrVec v1 = tetrahedrons[id].v1;
		SrVec v2 = tetrahedrons[id].v2;
		SrVec v3 = tetrahedrons[id].v3;
		SrVec v4 = tetrahedrons[id].v4;
		int id1 = state->getMotionId(tetrahedrons[id].motion1);
		int id2 = state->getMotionId(tetrahedrons[id].motion2);
		int id3 = state->getMotionId(tetrahedrons[id].motion3);
		int id4 = state->getMotionId(tetrahedrons[id].motion4);
		double w1 = 0.0;	
		double w2 = 0.0;
		double w3 = 0.0;
		double w4 = 0.0;
		getWeight(param, v1, v2, v3, v4, w1, w2, w3, w4);
		if (w1 >= 0 && w2 >= 0 && w3 >= 0 && w4 >= 0)
		{
			setPrevVec(param);
	//		std::cout << state->motions[id1]->name() << " " << state->motions[id2]->name() << " " << state->motions[id3]->name() << " " << state->motions[id4]->name() << std::endl;
			for (int i = 0; i < state->getNumMotions(); i++)
				state->weights[i] = 0.0;
			state->weights[id1] = w1;
			state->weights[id2] = w2;
			state->weights[id3] = w3;
			state->weights[id4] = w4;

			return true;
		}
		else
		{
			LOG("Not inside tetrahedron.");
		}
	}
	return false;
}

void ParameterManager::getParameter(float& x)
{
	x = 0.0f;
	for (int i = 0; i < getNumParameters(); i++)
	{
		int id = state->getMotionId(motionNames[i]);
		x += (float)state->weights[id] * parameters[i].x;
	}
}

void ParameterManager::getParameter(float& x, float& y)
{
	std::vector<int> indices;
	for (int i = 0; i < state->getNumMotions(); i++)
	{
		if (state->weights[i] > 0.0)
			indices.push_back(i);
	}
	if (indices.size() == 0)
		return;
	else if (indices.size() == 1)
	{
		int id = state->paramManager->getMotionId(state->motions[indices[0]]->getName());
		if (id >= 0)
		{
			x = state->paramManager->getVec(id).x;
			y = state->paramManager->getVec(id).y;
		}
		else
			return;
	}
	else
	{
		std::vector<SrVec> vecs;
		for (size_t i = 0; i < indices.size(); i++)
		{
			int id = state->paramManager->getMotionId(state->motions[indices[i]]->getName());
			if (id >= 0)
				vecs.push_back(state->paramManager->getVec(id));
			else
				return;
		}
		SrVec vec;
		for (size_t i = 0; i < indices.size(); i++)
			vec = vec + vecs[i] * (float)state->weights[indices[i]];
		x = vec.x;
		y = vec.y;
	}
}

void ParameterManager::getParameter(float& x, float& y, float& z)
{
	std::vector<int> indices;
	for (int i = 0; i < state->getNumMotions(); i++)
	{
		if (state->weights[i] > 0.0)
			indices.push_back(i);
	}
	if (indices.size() == 0)
		return;
	else if (indices.size() == 1)
	{
		int id = state->paramManager->getMotionId(state->motions[indices[0]]->getName());
		if (id >= 0)
		{
			x = state->paramManager->getVec(id).x;
			y = state->paramManager->getVec(id).y;
			z = state->paramManager->getVec(id).z;
		}
		else
			return;
	}
	else
	{
		std::vector<SrVec> vecs;
		for (size_t i = 0; i < indices.size(); i++)
		{
			int id = state->paramManager->getMotionId(state->motions[indices[i]]->getName());
			if (id >= 0)
				vecs.push_back(state->paramManager->getVec(id));
			else
				return;
		}
		SrVec vec;
		for (size_t i = 0; i < indices.size(); i++)
			vec = vec + vecs[i] * (float)state->weights[indices[i]];
		x = vec.x;
		y = vec.y;
		z = vec.z;
	}
}

void ParameterManager::addParameter(const std::string& motion, double x)
{
	SrVec vec;
	vec.x = (float)x;
	motionNames.push_back(motion);
	parameters.push_back(vec);
	type = 0;
}

void ParameterManager::addParameter(const std::string& motion, double x, double y)
{
	SrVec vec;
	vec.x = (float)x;
	vec.y = (float)y;
	motionNames.push_back(motion);
	parameters.push_back(vec);
	type = 1;
}

void ParameterManager::addParameter(const std::string& motion, double x, double y, double z)
{
	SrVec vec;
	vec.x = (float)x;
	vec.y = (float)y;
	vec.z = (float)z;
	motionNames.push_back(motion);
	parameters.push_back(vec);
	type = 2;
}

void ParameterManager::addTriangle(const std::string& motion1, const std::string& motion2, const std::string& motion3)
{
	TriangleInfo tInfo;
	SrVec v1 = getVec(motion1);
	SrVec v2 = getVec(motion2);
	SrVec v3 = getVec(motion3);
	tInfo.triangle = SrTriangle(v1, v2, v3);
	tInfo.motion1 = motion1;
	tInfo.motion2 = motion2;
	tInfo.motion3 = motion3;
	triangles.push_back(tInfo);
}

void ParameterManager::addTetrahedron(const std::string& motion1, const std::string& motion2, const std::string& motion3, const std::string& motion4)
{
	TetrahedronInfo tetraInfo;
	tetraInfo.v1 = getVec(motion1);
	tetraInfo.v2 = getVec(motion2);
	tetraInfo.v3 = getVec(motion3);
	tetraInfo.v4 = getVec(motion4);
	tetraInfo.motion1 = motion1;
	tetraInfo.motion2 = motion2;
	tetraInfo.motion3 = motion3;
	tetraInfo.motion4 = motion4;
	tetrahedrons.push_back(tetraInfo);
}

void ParameterManager::buildTetrahedron()
{
#if USE_TETGEN
	tetgenio ptIn, tetOut;
	// initialize input points
	ptIn.numberofpoints = parameters.size();
	ptIn.pointlist = new REAL[parameters.size() * 3];
	for (unsigned int i = 0; i < parameters.size(); i++)
	{
		ptIn.pointlist[i*3+0] = parameters[i].x;
		ptIn.pointlist[i*3+1] = parameters[i].y;
		ptIn.pointlist[i*3+2] = parameters[i].z;		
	}
	tetrahedralize((char*)"V",&ptIn,&tetOut);
//	std::cout << "Built Tetrahedron:" << std::endl;
	for (int i = 0; i < tetOut.numberoftetrahedra; i++)
	{
		std::string motions[4];
		for (int k = 0;k < tetOut.numberofcorners; k++)
		{
			int id = tetOut.tetrahedronlist[i * tetOut.numberofcorners + k];
			motions[k] = motionNames[id];
		}
//		std::cout << "[" << i << "]: " << motions[0] << ", " << motions[1] << ", " << motions[2] << ", " << motions[3] << std::endl; 
		addTetrahedron(motions[0], motions[1], motions[2], motions[3]);
	}
#endif
}

int ParameterManager::getType()
{
	return type;
}

void ParameterManager::setType(int typ)
{
	type = typ;
}

int ParameterManager::getNumParameters()
{
	return parameters.size();
}

int ParameterManager::getMinVecX()
{
	float min = 9999;
	int ret = -1;
	for (int i = 0; i < getNumParameters(); i++)
	{
		if (parameters[i].x < min)
		{
			min = parameters[i].x;
			ret = i;
		}
	}
	return ret;
}

int ParameterManager::getMinVecY()
{
	float min = 9999;
	int ret = -1;
	for (int i = 0; i < getNumParameters(); i++)
	{
		if (parameters[i].y < min)
		{
			min = parameters[i].y;
			ret = i;
		}
	}
	return ret;
}

int ParameterManager::getMaxVecX()
{
	float max = -9999;
	int ret = -1;
	for (int i = 0; i < getNumParameters(); i++)
	{
		if (parameters[i].x > max)
		{
			max = parameters[i].x;
			ret = i;
		}
	}
	return ret;
}

int ParameterManager::getMaxVecY()
{
	float max = -9999;
	int ret = -1;
	for (int i = 0; i < getNumParameters(); i++)
	{
		if (parameters[i].y > max)
		{
			max = parameters[i].y;
			ret = i;
		}
	}
	return ret;
}

SrVec ParameterManager::getVec(const std::string& motion)
{
	for (int i = 0; i < getNumParameters(); i++)
	{
		if (motionNames[i] == motion)
			return parameters[i];
	}
	return SrVec();
}

SrVec ParameterManager::getVec(int id)
{
	if (id < 0 || id > getNumParameters())
		return SrVec();
	return parameters[id];
}

SrVec ParameterManager::getPrevVec()
{
	return previousParam;
}

void ParameterManager::setPrevVec(SrVec& vec)
{
	previousParam = vec;
}

const std::string& ParameterManager::getMotionName(int id)
{
	if (id < 0 || id > getNumParameters())
		return emptyString;

	return motionNames[id];	
}

int ParameterManager::getMotionId(const std::string& name)
{
	for (int i = 0; i < getNumParameters(); i++)
	{
		if (motionNames[i] == name)
			return i;
	}
	return -1;
}

int ParameterManager::getNumTriangles()
{
	return triangles.size();
}

SrTriangle& ParameterManager::getTriangle(int id)
{
	return triangles[id].triangle;
}

float ParameterManager::getMinimumDist(SrVec& c, SrVec& a, SrVec& b, SrVec& minimumPt)
{
	SrVec ab = b - a;
	SrVec ac = c - a;
	float f = dot(ab, ac);
	float d = dot(ab, ab);
	if (f < 0)
	{
		minimumPt = a;
		return dist(c, a);
	}
	if (f > d)
	{
		minimumPt = b;
		return dist(c, b);
	}
	f = f / d;
	minimumPt = a + f * ab;
	return dist(c, minimumPt);
}

bool ParameterManager::insideTriangle(SrVec& pt, SrVec& v1, SrVec& v2, SrVec& v3)
{
	SrVec ptToV1 = v1 - pt;
	SrVec ptToV2 = v2 - pt;
	SrVec ptToV3 = v3 - pt;
	SrVec cross1 = cross(ptToV1, ptToV2);
	SrVec cross2 = cross(ptToV2, ptToV3);
	SrVec cross3 = cross(ptToV3, ptToV1);
	float dot1 = dot(cross1, cross2);
	float dot2 = dot(cross2, cross3);
	float dot3 = dot(cross3, cross1);
	if (dot1 >= 0 && dot2 >= 0 && dot3 >= 0)
		return true;
	else
		return false;
}

void ParameterManager::getWeight(SrVec& pt, SrVec& v1, SrVec& v2, SrVec& v3, double& w1, double& w2, double& w3)
{
	SrVec ptToV1 = v1 - pt;
	SrVec ptToV2 = v2 - pt;
	SrVec ptToV3 = v3 - pt;
	SrVec cross1 = cross(ptToV1, ptToV2);
	double area1 = cross1.len();
	SrVec cross2 = cross(ptToV2, ptToV3);
	double area2 = cross2.len();
	SrVec cross3 = cross(ptToV3, ptToV1);
	double area3 = cross3.len();
	double totalArea = area1 + area2 + area3;
	if (totalArea == 0)
	{
		w1 = 1.0;
		w2 = 0.0;
		w3 = 0.0;
		LOG("ParameterManager::getWeight Warning: parameters not set correctly, check the initalization.");
	}
	else
	{
		w1 = area2 / totalArea;
		w2 = area3 / totalArea;
		w3 = area1 / totalArea;
	}
}

void ParameterManager::getWeight(SrVec& pt, SrVec& v1, SrVec& v2, SrVec& v3, SrVec& v4, double& w1, double& w2, double& w3, double& w4)
{
	dMatrix mat(3, 3);
	mat(0, 0) = v1.x - v4.x;
	mat(0, 1) = v2.x - v4.x;
	mat(0, 2) = v3.x - v4.x;
	mat(1, 0) = v1.y - v4.y;
	mat(1, 1) = v2.y - v4.y;
	mat(1, 2) = v3.y - v4.y;
	mat(2, 0) = v1.z - v4.z;
	mat(2, 1) = v2.z - v4.z;
	mat(2, 2) = v3.z - v4.z;
	dMatrix invMat(3, 3);
	MeCtUBLAS::inverseMatrix(mat,invMat);
	dVector vecIn(3);
	dVector vecOut(3);
	vecIn(0) = pt.x - v4.x;
	vecIn(1) = pt.y - v4.y;
	vecIn(2) = pt.z - v4.z;
	MeCtUBLAS::matrixVecMult(invMat, vecIn, vecOut);
	w1 = vecOut(0);
	w2 = vecOut(1);
	w3 = vecOut(2);
	w4 = 1 - vecOut(0) - vecOut(1) - vecOut(2);
	if (fabs(w1) <= 0.00001)	
		w1 = 0.0;
	if (fabs(w2) <= 0.00001)	
		w2 = 0.0;
	if (fabs(w3) <= 0.00001)	
		w3 = 0.0;
	if (fabs(w4) <= 0.00001)	
		w4 = 0.0;
}


SrVec ParameterManager::closestPtPointTriangle(SrVec& p, SrVec& a, SrVec& b, SrVec& c)
{
	SrVec ab = b - a;
	SrVec ac = c - a;
	SrVec ap = p - a;
	float d1 = dot(ab, ap);
	float d2 = dot(ac, ap);
	if (d1 <= 0.0f && d2 <= 0.0f) return a;
	
	SrVec bp = p - b;
	float d3 = dot(ab, bp);
	float d4 = dot(ac, bp);
	if (d3 >= 0.0f && d4 <= d3) return b;

	float vc = d1 * d4 - d3 * d2;
	if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
	{
		float v = d1 / (d1 - d3);
		return a + v * ab;
	}

	SrVec cp = p - c;
	float d5 = dot(ab, cp);
	float d6 = dot(ac, cp);
	if (d6 >= 0.0f && d5 <= d6) return c;

	float vb = d5 * d2 - d1 * d6;
	if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
	{
		float w = d2 / (d2 - d6);
		return a + w * ac;
	}

	float va = d3 * d6 - d5 * d4;
	if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
	{
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		return b + w * (c - b);
	}

	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	return a + ab * v + ac * w;
}

int ParameterManager::PointOutsideOfPlane(SrVec p, SrVec a, SrVec b, SrVec c)
{
	return dot(p - a, cross(b - a, c - a)) >= 0.0f;
}

MotionParameters::MotionParameters(SkMotion* m, SkSkeleton* skel, std::string j)
{
	motion = m;
	motion->ref();
	skeleton = new SkSkeleton(skel);
	skeleton->ref();
	motion->connect(skeleton);
	if (j == "")
	{
		joint = skeleton->search_joint(motion->channels().name(0).c_str());
	}
	else
		joint = skeleton->search_joint(j.c_str());
	minFrameId = 0;
	maxFrameId = motion->frames();
}

MotionParameters::~MotionParameters()
{
	motion->disconnect();
	if (motion)
		motion->unref();
	if (skeleton)
		skeleton->unref();
}


void MotionParameters::setFrameId(int min, int max)
{
	minFrameId = min;
	maxFrameId = max;
}

void MotionParameters::setFrameId(double min, double max)
{
	double dt = motion->duration() / double(motion->frames() - 1);
	minFrameId = int(min / dt);
	maxFrameId = int(max / dt);
}

// TODO: Need Normalize
double MotionParameters::getParameter(int type)
{
	if (type == 0)
		return getAvgSpeed();
	if (type == 1)
		return getAccSpeed();
	if (type == 2)
		return getAvgAngularSpeed() * 100;
	if (type == 3)
		return getAccAngularSpeed() * 100;
	if (type == 4)
		return getTransitionX();
	if (type == 5)
		return getTransitionY();
	if (type == 6)
		return getTransitionZ();
	if (type == 7)
		return getAvgRootJointY();
	return -1.0;
}

double MotionParameters::getAvgSpeed()
{
	motion->apply_frame(minFrameId);
	skeleton->update_global_matrices();
	const SrMat& srcMat = joint->gmat();
	SrVec srcPt = SrVec(srcMat.get(12), srcMat.get(13), srcMat.get(14));
	motion->apply_frame(maxFrameId);
	skeleton->update_global_matrices();
	const SrMat& destMat = joint->gmat();
	SrVec destPt = SrVec(destMat.get(12), destMat.get(13), destMat.get(14));
	float distance = dist(srcPt, destPt);
	double avgSpd = double(distance / motion->duration());
	return avgSpd;
}

double MotionParameters::getAccSpeed()
{
	float distance = 0;
	for (int i = minFrameId; i < maxFrameId - 1; i++)
	{
		motion->apply_frame(i);
		skeleton->update_global_matrices();
		const SrMat& srcMat = joint->gmat();
		SrVec srcPt = SrVec(srcMat.get(12), srcMat.get(13), srcMat.get(14));
		motion->apply_frame(i + 1);
		skeleton->update_global_matrices();
		const SrMat& destMat = joint->gmat();
		SrVec destPt = SrVec(destMat.get(12), destMat.get(13), destMat.get(14));
		distance += dist(srcPt, destPt);
	}
	double accSpd = double(distance / motion->duration());
	return accSpd;
}

double MotionParameters::getAvgAngularSpeed()
{
	motion->apply_frame(minFrameId);
	skeleton->update_global_matrices();
	const SrMat& srcMat = joint->gmat();
	float rx, ry, rz;
	sr_euler_angles(rotType, srcMat, rx, ry, rz);
	float srcRotY = ry;
	motion->apply_frame(maxFrameId);
	skeleton->update_global_matrices();
	const SrMat& destMat = joint->gmat();
	sr_euler_angles(rotType, destMat, rx, ry, rz);
	float destRotY = ry;
	float diffRotY = destRotY - srcRotY;
	double avgAngularSpd = double(diffRotY / motion->duration());
	return avgAngularSpd;
}

double MotionParameters::getAccAngularSpeed()
{
	float diffRotY = 0.0;
	for (int i = minFrameId; i < maxFrameId - 1; i++)
	{
		motion->apply_frame(i);
		skeleton->update_global_matrices();
		const SrMat& srcMat = joint->gmat();
		float rx, ry, rz;
		sr_euler_angles(rotType, srcMat, rx, ry, rz);
		float srcRotY = ry;
		motion->apply_frame(i + 1);
		skeleton->update_global_matrices();
		const SrMat& destMat = joint->gmat();
		sr_euler_angles(rotType, destMat, rx, ry, rz);
		float destRotY = ry;
		float diff;
		if (destRotY * srcRotY < 0 && fabs(destRotY) > 1.0f)
			diff = - destRotY - srcRotY;
		else
			diff = destRotY - srcRotY;
		diffRotY += diff;
	}
	double accAngularSpd = double(diffRotY / motion->duration());
	return accAngularSpd;
}

double MotionParameters::getTransitionX()
{
	motion->apply_frame(minFrameId);
	skeleton->update_global_matrices();
	const SrMat& srcMat = joint->gmat();
	const SrMat& srcMat0 = joint->gmatZero();
	SrVec srcPnt = SrVec(srcMat.get(12), srcMat.get(13), srcMat.get(14));
	float rx, ry, rz;
	float rx0, ry0, rz0;
	sr_euler_angles(rotType, srcMat, rx, ry, rz);
	sr_euler_angles(rotType, srcMat0, rx0, ry0, rz0);
	motion->apply_frame(maxFrameId);
	skeleton->update_global_matrices();
	const SrMat& destMat = joint->gmat();
	SrVec destPnt = SrVec(destMat.get(12), destMat.get(13), destMat.get(14));
	SrVec transitionVec = destPnt - srcPnt;
	SrVec heading = SrVec(sin(ry - ry0 - 1.57f), 0, cos(ry - ry0 - 1.57f));
	double x = dot(transitionVec, heading);
	return x;
}

double MotionParameters::getTransitionY()
{
	motion->apply_frame(minFrameId);
	skeleton->update_global_matrices();
	const SrMat& srcMat = joint->gmat();
	const SrMat& srcMat0 = joint->gmatZero();
	SrVec srcPnt = SrVec(srcMat.get(12), srcMat.get(13), srcMat.get(14));
	float rx, ry, rz;
	float rx0, ry0, rz0;
	sr_euler_angles(rotType, srcMat, rx, ry, rz);
	sr_euler_angles(rotType, srcMat0, rx0, ry0, rz0);
	motion->apply_frame(maxFrameId);
	skeleton->update_global_matrices();
	const SrMat& destMat = joint->gmat();
	SrVec destPnt = SrVec(destMat.get(12), destMat.get(13), destMat.get(14));
	SrVec transitionVec = destPnt - srcPnt;
	SrVec heading = SrVec(sin(ry - ry0), 0, cos(ry - ry0));
	double y = dot(transitionVec, heading);
	return y;
}

double MotionParameters::getTransitionZ()
{
	motion->apply_frame(minFrameId);
	skeleton->update_global_matrices();
	const SrMat& srcMat = joint->gmat();
	motion->apply_frame(maxFrameId);
	skeleton->update_global_matrices();
	const SrMat& destMat = joint->gmat();
	double z = destMat.get(14) - srcMat.get(14);
	return z;
}

double MotionParameters::getAvgRootJointY()
{
	int numFrames = maxFrameId - minFrameId;
	if (numFrames == 0)
		return 0;
	double y = 0.0;
	for (int i = 0; i < numFrames; i++)
	{
		motion->apply_frame(minFrameId + i);
		skeleton->update_global_matrices();
		const SrMat& mat = joint->gmat();
		y += mat.get(13);
	}
	y = y / (double)numFrames;
	return y;
}