#include "me_ct_limb.hpp"

/*
MeCtLimbJointConstraint::MeCtLimbJointConstraint(int type , float amax , float amin)
:constraintType(type), angle_max(amax), angle_min(amin)
{

}
*/

MeCtLimb::MeCtLimb(void)
{
	skeleton = NULL;
}

MeCtLimb::~MeCtLimb(void)
{
}

void MeCtLimb::init(SkSkeleton* skel)
{
	skeleton = skel;
}

SkJoint* MeCtLimb::getChainRoot()
{
	if (joint_chain.size() > 0)
		return joint_chain[0];
	else
		return NULL;
}



void MeCtLimb::updateQuat(MeFrameData& frame, bool bRead)
{
	SrBuffer<float>& buffer = frame.buffer();

	for (int i=0;i<buf_index.size();i++)
	{
		int index = frame.toBufferIndex(buf_index[i]);
		if (bRead)
		{
			joint_quat[i].w = buffer[index];
			joint_quat[i].x = buffer[index+1];
			joint_quat[i].y = buffer[index+2];
			joint_quat[i].z = buffer[index+3];
		}
		else // write current joint quat to the buffer
		{
			SrQuat prevQuat = SrQuat(buffer[index],buffer[index+1],buffer[index+2],buffer[index+3]);
			SrQuat curQuat  = joint_quat[i];

			buffer[index] = joint_quat[i].w;
			buffer[index+1] = joint_quat[i].x;
			buffer[index+2] = joint_quat[i].y;
			buffer[index+3] = joint_quat[i].z;			
		}		
	}
	ik.joint_quat_list = joint_quat;
}

bool MeCtLimb::buildJointChain(SrArray<const char*>& joint_names, SrArray<MeCtLimbJointConstraint>& joint_constraint)
{
	joint_chain.capacity(joint_names.size());
	joint_chain.size(joint_names.size());

	// initialize joint chain list
	for (int i=0;i<joint_names.size();i++)
	{
		SkJoint* joint = skeleton->search_joint(joint_names[i]);
		if (!joint)
			return false;
				
		joint_chain[i] = joint;		
	}	
	buf_index.size(joint_chain.size());
	joint_quat.size(joint_chain.size());

	// initialize MeCtIKScenario
	ik.joint_info_list.size(joint_chain.size());
	for (int i=0;i<joint_chain.size();i++)
	{		
		MeCtIKScenarioJointInfo& joint_info = ik.joint_info_list[i];
		joint_info.sk_joint = joint_chain[i];
		//joint_info.type = JOINT_TYPE_BALL;
		
		int constraint = joint_constraint[i].constraintType;
		joint_info.type = constraint;
		if (constraint == JOINT_TYPE_BALL)
		{
			//joint_info.constraint.ball.max = joint_constraint[i].angle_max;
		}
		else if (constraint == JOINT_TYPE_HINGE)
		{
			joint_info.constraint.hinge.max = joint_constraint[i].angle_max;
			joint_info.constraint.hinge.min = joint_constraint[i].angle_min;
			joint_info.axis = SrVec(1,0,0); // assume x-axis for now
		}
		
		joint_info.is_support_joint = 0;		
	}
	ik.start_joint = &ik.joint_info_list[0];
	ik.end_joint   = &ik.joint_info_list[joint_chain.size()-1];
	ik.joint_quat_list = joint_quat;
	
}