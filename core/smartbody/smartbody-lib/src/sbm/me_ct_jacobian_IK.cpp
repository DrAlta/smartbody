#include "me_ct_jacobian_IK.hpp"
#include "me_ct_reach_IK.hpp"
#include "me_ct_locomotion_func.hpp"

#include <SR/sr_alg.h>
#include <algorithm>
#include <boost/foreach.hpp>
#include <sbm/gwiz_math.h>

const float PI_CONST = 3.14159265358979323846f;
const MeCtIKJointLimit limb_joint_limit[] = { 							
	{SrVec2(PI_CONST*0.3f, PI_CONST*0.3f), SrVec2(PI_CONST*0.3f,PI_CONST*0.3f),SrVec2(PI_CONST*0.3f,-PI_CONST*0.3f)} };

using namespace gwiz;
using namespace MeCtUBLAS;

MeCtIKTreeNode::MeCtIKTreeNode()
{
	joint = NULL;
	parent = NULL;
	child = NULL;
	brother = NULL;

	// a temporary hack for testing
	// To-Do : we should provide a way to let user indicate the joint limits parameters via input script or file
	jointLimit = limb_joint_limit[0]; 
	nodeLevel  = 0;	
}

SrVec MeCtIKTreeNode::getCoMPos()
{
	SrVec pos = gmat.get_translation();
	if (child)
	{
		pos += child->gmat.get_translation();
		pos *= 0.5;
	}
	return pos;
}

MeCtIKTreeNode::~MeCtIKTreeNode()
{

}

/************************************************************************/
/* IK Tree scenario to hold data/parameters for full-body IK            */
/************************************************************************/

MeCtIKTreeScenario::MeCtIKTreeScenario()
{
	ikTreeRoot = NULL;
	ikUseBalance = false;
	ikUseReference = true;
}

MeCtIKTreeScenario::~MeCtIKTreeScenario()
{

}

bool MeCtIKTreeScenario::checkJointLimit( const SrQuat& q, const MeCtIKJointLimit& limit, const SrQuat& qInit, SrVec& jointOffset )
{
	bool modified = false;
	// feng : for some reason, the twist axis is aligned with local x-axis, instead of z-axis
	// therefore we need to do the "shifted" version of swingTwist conversion.
	// so we map x->z, y->x, z->y for computing swing-twist parameteriation.
	// all of the angle limits are adjust accordingly.
	{		
		quat_t quat_st = quat_t(q.w, q.x, q.y, q.z);
		quat_t quat_init = quat_t(qInit.w, qInit.x, qInit.y, qInit.z);
		vector_t st = MeCtReachIK::quat2SwingTwist(quat_st);//quat_st.swingtwist();
		float sw_y = (float)st.x();
		float sw_z = (float)st.y();		

		float sw_limit_z, sw_limit_y;
		sw_limit_z = sw_z > 0 ? limit.x_limit[0] : limit.x_limit[1];
		sw_limit_y = sw_y > 0 ? limit.y_limit[0] : limit.y_limit[1];

		// project swing angles onto the joint limit ellipse		
		if( sr_in_ellipse( sw_z, sw_y, sw_limit_z, sw_limit_y ) > 0.0 )	{
			modified = true;
			sr_get_closest_on_ellipse( sw_limit_z, sw_limit_y, sw_z, sw_y );			
		}		
		// handle twist angle limit
  		float tw = (float)st.z(); 
		if( tw > limit.twist_limit[0] ) 
		{
			tw = limit.twist_limit[0];
			modified = true;
		}
		if( tw < limit.twist_limit[1] ) 
		{
			tw = limit.twist_limit[1];
			modified = true;
		}	

		quat_t ql = MeCtReachIK::swingTwist2Quat(vector_t(sw_y, sw_z, tw));//quat_t( sw_x, sw_y, tw);
		quat_t q_offset = ql*quat_init.conjugate();//ql*quat_st.conjugate();//ql*quat_init.conjugate();
		vector_t rot = q_offset.axisangle();		
		jointOffset = SrVec((float)rot.x(),(float)rot.y(),(float)rot.z());		
	}	
	return modified;
}


void MeCtIKTreeScenario::updateJointLimit()
{
	ikJointLimitNodes.clear();
	BOOST_FOREACH(MeCtIKTreeNode* node, ikTreeNodes)
	{		
		bool violate = checkJointLimit(ikQuatList[node->nodeIdx],node->jointLimit,ikInitQuatList[node->nodeIdx],node->jointOffset);
		if (violate)
			ikJointLimitNodes.push_back(node);
	}		
}

