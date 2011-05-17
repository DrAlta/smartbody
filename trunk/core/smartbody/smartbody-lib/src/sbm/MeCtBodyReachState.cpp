#include "MeCtBodyReachState.h"
#include "sbm/Event.h"

/************************************************************************/
/* Effector State                                                       */
/************************************************************************/
EffectorState::EffectorState()
{
	attachedPawn = NULL;
}

void EffectorState::attachPawnTarget( ReachTarget& target )
{
	if (target.getTargetPawn())
	{
		attachedPawn = target.getTargetPawn();
		attachMat = attachedPawn->get_world_offset_joint()->gmat()*curState.gmat().inverse();	

		target.setTargetState(target.getTargetState());
		//SRT newTarget; newTarget.gmat(attachedPawn->get_world_offset_joint()->gmat());
		//target.setTargetState(newTarget);
	}
}

void EffectorState::releasePawn()
{
	attachedPawn = NULL;
	attachMat = SrMat();
}

void EffectorState::updateAttachedPawn()
{
	if (attachedPawn)
	{
 		SrMat effectorWorld = curState.gmat();// motionParameter->getMotionFrameJoint(ikMotionFrame,reachEndEffector->name().get_string())->gmat();
 		SrMat newWorld = attachMat*effectorWorld;
 		attachedPawn->setWorldOffset(newWorld);	 		
 	}

}
/************************************************************************/
/* Reach Target                                                         */
/************************************************************************/
SRT ReachTarget::getTargetState()
{
	SRT st = targetState;
	if (useTargetPawn && targetPawn)
	{
		st.gmat(targetPawn->get_world_offset_joint()->gmat());
	}
	return st;	
}

bool ReachTarget::targetIsPawn()
{
	return (useTargetPawn && targetPawn);
}

SbmPawn* ReachTarget::getTargetPawn()
{
	if (useTargetPawn && targetPawn && targetPawn->colObj_p)
	{
		return targetPawn;
	}
	return NULL;
}


SRT ReachTarget::getGrabTargetState( SRT& naturalState )
{
	SRT st = naturalState;
	//st.tran = getTargetState().tran;
	// if there is a collider object, estimate the correct hand position & orientation
	if (useTargetPawn && targetPawn && targetPawn->colObj_p)
	{
		targetPawn->colObj_p->estimateHandPosture(naturalState.rot,st.tran,st.rot);				
	}
	return st;
}

ReachTarget::ReachTarget()
{
	targetPawn = NULL;
	useTargetPawn = false;
}

void ReachTarget::setTargetState( SRT& ts )
{
	targetState = ts;
	targetPawn = NULL;
	useTargetPawn = false;
}

void ReachTarget::setTargetPawn( SbmPawn* tpawn )
{
	targetPawn = tpawn;
	useTargetPawn = true;
}

/************************************************************************/
/* Reach Hand Action                                                    */
/************************************************************************/
void ReachHandAction::sendReachEvent( std::string cmd, float time /*= 0.0*/ )
{
	std::string eventType = "reach";		
	MotionEvent motionEvent;
	motionEvent.setType(eventType);			
	motionEvent.setParameters(cmd);
	EventManager* manager = EventManager::getEventManager();		
	manager->handleEvent(&motionEvent,time);
}

SRT ReachHandAction::getHandTargetStateOffset( ReachStateData* rd, SRT& naturalState )
{
	if (rd->effectorState.attachedPawn)
		return rd->effectorState.grabStateError;
	else
		return SRT();
}

/************************************************************************/
/* Reach Hand Pick-Up Action                                            */
/************************************************************************/
void ReachHandPickUpAction::reachPreCompleteAction( ReachStateData* rd )
{			
	ReachTarget& rtarget = rd->reachTarget;
	SbmPawn* targetPawn = rtarget.getTargetPawn();
	if (!targetPawn)
		return;
	std::string cmd;
	std::string charName = rd->charName;
	std::string targetName = targetPawn->name;
	cmd = "bml char " + charName + " <sbm:grab sbm:handle=\"" + charName + "_gc\" sbm:wrist=\"r_wrist\" sbm:grab-state=\"start\" target=\"" + targetName  + "\"/>";
	sendReachEvent(cmd);	
}

void ReachHandPickUpAction::reachCompleteAction( ReachStateData* rd )
{
	ReachTarget& rtarget = rd->reachTarget;
	SbmPawn* targetPawn = rtarget.getTargetPawn();
	if (!targetPawn)
		return;

	std::string cmd;
	std::string charName = rd->charName;
	std::string targetName = targetPawn->name;
	cmd = "bml char " + charName + " <sbm:grab sbm:handle=\"" + charName + "_gc\" sbm:wrist=\"r_wrist\" sbm:grab-state=\"reach\" target=\"" + targetName  + "\"/>";
	sendReachEvent(cmd);
	// object attachedment
	rd->effectorState.attachPawnTarget(rtarget);
}

