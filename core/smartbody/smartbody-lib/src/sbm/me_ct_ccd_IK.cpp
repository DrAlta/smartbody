#include "me_ct_ccd_IK.hpp"
#include <algorithm>

MeCtCCDIK::MeCtCCDIK( void )
{
	ikScenario = NULL;

}

MeCtCCDIK::~MeCtCCDIK( void )
{

}

typedef std::pair<std::string,int> StrIntPair;
typedef std::vector<StrIntPair> StrIntList;

static bool strIntComp(StrIntPair& p1, StrIntPair& p2)
{
	return (p1.second < p2.second);
}

void MeCtCCDIK::update( MeCtIKTreeScenario* scenario )
{
	
	
	
}

void MeCtCCDIK::sortConstraint( ConstraintMap* conMap, VecOfConstraintPtr& sortConList )
{
	StrIntList sortList;
	ConstraintMap::iterator vi = conMap->begin();
	for (vi  = conMap->begin();
		vi != conMap->end();
		vi++)
	{
		//conList.push_back(vi->second);
		EffectorConstraint* con = vi->second;
		MeCtIKTreeNode* node = ikScenario->findIKTreeNode(con->efffectorName.c_str());		
		StrIntPair sip;
		sip.first = con->efffectorName;
		sip.second = node->nodeLevel;
		sortList.push_back(sip);
	}
	std::sort(sortList.begin(),sortList.end(),strIntComp);

	sortConList.clear();
	for (int i=0;sortList.size();i++)
	{
		EffectorConstraint* con = (*conMap)[sortList[i].first];
		sortConList.push_back(con);
	}
}

void MeCtCCDIK::CCDRotate(MeCtIKTreeNode* startNode, MeCtIKTreeNode* endEffector )
{	
	SrVec targetPos = endEffector->targetPos;

	SrVec v1, v2, v3, v4;
	SrVec v, lTarget, lEndPos, lSrc;
	//SrVec axis, r_axis;
	SrVec rotAxis;
	SrQuat q;
	SrMat mat, matInv;
	if(!startNode->parent) matInv = ikScenario->ikGlobalMat.inverse();
	else matInv = startNode->parent->gmat.inverse();

	lSrc = startNode->gmat.get_translation()*matInv;	
	lTarget = endEffector->targetPos*matInv;
	lEndPos = endEffector->gmat.get_translation()* matInv;

	// assuming ball-socket joints
	{
		v1 = lEndPos - lSrc;
		v1.normalize();

		v2 = lTarget - lSrc;
		v2.normalize();

		double dot_v = dot(v1, v2);
		if(dot_v >= 0.9999995000000f) 
		{
			if(startNode == endEffector) return;
			dot_v = 1.0f;
		}
		else if(dot_v < -1.0f) dot_v = -1.0f;

		double angle = acos(dot_v);
		rotAxis = cross(v2, v1);
		rotAxis.normalize();
		mat.rot(rotAxis, (float)-angle);
	}
	
	q = startNode->getQuat(QUAT_CUR);
	q = mat * q;
	startNode->setQuat(q,QUAT_CUR);
	ikScenario->updateNodeGlobalMat(startNode,QUAT_CUR);	
}


// added by feng :
// Originally I thought all joints use the local Z axis as twist axis, but this is not the case for the smartbody skeleton.
// For arms, the twist axis is at local "X" axis.
// For legs, the twist axis is local "Z" axis.
// So if we want to convert arm joints quaternion into Swing-Twist, we need to invert ( shift ) XZ coordinate so we get correct results.
// For other limbs like legs, we use the original swing-twist conversion.
#define INVERT_XZ 
gwiz::vector_t MeCtCCDIK::quat2SwingTwist( gwiz::quat_t& quat )
{
	const float EPSILON6 = 0.0000001f;

#ifdef INVERT_XZ	
	if( ( quat.w() < EPSILON6 )&&( quat.x() < EPSILON6 ) )	{
		return( gwiz::vector_t( 0.0, 0.0, M_PI ) );
	}
#else
	if( ( quat.w() < EPSILON6 )&&( quat.z() < EPSILON6 ) )	{
		return( gwiz::vector_t( M_PI, 0.0, 0.0 ) );
	}
#endif

	gwiz::quat_t q;
	if( quat.w() < 0.0 )	{
		q = quat.complement();
	}
	else	{
		q = quat;
	}

#ifdef INVERT_XZ
	double gamma = atan2( q.x(), q.w() );
	double beta = atan2( sqrt( q.z()*q.z() + q.y()*q.y() ), sqrt( q.x()*q.x() + q.w()*q.w() ) );
	double sinc = 1.0;
	if( beta > EPSILON6 )	{
		sinc = sin( beta )/beta;
	}
	double s = sin( gamma );
	double c = cos( gamma );
	double sinc2 = 2.0 / sinc;
	double swing_x = sinc2 * ( c * q.y() - s * q.z());
	double swing_y = sinc2 * ( s * q.y() + c * q.z());
	double twist = 2.0 * gamma;
#else
	double gamma = atan2( q.z(), q.w() );
	double beta = atan2( sqrt( q.x()*q.x() + q.y()*q.y() ), sqrt( q.z()*q.z() + q.w()*q.w() ) );
	double sinc = 1.0;
	if( beta > EPSILON6 )	{
		sinc = sin( beta )/beta;
	}
	double s = sin( gamma );
	double c = cos( gamma );
	double sinc2 = 2.0 / sinc;
	double swing_x = sinc2 * ( c * q.x() - s * q.y() );
	double swing_y = sinc2 * ( s * q.x() + c * q.y());
	double twist = 2.0 * gamma;
#endif
	return( gwiz::vector_t( ( swing_x ), ( swing_y ), ( twist ) ) );
}

gwiz::quat_t MeCtCCDIK::swingTwist2Quat( gwiz::vector_t& sw )
{
	using namespace gwiz;
#ifdef INVERT_XZ
	quat_t swing = quat_t( vector_t( 0.0, sw.x(), sw.y() ) );	
	quat_t result = swing * quat_t( sw.z(), vector_t( 1.0, 0.0, 0.0 ), 1);
#else
	quat_t swing = quat_t(vector_t( sw.x(), sw.y(), 0.0) );	
	quat_t result = swing * quat_t( sw.z(), vector_t( 0.0, 0.0, 1.0 ), 1);
#endif

	return result;
}

