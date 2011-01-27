#pragma once
#include "me_ct_IK_scenario.hpp"
#include "gwiz_math.h"


// Contains the joint chains and rotation info for IK
class MeCtLimb
{		
public:
	SkSkeleton* skeleton;
	MeCtIKScenario ik;	
	SrArray<SkJoint*> joint_chain;
	SrArray<int>      buf_index;
	SrArray<SrQuat>     joint_quat; // temp buffer for joint quats from previous frame
public:
	MeCtLimb(void);
	~MeCtLimb(void);

	SkJoint* getChainRoot();
	SkJoint* getChainEndEffector();
	void init(SkSkeleton* skel);
	// To-Do : build a tree traversal to find directed path between root node and effector node
	bool buildJointChain(const char* root_name, const char* effector_name) {}

	// User provide the joint list and all joint limits. Joint[0] = root, Joint[Size-1] = end effector.
	bool buildJointChain(SrArray<const char*>& joint_names, SrArray<MeCtIKJointLimit>& joint_limit);
	float computeLimbLength();
	void updateQuat(MeFrameData& frame, bool bRead = true);
};
