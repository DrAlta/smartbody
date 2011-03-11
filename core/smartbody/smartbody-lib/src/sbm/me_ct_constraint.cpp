#include "me_ct_constraint.hpp"
#include <assert.h>
#include <boost/foreach.hpp>
#include <sr/sr_timer.h>
#include <sbm/gwiz_math.h>
using namespace gwiz;

const char* MeCtConstraint::CONTROLLER_TYPE = "Constraint";

bool MeCtConstraint::useBalance = false;
bool MeCtConstraint::useReferenceJoint = true;
bool MeCtConstraint::useIKConstraint = false;

MeCtConstraint::MeCtConstraint( SkSkeleton* skeleton ) 
{
	_skeleton = skeleton;
	prev_time = -1.0;
	blendWeight = 0.0;
}

MeCtConstraint::~MeCtConstraint(void)
{
	
}

void MeCtConstraint::updateChannelBuffer(MeFrameData& frame, std::vector<SrQuat>& quatList, bool bRead)
{
	SrBuffer<float>& buffer = frame.buffer();
	int count = 0;
	BOOST_FOREACH(SrQuat& quat, quatList)
	{
		int index = frame.toBufferIndex(_toContextCh[count++]);		
		if (bRead)
		{
			quat.w = buffer[index] ;
			quat.x = buffer[index + 1] ;
			quat.y = buffer[index + 2] ;
			quat.z = buffer[index + 3] ;			
		}
		else
		{
			buffer[index] = quat.w;
			buffer[index + 1] = quat.x;
			buffer[index + 2] = quat.y;
			buffer[index + 3] = quat.z;
		}		
	}	
}


void MeCtConstraint::init()
{
	assert(_skeleton);	
	// root is "world_offset", so we use root->child to get the base joint.
	SkJoint* rootJoint = _skeleton->root()->child(0);//_skeleton->search_joint("base"); // test for now

	ik_scenario.buildIKTreeFromJointRoot(rootJoint);
	int numConstraint = 1;
	//ik_scenario.ikEndEffectors.resize(numConstraint);
	//ik_scenario.ikTargetPos.resize(numConstraint);	

	const IKTreeNodeList& nodeList = ik_scenario.ikTreeNodes;	
	
	for (int i=0;i<nodeList.size();i++)
	{
		SkJoint* joint = nodeList[i]->joint;
		_channels.add(joint->name().get_string(), SkChannel::Quat);
	}
	//ik_scenario.ikEndEffectors[0] = ik_scenario.findIKTreeNode("r_wrist"); // test for now
 	//ik_scenario.ikEndEffectors[1] = ik_scenario.findIKTreeNode("l_wrist");
  	//ik_scenario.ikEndEffectors[1] = ik_scenario.findIKTreeNode("r_toe"); // test for now
  	//ik_scenario.ikEndEffectors[3] = ik_scenario.findIKTreeNode("l_toe");

// 	for (int i=1;i<numConstraint;i++)
// 	{
// 		const SrMat& gmat = ik_scenario.ikEndEffectors[i]->joint->gmat();
// 		ik_scenario.ikEndEffectors[i]->targetPos = SrVec(gmat.get(12),gmat.get(13),gmat.get(14));
// 	}

 	MeController::init();
}

void MeCtConstraint::controller_map_updated() 
{		
	
}

