#include "me_ct_reach_IK.hpp"

MeCtReachIK::MeCtReachIK(void)
{
}

MeCtReachIK::~MeCtReachIK(void)
{
}


void MeCtReachIK::update(MeCtIKScenario* scenario)
{
	this->scenario = scenario;
	int modified = 0;
	SrMat inv_end;

	SrQuat quat;
	SrMat mat;	
	init();
	//unstretch_joints();

	adjust();

	/*
	for(int k = 0; k < support_joint_num; ++k)
	{
		if(manipulated_joint_index - start_joint_index == 1) adjust_2_joints();
		else adjust();

		//handles the support joints
		SrQuat before = scenario->joint_quat_list.get(manipulated_joint_index);
		//temp

		pm = scenario->joint_global_mat_list.get(manipulated_joint_index);
		pm.set(12, 0.0f);//??
		pm.set(13, 0.0f);
		pm.set(14, 0.0f);

		lm = joint_init_mat_list.get(manipulated_joint_index);
		lm.set(12, 0.0f);//??
		lm.set(13, 0.0f);
		lm.set(14, 0.0f);

		inv_end = lm*pm.inverse();
		before = inv_end * before;
		SrQuat after = before;
		//temp

		modified = check_constraint(&after, manipulated_joint_index);
		scenario->joint_quat_list.set(manipulated_joint_index, before);
		if(modified == 0) 
		{
			start_joint_index = manipulated_joint_index;
			get_next_support_joint();
			get_limb_section_local_pos(start_joint_index, -1);
			calc_target(scenario->ik_orientation, scenario->ik_offset);
		}
		else 
		{
			recrod_endmat = 0;
			get_next_support_joint();
			get_limb_section_local_pos(0, -1);
			scenario->joint_quat_list.set(manipulated_joint_index-1, after);
			calc_target(scenario->ik_orientation, scenario->ik_offset);
		}
	}
	*/
}

void MeCtReachIK::calc_target(SrVec& orientation, SrVec& offset)
{
	SrVec pos = scenario->joint_pos_list.get(manipulated_joint_index);
	target.set(manipulated_joint_index,offset);
}