void MeCtIKTreeScenario::buildIKTreeFromJointRoot( SkJoint* root )
{
	clearNodes(); // clear all existing nodes

	ikTreeRoot = new MeCtIKTreeNode();
	ikTreeRoot->joint = root;
	ikTreeRoot->nodeIdx = ikTreeNodes.size();
	ikTreeRoot->nodeLevel = 0;
	ikTreeNodes.push_back(ikTreeRoot);
	traverseJoint(root,ikTreeRoot,ikTreeNodes);
	
	ikQuatList.resize(ikTreeNodes.size());
	ikInitQuatList.resize(ikTreeNodes.size());
	ikRefQuatList.resize(ikTreeNodes.size());
}

int MeCtIKTreeScenario::traverseJoint(SkJoint* joint, MeCtIKTreeNode* jointNode, std::vector<MeCtIKTreeNode*>& nodeList )
{
	int nNodes = 1;
	MeCtIKTreeNode* prevNode = NULL;
	for (int i=0;i<joint->num_children();i++)
	{
		SkJoint* child = joint->child(i);
		MeCtIKTreeNode* childNode = new MeCtIKTreeNode();
		childNode->nodeLevel = jointNode->nodeLevel + 1;
		childNode->joint = child;
		childNode->nodeIdx = nodeList.size();
		childNode->parent = jointNode;
		nodeList.push_back(childNode);

		if (i==0)
			jointNode->child = childNode;		
		if (prevNode)
			prevNode->brother = childNode;

		if (strcmp(child->name().get_string(),"skullbase")==0)// || 
			//strcmp(child->name().get_string(),"l_wrist")==0 ||
			//strcmp(child->name().get_string(),"r_wrist")==0 )
		{
			nNodes += 1; // don't traverse their children
		}
		else
		{
			nNodes += traverseJoint(child,childNode,nodeList);
		}
		prevNode = childNode;
	}
	return nNodes;
}

void MeCtIKTreeScenario::updateQuat( const dVector& dTheta )
{
	if (dTheta.size() != ikInitQuatList.size()*3)
		return;

	for (unsigned int i=0;i<ikInitQuatList.size();i++)
	{
		SrVec axisAngle = SrVec((float)dTheta(i*3),(float)dTheta(i*3+1),(float)dTheta(i*3+2));
		MeCtIKTreeNode* node = ikTreeNodes[i];
		SrQuat newQuat = SrQuat(axisAngle)*ikInitQuatList[i]; // read from initQuat
		newQuat.normalize();
		ikQuatList[i] = newQuat; // write to quat		
	}
}

void MeCtIKTreeScenario::updateGlobalMat()
{	
	updateNodeGlobalMat(ikTreeRoot);	
}

// feng : this part seems redundant to sk_skeleton. but since a typical skeleton 
// contains much more bones like facial bones & fingers, it makes more sense to just update the nodes we are using for IK.
void MeCtIKTreeScenario::updateNodeGlobalMat( MeCtIKTreeNode* jointNode )
{
	SrMat pmat = ikGlobalMat;
	int idx = jointNode->nodeIdx;
	if (jointNode->parent)
		pmat = jointNode->parent->gmat;

	jointNode->gmat = get_lmat(jointNode->joint,&ikQuatList[idx])*pmat;
	if (jointNode->child)
		updateNodeGlobalMat(jointNode->child);
	if (jointNode->brother)
		updateNodeGlobalMat(jointNode->brother);
}

MeCtIKTreeNode* MeCtIKTreeScenario::findIKTreeNode( const char* jointName )
{
	int idx = findIKTreeNodeInList(jointName,ikTreeNodes);	
	if (idx == -1)
		return NULL;
	return ikTreeNodes[idx];
}

int MeCtIKTreeScenario::findIKTreeNodeInList( const char* jointName, IKTreeNodeList& nodeList )
{
	int iCount = 0;
	BOOST_FOREACH(MeCtIKTreeNode* node, nodeList)
	{
		if (strcmp(node->joint->name().get_string(),jointName) == 0)
			return iCount;		
		iCount++;
	}
	return -1;
}

void MeCtIKTreeScenario::clearNodes()
{
	for (int i=0;i<ikTreeNodes.size();i++)
	{
		MeCtIKTreeNode* node = ikTreeNodes[i];
		delete node;
	}
	ikTreeNodes.clear();
	ikTreeRoot = NULL;
}

/************************************************************************/
/* Jacobian based IK                                                    */
/************************************************************************/



MeCtJacobianIK::MeCtJacobianIK(void)
{
	dampJ = 150.0;
}

MeCtJacobianIK::~MeCtJacobianIK(void)
{
}

