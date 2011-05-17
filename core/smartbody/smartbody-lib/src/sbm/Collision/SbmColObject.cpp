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

void SbmRigidTransform::gmat(const SrMat& inMat )
{
	tran = inMat.get_translation();
	rot  = SrQuat(inMat);
}

SbmRigidTransform SbmRigidTransform::diff( const SbmRigidTransform& r1, const SbmRigidTransform& r2 )
{
	SbmRigidTransform rout;
	rout.tran = r2.tran - r1.tran;
	rout.rot  = r1.rot.inverse()*r2.rot;	
	rout.rot.normalize();
	return rout;
}

SbmRigidTransform SbmRigidTransform::blend( SbmRigidTransform& r1, SbmRigidTransform& r2, float weight )
{
	SbmRigidTransform rout;
	rout.tran = r1.tran*(1.f-weight) + r2.tran*weight;
	rout.rot  = slerp( r1.rot, r2.rot, weight );
	rout.rot.normalize();
	return rout;
}

SbmRigidTransform& SbmRigidTransform::operator=( const SbmRigidTransform& rt )
{
	tran = rt.tran;
	rot  = rt.rot;
	return *this;
}

float SbmRigidTransform::dist( const SbmRigidTransform& r1, const SbmRigidTransform& r2 )
{
	SbmRigidTransform diffT = diff(r1,r2);
	return diffT.tran.norm();
}

