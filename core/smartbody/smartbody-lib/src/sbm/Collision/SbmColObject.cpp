#include "SbmColObject.h"
#include <sbm/gwiz_math.h>

SrVec SbmRigidTransform::localToGlobal( const SrVec& vLocal )
{
	return vLocal*rot + tran;	
}

SrVec SbmRigidTransform::globalToLocal( const SrVec& vGlobal )
{
	return (vGlobal-tran)*rot.inverse();
}

SrMat SbmRigidTransform::gmat()
{
	SrMat mat;
	mat = rot.get_mat(mat);
	mat.setl4(tran);
	return mat;
}


SbmColObject::SbmColObject(void)
{
	isUpdate = false;	
}

SbmColObject::~SbmColObject(void)
{
	
}

SrVec SbmColObject::getCenter()
{
	return worldState.tran;
}

void SbmColObject::updateTransform( const SrMat& newState )
{
	SrQuat newQuat = SrQuat(newState);
	SrVec newPos = newState.get_translation();
	if (newQuat != worldState.rot || newPos != worldState.tran)
	{
		worldState.rot  = newQuat;
		worldState.tran = newPos;
		isUpdate = true;
	}	
}

/************************************************************************/
/* Sphere collider                                                      */
/************************************************************************/

bool SbmColSphere::isInside( const SrVec& gPos, float offset )
{
	SrVec lpos = worldState.globalToLocal(gPos);
	if (lpos.norm() < radius + offset)
		return true;
	return false;
}

SbmColSphere::~SbmColSphere()
{

}

bool SbmColSphere::isIntersect( const SrVec& gPos1, const SrVec& gPos2 )
{
	SrVec p1 = worldState.globalToLocal(gPos1);
	SrVec p2 = worldState.globalToLocal(gPos2);

	if ( (p1).norm() < radius || (p2).norm() < radius)
		return true;

	SrVec p2p1 = p2-p1;
	SrVec p3p1 = -p1;
	float u = dot(p2p1,p3p1)/dot(p2p1,p2p1);
	if (u > 0.f && u < 1.f && (p1+p2p1*u).norm() < radius)
		return true;

	return false;
}
/************************************************************************/
/* Box collider                                                         */
/************************************************************************/

SbmColBox::SbmColBox( const SrVec& ext )
{
	extent = ext;	
}

SbmColBox::~SbmColBox()
{

}

bool SbmColBox::isInside( const SrVec& gPos, float offset )
{
	SrVec lpos = worldState.globalToLocal(gPos);
	
	if (lpos.x > -extent.x - offset && lpos.x < extent.x + offset && 
		lpos.y > -extent.y - offset && lpos.y < extent.y + offset &&
		lpos.z > -extent.z - offset && lpos.z < extent.z + offset)
		return true;

	return false;
}

bool SbmColBox::isIntersect( const SrVec& gPos1, const SrVec& gPos2 )
{
	SrVec p1 = worldState.globalToLocal(gPos1);
	SrVec p2 = worldState.globalToLocal(gPos2);

	SrVec d = (p2 - p1)*0.5f;    
	SrVec e = extent;    
	SrVec c = p1 + d ;    
	SrVec ad = d; 	ad.abs();
	// Returns same vector with all components positive    
	if (fabsf(c[0]) > e[0] + ad[0])        return false;    
	if (fabsf(c[1]) > e[1] + ad[1])        return false;    
	if (fabsf(c[2]) > e[2] + ad[2])        return false;      
	if (fabsf(d[1] * c[2] - d[2] * c[1]) > e[1] * ad[2] + e[2] * ad[1] + gwiz::epsilon10())        return false;    
	if (fabsf(d[2] * c[0] - d[0] * c[2]) > e[2] * ad[0] + e[0] * ad[2] + gwiz::epsilon10())        return false;    
	if (fabsf(d[0] * c[1] - d[1] * c[0]) > e[0] * ad[1] + e[1] * ad[0] + gwiz::epsilon10())        return false;                
	return true;
}
/************************************************************************/
/* Capsule collider                                                     */
/************************************************************************/

static float findPointDistOnLineSegment(const SrVec& Point, SrVec e[2], SrVec& closePt)
{
	SrVec vDiff = Point - e[0];
	SrVec vDir = e[1]-e[0]; 
	float fDist = vDir.norm(); vDir.normalize();
	float fDot = dot(vDiff,vDir);
	if (fDot < 0)
	{
		closePt = e[0];
	}
	else if (fDot > fDist)
	{
		closePt = e[1];
	}
	else
	{
		closePt = e[0] + vDir*fDot;
	}

	float fPointDist = (Point-closePt).norm();
	return fPointDist;
}

SbmColCapsule::SbmColCapsule( float len, float r )
{
	extent = len*0.5f;
	radius = r;
	endPts[0] = SrVec(0,-extent,0);
	endPts[1]   = SrVec(0,extent,0);
}

SbmColCapsule::~SbmColCapsule()
{

}

bool SbmColCapsule::isInside( const SrVec& gPos, float offset)
{
	SrVec lpos = worldState.globalToLocal(gPos);
	SrVec cPts;
	float dist = findPointDistOnLineSegment(lpos,endPts,cPts);
	if (dist < radius + offset )
		return true;
	return false;
}

bool SbmColCapsule::isIntersect( const SrVec& gPos1, const SrVec& gPos2 )
{
	SrVec p1 = worldState.globalToLocal(gPos1);
	SrVec p2 = worldState.globalToLocal(gPos2);
	SrVec cpt;
	float dp1 = findPointDistOnLineSegment(p1,endPts,cpt);	
	float dp2 = findPointDistOnLineSegment(p2,endPts,cpt);	
	
	SrVec u = p2-p1;
	SrVec v = endPts[1] - endPts[0];
	SrVec w0 = p1 - endPts[0];
	float a = dot(u,u);
	float b = dot(u,v);
	float c = dot(v,v);
	float d = dot(v,w0);
	float e = dot(u,w0);
	
	float det = (a*c-b*b);
	if (det == 0) 
	{
		if (dp1 < radius || dp2 < radius)
			return true;
		else
			return false;	
	}

	float sc = (b*e - c*d)/det;
	float tc = (a*e - b*d)/det;
	float dline = (w0 + u*sc - v*tc).norm();


	return false;
}