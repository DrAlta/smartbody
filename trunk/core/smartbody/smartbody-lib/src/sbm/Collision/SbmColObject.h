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
	SbmRigidTransform() {}
	SbmRigidTransform(const SrQuat& q, const SrVec& t) { rot = q; tran = t;}
	SrVec localToGlobal(const SrVec& vLocal);
	SrVec globalToLocal(const SrVec& vGlobal);
	SrMat gmat();	
	void  gmat(const SrMat& inMat);
	void  add(const SbmRigidTransform& delta);
	static SbmRigidTransform diff(const SbmRigidTransform& r1, const SbmRigidTransform& r2);
	static SbmRigidTransform blend(SbmRigidTransform& r1, SbmRigidTransform& r2, float weight );
	static float             dist(const SbmRigidTransform& r1, const SbmRigidTransform& r2);

	SbmRigidTransform& operator= (const SbmRigidTransform& rt);
};

typedef SbmRigidTransform SRT;

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
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2, float offset = 0.f) { return false; }; // check if a line segment is intersect with the object
	// estimate the hand position and orientation
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot) = 0;
};

class SbmColSphere : public SbmColObject
{
public:
	float radius;
public:
	SbmColSphere(float r) { radius = r; }
	virtual ~SbmColSphere();
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f);	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2, float offset = 0.f);
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot);
};

class SbmColBox : public SbmColObject
{
public:
	SrVec extent;
public:
	SbmColBox(const SrVec& ext);
	virtual ~SbmColBox();
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f);	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2, float offset = 0.f);
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot);
};

// assuming the length is along local y-axis
class SbmColCapsule : public SbmColObject
{
public:
	float extent, radius;	
	SrVec endPts[2];
public:
	SbmColCapsule(float length, float radius);
	SbmColCapsule(const SrVec& p1, const SrVec& p2, float radius);
	virtual ~SbmColCapsule();
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f);	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2, float offset = 0.f);
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot);
};

typedef std::vector<SbmColObject*> VecOfSbmColObj;


class SbmCollisionUtil
{
public:
	static bool checkCollision(SbmColObject* obj1, SbmColObject* obj2);
};

