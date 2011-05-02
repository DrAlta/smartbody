#pragma once
#include <SR/sr_mat.h>
#include <SR/sr_quat.h>
#include <SR/sr_vec.h>
#include <vector>

class SbmRigidTransform
{
public:
	SrQuat rot;
	SrVec  tran;
public:
	SrVec localToGlobal(const SrVec& vLocal);
	SrVec globalToLocal(const SrVec& vGlobal);
	SrMat gmat();	
};

class SbmColObject
{
public:
	SbmRigidTransform worldState;		
	bool              isUpdate;
public:
	SbmColObject(void);
	void updateTransform(const SrMat& newState);
	virtual ~SbmColObject(void);
	virtual SrVec getCenter();	
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f) = 0; // check if a point is inside the object	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2) { return false; }; // check if a line segment is intersect with the object
};

class SbmColSphere : public SbmColObject
{
public:
	float radius;
public:
	SbmColSphere(float r) { radius = r; }
	virtual ~SbmColSphere();
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f);	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2);
};

class SbmColBox : public SbmColObject
{
public:
	SrVec extent;
public:
	SbmColBox(const SrVec& ext);
	virtual ~SbmColBox();
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f);	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2);
};

// assuming the length is along local y-axis
class SbmColCapsule : public SbmColObject
{
public:
	float extent, radius;	
	SrVec endPts[2];
public:
	SbmColCapsule(float length, float radius);
	virtual ~SbmColCapsule();
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f);	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2);
};