void MeCtJacobianIK::update( MeCtIKTreeScenario* scenario )
{
	scenario->updateGlobalMat();
	// solve for initial jacobian	
	computeJacobian(scenario);		
	solveDLS(matJ,dS,dampJ,dTheta, matJInv);
	// update initial dTheta into new joints	
	scenario->updateQuat(dTheta);	

	// check for joint limit violation
// 	scenario->updateJointLimit();
// 	// solve for auxiliary Jacobian	
// 	if (updateJointLimitJacobian(scenario))
// 	{		
// 		//solveDLS(matJ,dS,dampJ,dTheta, matJInv);
// 		//printf("Solve Joint Limit\n");
// 		solveDLSwithSVD(matJ,dS,dampJ,dTheta, matJInv);
// 		dTheta += dThetaAux;	
// 	    scenario->updateQuat(dTheta);
// 	}	

	// update null matrix		
	dMatrix temp; 
	matrixMatMult(matJInv,matJ,temp);
	matJnull -= temp;		
	float weight = 1.0;
	if (scenario->ikUseReference)
	{
		updateReferenceJointJacobian(scenario);		
		dTheta += dThetaAux;		
		scenario->updateQuat(dTheta);	
	}
}

bool MeCtJacobianIK::updateReferenceJointJacobian( MeCtIKTreeScenario* s )
{	
	dSref = ublas::zero_vector<double>(s->ikTreeNodes.size()*3);
	for (unsigned int i=0;i<s->ikTreeNodes.size();i++)
	{
		MeCtIKTreeNode* endNode = s->ikTreeNodes[i];
		SrQuat nodeQuat = s->ikQuatList[endNode->nodeIdx];
		SrQuat refQuat = s->ikRefQuatList[endNode->nodeIdx];
		SrQuat diff = refQuat*nodeQuat.inverse();
		diff.normalize();
		SrVec axis;
		float angle;
		diff.get(axis,angle);
		float weight = (float)(endNode->nodeLevel + 1);
		for (int k=0;k<3;k++)
		{			
			dSref(i*3+k) = axis[k]*angle*0.9;
		}
	}
	matrixVecMult(matJnull,dSref,dThetaAux);	
	return true;	
}



void MeCtJacobianIK::computeJacobian(MeCtIKTreeScenario* s)
{	
	matJ = ublas::zero_matrix<double>(s->ikEndEffectors.size()*3 ,(s->ikTreeNodes.size())*3);//.resize(s->ikEndEffectors.size()*3,(s->ikTreeNodes.size())*3);		
	matJnull = ublas::identity_matrix<double>(s->ikTreeNodes.size()*3);
	dWeight.resize(s->ikTreeNodes.size()*3);
	for (unsigned int i=0;i<dWeight.size();i++)
		dWeight(i) = 1.0;
	
	dS.resize(s->ikEndEffectors.size()*3 );
	float maxOffset = 4.0f;
	for (unsigned int i=0;i<s->ikEndEffectors.size();i++)
	{
		MeCtIKTreeNode* endNode = s->ikEndEffectors[i];
		const SrMat& endMat = endNode->gmat;
		SrVec endPos = SrVec(endMat.get(12),endMat.get(13),endMat.get(14));
		SrVec offset = endNode->targetPos - endPos;			
		SrVec targetPos;
		if (offset.len() > maxOffset)
		{
			offset.normalize();
			targetPos = offset*maxOffset;
		}
		else
			targetPos = offset;
			
		dS[i*3] = targetPos[0];
		dS[i*3+1] = targetPos[1];
		dS[i*3+2] = targetPos[2];
		MeCtIKTreeNode* node = endNode->parent;
		float fRatio = 1.f/(float)(endNode->nodeLevel + 1);
		while(node && node->parent) // no root node
		{
			int idx = node->nodeIdx;
			double expo = (double)-node->nodeLevel;
			float weight = 1.0;//(node->nodeLevel > 5) ? exp(expo) : 1000.0;//fRatio * (node->nodeLevel + 1);	
			
			const SrMat& nodeMat = node->gmat;
			SrMat parentMat; 
			if (node->parent)
				parentMat = node->parent->gmat;
			else
				parentMat = s->ikGlobalMat;

			SrVec nodePos = SrVec(nodeMat.get(12),nodeMat.get(13),nodeMat.get(14));
			SrVec axis[3];
			for (int k=0;k<3;k++)
			{
				dWeight(idx*3+k) = weight;
				axis[k] = SrVec(parentMat.get(k,0),parentMat.get(k,1),parentMat.get(k,2));
				SrVec jVec = cross(axis[k],endPos-nodePos);
				matJ(i*3+0,idx*3+k) = jVec[0];	
				matJ(i*3+1,idx*3+k) = jVec[1];	
				matJ(i*3+2,idx*3+k) = jVec[2];		
			}		
			node = node->parent;
		}
	}	
}


