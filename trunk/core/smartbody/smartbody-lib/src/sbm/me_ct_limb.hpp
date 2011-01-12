#pragma once
#include "me_ct_IK_scenario.hpp"
#include "gwiz_math.h"


struct MeCtLimbJointConstraint
{
public:
	int constraintType;
	float angle_max;
	float angle_min;
	
public:
	//MeCtLimbJointConstraint();
	//MeCtLimbJointConstraint(int type = JOINT_TYPE_BALL, float amax = M_PI, float amin = -M_PI);
	//~MeCtLimbJointConstraint();
};

// Contains the joint chains and rotation info for IK
class MeCtLimb
{		
public:
	SkSkeleton* skeleton;
	MeCtIKScenario ik;	
	SrArray<SkJoint*> joint_chain;
	SrArray<int>      buf_index;
	SrArray<SrQuat>     joint_quat; // temp buffer for quat list	
public:
	MeCtLimb(void);
	~MeCtLimb(void);

	SkJoint* getChainRoot();
	void init(SkSkeleton* skel);
	// To-Do : build a tree traversal to find directed path between root node and effector node
	bool buildJointChain(const char* root_name, const char* effector_name) {}
	// User provide the joint list. Joint[0] = root, Joint[Size-1] = end effector.
	bool buildJointChain(SrArray<const char*>& joint_names, SrArray<MeCtLimbJointConstraint>& joint_constraint);
	void updateQuat(MeFrameData& frame, bool bRead = true);
};
