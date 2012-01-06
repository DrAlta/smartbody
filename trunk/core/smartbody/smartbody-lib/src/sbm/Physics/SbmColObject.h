#ifndef _SBMCOLOBJECT_H_
#define _SBMCOLOBJECT_H_

#include <sr/sr_mat.h>
#include <sr/sr_quat.h>
#include <sr/sr_vec.h>
#include <sr/sr_model.h>
#include <vector>
#include <string>

class SbmTransform
{
public:
	SrQuat rot;
	SrVec  tran;
public:
	SbmTransform();
	SbmTransform(const SrQuat& q, const SrVec& t) { rot = q; tran = t;}
	SrVec localToGlobal(const SrVec& vLocal);
	SrVec globalToLocal(const SrVec& vGlobal);
	SrMat gmat() const;	
	void  gmat(const SrMat& inMat);
	void  add(const SbmTransform& delta);	
	static SbmTransform diff(const SbmTransform& r1, const SbmTransform& r2);
	static SbmTransform mult(const SbmTransform& r1, const SbmTransform& r2);  // return r1*r2
	static SbmTransform blend(SbmTransform& r1, SbmTransform& r2, float weight );
	static float             dist(const SbmTransform& r1, const SbmTransform& r2);

	SbmTransform& operator= (const SbmTransform& rt);
};

typedef SbmTransform SRT;

class SbmPhysicsObjInterface;

enum { GEOM_NULL = 0, GEOM_SPHERE, GEOM_BOX, GEOM_CAPSULE, GEOM_MESH, NUM_OF_GEOMS };

class SbmGeomObject
{
public:
	std::string  color;	
protected:	
	SbmPhysicsObjInterface* attachedPhyObj;
	SbmTransform localTransform;	
	SbmTransform combineTransform;	
public:
	SbmGeomObject(void);	
	virtual ~SbmGeomObject(void);	
	void attachToPhyObj(SbmPhysicsObjInterface* phyObj);
	SbmTransform& getLocalTransform() { return localTransform; }	
	void          setLocalTransform(SbmTransform& newLocalTran);
	SbmTransform& getCombineTransform();
	virtual SrVec getCenter();	
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f) = 0; // check if a point is inside the object	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2, float offset = 0.f) { return false; }; // check if a line segment is intersect with the object
	// estimate the hand position and orientation
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot, float offsetDist = 0.f) = 0;
	virtual std::string geomType() = 0;
	virtual SrVec       getGeomSize() = 0;

	static SbmGeomObject* createGeometry(std::string& type, SrVec extends);
};

// a default null object with no geometry
class SbmGeomNullObject : public SbmGeomObject
{
public:
	SbmGeomNullObject() {}
	~SbmGeomNullObject() {}
	virtual bool isInside(const SrVec& gPos, float offset = 0.f) { return false;}
	virtual bool estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot, float offsetDist);
	virtual std::string  geomType() { return "null"; }
	virtual SrVec        getGeomSize() { return SrVec(1,1,1); }
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
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot, float offsetDist);
	virtual std::string  geomType() { return "sphere"; }
	virtual SrVec        getGeomSize() { return SrVec(radius,radius,radius); }
};

class SbmGeomBox : public SbmGeomObject
{
public:
	SrVec extent;
public:
	SbmGeomBox(const SrVec& ext);
	SbmGeomBox(SrBox& bbox); 
	virtual ~SbmGeomBox();
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f);	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2, float offset = 0.f);
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot, float offsetDist);
	virtual std::string  geomType() { return "box"; }
	virtual SrVec        getGeomSize() { return extent; }
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
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot, float offsetDist);
	virtual std::string  geomType() { return "capsule"; }
	virtual SrVec        getGeomSize() { return SrVec(extent,radius,extent); }
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
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot, float offsetDist) { return false;}
	virtual std::string  geomType() { return "mesh"; }
	virtual SrVec        getGeomSize() { return SrVec(1,1,1); }
};

typedef std::vector<SbmGeomObject*> VecOfSbmColObj;


class SbmCollisionUtil
{
public:
	static bool checkIntersection(SbmGeomObject* obj1, SbmGeomObject* obj2);
};

#endif

