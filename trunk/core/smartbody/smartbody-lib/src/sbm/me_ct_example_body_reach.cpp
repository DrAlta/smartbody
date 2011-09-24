#include <assert.h>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <sr/sr_timer.h>
#include "mcontrol_util.h"
#include "me_ct_example_body_reach.hpp"
#include "me_ct_barycentric_interpolation.h"
#include "sbm/Event.h"
#include "MeCtBodyReachState.h"


using namespace boost;


/************************************************************************/
/* Exampled-Based Reach Controller                                      */
/************************************************************************/

std::string MeCtExampleBodyReach::CONTROLLER_TYPE = "BodyReach";

MeCtExampleBodyReach::MeCtExampleBodyReach( std::map<int,MeCtReachEngine*>& reMap )  : SmartBody::SBController()
{
	currentReachData = NULL;
	currentReachEngine = NULL;

	_duration = -1.f;	
	footIKFix = true;
	useProfileInterpolation = false;
	useRetiming = false;
	isMoving = false;
	startReach = false;
	endReach = false;
	autoReturnDuration = -1.f;
	defaultReachType = -1;
	reachVelocityScale = 1.f; 
	addDefaultAttributeFloat("reach.autoReturnDuration",-1.f,&autoReturnDuration);
	addDefaultAttributeFloat("reach.velocityScale",1.f,&reachVelocityScale);
	addDefaultAttributeBool("reach.footIK",true,&footIKFix);
	addDefaultAttributeBool("reach.useProfileInterpolation",false,&useProfileInterpolation);
	addDefaultAttributeBool("reach.useRetiming",false,&useRetiming);

	reachEngineMap = reMap;
	ReachEngineMap::iterator mi;
	for (mi  = reachEngineMap.begin();
		mi != reachEngineMap.end();
		mi++)
	{
		MeCtReachEngine* re = mi->second;
		if (re)
		{
			re->getReachData()->reachControl = this;
		}
	}
	if (reachEngineMap.size() > 0)
	{
		currentReachEngine = reachEngineMap[MeCtReachEngine::RIGHT_ARM];
		currentReachData = currentReachEngine->getReachData();
	}	
}

MeCtExampleBodyReach::~MeCtExampleBodyReach( void )
{

}

void MeCtExampleBodyReach::setDefaultReachType( const std::string& reachTypeName )
{
	defaultReachType = MeCtReachEngine::getReachType(reachTypeName);	
}

void MeCtExampleBodyReach::setHandActionState( MeCtReachEngine::HandActionState newState )
{
	//curHandActionState = newState;	
	currentReachEngine->curHandActionState = newState;
}


void MeCtExampleBodyReach::setReachCompleteDuration( float duration )
{
	//reachData->autoReturnTime = duration;
	autoReturnDuration = duration;
}

void MeCtExampleBodyReach::setFinishReaching( bool isFinish )
{
	//currentReachData->endReach = isFinish;
	endReach = isFinish;
}

void MeCtExampleBodyReach::setFootIK( bool useIK )
{
	//reachEngine->footIKFix = useIK;
	footIKFix = useIK;
}

void MeCtExampleBodyReach::setLinearVelocity( float vel )
{
	currentReachData->linearVel = vel;
}

bool MeCtExampleBodyReach::addHandConstraint( SkJoint* targetJoint, const char* effectorName )
{
	return currentReachEngine->addHandConstraint(targetJoint,effectorName);
}

void MeCtExampleBodyReach::setReachTargetPawn( SbmPawn* targetPawn )
{
	//reachTargetPawn = targetPawn;	
	ReachTarget& t = currentReachData->reachTarget;
	EffectorState& estate = currentReachData->effectorState;

	t.setTargetPawn(targetPawn);	
	//currentReachData->startReach = true;	
	startReach = true;
}

void MeCtExampleBodyReach::setReachTargetJoint( SkJoint* targetJoint )
{
	ReachTarget& t = currentReachData->reachTarget;
	t.setTargetJoint(targetJoint);
	startReach = true;
	//currentReachData->startReach = true;	
}

