#include "MeCtBodyReachState.h"
#include "sbm/Event.h"
#include "sbm/mcontrol_util.h"
#include <boost/lexical_cast.hpp>

/************************************************************************/
/* Effector State                                                       */
/************************************************************************/
EffectorState::EffectorState()
{
	attachedPawn = NULL;
}

void EffectorState::setAttachedPawn( ReachStateData* rd )
{
	ReachTarget& target = rd->reachTarget;
	if (target.getTargetPawn())
	{
		attachedPawn = target.getTargetPawn();
		attachMat = attachedPawn->get_world_offset_joint()->gmat()*curState.gmat().inverse();	
		SRT state = target.getTargetState();
		target.setTargetState(state);
	}
}

void EffectorState::removeAttachedPawn(ReachStateData* rd)
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
	if (targetIsPawn())
	{
		st.gmat(targetPawn->get_world_offset_joint()->gmat());
	}
	else if (targetIsJoint())
	{
		st.gmat(targetJoint->gmat());
	}
	return st;	
}

bool ReachTarget::targetIsPawn()
{
	return (useTargetPawn && targetPawn);
}

bool ReachTarget::targetIsJoint()
{
	return (useTargetJoint && targetJoint);
}

SbmPawn* ReachTarget::getTargetPawn()
{
	if (useTargetPawn && targetPawn)// && targetPawn->colObj_p)
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
	targetJoint = NULL;
	useTargetPawn = false;
	useTargetJoint = false;
}

void ReachTarget::setTargetPawn( SbmPawn* tpawn )
{
	targetPawn = tpawn;
	useTargetPawn = true;
	useTargetJoint = false;
	targetJoint = NULL;
}

void ReachTarget::setTargetJoint( SkJoint* tjoint )
{
	targetJoint = tjoint;
	useTargetJoint = true;
	useTargetPawn = false;
	targetPawn = NULL;
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
// 	if (rd->effectorState.attachedPawn)
// 		return rd->effectorState.grabStateError;
// 	else
// 		return SRT();
	if (rd->reachTarget.getTargetPawn())
	{
		SRT handState = rd->reachTarget.getGrabTargetState(naturalState);	
		return SRT::diff(naturalState,handState);
	}	
	else
		return rd->effectorState.grabStateError;
}

void ReachHandAction::reachPreCompleteAction( ReachStateData* rd )
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

void ReachHandAction::reachCompleteAction( ReachStateData* rd )
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
	rd->effectorState.setAttachedPawn(rd);
}

void ReachHandAction::reachNewTargetAction( ReachStateData* rd )
{
	std::string cmd;
	std::string charName = rd->charName;
	cmd = "bml char " + charName + " <sbm:grab sbm:handle=\"" + charName + "_gc\" sbm:grab-state=\"finish\"/>";
	sendReachEvent(cmd);
	rd->effectorState.removeAttachedPawn(rd);
}

void ReachHandAction::reachReturnAction( ReachStateData* rd )
{
	std::string cmd;
	std::string charName = rd->charName;
	cmd = "bml char " + charName + " <sbm:grab sbm:handle=\"" + charName + "_gc\" sbm:grab-state=\"return\"/>";
	sendReachEvent(cmd);	
	rd->effectorState.removeAttachedPawn(rd);
}

bool ReachHandAction::isPickingUpNewPawn( ReachStateData* rd )
{
	ReachTarget& rtarget = rd->reachTarget;
	EffectorState& estate = rd->effectorState;
	if (rd->startReach && rtarget.targetIsPawn())
	{
		return (estate.attachedPawn != rtarget.getTargetPawn());
	}
	return false;
}

void ReachHandAction::pickUpAttachedPawn( ReachStateData* rd )
{
	// send attachment to hand controller
	SbmPawn* attachedPawn = rd->effectorState.attachedPawn;
	if (!attachedPawn)
		return;

	std::string charName = rd->charName;
	std::string targetName = attachedPawn->name;
	std::string cmd;
	cmd = "bml char " + charName + " <sbm:grab sbm:handle=\"" + charName + "_gc\" sbm:source-joint=\"" + "r_wrist" + "\" sbm:attach-pawn=\"" + targetName + "\"/>";
	rd->curHandAction->sendReachEvent(cmd);
	cmd = "pawn " + targetName + " physics off";
	rd->curHandAction->sendReachEvent(cmd);
}

