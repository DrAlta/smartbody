#pragma once
#include "me_ct_IK_scenario.hpp"
#include "gwiz_math.h"
#include "me_ct_ublas.hpp"

class MeCtIKTreeNode
{
public:
	int              nodeIdx;
	int              nodeLevel;
	MeCtIKTreeNode   *parent, *child, *brother;

	SrVec            targetPos; // for end effector
	SkJoint          *joint;	
	MeCtIKJointLimit jointLimit;
	SrVec            jointOffset;
	SrMat            gmat;	
	float            mass;

	MeCtIKTreeNode();
	~MeCtIKTreeNode();
	SrVec getCoMPos(); 
};

typedef std::vector<MeCtIKTreeNode*> IKTreeNodeList;

class MeCtIKTreeScenario
{
public:	
	SkSkeleton*                  ikSkeleton;
	IKTreeNodeList               ikTreeNodes;
	MeCtIKTreeNode*              ikTreeRoot; // contains the tree structure for IK tree ( which may contain multiple end effectors/IK chains )	
	IKTreeNodeList               ikEndEffectors;	
	
	SrMat                        ikGlobalMat;
	std::vector<SrQuat>          ikQuatList; // current quat list
	std::vector<SrQuat>          ikInitQuatList; // initial quat before solving IK
	std::vector<SrQuat>          ikRefQuatList;  // reference quat list

	IKTreeNodeList               ikJointLimitNodes; // joints that violate joint limits	
	bool                         ikUseBalance, ikUseReference;

public:
	MeCtIKTreeScenario();
	~MeCtIKTreeScenario();
public:	
	void buildIKTreeFromJointRoot(SkJoint* root);	
	void updateQuat(const dVector& dTheta);
	void updateJointLimit();
	void updateGlobalMat();
	MeCtIKTreeNode* findIKTreeNode(const char* jointName);	
	static int findIKTreeNodeInList(const char* jointName, IKTreeNodeList& nodeList);	
protected:
	void clearNodes();
	int traverseJoint(SkJoint* joint, MeCtIKTreeNode* jointNode, std::vector<MeCtIKTreeNode*>& nodeList);	
	void updateNodeGlobalMat(MeCtIKTreeNode* jointNode);	
	// return axis-angle rotation offset to move joint rotation back.
	static bool checkJointLimit(const SrQuat& q, const MeCtIKJointLimit& limit, const SrQuat& qInit, SrVec& jointOffset); 
};

// a full-body IK solver based on Jacobian pseudo-inverse
class MeCtJacobianIK	
{	
protected:
	dMatrix matJ, matJnull, matJInv; // Jacobian associated with end effector
	dMatrix matJref, matJrefInv;
	dVector dWeight;
	dVector dTheta, dS;

	dMatrix matJbal, matJbalInv, matJAux, matJAuxInv; // Jacobian for balance control
	dVector dThetaAux,dSbal, dSref, dSAux;
	
	double  dampJ; // damping constant	
public:
	MeCtJacobianIK(void);
	~MeCtJacobianIK(void);
public:
	virtual void update(MeCtIKTreeScenario* scenario);
	float getDt() const { return dt; }
	void setDt(float val) { dt = val; }	
protected:
	float dt; // for constraining rotation speed	
	void computeJacobian(MeCtIKTreeScenario* scenario);			
	bool updateReferenceJointJacobian(MeCtIKTreeScenario* scenario);	
	void solveDLS(const dMatrix& mat, const dVector& v, const double damping, dVector& out, dMatrix& invAtA);
	
	// feng : these are some experimental features such as infering the damping factor based on eigenvalues of Jacobian, 
	// giving different weighting for joint angles, restricting joint movement ranges, and global character balance constraints.
	// they are either not robust enough or simply doesn't work right now. So they are "commented out" to improve readibility for now.
#ifdef TEST_IK_EXPERIMENT 
	void solveWeightDLS(const dMatrix& mat, const dVector& v, const double damping, dVector& out, dMatrix& invAtA);
	void solveDLSwithSVD(const dMatrix& mat, const dVector& v, const double damping, dVector& out, dMatrix& invAtA);
	bool updateJointLimitJacobian(MeCtIKTreeScenario* scenario);
	bool updateBalanceJacobian(MeCtIKTreeScenario* scenario);	
	void updateNullSpace(dMatrix& Jnull, dMatrix& Ji, dVector& dQ, dVector& dX);
#endif
};