void MeCtExampleBodyReach::setReachTargetPos( SrVec& targetPos )
{
	ReachTarget& t = currentReachData->reachTarget;
	SRT ts;
	ts.tran = targetPos;
	ts.rot = t.targetState.rot;
	t.setTargetState(ts);
	//currentReachData->startReach = true;	
	startReach = true;
}

bool MeCtExampleBodyReach::updateLocomotion()
{	
	// we only move the character when it is idle
	if (currentReachEngine->getCurrentState()->curStateName() != "Idle")
		return true;

	float x,y,z,h,p,r;
	SbmCharacter* character = currentReachEngine->getCharacter();
	character->get_world_offset(x,y,z,h,p,r);	
	SrVec curPos = SrVec(x,y,z);

	SrVec targetXZ = currentReachData->reachTarget.getTargetState().tran; targetXZ.y = 0.f;
	SrVec distanceVec(x, y, z);
	float dist = currentReachData->XZDistanceToTarget(distanceVec);	
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (dist > character->getHeight()*0.35f && !isMoving && startReach && mcu.steerEngine.isInitialized() )//currentReachData->startReach) 
	{	
		// if the target is far away, move the character first
		//printf("idle to walk\n");
		std::string cmd;
		std::string charName = character->getName();		
		SrVec curXZ = curPos; curXZ.y = 0.f;
		SrVec targetDir = targetXZ - curXZ; targetDir.normalize();					
		SrVec steerTarget = curXZ + targetDir*(dist - character->getHeight()*0.2f);
		float facing = ((float)acos(dot(targetDir,SrVec(0,0,1))))*180.f/(float)M_PI;
		if (dot(cross(targetDir,SrVec(0,0,1)),SrVec(0,1,0)) > 0.f)
			facing = -facing;
		//LOG("facing = %f\n",facing);
		cmd = "bml char " + charName + " <locomotion target=\"" + boost::lexical_cast<std::string>(steerTarget.x) + " " + 
			boost::lexical_cast<std::string>(steerTarget.z) + "\"/>";//+ "\" facing=\"" + boost::lexical_cast<std::string>(facing) +"\"/>";//"\" proximity=\"" +  boost::lexical_cast<std::string>(rd->characterHeight*0.8f*0.01f) +"\"/>";
		//rd->curHandAction->sendReachEvent(cmd);			
		mcuCBHandle::singleton().execute(const_cast<char*>(cmd.c_str()));
		isMoving = true;
		//currentReachData->startReach = false;
		startReach = false;
		return false;
	}
	else if (!isMoving && startReach)//currentReachData->startReach) // the object is already close to character, no need to move
	{		
		LOG("reach in place\n");
		updateReachType(targetXZ);
		//setFadeIn(0.5f);
		return true;
	}

	if (isMoving && character->_reachTarget && !character->_lastReachStatus) // character is moving and has reached the target
	{
		if (dist < character->getHeight()*0.25f)
		{			
			// choose the correct hand
			//LOG("reach after locomotion\n");
			updateReachType(targetXZ);
			//currentReachData->startReach = true;
			startReach = true;
			isMoving = false;
			//setFadeIn(0.5f);
			return true;
		}
		else
		{
			LOG("[Reach Controller] Warning : Locomotion can not reach the target\n");
			//currentReachData->startReach = false;
			startReach = false;
			isMoving = false;
			return true;
		}					
	}
	return false;
}

void MeCtExampleBodyReach::updateReachType(SrVec& targetPos)
{
	if (currentReachEngine->curHandActionState == MeCtReachEngine::PUT_DOWN_OBJECT) // always putdown the object with current hand
		return;

	float x,y,z,h,p,r;
	SbmCharacter* character = currentReachEngine->getCharacter();
	character->get_world_offset(x,y,z,h,p,r);
	SrVec targetDir = SrVec(targetPos.x - x, 0, targetPos.z - z); targetDir.normalize();
	SrVec charDir = character->getFacingDirection(); 


	MeCtReachEngine* newEngine = currentReachEngine;	
	SrVec crossDir = cross(targetDir,charDir);

	if (dot(crossDir,SrVec(0,1,0)) > 0 && isValidReachEngine(MeCtReachEngine::RIGHT_ARM)) // right hand
	{
		MeCtReachEngine* re = reachEngineMap[MeCtReachEngine::RIGHT_ARM];		
		newEngine = reachEngineMap[MeCtReachEngine::RIGHT_ARM];
	}	
	else if (isValidReachEngine(MeCtReachEngine::LEFT_ARM))
	{
		MeCtReachEngine* re = reachEngineMap[MeCtReachEngine::LEFT_ARM];		
		newEngine = reachEngineMap[MeCtReachEngine::LEFT_ARM];
	}
	setNewReachEngine(newEngine);	
}


