#include "me_ct_reach.hpp"
#include <assert.h>

const char* MeCtReach::CONTROLLER_TYPE = "Reach";

const int  NUM_LIMBS = 4;
const char limb_chain[][20] = {"r_shoulder", "r_elbow", "r_forearm", "r_wrist"};
const MeCtLimbJointConstraint limb_constraint[] = { {JOINT_TYPE_BALL, M_PI*0.5f, M_PI*0.5f}, {JOINT_TYPE_BALL, M_PI*0.5f, M_PI*0.5f}, {JOINT_TYPE_HINGE, M_PI*0.7f, M_PI*0.1f}, {JOINT_TYPE_BALL, M_PI*0.5f, M_PI*0.5f}  };

MeCtReach::MeCtReach(void)
{
	reach_mode = TARGET_POS;
	target_pos = SrVec(0.f,150.f,0.f);
	target_joint_ref = NULL;
}

MeCtReach::~MeCtReach(void)
{
}

SrVec MeCtReach::get_reach_target()
{
	SrVec target;
	if (reach_mode == TARGET_POS)
	{
		target = target_pos;
	}
	else if (reach_mode == TARGET_JOINT_POS)
	{
		assert(target_joint_ref);
		if( target_joint_ref )	
		{
			SrMat sr_M;
			matrix_t M;
			int i, j;
			
			target_joint_ref->update_gmat_up();
			sr_M = target_joint_ref->gmat();			
			target = SrVec(sr_M.get(12),sr_M.get(13),sr_M.get(14));			
	    }
	}
	return target;
}

void MeCtReach::set_target_joint(SkJoint* target_joint)
{
	if (target_joint)
	{
		reach_mode = TARGET_JOINT_POS;
		target_joint_ref = target_joint;
	}
	else
	{
		reach_mode = TARGET_POS;
	}
}

void MeCtReach::init()
{
	joint_name.size(NUM_LIMBS);
	joint_constraint.size(NUM_LIMBS);
	for (int i=0;i<NUM_LIMBS;i++)
	{
		joint_name[i] = limb_chain[i];
		joint_constraint[i] = limb_constraint[i];
		_channels.add(joint_name[i], SkChannel::Quat);
		//_channels.add("l_shoulder", SkChannel::Quat);
	}

	MeController::init();
}

void MeCtReach::controller_map_updated() 
{
	// init limbs here
	SkSkeleton* skeleton;
	if( _context->channels().size() > 0 )	{
		skeleton = _context->channels().skeleton();
	}
	
	if (!skeleton)
		return;

	limb.init(skeleton);
	limb.buildJointChain(joint_name,joint_constraint);
	// set buffer index mapping
	for (int i=0;i<joint_name.size();i++)
	{
		limb.buf_index[i] = _toContextCh[i];
	}
	

	//printf("Finish update controller map\n");
}

bool MeCtReach::controller_evaluate( double t, MeFrameData& frame )
{	
	// update from frame buffer to joint quat in the limb
	limb.updateQuat(frame,true);
	limb.ik.ik_offset = get_reach_target(); // set the target

	MeCtIKScenario* ik_scenario = &limb.ik;
	
	limb.skeleton->update_global_matrices();
	SkJoint* chain_root = limb.getChainRoot();
	ik_scenario->gmat = chain_root->parent()->gmat();
	
	ik.update(ik_scenario);
	limb.joint_quat = ik_scenario->joint_quat_list;
	// write results from limb to buffer
	limb.updateQuat(frame,false);

	return true;

}

void MeCtReach::controller_start()
{
}

void MeCtReach::print_state(int tabs)
{
}




