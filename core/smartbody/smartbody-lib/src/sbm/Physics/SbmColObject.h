#ifndef _SBMCOLOBJECT_H_
#define _SBMCOLOBJECT_H_

#include <sr/sr_mat.h>
#include <sr/sr_quat.h>
#include <sr/sr_vec.h>
#include <sr/sr_model.h>
#include <vector>
#include <list>
#include <map>
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

class SbmTransformObjInterface
{	
public:
	virtual SbmTransform& getGlobalTransform() = 0;
	virtual void setGlobalTransform(SbmTransform& newGlobalTransform) = 0;
};

class SbmGeomObject
{
public:
	std::string  color;	
protected:	
	// a SbmGeomObject can be attached to a pawn or physics object as needed
	SbmTransformObjInterface* attachedObj;
	SbmTransform globalTransform;
	SbmTransform localTransform;	
	SbmTransform combineTransform;	
public:
	SbmGeomObject(void);	
	virtual ~SbmGeomObject(void);	
	void attachToObj(SbmTransformObjInterface* phyObj);
	SbmTransformObjInterface* getAttachObj();

	SbmTransform& getLocalTransform() { return localTransform; }	
	void          setLocalTransform(SbmTransform& newLocalTran);	
	SbmTransform& getGlobalTransform();
	void          setGlobalTransform(SbmTransform& newGlobalTran);

	SbmTransform& getCombineTransform();
	virtual SrVec getCenter();	
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f) { return false; } // check if a point is inside the object	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2, float offset = 0.f) { return false; }; // check if a line segment is intersect with the object
	// estimate the hand position and orientation
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot, float offsetDist = 0.f) { return false; }
	virtual std::string geomType() { return ""; }
	virtual SrVec       getGeomSize() { return SrVec(); }
	virtual void	    setGeomSize(SrVec& size) { return; }

	static SbmGeomObject* createGeometry(const std::string& type, SrVec extends, SrVec from = SrVec(), SrVec to = SrVec() );
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
	virtual void         setGeomSize(SrVec& size) {}
};

class SbmGeomSphere : public SbmGeomObject
{
public:
	float radius;
public:
	SbmGeomSphere(float r);
	virtual ~SbmGeomSphere();
	virtual bool  isInside(const SrVec& gPos, float offset = 0.f);	
	virtual bool  isIntersect(const SrVec& gPos1, const SrVec& gPos2, float offset = 0.f);
	virtual bool  estimateHandPosture(const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot, float offsetDist);
	virtual std::string  geomType() { return "sphere"; }
	virtual SrVec        getGeomSize() { return SrVec(radius,radius,radius); }
	virtual void         setGeomSize(SrVec& size) { radius = size[0]; }
	virtual float getRadius();
	virtual void setRadius(float val);

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
	virtual void         setGeomSize(SrVec& size) { extent = size; }
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
	virtual void         setGeomSize(SrVec& size);
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
	virtual void         setGeomSize(SrVec& size) { }
};

typedef std::vector<SbmGeomObject*> VecOfSbmColObj;

// struct for contact point
class SbmGeomContact 
{
public:
	SrVec contactPoint;
	SrVec contactNormal;
	float penetrationDepth;	
public:
	SbmGeomContact& operator= (const SbmGeomContact& rt);
};

typedef std::pair<std::string,std::string> SbmCollisionPair;
typedef std::vector<SbmCollisionPair> SbmCollisionPairList;

class SbmCollisionSpace
{
protected:
	//std::list<SbmGeomObject*> collisionObjs;
	std::map<std::string,SbmGeomObject*> collsionObjMap;
public:
	SbmCollisionSpace();
	~SbmCollisionSpace();
	virtual void addCollisionObjects(const std::string& objName);
	virtual void removeCollisionObjects(const std::string& objName);
	virtual void getPotentialCollisionPairs(std::vector<SbmCollisionPair>& collisionPairs) = 0;
};

class SbmCollisionUtil
{
public:
	static bool checkIntersection(SbmGeomObject* obj1, SbmGeomObject* obj2);
	static void collisionDetection(SbmGeomObject* obj1, SbmGeomObject* obj2, std::vector<SbmGeomContact>& contactPts);
};

#endif