SRT ReachHandPickUpAction::getHandTargetStateOffset( ReachStateData* rd, SRT& naturalState )
{		
	if (rd->reachTarget.getTargetPawn())
	{
		SRT handState = rd->reachTarget.getGrabTargetState(naturalState);	
		return SRT::diff(naturalState,handState);
	}	
	else
		return rd->effectorState.grabStateError;
}

bool ReachHandPickUpAction::pickUpNewPawn( ReachStateData* rd )
{
	ReachTarget& rtarget = rd->reachTarget;
	EffectorState& estate = rd->effectorState;
	if (rd->startReach && rtarget.targetIsPawn())
	{
		return (estate.attachedPawn != rtarget.getTargetPawn());
	}
	return false;	
}

void ReachHandPickUpAction::reachNewTargetAction( ReachStateData* rd )
{
	std::string cmd;
	std::string charName = rd->charName;
	cmd = "bml char " + charName + " <sbm:grab sbm:handle=\"" + charName + "_gc\" sbm:grab-state=\"finish\"/>";
	sendReachEvent(cmd);
	rd->effectorState.releasePawn();
}
/************************************************************************/
/* Reach Hand Put-Down Action                                           */
/************************************************************************/
void ReachHandPutDownAction::reachCompleteAction( ReachStateData* rd )
{
	std::string cmd;
	std::string charName = rd->charName;
	cmd = "bml char " + charName + " <sbm:grab sbm:handle=\"" + charName + "_gc\" sbm:grab-state=\"finish\"/>";
	sendReachEvent(cmd);
	rd->effectorState.releasePawn();
}

void ReachHandPutDownAction::reachReturnAction( ReachStateData* rd )
{
	std::string cmd;
	std::string charName = rd->charName;
	cmd = "bml char " + charName + " <sbm:grab sbm:handle=\"" + charName + "_gc\" sbm:grab-state=\"return\"/>";
	sendReachEvent(cmd);	
	rd->effectorState.releasePawn();
}

SRT ReachHandPutDownAction::getHandTargetStateOffset( ReachStateData* rd, SRT& naturalState )
{
	return rd->effectorState.grabStateError;
}
/************************************************************************/
/* Reach State Data                                                     */
/************************************************************************/

ReachStateData::ReachStateData()
{
	curTime = 0.f;
	curRefTime = 0.f;
	dt = du = 0.f;
	startReach = endReach = useExample = false;
	autoReturnTime = -1.f;

	interpMotion = NULL;
	motionParameter = NULL; 
	dataInterpolator = NULL;
	curHandAction = NULL;

	angularVel = 0.1f;
	linearVel = 70.f;
}

void ReachStateData::updateReachState(const SrMat& worldOffset, BodyMotionFrame& motionFrame )
{
	gmat = worldOffset;
	curMotionFrame = motionFrame;
	effectorState.curState = getPoseState(motionFrame);
	effectorState.updateAttachedPawn();
}

void ReachStateData::updateBlendWeight( SrVec paraPos )
{
	SrVec localTarget = paraPos*gmat.inverse();
	dVector para; para.resize(3);
	for (int i=0;i<3;i++)
		para[i] = localTarget[i];
	dataInterpolator->predictInterpWeights(para,interpMotion->weight);	
}

SRT ReachStateData::getBlendPoseState( SrVec paraPos, float refTime )
{
	BodyMotionFrame outFrame;
	updateBlendWeight(paraPos);
	getInterpFrame(refTime,outFrame);
	return getPoseState(outFrame);
}


SRT ReachStateData::getPoseState( BodyMotionFrame& frame )
{
	SkJoint* targetJoint = motionParameter->getMotionFrameJoint(frame,effectorState.effectorName.c_str());	
	SRT poseState;
	poseState.tran = targetJoint->gmat().get_translation();
	poseState.rot  = SrQuat(targetJoint->gmat());
	return poseState;
}

void ReachStateData::getInterpFrame( float refTime, BodyMotionFrame& outFrame )
{
	interpMotion->getMotionFrame(refTime,motionParameter->skeletonRef,motionParameter->affectedJoints,outFrame);	
}

bool ReachStateData::useInterpolation()
{
	return (interpMotion && useExample);
}

/************************************************************************/
/* Reach State Interface                                                */
/************************************************************************/