void MeCtJacobianIK::solveDLS(const dMatrix& mat, const dVector& v, const double damping, dVector& out, dMatrix& matPInv)
{	
	dMatrix AtA; 	
	dMatrix temp(mat);
	dMatrix matJt = ublas::trans(temp);	

	matrixMatMult(temp,matJt,AtA);	
	dMatrix dampAtA;
	dMatrix invAtA;
	dMatrix matPtInv;
	invAtA.resize(AtA.size1(),AtA.size2());	
	
	//inverseMatrix(AtA,invAtA);
	//matrixMatMult(matJt,invAtA,matPInv);	
	
#if 0
	dMatrix dampMat(AtA.size1(),AtA.size2());
	{
		// test joint weighting when solving least square
		dMatrix tempDamp;
		matrixMatMult(invAtA,matJ,matPtInv);
		matrixMatMult(ublas::diagonal_matrix<double>(dWeight.size(),dWeight.data()),matJt,tempDamp);
		matrixMatMult(matPtInv,tempDamp,dampMat);
	}
#endif

	dampAtA = AtA;// + dampMat*damping;// + ublas::identity_matrix<double>(AtA.size1())*damping;
  	for (unsigned int i=0;i<dampAtA.size1();i++)
  		dampAtA(i,i) += damping;	
	dVector tempV;	
	inverseMatrix(dampAtA,invAtA);
	matrixVecMult(invAtA,v,tempV);
	matrixVecMult(matJt,tempV,out);
	
	matrixMatMult(matJt,invAtA,matPInv);	
}

#ifdef TEST_IK_EXPERIMENT
void MeCtJacobianIK::solveWeightDLS( const dMatrix& mat, const dVector& v, const double damping, dVector& out, dMatrix& matPInv )
{
	dMatrix AtA; 	
	dMatrix temp(mat);
	dMatrix matJt = ublas::trans(temp);	

	matrixMatMult(matJt,temp,AtA);	
	dMatrix dampAtA;
	dMatrix invAtA;
	dMatrix matPtInv;
	invAtA.resize(AtA.size1(),AtA.size2());	

	inverseMatrix(AtA,invAtA);
	matrixMatMult(invAtA,matJt,matPInv);
	dMatrix dampMat(AtA.size1(),AtA.size2());
	if (0)
	{
		// test joint weighting when solving least square
		dMatrix tempDamp;
		matrixMatMult(invAtA,matJ,matPtInv);
		matrixMatMult(ublas::diagonal_matrix<double>(dWeight.size(),dWeight.data()),matJt,tempDamp);
		matrixMatMult(matPtInv,tempDamp,dampMat);
	}

	dampAtA = AtA;// + dampMat*damping;// + ublas::identity_matrix<double>(AtA.size1())*damping;
	for (unsigned int i=0;i<dampAtA.size1();i++)
		dampAtA(i,i) += damping*dWeight(i);	
	dVector tempV;	
	inverseMatrix(dampAtA,invAtA);
	matrixVecMult(matJt,v,tempV);
	matrixVecMult(invAtA,tempV,out);
}