bool MeCtConstraint::controller_evaluate( double t, MeFrameData& frame )
{	
	SrTimer time;
	time.start();
	float dt = 0.001f;
	if (prev_time == -1.0) // first start
	{
		dt = 0.001f;		
		// for first frame, update from frame buffer to joint quat in the limb
		// any future IK solving will simply use the joint quat from the previous frame.
		//limb.updateQuat(frame,true);
		updateChannelBuffer(frame,ik_scenario.ikInitQuatList,true);			
	}
	else
	{		
		dt = ((float)(t-prev_time));
	}

	updateChannelBuffer(frame,ik_scenario.ikRefQuatList,true);	
			
	prev_time = (float)t;

	assert(ik_scenario.ikEndEffectors.size() == targetJointList.size());
	for (int i=0;i<targetJointList.size();i++)
	{
		ik_scenario.ikEndEffectors[i]->targetPos = get_reach_target(targetJointList[i]);
	}

	ik_scenario.ikTreeRoot->joint->parent()->update_gmat(); // update world offset global transformation
	ik_scenario.ikGlobalMat = ik_scenario.ikTreeRoot->joint->parent()->gmat();
	ik_scenario.ikUseBalance = useBalance;
	ik_scenario.ikUseReference = useReferenceJoint;
	ik.setDt(dt);

	updateFading(dt);
	//for (int i=0;i<4;i++)	
	std::vector<SrQuat> tempQuatList; tempQuatList.resize(ik_scenario.ikInitQuatList.size());
	if (useIKConstraint && ik_scenario.ikEndEffectors.size() != 0)	
	{
		{
			ik.update(&ik_scenario);
			ik_scenario.ikInitQuatList = ik_scenario.ikQuatList;
			for (int i=0;i<ik_scenario.ikQuatList.size();i++)
			{
				SrQuat qEval = ik_scenario.ikQuatList[i];
				SrQuat qInit = ik_scenario.ikRefQuatList[i];
				ik_scenario.ikRefQuatList[i] = slerp(qInit,qEval,blendWeight);				
			}			
		}
		updateChannelBuffer(frame,ik_scenario.ikRefQuatList);
	}
	//printf("Time = %f\n",time.t());	
	return true;

}

void MeCtConstraint::controller_start()
{
}

void MeCtConstraint::print_state(int tabs)
{
}


SrVec MeCtConstraint::get_reach_target(SkJoint* joint)
{
	SrVec target;	
	if( joint )	
	{
		SrMat sr_M;
		matrix_t M;			
		joint->update_gmat_up();
		sr_M = joint->gmat();			
		target = SrVec(sr_M.get(12),sr_M.get(13),sr_M.get(14));			
	}
	return target;
}

bool MeCtConstraint::addJointEndEffectorPair( SkJoint* targetJoint, const char* effectorName )
{
	MeCtIKTreeNode* node = ik_scenario.findIKTreeNode(effectorName);
	if (!node)
		return false;

	assert(ik_scenario.ikEndEffectors.size() == targetJointList.size());

	int idx = MeCtIKTreeScenario::findIKTreeNodeInList(effectorName,ik_scenario.ikEndEffectors);
	if (idx != -1)
	{
		targetJointList[idx] = targetJoint;
	}
	else // add effector-joint pair
	{
		ik_scenario.ikEndEffectors.push_back(node);
		targetJointList.push_back(targetJoint);
	}
	return true;
}

void MeCtConstraint::setFadeIn( float interval )
{
	fadeInterval = interval;
	fadeRemainTime = interval;
	fadeMode = FADING_MODE_IN;
	useIKConstraint = true;
}

void MeCtConstraint::setFadeOut( float interval )
{
	fadeInterval = interval;
	fadeRemainTime = interval;
	fadeMode = FADING_MODE_OUT;
}

bool MeCtConstraint::updateFading( float dt )
{
	const float FADE_EPSILON = 0.001f;
	bool finishFadeOut = false;
	if (fadeMode)
	{
		fadeRemainTime -= dt;
		if (fadeRemainTime <= 0.0)
			fadeRemainTime = 0.0;

		if (fadeMode == FADING_MODE_IN)
		{			
			float fadeNormal = 1.0 - fadeRemainTime/fadeInterval;
			blendWeight = fadeNormal;
			if (blendWeight > 1.0 - FADE_EPSILON)
			{
				blendWeight = 1.0;
				fadeMode = FADING_MODE_OFF;
			}						
		}
		else
		{
			float fadeNormal = fadeRemainTime/fadeInterval;
			blendWeight = fadeNormal;
			if (blendWeight < FADE_EPSILON)
			{
				blendWeight = 0.0;
				fadeMode = FADING_MODE_OFF;
				finishFadeOut = true;
				useIKConstraint = false;
			}	
		}
	}
	return finishFadeOut;
}