#include "me_ct_limb.hpp"

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


SkJoint* MeCtLimb::getChainEndEffector()
{
	if (joint_chain.size() > 0)
		return joint_chain[joint_chain.size()-1];
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
}

bool MeCtLimb::buildJointChain(SrArray<const char*>& joint_names, SrArray<MeCtIKJointLimit>& joint_limit)
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
		ik.joint_info_list[i] = MeCtIKScenarioJointInfo();
		MeCtIKScenarioJointInfo& joint_info = ik.joint_info_list[i];
		joint_info.sk_joint = joint_chain[i];
		joint_info.type = JOINT_TYPE_BALL;			
		joint_quat[i] = SrQuat(0,0,0,1.0);			
		joint_info.is_support_joint = 0;	
		joint_info.joint_limit = joint_limit[i];
	}
	ik.start_joint = &ik.joint_info_list[0];
	ik.end_joint   = &ik.joint_info_list[joint_chain.size()-1];
	ik.joint_quat_list = joint_quat;

	return true;	
}

float MeCtLimb::computeLimbLength()
{
	float fLength = 0.f;
	for (int i=0;i<joint_chain.size()-1;i++)
	{
		fLength += joint_chain[i]->offset().len();
	}
	return fLength;
}