void MeCtJacobianIK::solveDLSwithSVD( const dMatrix& mat, const dVector& v, const double damping, dVector& out, dMatrix& matPInv )
{	
	dMatrix matIn;
	if (mat.size1() > mat.size2())
		matIn = ublas::trans(mat);
	else
		matIn = mat;

	dMatrix U,V;	
	dVector vS(matIn.size1()), vSDamp(matIn.size1());
	U.resize(matIn.size1(),matIn.size1());
	V.resize(matIn.size2(),matIn.size2());
	lapack::gesvd(matIn,vS,U,V);
	//lapack::gesdd(matIn,vS,U,V);	
	double maxSingular = vS(0);	
	double minSingular = vS(vS.size()-1);	
	// 	//printf("maxSingular = %f, minSingular = %f\n",maxSingular,minSingular);
	double d = sqrtf(ublas::norm_2(v))/0.1;///minSingular;//
	double damp = d;
	double alpha = 1.0;//*std::min(0.25,minSingular);
	if (minSingular > d)
		damp = 0.0;
	else if (minSingular >= d*0.5 && minSingular <= d)
		damp = sqrtf(minSingular*(d - minSingular));
	else if (minSingular <= d*0.5)
		damp = d*0.5;

	solveDLS(mat,v,damp*damp,out,matPInv);

	return;

	/*
	for (int i=0;i<vS.size();i++)
	{
	//vS(i) = 1.0/vS(i);
	vSDamp(i) = alpha*vS(i)/(vS(i)*vS(i) + damp*damp);// + (1.0-alpha)*vS(i)*0.0001;//vS(i)/(vS(i)*vS(i) + damp*damp);
	}
	ublas::diagonal_matrix<double> matS;
	matS = ublas::diagonal_matrix<double>(U.size1(),V.size1());
	for (int i=0;i<vSDamp.size();i++)
	matS(i,i) = vSDamp(i);	

	dMatrix temp(V.size2(),matS.size2());
	out.resize(mat.size2());
	//dMatrix invAtA;
	//axpy_prod(ublas::trans(V),ublas::trans(matS),temp);
	matrixMatMult(ublas::trans(V),ublas::trans(matS),temp);
	//matPInv.resize(temp.size1(),U.size1());
	matrixMatMult(temp,ublas::trans(U),matPInv);
	matrixVecMult(matPInv,v,out);	

	dMatrix Vt = ublas::trans(V);
	matPInv.resize(Vt.size1(),V.size2());
	matrixMatMult(Vt,V,matPInv);
	*/

}

bool MeCtJacobianIK::updateJointLimitJacobian( MeCtIKTreeScenario* scenario )
{
	int numJointLimit = scenario->ikJointLimitNodes.size();
	int numPinnedJoint = 0;//scenario->ikPinNodes.size();

	int numTotalAux = numJointLimit + numPinnedJoint;

	if (numTotalAux == 0)
		return false;	
	dThetaAux.resize(matJ.size2());
	dThetaAux.clear();
	for (int i=0;i<numJointLimit;i++)
	{
		MeCtIKTreeNode* node = scenario->ikJointLimitNodes[i];
		for (int k=0;k<3;k++)
		{
			matJnull(node->nodeIdx*3+k,node->nodeIdx*3+k) = 0.0001;
			dThetaAux(node->nodeIdx*3+k) = node->jointOffset[k];	
		}
	}

	updateNullSpace(matJnull,matJ,dThetaAux,dS);

	return true;
}

bool MeCtJacobianIK::updateBalanceJacobian( MeCtIKTreeScenario* s )
{
	matJbal = ublas::zero_matrix<double>(2,(s->ikTreeNodes.size())*3);
	dSbal = ublas::zero_vector<double>(2);
	float massSum = 0.f;
	for (unsigned int i=0;i<s->ikTreeNodes.size();i++)
		massSum += s->ikTreeNodes[i]->joint->mass();

	SrVec curComPos = SrVec(0,0,0);
	SrVec comPos = s->ikTreeRoot->getCoMPos();

	for (unsigned int i=0;i<s->ikTreeNodes.size();i++)
	{
		MeCtIKTreeNode* endNode = s->ikTreeNodes[i];
		float nodeMass = endNode->joint->mass()/massSum;
		SrVec endPos = endNode->getCoMPos();		
		curComPos += endPos*nodeMass;
		MeCtIKTreeNode* node = endNode->parent;		
		while(node && node->parent)
		{
			int idx = node->nodeIdx;
			const SrMat& nodeMat = node->gmat;
			SrMat parentMat; 
			if (node->parent)
				parentMat = node->parent->gmat;
			else
				parentMat = s->ikGlobalMat;

			SrVec nodePos = SrVec(nodeMat.get(12),nodeMat.get(13),nodeMat.get(14));
			SrVec axis[3];
			for (int k=0;k<3;k++)
			{				
				axis[k] = SrVec(parentMat.get(k,0),parentMat.get(k,1),parentMat.get(k,2));
				SrVec jVec = cross(axis[k],endPos-nodePos);
				matJbal(0,idx*3+k) += jVec[0]*nodeMass;	
				matJbal(1,idx*3+k) += jVec[2]*nodeMass;					
			}		
			node = node->parent;
		}		
	}

	dSbal(0) = comPos[0] - curComPos[0];
	dSbal(1) = comPos[2] - curComPos[2];		

	updateNullSpace(matJnull,matJbal,dTheta,dSbal);	

	return true;
}

void MeCtJacobianIK::updateNullSpace( dMatrix& Jnull, dMatrix& Ji, dVector& dQ, dVector& dX )
{
	dVector temp; 
	matrixVecMult(Ji,dQ,temp);
	dX -= temp;
	dMatrix tempJ;	
	matrixMatMult(Ji,Jnull,tempJ);
	Ji = tempJ;	
}

#endif