void SbmRigidTransform::add( const SbmRigidTransform& delta )
{
	tran = tran + delta.tran;//curEffectorPos + offset;
	rot  = rot*delta.rot;//rotOffset;
	rot.normalize();
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

bool SbmColSphere::isIntersect( const SrVec& gPos1, const SrVec& gPos2, float offset)
{
	SrVec p1 = worldState.globalToLocal(gPos1);
	SrVec p2 = worldState.globalToLocal(gPos2);

	if ( (p1).norm() < radius || (p2).norm() < radius + offset )
		return true;

	SrVec p2p1 = p2-p1;
	SrVec p3p1 = -p1;
	float u = dot(p2p1,p3p1)/dot(p2p1,p2p1);
	if (u > 0.f && u < 1.f && (p1+p2p1*u).norm() < radius + offset)
		return true;

	return false;
}

bool SbmColSphere::estimateHandPosture( const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot )
{
	outHandPos = getCenter() + SrVec(0,radius*1.5f,0)*naturalRot;
	outHandRot = naturalRot;
	return true;
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

bool SbmColBox::isIntersect( const SrVec& gPos1, const SrVec& gPos2, float offset)
{
	SrVec p1 = worldState.globalToLocal(gPos1);
	SrVec p2 = worldState.globalToLocal(gPos2);

	SrVec d = (p2 - p1)*0.5f;    
	SrVec e = extent + SrVec(offset,offset,offset);    
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

bool SbmColBox::estimateHandPosture( const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot )
{
	
	SrVec yAxis = SrVec(0,1,0);
	yAxis = yAxis*naturalRot;
	SrVec ly = yAxis*worldState.rot.inverse();//worldState.globalToLocal(yAxis);
	SrVec axis[6] = {SrVec(1,0,0), SrVec(0,1,0), SrVec(0,0,1),SrVec(-1,0,0), SrVec(0,-1,0), SrVec(0,0,-1) };
	SrVec graspAxis;
	float minAngle = 100.f;	
	float objSize = -1;
	for (int i=0;i<6;i++)
	{
		float rotAngle = acosf(dot(ly,axis[i]));
		if (rotAngle < minAngle)
		{
			minAngle = rotAngle;
			graspAxis = axis[i];
			objSize = extent[i%3];//dot(graspAxis,extent);
		}		
	}
	//sr_out << "minAngle = " << minAngle << "  , grasp axis = " << graspAxis << srnl;
	graspAxis = graspAxis*worldState.rot;
	SrVec rotAxis = cross(yAxis,graspAxis); rotAxis.normalize();
	SrQuat alignRot = SrQuat(rotAxis,minAngle);
	
	outHandRot = alignRot*naturalRot;
	outHandPos = getCenter() + SrVec(0,objSize*1.8f,0)*outHandRot;
	
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

static float findLineSegmentDistOnLineSegment(SrVec e1[2], SrVec e2[2], SrVec& closePt)
{		
	SrVec u = e1[1]-e1[0];
	SrVec v = e2[1]-e2[0];
	SrVec w0 = e1[0]-e2[0];
	float a = dot(u,u);
	float b = dot(u,v);
	float c = dot(v,v);
	float d = dot(u,w0);
	float e = dot(v,w0);

	float sd,td;

	sd = td = (a*c-b*b);	
	float sc = (b*e - c*d);
	float tc = (a*e - b*d);

	if (sc < 0.0) {       // sc < 0 => the s=0 edge is visible
        sc = 0.0;
        tc = e;
        td = c;
    }
    else if (sc > sd) {  // sc > 1 => the s=1 edge is visible
            sc = sd;
            tc = e + b;
            td = c;
    }
    

    if (tc < 0.0) {           // tc < 0 => the t=0 edge is visible
        tc = 0.0;
        // recompute sc for this edge
        if (-d < 0.0)
            sc = 0.0;
        else if (-d > a)
            sc = sd;
        else {
            sc = -d;
            sd = a;
        }
    }
    else if (tc > td) {      // tc > 1 => the t=1 edge is visible
        tc = td;
        // recompute sc for this edge
        if ((-d + b) < 0.0)
            sc = 0;
        else if ((-d + b) > a)
            sc = sd;
        else {
            sc = (-d + b);
            sd = a;
        }
    }

	float so = sc/sd;
	float to = tc/td;
	float dline = (w0 + u*so - v*to).norm();	
	return dline;
}

SbmColCapsule::SbmColCapsule( float len, float r )
{
	extent = len*0.5f;
	radius = r;
	endPts[0] = SrVec(0,-extent,0);
	endPts[1]   = SrVec(0,extent,0);
}

SbmColCapsule::SbmColCapsule( const SrVec& p1, const SrVec& p2, float r )
{
	extent = (p2-p1).norm()*0.5f;
	endPts[0] = p1;
	endPts[1] = p2;
	radius = r;
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
	{		
		return true;
	}
	return false;
}

bool SbmColCapsule::isIntersect( const SrVec& gPos1, const SrVec& gPos2, float offset)
{
	SrVec lpos[2];
	SrVec cpt;
	lpos[0] = worldState.globalToLocal(gPos1);
	lpos[1] = worldState.globalToLocal(gPos2);
	
	float dist = findLineSegmentDistOnLineSegment(lpos,endPts,cpt);

	//printf("test capsule intersection\n");
	//printf("dist=%f\n",dist);
	if (dist < offset + radius)
	{
		//printf("intersect dist=%f\n",dist);
		return true;
	}
	return false;
}

bool SbmColCapsule::estimateHandPosture( const SrQuat& naturalRot, SrVec& outHandPos, SrQuat& outHandRot )
{
	SrVec capAxis = (endPts[1]-endPts[0]); capAxis.normalize();
	capAxis = capAxis*worldState.rot;
	SrVec handAxis = SrVec(0,1,0)*naturalRot; handAxis.normalize();
	SrVec handXAxis = SrVec(-1,0,0)*naturalRot;	
	SrVec orienAxis = cross(handXAxis,capAxis); orienAxis.normalize();	

	SrVec crossHand = cross(handAxis,orienAxis); crossHand.normalize();
	if (dot(crossHand,handXAxis) < 0.0 && dot(handAxis,orienAxis) < 0.7)
		orienAxis = -orienAxis;

	SrQuat rot = SrQuat(handAxis,orienAxis);
	outHandRot = rot*naturalRot;//naturalRot*rot;
	outHandPos = getCenter() + SrVec(0,radius*1.5f,0)*outHandRot;
	return true;
}

bool SbmCollisionUtil::checkCollision( SbmColObject* obj1, SbmColObject* obj2 )
{
	if (dynamic_cast<SbmColSphere*>(obj1))
	{
		SbmColSphere* sph = dynamic_cast<SbmColSphere*>(obj1);
		return obj2->isInside(obj1->worldState.tran,sph->radius);
	}
	else if (dynamic_cast<SbmColCapsule*>(obj1))
	{
		SbmColCapsule* cap = dynamic_cast<SbmColCapsule*>(obj1);
		SrVec g1,g2;
		g1 = cap->endPts[0]*cap->worldState.gmat();
		g2 = cap->endPts[1]*cap->worldState.gmat();
		return obj2->isIntersect(g1,g2,cap->radius);		
	}
	else if (dynamic_cast<SbmColBox*>(obj1))
	{
		return false;
	}
	else
	{
		return false;
	}
}