void ReachHandAction::putDownAttachedPawn( ReachStateData* rd )
{
	SbmPawn* attachedPawn = rd->effectorState.attachedPawn;
	if (!attachedPawn || !attachedPawn->colObj_p)
		return;

	std::string charName = rd->charName;	
	std::string cmd;
	cmd = "bml char " + charName + " <sbm:grab sbm:handle=\"" + charName + "_gc\" sbm:release-pawn=\"true\"/>";
	rd->curHandAction->sendReachEvent(cmd);
	std::string targetName = "";	
	if (attachedPawn)
		targetName = attachedPawn->name;
	cmd = "pawn " + targetName + " physics on";
	rd->curHandAction->sendReachEvent(cmd);
}

void ReachHandAction::reachPreReturnAction( ReachStateData* rd )
{
	ReachHandAction::reachNewTargetAction(rd);
}
/************************************************************************/
/* Reach Hand Pick-Up Action                                            */
/************************************************************************/
// void ReachHandPickUpAction::reachPreCompleteAction( ReachStateData* rd )
// {			
// 	ReachTarget& rtarget = rd->reachTarget;
// 	SbmPawn* targetPawn = rtarget.getTargetPawn();
// 	if (!targetPawn)
// 		return;
// 	std::string cmd;
// 	std::string charName = rd->charName;
// 	std::string targetName = targetPawn->name;
// 	cmd = "bml char " + charName + " <sbm:grab sbm:handle=\"" + charName + "_gc\" sbm:wrist=\"r_wrist\" sbm:grab-state=\"start\" target=\"" + targetName  + "\"/>";
// 	sendReachEvent(cmd);	
// }

void ReachHandPickUpAction::reachCompleteAction( ReachStateData* rd )
{
	ReachHandAction::reachCompleteAction(rd);	
	pickUpAttachedPawn(rd);
}

// SRT ReachHandPickUpAction::getHandTargetStateOffset( ReachStateData* rd, SRT& naturalState )
// {		
// 	if (rd->reachTarget.getTargetPawn())
// 	{
// 		SRT handState = rd->reachTarget.getGrabTargetState(naturalState);	
// 		return SRT::diff(naturalState,handState);
// 	}	
// 	else
// 		return rd->effectorState.grabStateError;
// }

// bool ReachHandPickUpAction::pickUpNewPawn( ReachStateData* rd )
// {
// 	ReachTarget& rtarget = rd->reachTarget;
// 	EffectorState& estate = rd->effectorState;
// 	if (rd->startReach && rtarget.targetIsPawn())
// 	{
// 		return (estate.attachedPawn != rtarget.getTargetPawn());
// 	}
// 	return false;	
// }

void ReachHandPickUpAction::reachNewTargetAction( ReachStateData* rd )
{
// 	std::string cmd;
// 	std::string charName = rd->charName;
// 	cmd = "bml char " + charName + " <sbm:grab sbm:handle=\"" + charName + "_gc\" sbm:grab-state=\"finish\"/>";
// 	sendReachEvent(cmd);
// 	rd->effectorState.removeAttachedPawn(rd);
	putDownAttachedPawn(rd);
	ReachHandAction::reachNewTargetAction(rd);

}
/************************************************************************/
/* Reach Hand Put-Down Action                                           */
/************************************************************************/
void ReachHandPutDownAction::reachCompleteAction( ReachStateData* rd )
{
	putDownAttachedPawn(rd);
	ReachHandAction::reachNewTargetAction(rd);
// 	std::string cmd;
// 	std::string charName = rd->charName;
// 	cmd = "bml char " + charName + " <sbm:grab sbm:handle=\"" + charName + "_gc\" sbm:grab-state=\"finish\"/>";
// 	sendReachEvent(cmd);
// 	rd->effectorState.removeAttachedPawn(rd);
}

