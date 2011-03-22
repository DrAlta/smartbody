#pragma once
#include <map>
#include "me_ct_IK_scenario.hpp"
#include "gwiz_math.h"
#include "me_ct_ublas.hpp"

enum NodeQuatType { QUAT_INIT = 0, QUAT_REF, QUAT_PREVREF, QUAT_CUR, QUAT_DUMMY, QUAT_SIZE };
enum ConstraintType { CONSTRAINT_POS = 0, CONSTRAINT_ROT };
class MeCtIKTreeNode
{
public:
	int              nodeIdx;
	int              nodeLevel;
	std::string      nodeName;
	MeCtIKTreeNode   *parent, *child, *brother;

	SrVec            targetPos; // target pos is a global position constraint for this end effector joint
	SrQuat           targetDir; // target dir is a global rotation constraint ( parameterized as quaternion )
	SkJoint          *joint;	
	MeCtIKJointLimit jointLimit;
	SrVec            jointOffset;	
	SrMat            gmat;	
	SrQuat           nodeQuat[QUAT_SIZE];
	float            mass;

	MeCtIKTreeNode();
	~MeCtIKTreeNode();
	SrVec getCoMPos(); 
	SrVec getParentGlobalPos();
	SrVec getGlobalPos();
	SrQuat& getQuat(NodeQuatType type = QUAT_CUR);
	bool setQuat(const SrQuat& q, NodeQuatType type = QUAT_CUR);
};


typedef std::vector<MeCtIKTreeNode*> IKTreeNodeList;
typedef std::vector<std::string> VecOfString;

class EffectorConstraint
{
public:
	std::string     efffectorName;
	std::string     rootName; // root of influence for this constraint	
public:
	EffectorConstraint() {}
	~EffectorConstraint() {}

	virtual SrVec getPosConstraint() = 0;
	virtual SrQuat getRotConstraint() = 0;
};

typedef std::map<std::string,EffectorConstraint*> ConstraintMap;

class MeCtIKTreeScenario
{

public:	
	SkSkeleton*                  ikSkeleton;
	IKTreeNodeList               ikTreeNodes;
	MeCtIKTreeNode*              ikTreeRoot; // contains the tree structure for IK tree ( which may contain multiple end effectors/IK chains )	
	//IKTreeNodeList               ikEndEffectors;

	//IKTreeNodeList               ikPosEffectors; // positional constraint
	//IKTreeNodeList               ikRotEffectors; // rotational constraint
	//VecOfString                  ikPosEffectors;
	//VecOfString                  ikRotEffectors;
	ConstraintMap*               ikRotEffectors;
	ConstraintMap*               ikPosEffectors;

	
	SrMat                        ikGlobalMat;	
	IKTreeNodeList               ikJointLimitNodes; // joints that violate joint limits	

public:
	MeCtIKTreeScenario();
	~MeCtIKTreeScenario();
public:	
	void buildIKTreeFromJointRoot(SkJoint* root);	
	void updateQuat(const dVector& dTheta);
	void copyTreeNodeQuat(NodeQuatType typeFrom, NodeQuatType typeTo);
	void setTreeNodeQuat(const std::vector<SrQuat>& inQuatList,NodeQuatType type);
	void getTreeNodeQuat(std::vector<SrQuat>& inQuatList, NodeQuatType type);

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

class MeCtIKInterface
{
protected:
	float dt; 
public:
	virtual void update(MeCtIKTreeScenario* scenario) = 0;
	float getDt() const { return dt; }
	void setDt(float val) { dt = val; }	
};


// a full-body IK solver based on Jacobian pseudo-inverse
class MeCtJacobianIK : public MeCtIKInterface	
{	
protected:
	dMatrix matJ, matJnull, matJInv; // Jacobian associated with end effector
	dMatrix matJref, matJrefInv;
	dVector dWeight;
	dVector dTheta, dS;

	dMatrix matJbal, matJbalInv, matJAux, matJAuxInv; // Jacobian for balance control
	dVector dThetaAux,dSbal, dSref, dSAux;

	MeCtIKTreeScenario* ikScenario;
	
public:
	double  dampJ, refDampRatio; // damping constant	
	bool    ikUseBalance, ikUseReference;
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
