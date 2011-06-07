#pragma once
#include <SR/sr_mat.h>
#include <SR/sr_quat.h>
#include <SR/sr_vec.h>
#include <SR/sr_model.h>
#include <vector>

class SbmTransform
{
public:
	SrQuat rot;
	SrVec  tran;
public:
	SbmTransform() {}
	SbmTransform(const SrQuat& q, const SrVec& t) { rot = q; tran = t;}
	SrVec localToGlobal(const SrVec& vLocal);
	SrVec globalToLocal(const SrVec& vGlobal);
	SrMat gmat();	
	void  gmat(const SrMat& inMat);
	void  add(const SbmTransform& delta);
	static SbmTransform diff(const SbmTransform& r1, const SbmTransform& r2);
	static SbmTransform blend(SbmTransform& r1, SbmTransform& r2, float weight );
	static float             dist(const SbmTransform& r1, const SbmTransform& r2);

	SbmTransform& operator= (const SbmTransform& rt);
};

typedef SbmTransform SRT;

class SbmGeomObject
{
public:
	SbmTransform worldState;			
	bool         isUpdate;
public:
	SbmGeomObject(void);
	void updateTransform(const SrMat& newState);
	virtual ~SbmGeomObject(void);
	virtual SrVec getCenter();	
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f) = 0; // check if a point is inside the object	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2, float offset = 0.f) { return false; }; // check if a line segment is intersect with the object
	// estimate the hand position and orientation
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot) = 0;
};

// a default null object with no geometry
class SbmGeomNullObject : public SbmGeomObject
{
public:
	SbmGeomNullObject() {}
	~SbmGeomNullObject() {}
	virtual bool isInside(const SrVec& gPos, float offset = 0.f) { return false;}
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot);
};

class SbmGeomSphere : public SbmGeomObject
{
public:
	float radius;
public:
	SbmGeomSphere(float r) { radius = r; }
	virtual ~SbmGeomSphere();
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f);	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2, float offset = 0.f);
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot);
};

class SbmGeomBox : public SbmGeomObject
{
public:
	SrVec extent;
public:
	SbmGeomBox(const SrVec& ext);
	virtual ~SbmGeomBox();
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f);	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2, float offset = 0.f);
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot);
};

// assuming the length is along local y-axis
class SbmGeomCapsule : public SbmGeomObject
{
public:
	float extent, radius;	
	SrVec endPts[2];
public:
	SbmGeomCapsule(float length, float radius);
	SbmGeomCapsule(const SrVec& p1, const SrVec& p2, float radius);
	virtual ~SbmGeomCapsule();
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f);	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2, float offset = 0.f);
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot);
};

// this is a adapting interface to integrate SrModel tri-mesh into physical simulation framework
class SbmGeomTriMesh : public SbmGeomObject
{
public:
	SrModel* geoMesh;
public:
	SbmGeomTriMesh(SrModel* model) { geoMesh = model;  }
	virtual ~SbmGeomTriMesh() { }
	// no operations for now
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f) { return false; }
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2, float offset = 0.f) { return false; }
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot) { return false;}
};

typedef std::vector<SbmGeomObject*> VecOfSbmColObj;


class SbmCollisionUtil
{
public:
	static bool checkIntersection(SbmGeomObject* obj1, SbmGeomObject* obj2);
};