void ReachStateInterface::updateReturnToIdle( ReachStateData* rd )
{
	// only change effector target state, no change on para blend weight	
	EffectorState& estate = rd->effectorState;
	estate.targetState = rd->getPoseState(rd->idleRefFrame);
	if (rd->useInterpolation())
	{
		float rtime = (float)rd->interpMotion->motionDuration(BodyMotionInterface::DURATION_REF)*0.999f;
		rd->getInterpFrame(rtime,rd->targetRefFrame);
		estate.targetState = rd->getPoseState(rd->targetRefFrame);
	}	
}

void ReachStateInterface::updateReachToTarget( ReachStateData* rd )
{	
	EffectorState& estate = rd->effectorState;
	ReachTarget& rtarget = rd->reachTarget;
	SRT ts = rtarget.getTargetState();
	SRT tsBlend = rd->getPoseState(rd->targetRefFrame);

	if (rd->useInterpolation())
	{
		float stime = (float)rd->interpMotion->strokeEmphasisTime();
		estate.paraTarget = ts.tran;	
		rd->updateBlendWeight(estate.paraTarget);
		tsBlend = rd->getBlendPoseState(estate.paraTarget,stime);		
		rd->getInterpFrame(stime,rd->targetRefFrame);
	}		
	
	tsBlend.tran = ts.tran;
	SRT offset = rd->curHandAction->getHandTargetStateOffset(rd,tsBlend);
	tsBlend.add(offset);		
	estate.targetState = tsBlend;
	estate.grabStateError = offset;
}

void ReachStateInterface::updateMotionIK( ReachStateData* rd )
{
	EffectorState& estate = rd->effectorState;
	float reachStep = rd->linearVel*rd->dt;

	SRT diff = SRT::diff(estate.curTargetState,estate.targetState);
	SrVec offset = diff.tran;

	float offsetLength = offset.norm();								
	if (offset.norm() > reachStep)
	{
		offset.normalize();
		offset = offset*reachStep;
	}
	float morphWeight = offsetLength > 0.f ? (offset.norm()+reachStep)/(offsetLength+reachStep) : 1.f;	

	BodyMotionFrame morphFrame;				
	MotionExampleSet::blendMotionFrame(rd->currentRefFrame,rd->targetRefFrame,morphWeight,morphFrame);						
	estate.curTargetState = SRT::blend(estate.curTargetState,estate.targetState,morphWeight);
	rd->currentRefFrame = morphFrame;	
}

void ReachStateInterface::updateMotionInterp( ReachStateData* rd )
{
	ResampleMotion* interpMotion = rd->interpMotion;

	if (!interpMotion || !rd->useExample)
	{
		updateMotionIK(rd);
		return;
	}
	EffectorState& estate = rd->effectorState;
	SRT stateOffset = SRT::diff(rd->getPoseState(rd->currentRefFrame),estate.curTargetState);
	BodyMotionFrame outFrame;
	rd->getInterpFrame(rd->curRefTime,outFrame);
	SRT interpState = rd->getPoseState(outFrame);	
	SRT stateError = SRT::diff(rd->getPoseState(rd->targetRefFrame),estate.targetState);	

	float percentTime = curStatePercentTime(rd,rd->curRefTime);		
	float deltaPercent = percentTime - curStatePercentTime(rd,rd->curRefTime-rd->du);
	float remain = 1.f - percentTime;
	float weight = 1.f;
	if (deltaPercent < remain && remain != 0.f)
		weight = deltaPercent /remain;

	SRT delta = SRT::blend(stateOffset,stateError,weight);
	interpState.add(delta);
	estate.curTargetState = interpState;
	rd->currentRefFrame = outFrame;	

	rd->du = (float)interpMotion->getRefDeltaTime(rd->curRefTime,rd->dt);
	rd->curRefTime += rd->du*0.7f;
}

bool ReachStateInterface::ikTargetReached( ReachStateData* rd )
{
	EffectorState& estate = rd->effectorState;
	float dist = SRT::dist(estate.curState,estate.targetState);
	return (dist < rd->reachRegion*0.1f);
}

bool ReachStateInterface::interpTargetReached( ReachStateData* rd )
{
	if (rd->useInterpolation())
		return (curStatePercentTime(rd,rd->curRefTime) >= 1.f);
	else
		return ikTargetReached(rd);
}
/************************************************************************/
/* Reach State Idle                                                       */
/************************************************************************/
void ReachStateIdle::updateEffectorTargetState( ReachStateData* rd )
{
	ReachStateInterface::updateReturnToIdle(rd);	
}

void ReachStateIdle::update( ReachStateData* rd )
{
	ReachStateInterface::updateMotionIK(rd);	
}

