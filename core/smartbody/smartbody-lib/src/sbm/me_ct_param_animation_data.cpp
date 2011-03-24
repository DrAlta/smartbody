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

PAStateData::PAStateData(std::string name)
{
	stateName = name;
	cycle = false;
	paramManager = new ParameterManager(this);
}

PAStateData::~PAStateData()
{
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

int PAStateData::getMotionId(std::string motion)
{
	for (int i = 0; i < getNumMotions(); i++)
	{
		std::string mName = motions[i]->name();
		if (motion == mName)
			return i;
	}
	return -1;
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
		setPrevVec(SrVec((float)x, 0.0f, 0.0f));
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
			setPrevVec(SrVec((float)x, 0.0, 0.0));
		}
		if (leftId >=0 && rightId < 0)
		{
			state->weights[leftId] = 1.0;
			setPrevVec(SrVec((float)left, 0.0, 0.0));
		}
		if (rightId >=0 && leftId < 0)
		{
			state->weights[rightId] = 1.0;
			setPrevVec(SrVec((float)right, 0.0, 0.0));
		}
	}
	return true;
}

bool ParameterManager::setWeight(double x, double y)
{
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
	for (int i = 0; i < state->getNumMotions(); i++)
		state->weights[i] = 0.0;
	if (triangleId >= 0)
	{
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
		int id = state->paramManager->getMotionId(state->motions[indices[0]]->name());
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
			int id = state->paramManager->getMotionId(state->motions[indices[i]]->name());
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

void ParameterManager::addParameter(std::string motion, double x)
{
	SrVec vec;
	vec.x = (float)x;
	motionNames.push_back(motion);
	parameters.push_back(vec);
	type = 0;
}

void ParameterManager::addParameter(std::string motion, double x, double y)
{
	SrVec vec;
	vec.x = (float)x;
	vec.y = (float)y;
	motionNames.push_back(motion);
	parameters.push_back(vec);
	type = 1;
}


void ParameterManager::addTriangle(std::string motion1, std::string motion2, std::string motion3)
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

SrVec ParameterManager::getVec(std::string motion)
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

std::string ParameterManager::getMotionName(int id)
{
	if (id < 0 || id > getNumParameters())
		return "";
	return motionNames[id];	
}

int ParameterManager::getMotionId(std::string name)
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
	w1 = area2 / totalArea;
	w2 = area3 / totalArea;
	w3 = area1 / totalArea;
}

MotionParameters::MotionParameters(SkMotion* m, SbmCharacter* c, std::string j)
{
	motion = m;
	skeleton = new SkSkeleton(c->skeleton_p);
	motion->connect(skeleton);
	if (j == "")
	{
		joint = skeleton->search_joint(motion->channels().name(0).get_string());
		std::string jName = joint->name().get_string();
	}
	else
		joint = skeleton->search_joint(j.c_str());
	minFrameId = 0;
	maxFrameId = motion->frames();
}

MotionParameters::~MotionParameters()
{
	motion->disconnect();
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
		return getAvgAngularSpeed() * 200;
	if (type == 3)
		return getAccAngularSpeed() * 200;
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