void ReachHandPutDownAction::reachReturnAction( ReachStateData* rd )
{
	std::string cmd;
	std::string charName = rd->charName;
	cmd = "bml char " + charName + " <sbm:grab sbm:handle=\"" + charName + "_gc\" sbm:grab-state=\"return\"/>";
	sendReachEvent(cmd);		
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
	stateTime = 0.f;
	blendWeight = 0.f;
	dt = du = 0.f;
	startReach = endReach = useExample = locomotionComplete = newTarget = hasSteering = false;
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
	//effectorState.updateAttachedPawn();
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

float ReachStateData::XZDistanceToTarget()
{
	SrVec targetXZ = reachTarget.getTargetState().tran; targetXZ.y = 0.f;
	SrVec curXZ = effectorState.curState.tran; curXZ.y = 0.f;
	float dist = (targetXZ - curXZ).norm();
	return dist;
}
/************************************************************************/
/* Reach State Interface                                                */
/************************************************************************/

void ReachStateInterface::updateReturnToIdle( ReachStateData* rd )
{
	// only change effector target state, no change on para blend weight	
	EffectorState& estate = rd->effectorState;	
	if (rd->useInterpolation())
	{
		float rtime = (float)rd->interpMotion->motionDuration(BodyMotionInterface::DURATION_REF)*0.999f;
		BodyMotionFrame targetRefFrame;
		//rd->getInterpFrame(rtime,rd->targetRefFrame);		
		rd->targetRefFrame = rd->idleRefFrame;		
	}	
	estate.targetState = rd->getPoseState(rd->idleRefFrame);
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
	BodyMotionFrame outFrame,interpFrame;
	rd->getInterpFrame(rd->curRefTime,interpFrame);
	MotionExampleSet::blendMotionFrame(interpFrame,rd->idleRefFrame,rd->blendWeight,outFrame);
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
	rd->curRefTime += rd->du;
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
	rd->effectorState.curTargetState = rd->effectorState.targetState;
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
		// test the distance to the target	
#ifdef LOCOMOTION_REACH
		float dist = rd->XZDistanceToTarget();		
		if (dist > rd->characterHeight*0.5f) 
		{	
			// if the target is far away, move the character first
			//printf("idle to walk\n");
			std::string cmd;
			std::string charName = rd->charName;			
			SrVec targetXZ = rd->reachTarget.getTargetState().tran; targetXZ.y = 0.f;
			SrVec curXZ = rd->effectorState.curState.tran; curXZ.y = 0.f;
			SrVec dir = targetXZ - curXZ; dir.normalize();
			float facing = ((float)acos(dot(dir,SrVec(0,0,1))))*180.f/(float)M_PI;
			if (dot(cross(dir,curXZ),SrVec(0,1,0)) < 0.f)
				facing = -facing;
			//LOG("facing = %f\n",facing);
			cmd = "bml char " + charName + " <locomotion target=\"" + boost::lexical_cast<std::string>(targetXZ.x) + " " + 
				   boost::lexical_cast<std::string>(targetXZ.z) +"\"/>"; //"\" facing=\"" + boost::lexical_cast<std::string>(facing) +"\"/>";//"\" proximity=\"" +  boost::lexical_cast<std::string>(rd->characterHeight*0.8f*0.01f) +"\"/>";
			//rd->curHandAction->sendReachEvent(cmd);			
			mcuCBHandle::singleton().execute(const_cast<char*>(cmd.c_str()));
			nextStateName = "Move";
		}
		else // otherwise do the reach directly
#endif
		{
			//printf("idle to start\n");
			rd->reachControl->setFadeIn(0.5f);
			nextStateName = "Start";
		}		
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
	rd->blendWeight = max(0.f,(1.f - curStatePercentTime(rd,rd->curRefTime)*4.f));
}

std::string ReachStateStart::nextState( ReachStateData* rd )
{
	std::string nextStateName = "Start";
	if (interpTargetReached(rd))
	{
		//if (ikTargetReached(rd))
		{
			rd->curHandAction->reachCompleteAction(rd);
			rd->startReach = false;
			nextStateName = "Complete";
		}
// 		else
// 		{
// 			rd->startReach = false;
// 			nextStateName = "Return";
// 		}		
	}
	else if (rd->endReach)
	{
		nextStateName = "Return";
	}
	return nextStateName;
}

/************************************************************************/
/* Reach State Move                                                     */
/************************************************************************/
std::string ReachStateMove::nextState( ReachStateData* rd )
{
	std::string nextStateName = "Move";
	static float prevDist = 0.f;
	float curDist = rd->XZDistanceToTarget();
	
	if (rd->stateTime < 0.5f)
		return nextStateName;
	//if (!rd->locomotionComplete)
	//printf("locomotion = %d, dist = %f\n",rd->locomotionComplete,curDist);
	if (rd->locomotionComplete && fabs(curDist-prevDist) < rd->characterHeight*0.001f)
	{
		if (curDist < rd->characterHeight*0.5f)
		{
			rd->reachControl->setFadeIn(0.5f);
			nextStateName = "Start";
		}
		else
		{
			LOG("Reach Controller State : [MOVE], Can not reach target\n");
			nextStateName = "Idle";
			rd->startReach = false;
		}		
	}
	else if (!rd->hasSteering)
	{
		// whenever there is no steering, just proceed with start
		rd->reachControl->setFadeIn(0.5f);
		nextStateName = "Start";
	}

	prevDist = curDist;
	return nextStateName;
}

void ReachStateMove::update( ReachStateData* rd )
{
	ReachStateInterface::updateReturnToIdle(rd);	
	rd->effectorState.curTargetState = rd->effectorState.targetState;
}

void ReachStateMove::updateEffectorTargetState( ReachStateData* rd )
{
	ReachStateInterface::updateMotionIK(rd);	
}
/************************************************************************/
/* Reach State Complete                                                 */
/************************************************************************/
void ReachStateComplete::updateEffectorTargetState( ReachStateData* rd )
{
	ReachStateInterface::updateReachToTarget(rd);
	if (rd->useInterpolation())
		rd->curRefTime = (float)rd->interpMotion->strokeEmphasisTime();
}

void ReachStateComplete::update( ReachStateData* rd )
{
	ReachStateInterface::updateMotionIK(rd);
	//completeTime += rd->dt;
}

std::string ReachStateComplete::nextState( ReachStateData* rd )
{
	std::string nextStateName = "Complete";
	bool toNextState = rd->autoReturnTime > 0.f ? rd->autoReturnTime < rd->stateTime : rd->endReach;

	if (rd->endReach)
		toNextState = true;
	
	if (toNextState)
	{
		//rd->curHandAction->reachNewTargetAction(rd);
		rd->curHandAction->reachPreReturnAction(rd);
		rd->endReach = false;		
		//completeTime = 0.f; // reset complete time
		nextStateName = "PreReturn";
	}
	else if (rd->curHandAction->isPickingUpNewPawn(rd))
	{
		rd->curHandAction->reachNewTargetAction(rd);
		rd->newTarget = true;
		//completeTime = 0.f;		
		nextStateName = "PreReturn";//"NewTarget";
	}
	return nextStateName;
}
/************************************************************************/
/* Reach State New Target                                               */
/************************************************************************/
void ReachStateNewTarget::updateEffectorTargetState( ReachStateData* rd )
{
	ReachStateInterface::updateReachToTarget(rd);
	if (rd->useInterpolation())
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
	else if (rd->endReach)
	{
		rd->startReach = false;
		nextStateName = "PreReturn";
	}

	return nextStateName;
}

/************************************************************************/
/* Reach State Pre-Return                                               */
/************************************************************************/
std::string ReachStatePreReturn::nextState( ReachStateData* rd )
{
	std::string nextStateName = "PreReturn";
	bool toNextState = rd->stateTime > 0.3;
	if (toNextState)
	{
		completeTime = 0.f; // reset complete time
		if (rd->newTarget)
		{
			rd->newTarget = false;
			nextStateName = "NewTarget";			
		}
		else // return to rest pose
		{
			rd->curHandAction->reachReturnAction(rd);					
			nextStateName = "Return";
		}		
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
	//LOG("curRefTime = %f\n",rd->curRefTime);
	ReachStateInterface::updateMotionInterp(rd);
	rd->blendWeight = curStatePercentTime(rd,rd->curRefTime);	
	//LOG("Blend weight = %f\n",rd->blendWeight);
}

std::string ReachStateReturn::nextState( ReachStateData* rd )
{
	std::string nextStateName = "Return";
	if (interpTargetReached(rd))
	{
		rd->startReach = false;
		rd->endReach = false;
		rd->reachControl->setFadeOut(0.5);
		rd->blendWeight = 0.f;
		nextStateName = "Idle";		
	}
	return nextStateName;
}

