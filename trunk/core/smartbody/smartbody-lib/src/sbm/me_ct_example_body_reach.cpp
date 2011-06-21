#include <assert.h>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <boost/foreach.hpp>
#include <SR/sr_timer.h>
#include "mcontrol_util.h"
#include "me_ct_example_body_reach.hpp"
#include "me_ct_barycentric_interpolation.h"
#include "sbm/Event.h"
#include "MeCtBodyReachState.h"


using namespace boost;


/************************************************************************/
/* Exampled-Based Reach Controller                                      */
/************************************************************************/

const char* MeCtExampleBodyReach::CONTROLLER_TYPE = "BodyReach";

MeCtExampleBodyReach::MeCtExampleBodyReach( MeCtReachEngine* re )
{
	reachEngine = re;
	reachData = re->getReachData();
	reachData->reachControl = this;			
	_duration = -1.f;	
}

MeCtExampleBodyReach::~MeCtExampleBodyReach( void )
{

}

void MeCtExampleBodyReach::setHandActionState( MeCtReachEngine::HandActionState newState )
{
	//curHandActionState = newState;	
	reachEngine->curHandActionState = newState;
}


void MeCtExampleBodyReach::setReachCompleteDuration( float duration )
{
	reachData->autoReturnTime = duration;
}

void MeCtExampleBodyReach::setFinishReaching( bool isFinish )
{
	reachData->endReach = isFinish;
}

void MeCtExampleBodyReach::setFootIK( bool useIK )
{
	reachEngine->footIKFix = useIK;
	//footIKFix = useIK;
}

void MeCtExampleBodyReach::setLinearVelocity( float vel )
{
	reachData->linearVel = vel;
}

bool MeCtExampleBodyReach::addHandConstraint( SkJoint* targetJoint, const char* effectorName )
{
	return reachEngine->addHandConstraint(targetJoint,effectorName);
}

void MeCtExampleBodyReach::setReachTargetPawn( SbmPawn* targetPawn )
{
	//reachTargetPawn = targetPawn;	
	ReachTarget& t = reachData->reachTarget;
	t.setTargetPawn(targetPawn);	
	reachData->startReach = true;	
}

void MeCtExampleBodyReach::setReachTargetJoint( SkJoint* targetJoint )
{
	ReachTarget& t = reachData->reachTarget;
	t.setTargetJoint(targetJoint);
	reachData->startReach = true;	
}

void MeCtExampleBodyReach::setReachTargetPos( SrVec& targetPos )
{
	ReachTarget& t = reachData->reachTarget;
	SRT ts;
	ts.tran = targetPos;
	ts.rot = t.targetState.rot;
	t.setTargetState(ts);
	reachData->startReach = true;	
}

bool MeCtExampleBodyReach::controller_evaluate( double t, MeFrameData& frame )
{	
	updateDt((float)t);	
	updateChannelBuffer(frame,inputMotionFrame,true);	
	reachEngine->updateReach((float)t,dt,inputMotionFrame);

	// blending the input frame with ikFrame based on current fading
	bool finishFadeOut = updateFading(dt);
	reachEngine->fadingWeight = blendWeight;
	//printf("blend weight = %f\n",blendWeight);
	BodyMotionFrame outMotionFrame;
	MotionExampleSet::blendMotionFrame(inputMotionFrame,reachEngine->outputMotion(),blendWeight,outMotionFrame);	


	ConstraintMap& handConstraint = reachEngine->getHandConstraint();
	ConstraintMap::iterator si;
	for ( si  = handConstraint.begin();
		si != handConstraint.end();
		si++)
	{	
		EffectorJointConstraint* cons = dynamic_cast<EffectorJointConstraint*>(si->second);//rotConstraint[i];
		SrVec targetPos = reachEngine->getMotionParameter()->getMotionFrameJoint(outMotionFrame,cons->efffectorName.c_str())->gmat().get_translation();
		for (int k=0;k<3;k++)
			cons->targetJoint->pos()->value(k,targetPos[k]);		
		cons->targetJoint->update_gmat();
	}
	updateChannelBuffer(frame,outMotionFrame);
	return true;
}

void MeCtExampleBodyReach::init()
{	
	IKTreeNodeList& nodeList = reachEngine->ikTreeNodes();
	MeCtIKTreeNode* rootNode = nodeList[0];
	for (int i=0;i<3;i++)
		_channels.add(rootNode->joint->name(), (SkChannel::Type)(SkChannel::XPos+i));
	affectedJoints.clear();
	for (unsigned int i=0;i<nodeList.size();i++)
	{
		MeCtIKTreeNode* node = nodeList[i];
		SkJoint* joint = node->joint;
		SkJointQuat* skQuat = joint->quat();				
		affectedJoints.push_back(joint);
		_channels.add(joint->name().get_string(), SkChannel::Quat);		
	}		
	blendWeight = reachEngine->fadingWeight;
	LOG("init blend weight = %f\n",blendWeight);
	MeController::init();	
}

void MeCtExampleBodyReach::updateChannelBuffer( MeFrameData& frame, BodyMotionFrame& motionFrame, bool bRead /*= false*/ )
{
	SrBuffer<float>& buffer = frame.buffer();
	int count = 0;
	// update root translation
	for (int i=0;i<3;i++)
	{
		int index = frame.toBufferIndex(_toContextCh[count++]);
		if (bRead)
		{
			motionFrame.rootPos[i] = buffer[index] ;
		}
		else
		{
			buffer[index] = motionFrame.rootPos[i];			
		}
	}

	if (motionFrame.jointQuat.size() != affectedJoints.size())
		motionFrame.jointQuat.resize(affectedJoints.size());

	for (unsigned int i=0;i<motionFrame.jointQuat.size();i++)
	{
		SrQuat& quat = motionFrame.jointQuat[i];		
		int index = frame.toBufferIndex(_toContextCh[count++]);	
		//printf("buffer index = %d\n",index);		
		if (index == -1)
		{
			if (bRead)
			{
				quat = SrQuat();
			}
		}
		else
		{
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
}

void MeCtExampleBodyReach::print_state( int tabs )
{

}

void MeCtExampleBodyReach::controller_start()
{
	//restart = true;
	controlRestart();
}

void MeCtExampleBodyReach::controller_map_updated()
{

}