std::string ReachStateIdle::nextState( ReachStateData* rd )
{
	std::string nextStateName = "Idle";
	if (rd->startReach)
	{		
		rd->curRefTime = 0.f;
		nextStateName = "Start";
	}
	return nextStateName;
}

/************************************************************************/
/* Reach State Start                                                    */
/************************************************************************/
void ReachStateStart::updateEffectorTargetState( ReachStateData* rd )
{
	ReachStateInterface::updateReachToTarget(rd);
}

float ReachStateStart::curStatePercentTime( ReachStateData* rd, float refTime)
{
	if (!rd->useInterpolation())
		return ReachStateInterface::curStatePercentTime(rd,refTime);

	ResampleMotion* interpMotion = rd->interpMotion;
	float percent = refTime/ (float)interpMotion->strokeEmphasisTime();
	return min(max(0.f,percent),1.f);
}

void ReachStateStart::update( ReachStateData* rd )
{
	ReachStateInterface::updateMotionInterp(rd);
	if (curStatePercentTime(rd,rd->curRefTime) > 0.8f && rd->startReach)
	{
		rd->curHandAction->reachPreCompleteAction(rd); 
		rd->startReach = false;
	}
}

std::string ReachStateStart::nextState( ReachStateData* rd )
{
	std::string nextStateName = "Start";
	if (interpTargetReached(rd))
	{
		rd->curHandAction->reachCompleteAction(rd);
		rd->startReach = false;
		nextStateName = "Complete";
	}
	return nextStateName;
}

/************************************************************************/
/* Reach State Complete                                                 */
/************************************************************************/
void ReachStateComplete::updateEffectorTargetState( ReachStateData* rd )
{
	ReachStateInterface::updateReachToTarget(rd);
	rd->curRefTime = (float)rd->interpMotion->strokeEmphasisTime();
}

void ReachStateComplete::update( ReachStateData* rd )
{
	ReachStateInterface::updateMotionIK(rd);
	completeTime += rd->dt;
}

std::string ReachStateComplete::nextState( ReachStateData* rd )
{
	std::string nextStateName = "Complete";
	bool toNextState = rd->autoReturnTime > 0.f ? rd->autoReturnTime < completeTime : rd->endReach;
	
	if (toNextState)
	{
		rd->curHandAction->reachReturnAction(rd);
		rd->endReach = false;		
		completeTime = 0.f; // reset complete time
		nextStateName = "Return";
	}
	else if (rd->curHandAction->pickUpNewPawn(rd))
	{
		rd->curHandAction->reachNewTargetAction(rd);
		completeTime = 0.f;
		nextStateName = "NewTarget";
	}
	return nextStateName;
}
/************************************************************************/
/* Reach State New Target                                               */
/************************************************************************/
void ReachStateNewTarget::updateEffectorTargetState( ReachStateData* rd )
{
	ReachStateInterface::updateReachToTarget(rd);
	rd->curRefTime = (float)rd->interpMotion->strokeEmphasisTime();
}

void ReachStateNewTarget::update( ReachStateData* rd )
{
	ReachStateInterface::updateMotionIK(rd);	
}
std::string ReachStateNewTarget::nextState( ReachStateData* rd )
{
	std::string nextStateName = "NewTarget";
	if (ikTargetReached(rd))
	{
		rd->curHandAction->reachCompleteAction(rd);
		rd->startReach = false;
		nextStateName = "Complete";
	}
	return nextStateName;
}
/************************************************************************/
/* Reach State Return                                                   */
/************************************************************************/
void ReachStateReturn::updateEffectorTargetState( ReachStateData* rd )
{
	ReachStateInterface::updateReturnToIdle(rd);	
}

float ReachStateReturn::curStatePercentTime( ReachStateData* rd, float refTime )
{
	if (!rd->useInterpolation())
		return ReachStateInterface::curStatePercentTime(rd,refTime);

	ResampleMotion* interpMotion = rd->interpMotion;
	float percent = (refTime - (float)interpMotion->strokeEmphasisTime())/(float)(interpMotion->motionDuration(BodyMotionInterface::DURATION_REF)-interpMotion->strokeEmphasisTime());	
	return min(max(0.f,percent),1.f);
}

void ReachStateReturn::update( ReachStateData* rd )
{
	ReachStateInterface::updateMotionInterp(rd);
}

std::string ReachStateReturn::nextState( ReachStateData* rd )
{
	std::string nextStateName = "Return";
	if (interpTargetReached(rd))
	{
		rd->startReach = false;
		rd->endReach = false;
		nextStateName = "Idle";
	}
	return nextStateName;
}