void MeCtExampleBodyReach::setNewReachEngine( MeCtReachEngine* newEngine )
{
	if (newEngine == currentReachEngine) // no need to change if it is already the current engine
		return;

	ReachStateData* newData = newEngine->getReachData();
	newData->reachTarget = currentReachData->reachTarget;
	newData->startReach  = currentReachData->startReach;
	newData->endReach = currentReachData->endReach;
	newEngine->curHandActionState = currentReachEngine->curHandActionState;	
	newEngine->fadingWeight       = currentReachEngine->fadingWeight;
	currentReachEngine = newEngine;
	currentReachData   = newData;	
}

bool MeCtExampleBodyReach::controller_evaluate( double t, MeFrameData& frame )
{	
	//updateDefaultVariables(frame);
	updateDt((float)t);	
	updateChannelBuffer(frame,inputMotionFrame,true);	

	// add logic to steer the character if it is too far away

	bool canReach = updateLocomotion();
	if (defaultReachType != -1 && reachEngineMap.find(defaultReachType) != reachEngineMap.end())
	{
		MeCtReachEngine* newEngine = reachEngineMap[defaultReachType];
		setNewReachEngine(newEngine);
		//setFadeIn(0.5f);
	}
	// update control parameters
	currentReachEngine->fadingWeight = blendWeight;
	currentReachData->autoReturnTime = autoReturnDuration;	
	currentReachData->useProfileInterpolation = useProfileInterpolation;
	currentReachData->useRetiming = useRetiming;
	currentReachData->linearVel = currentReachEngine->ikDefaultVelocity*reachVelocityScale;
	currentReachEngine->footIKFix    = footIKFix;
	//if (canReach)
	if (startReach)
	{
		currentReachData->startReach     = startReach;
		startReach = false;
	}
	if (endReach)
	{
		currentReachData->endReach = endReach;
		endReach = false;
	}
	

	//startReach = currentReachData->startReach;

	// blending the input frame with ikFrame based on current fading
	bool finishFadeOut = updateFading(dt);
	currentReachEngine->updateReach((float)t,dt,inputMotionFrame,blendWeight);
	//printf("blend weight = %f\n",blendWeight);
	BodyMotionFrame outMotionFrame;
	MotionExampleSet::blendMotionFrame(inputMotionFrame,currentReachEngine->outputMotion(),blendWeight,outMotionFrame);	

	ConstraintMap& handConstraint = currentReachEngine->getHandConstraint();
	ConstraintMap::iterator si;
	for ( si  = handConstraint.begin();
		si != handConstraint.end();
		si++)
	{	
		EffectorJointConstraint* cons = dynamic_cast<EffectorJointConstraint*>(si->second);//rotConstraint[i];
		SrVec targetPos = currentReachEngine->getMotionParameter()->getMotionFrameJoint(outMotionFrame,cons->efffectorName.c_str())->gmat().get_translation();
		for (int k=0;k<3;k++)
			cons->targetJoint->pos()->value(k,targetPos[k]);		
		cons->targetJoint->update_gmat();
	}
	updateChannelBuffer(frame,outMotionFrame);
	return true;
}

void MeCtExampleBodyReach::init(SbmPawn* pawn)
{	
	IKTreeNodeList& nodeList = currentReachEngine->ikTreeNodes();
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
		_channels.add(joint->name(), SkChannel::Quat);		
	}		
	blendWeight = currentReachEngine->fadingWeight;
	//LOG("init blend weight = %f\n",blendWeight);	
	MeController::init(pawn);	
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

bool MeCtExampleBodyReach::isValidReachEngine( int reachType )
{
	if (reachEngineMap.find(reachType) != reachEngineMap.end())
		return true;

	//MeCtReachEngine* re = reachEngineMap[reachType];
	//return re->isValid();
	return false;
}

