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

using namespace boost;



EffectorConstantConstraint& EffectorConstantConstraint::operator=( const EffectorConstantConstraint& rhs )
{
	efffectorName = rhs.efffectorName;
	rootName    = rhs.rootName;	
	targetPos = rhs.targetPos;//SrQuat(SrVec(0,1,0),M_PI);
	targetRot = rhs.targetRot;
	return *this;

}

/************************************************************************/
/* Exampled-Based Reach Controller                                      */
/************************************************************************/

const char* MeCtExampleBodyReach::CONTROLLER_TYPE = "BodyReach";

#define USE_FOOT_IK 0
#define AVOID_OBSTACLE 0
#define AVOID_OBSTACLE_BLEND 0

const std::string lFootName[] = {"l_forefoot", "l_ankle" };
const std::string rFootName[] = {"r_forefoot", "r_ankle" };

MeCtExampleBodyReach::MeCtExampleBodyReach(std::string charName, SkSkeleton* sk, SkJoint* effectorJoint )
{	
	// here we create a copy of skeleton as an intermediate structure.
	// this will make it much easier to grab a key-frame from a SkMotion.
	// we use the "copy" of original skeleton to avoid corrupting the channel data by these internal operations.
	characterName = charName;
	skeletonCopy = new SkSkeleton(sk); 
	skeletonRef  = sk;
	prev_time = -1.0;
	dataInterpolator = NULL;
	testDataInterpolator =NULL;

	refMotion = NULL;
	_duration = -1.0f;
	reachTime = 0.0;	
	useIKConstraint = true;
	useInterpolation = false;
	useTargetPawn   = true;

	
	interactiveReach = false;
	startReaching = false;
	finishReaching = false;
	sendGrabMsg = false;
	reachEndEffector = effectorJoint;
	//skeletonRef->search_joint(endEffectorName);	

	curReachState = REACH_START;
	curGrabState  = TOUCH_OBJECT;
	//reachTargetJoint = NULL;
	reachTargetPawn = NULL;
	attachedPawn = NULL;
	interpMotion = NULL;
	motionParameter = NULL;

	reachTarget = SrVec();
	reachVelocity = 50.f;
	reachCompleteDuration = 0.6f;
	curPercentTime = 0.0;
	prevPercentTime = 0.0;

	simplexIndex = 0;
	obstacleScale = 7.f;
	posPlanner = NULL;
	blendPlanner = NULL;
}

MeCtExampleBodyReach::~MeCtExampleBodyReach( void )
{
	#define FREE_DATA(data) if (data) delete data; data=NULL;
	FREE_DATA(dataInterpolator);
	FREE_DATA(interpMotion);
	FREE_DATA(motionParameter);
	FREE_DATA(skeletonCopy);	
}

void MeCtExampleBodyReach::setReachTargetPawn( SbmPawn* targetPawn )
{
	reachTargetPawn = targetPawn;	
	useTargetPawn = true;	
	startReaching = true;	
}

void MeCtExampleBodyReach::setReachTargetPos( SrVec& targetPos )
{
	reachTargetPos = targetPos;
	useTargetPawn = false;
	startReaching = true;
}

bool MeCtExampleBodyReach::addHandConstraint( SkJoint* targetJoint, const char* effectorName )
{
	MeCtIKTreeNode* node = ikScenario.findIKTreeNode(effectorName);
	if (!node)
		return false;

	std::string str = effectorName;		
	ConstraintMap::iterator ci = handConstraint.find(str);
	if (ci != handConstraint.end())//idx != effectorList.size())
	{
		//jEffectorList[idx].targetJoint = targetJoint;	
		//EffectorJointConstraint& cons = jEffectorList[idx];
		EffectorJointConstraint* cons = dynamic_cast<EffectorJointConstraint*>((*ci).second);
		cons->targetJoint = targetJoint;
	}
	else // add effector-joint pair
	{
		// initialize constraint
		EffectorJointConstraint* cons = new EffectorJointConstraint();		
		cons->efffectorName = effectorName;
		cons->targetJoint = targetJoint;
		handConstraint[str] = cons;		
	}
	return true;
}


void MeCtExampleBodyReach::findReachTarget( SrVec& rTarget, SrVec& rError, double time )
{
	//if (reachTargetJoint)
	SrVec offset = SrVec(0.07f,0.06f,0.f);
	{
		//if (reachTargetJoint)
		//	reachTargetJoint->update_gmat_up();
		if (reachTargetPawn)
		{
			SkJoint* root = const_cast<SkJoint*>(reachTargetPawn->get_world_offset_joint());
			root->update_gmat_up();
			//reachTargetPawn->get_world_offset_joint();
		}

		SrVec newReachTarget = (reachTargetPawn && useTargetPawn) ? reachTargetPawn->get_world_offset_joint()->gmat().get_translation() : reachTargetPos;
		//newReachTarget += offset*reachEndEffector->gmat().get_rotation();
		//if ( (newReachTarget - reachTarget).norm() > 0.001 ) // interrupt and reset reach state if the reach target is moved
		bool changeReachTarget = (newReachTarget - reachTarget).norm() > 0.001 ;
		if (changeReachTarget || startReaching)
		{				
			if (curReachState == REACH_IDLE)// && startReaching)
			{
				curReachState = REACH_START;
				curReachIKOffset = SrVec();
				ikOffset = SrVec();
				reachTime = 0.f;	
				sendGrabMsg = false;
			}				
			reachTarget = newReachTarget;				
			startReaching = false;
		}

		if (curReachState == REACH_START || curReachState == REACH_COMPLETE)
		{
			rTarget = reachTarget;		
		}
		else if (curReachState == REACH_RETURN || curReachState == REACH_IDLE)
		{
			if (interpMotion && useInterpolation)
			{
				dVector initPos;
				double refDuration = interpMotion->motionDuration(BodyMotionInterface::DURATION_REF);
				interpMotion->getMotionFrame(0.f,skeletonCopy,affectedJoints,initMotionFrame);
				motionParameter->getMotionFrameParameter(interpMotion,(float)refDuration*0.999f,initPos);
				for (int i=0;i<3;i++)
					returnTarget[i] = (float)initPos[i];						
			}
			else
			{
				returnTarget = getCurrentHandPos(idleMotionFrame);
			}
 			rTarget = returnTarget;
		}

		// determine the error offset between interpolation result & actual target
		skeletonRef->update_global_matrices();

		const char* rootName = ikScenario.ikTreeRoot->joint->parent()->name().get_string();
		SrMat gmat = skeletonRef->root()->gmat();//skeletonRef->search_joint(rootName)->gmat();
		SrVec localTarget = rTarget*gmat.inverse();
		dVector para; para.resize(3);
		for (int i=0;i<3;i++)
			para[i] = localTarget[i];		
		currentInterpTarget = reachEndEffector->gmat().get_translation();

		ikRotTarget = SrQuat(0,0,0,0);
		if (dataInterpolator && interpMotion && useInterpolation)
		{			
			dVector acutualReachTarget;

			//simplexIndex = testDataInterpolator->getPointSimplexIndex(para);

			if (curReachState == REACH_START || curReachState == REACH_COMPLETE)
			{
				// compute the new interpolation weight based on current target
				dataInterpolator->predictInterpWeights(para,interpMotion->weight);	
				double refTime = interpMotion->strokeEmphasisTime();											
				motionParameter->getMotionFrameParameter(interpMotion,(float)refTime,acutualReachTarget);	
				
				if ( curGrabState == PICK_UP_OBJECT && reachTargetPawn && reachTargetPawn->colObj_p)
				{
					BodyMotionFrame interpFrame;
					interpMotion->getMotionFrame((float)refTime,skeletonCopy,affectedJoints,interpFrame);
					SkJoint* handJoint = motionParameter->getMotionFrameJoint(interpFrame,reachEndEffector->name().get_string());
					SkJoint* handParentJoint = motionParameter->getMotionFrameJoint(interpFrame,reachEndEffector->parent()->name().get_string());
					SkJoint* handChildJoint = motionParameter->getMotionFrameJoint(interpFrame,reachEndEffector->child(0)->name().get_string());

					SrQuat naturalRot = SrQuat(handJoint->gmat());
					SrQuat parentChild = SrQuat(handParentJoint->gmat());//slerp(SrQuat(handParentJoint->gmat()),SrQuat(handChildJoint->gmat()),0.5f);
					SrVec newTarget;					
					reachTargetPawn->colObj_p->estimateHandPosture(parentChild,newTarget,ikRotTarget);
					ikOffset = newTarget - rTarget;
					ikRotError = naturalRot.inverse()*ikRotTarget;
				}
			}
			else if (curReachState == REACH_RETURN || curReachState == REACH_IDLE)				
			{	
				// when generating the return motion, use the same weight for reaching.
				double refDuration = interpMotion->motionDuration(BodyMotionInterface::DURATION_REF)*0.999;			
				motionParameter->getMotionFrameParameter(interpMotion,(float)refDuration,acutualReachTarget);	
				ikOffset = SrVec(0,0,0);
// 				BodyMotionFrame interpFrame;
// 				interpMotion->getMotionFrame(refDuration,skeletonCopy,affectedJoints,interpFrame);
// 				SkJoint* handJoint = motionParameter->getMotionFrameJoint(interpFrame,reachEndEffector->name().get_string());				
// 				ikRotTarget = SrQuat(handJoint->gmat());
				//ikRotError = ;
			}
			double refDuration = interpMotion->motionDuration(BodyMotionInterface::DURATION_REF);	
			interpMotion->getMotionFrame((float)refDuration*0.999f,skeletonCopy,affectedJoints,idleMotionFrame);
			for (int i=0;i<3;i++)
				currentInterpTarget[i] = (float)acutualReachTarget[i];		
		}			
		rError = rTarget + ikOffset - currentInterpTarget;	

		//sr_out << "rError = " << rError << srnl;
	}		
}

void MeCtExampleBodyReach::updateIK( SrVec& rTrajectory, BodyMotionFrame& refFrame, BodyMotionFrame& outFrame )
{
	static bool bInit = false;
	if (!bInit)
	{			
		ikScenario.setTreeNodeQuat(refFrame.jointQuat,QUAT_INIT);
		ikScenario.setTreeNodeQuat(refFrame.jointQuat,QUAT_CUR);	
		bInit = true;
	}		
	
	EffectorConstantConstraint* cons = dynamic_cast<EffectorConstantConstraint*>(reachPosConstraint[reachEndEffector->name().get_string()]);
	cons->targetPos = rTrajectory;	
	
	skeletonRef->invalidate_global_matrices();
	skeletonRef->update_global_matrices();
	const char* rootName = ikScenario.ikTreeRoot->joint->parent()->name().get_string();
	ikScenario.ikGlobalMat = skeletonRef->search_joint(rootName)->gmat();//ikScenario.ikTreeRoot->joint->parent()->gmat();	
	ikScenario.ikTreeRootPos = refFrame.rootPos;
	ikScenario.setTreeNodeQuat(refFrame.jointQuat,QUAT_REF);		
	//ikScenario.setTreeNodeQuat(idleMotionFrame.jointQuat,QUAT_REF);	

	ikScenario.ikPosEffectors = &reachPosConstraint;

// 	if (ikRotTarget == SrQuat(0,0,0,0))
// 	{
// 		ikScenario.ikRotEffectors = &reachNoRotConstraint;
// 	}
// 	else
	{
		EffectorConstantConstraint* cons = dynamic_cast<EffectorConstantConstraint*>(reachRotConstraint[reachEndEffector->name().get_string()]);		
		cons->targetRot = ikRotTrajectory;//ikRotTarget;//motionParameter->getMotionFrameJoint(interpMotionFrame,reachEndEffector->name().get_string())->gmat();//ikRotTarget;	
		//ikScenario.ikRotEffectors = &reachRotConstraint;
		ikScenario.ikRotEffectors = &reachNoRotConstraint;
	}


	ik.maxOffset = ikMaxOffset;
	ik.dampJ = ikDamp;
	ik.refDampRatio = 0.1;
 	for (int i=0;i<2;i++)
 	{
 		ik.update(&ikScenario);		
 		ikScenario.copyTreeNodeQuat(QUAT_CUR,QUAT_INIT);		
 	}

#if USE_FOOT_IK
	for (int i=0;i<2;i++)
	{
		EffectorConstantConstraint* lfoot = dynamic_cast<EffectorConstantConstraint*>(leftFootConstraint[lFootName[i]]);
		lfoot->targetPos = motionParameter->getMotionFrameJoint(idleMotionFrame,lFootName[i].c_str())->gmat().get_translation();
		EffectorConstantConstraint* rfoot = dynamic_cast<EffectorConstantConstraint*>(rightFootConstraint[rFootName[i]]);
		rfoot->targetPos = motionParameter->getMotionFrameJoint(idleMotionFrame,rFootName[i].c_str())->gmat().get_translation();	
	} 			
	ikScenario.ikPosEffectors = &leftFootConstraint;
	ikCCD.update(&ikScenario);
 	ikScenario.ikPosEffectors = &rightFootConstraint;
 	ikCCD.update(&ikScenario);	
	ikScenario.copyTreeNodeQuat(QUAT_CUR,QUAT_INIT);
#endif
	ikScenario.getTreeNodeQuat(outFrame.jointQuat,QUAT_CUR); 	
}

bool MeCtExampleBodyReach::updateInterpolation(float time, float dt, BodyMotionFrame& outFrame, float& du)
{
	bool interpHasReach = false;		
	interpMotion->getMotionFrame(reachTime,skeletonCopy,affectedJoints,interpMotionFrame);		
	{			
		double strokTime = interpMotion->strokeEmphasisTime();
		double refDuration = interpMotion->motionDuration(BodyMotionInterface::DURATION_REF);

		// set ik rot target :
		if (curReachState == REACH_START)
		{
			curPercentTime = reachTime/strokTime;	
			// start morph hand to reach pose
			
			if (!attachedPawn && reachTargetPawn && curGrabState == PICK_UP_OBJECT && curPercentTime > 0.8 && !sendGrabMsg)
			{
				sendGrabMsg = true;
				std::string eventType = "reach";		
				MotionEvent motionEvent;
				motionEvent.setType(eventType);		
				std::string cmd;
				std::string targetName = reachTargetPawn->name;
				cmd = "bml char " + characterName + " <sbm:grab sbm:handle=\"" + characterName + "_gc\" sbm:wrist=\"r_wrist\" sbm:grab-state=\"start\" target=\"" + targetName  + "\"/>";
				cout << "reach target cmd = " << cmd << endl;
				motionEvent.setParameters(cmd);
				EventManager* manager = EventManager::getEventManager();		
				manager->handleEvent(&motionEvent,time);	
			}	
		}
		else if (curReachState == REACH_RETURN)
		{
			curPercentTime = (reachTime-strokTime)/(refDuration-strokTime);
		}

		double deltaPercent = fabs(curPercentTime-prevPercentTime);
		double timeRemain = 1.0 - curPercentTime;			

		if ((curReachState == REACH_START && strokTime <= reachTime)
			|| (curReachState == REACH_RETURN && refDuration <= reachTime) 
			|| curReachState == REACH_COMPLETE || curReachState == REACH_IDLE )
			interpHasReach = true;

		double ratio = 0.0;			
		if (timeRemain > 0.f)
			ratio = 1.0/timeRemain;
		interpPos = getCurrentHandPos(interpMotionFrame);

		SkJoint* handJoint = motionParameter->getMotionFrameJoint(interpMotionFrame,reachEndEffector->name().get_string());		
		SrQuat interpRot = SrQuat(handJoint->gmat());

		if (curReachState == REACH_START)
		{
			ikRotTrajectory = interpRot*slerp(SrQuat(),ikRotError,(float)curPercentTime);
		}
		else if (curReachState == REACH_RETURN)
		{
			ikRotTrajectory = interpRot*slerp(ikRotError,SrQuat(),(float)curPercentTime);
		}

		if ( curReachState == REACH_COMPLETE )
			// use normal IK to compute the hand movement after touching the object
		{			
			float offsetLength = (getIKTarget() - curReachIKTrajectory).norm();
			float reachStep = reachVelocity*dt;
			SrVec offset;

#if AVOID_OBSTACLE
			static float interpLen = 0.f;
			bool pathUpdate = updatePlannerPath(curReachIKTrajectory,getIKTarget());
			SkPosPath* path = posPlanner->path();
			SrVec newPos = curReachIKTrajectory;
			float pathStep = planStep;
			if (pathUpdate)
				interpLen = 0.f;

			if (path->len() > reachStep && interpLen < path->len())
			{
				interpLen += reachStep;
				//printf("long path\n");				
				path->interp(interpLen,cfgPath);
				newPos = *cfgPath;
			}	
			else if (offsetLength < reachStep)
			{
				newPos = getIKTarget();
			}
			offset = newPos - curReachIKTrajectory;
#endif		

#if AVOID_OBSTACLE_BLEND
			static float interpLen = 0.f;

			VecOfInterpWeight curweight;
			float refTime;
			if (cfgBlendPath->weight.size() == 0)
			{
				curweight = interpMotion->weight;
				refTime = reachTime;
			}
			else
			{
				curweight = cfgBlendPath->weight;
				refTime = cfgBlendPath->refTime;
			}

			bool pathUpdate = updatePlannerBlendPath(curweight,refTime,interpMotion->weight,reachTime);
			SkBlendPath* path = blendPlanner->path();
			SrVec newPos = curReachIKTrajectory;
			float pathStep = planStep;
			if (pathUpdate)
				interpLen = 0.f;

			
			interpLen += reachStep;
			if (interpLen > path->len())
				interpLen = path->len();

			//if (path->len() > 0.f)
			{
				path->interp(interpLen,cfgBlendPath);	
				cfgBlendPath->apply();
				interpStartFrame = cfgBlendPath->blendPose;
				interpMotionFrame = cfgBlendPath->blendPose;
			}
#else		
			offset = (getIKTarget() - curReachIKTrajectory);			
			curOffsetDir = offset;					
			if (offset.norm() > reachStep)
			{
				offset.normalize();
				offset = offset*reachStep;
			}
			float morphWeight = offsetLength > 0.f ? (offset.norm()+reachVelocity*dt)/(offsetLength+reachVelocity*dt) : 1.f;
			BodyMotionFrame morphFrame;				
			MotionExampleSet::blendMotionFrame(interpStartFrame,interpMotionFrame,morphWeight,morphFrame);				
			curReachIKOffset = (getIKTarget() - interpPos);
			curReachIKTrajectory = curReachIKTrajectory + offset;//curEffectorPos + offset;
			interpStartFrame = morphFrame;
			interpMotionFrame = morphFrame;	
#endif
						
		}
		else
		{
			curReachIKOffset += (reachError - curReachIKOffset)*(float)ratio*(float)deltaPercent;

			float normOffset = curReachIKOffset.norm();
			curReachIKTrajectory = interpPos + curReachIKOffset;								
		}		
	    interpPos = getCurrentHandPos(interpMotionFrame);
	}		
	du = (float)interpMotion->getRefDeltaTime(reachTime,dt);	
	outFrame = interpMotionFrame;

	return interpHasReach;
}

bool MeCtExampleBodyReach::controller_evaluate( double t, MeFrameData& frame )
{
	//mcuCBHandle::singleton().mark("main",0,"A");
	//LOG("example reach running\n");
	float dt = 0.001f;
	float du = 0.0;
	if (prev_time == -1.0) // first start
	{
		dt = 0.001f;		
		// for first frame, update from frame buffer to joint quat in the limb
		// any future IK solving will simply use the joint quat from the previous frame.
		updateChannelBuffer(frame,idleMotionFrame,true);
		initMotionFrame = idleMotionFrame;
		curEffectorPos = getCurrentHandPos(idleMotionFrame);			
		curReachState = REACH_IDLE;
		//reachTarget = (reachTargetJoint && useTargetJoint) ? reachTargetJoint->gmat().get_translation() : reachTargetPos;;
	}
	else
	{		
		dt = ((float)(t-prev_time));
	}
	prev_time = (float)t;	

	updateChannelBuffer(frame,inputMotionFrame,true);

	// To-Do (Wei-Wen) : these run-time steps seem more convoluted than it should be. 
	// For the things we want to achieve ( moving the hand to the target, stay there for some time, then return to the original location )
	// there seems to be better and more compact ways to handle them. For now I just separated the steps into some ugly functions, but later I should 
	// re-organize these parts ( maybe as a state machine ? ) for better code maintenance in the future.

	if (attachedPawn)
	{
		SrMat effectorWorld = motionParameter->getMotionFrameJoint(ikMotionFrame,reachEndEffector->name().get_string())->gmat();
		SrMat newWorld = pawnAttachMat*effectorWorld;
		attachedPawn->setWorldOffset(newWorld);	
		//reachTargetPawn = NULL;
	}

	updateSkeletonCopy();	
	findReachTarget(ikTarget,reachError,t);

	bool interpHasReach = false;
	if (interpMotion && useInterpolation)
	{	
		interpHasReach = updateInterpolation((float)t,dt,interpMotionFrame,du);
		ikMotionFrame = interpMotionFrame;
	}	
	else // we don't have any data, so just infer the hand trajectory directly
	{
		 // assuming some constant hand moving speed
		SrVec reachDir = getIKTarget() - curEffectorPos;	
		{
			if (reachDir.norm() > reachVelocity*dt)
			{
				reachDir.normalize();
				reachDir = reachDir*reachVelocity*dt;
			}		
			curReachIKTrajectory = curEffectorPos + reachDir;
		}			
		interpMotionFrame = idleMotionFrame;
		ikMotionFrame = interpMotionFrame;
	}		
	
	// hand position from reference pose
	interpPos = getCurrentHandPos(interpMotionFrame);
		
	// update interpolated motion using IK constraint
	if (useIKConstraint)
	{
		ikMaxOffset = reachVelocity*dt;
		updateIK(curReachIKTrajectory,interpMotionFrame,ikMotionFrame);					
	}	

	//ikFootTarget = leftFootConstraint[lFootName]->getPosConstraint();//ikScenario.ikTreeRoot->joint->parent()->gmat().get_translation();//gmat.get_translation();

	// hand position after solving IK
	curEffectorPos = getCurrentHandPos(ikMotionFrame);

	std::map<std::string, SbmColObject*>::iterator mi;
	for ( mi  = jointColliderMap.begin();
		  mi != jointColliderMap.end();
		  mi++)
	{
		SbmColObject* col = mi->second;
		std::string jointName = mi->first;
		col->updateTransform(motionParameter->getMotionFrameJoint(ikMotionFrame,jointName.c_str())->gmat());
	}
		

	bool ikHasReach = (useInterpolation && dataInterpolator) ? false : (curEffectorPos - getIKTarget()).norm() < ikReachRegion;
	if ( ikHasReach || interpHasReach || finishReaching )
	{
		updateState(t);		
	}	

	prevPercentTime = curPercentTime;	
	reachCompleteTime += dt;
	if (curReachState == REACH_RETURN || curReachState == REACH_START)
		reachTime += du*0.5f; // add the reference delta time

	// blending the input frame with ikFrame based on current fading
	bool finishFadeOut = updateFading(dt);
	BodyMotionFrame outMotionFrame;
	MotionExampleSet::blendMotionFrame(inputMotionFrame,ikMotionFrame,blendWeight,outMotionFrame);
	//outMotionFrame = ikMotionFrame;

	ConstraintMap::iterator si;
	for ( si  = handConstraint.begin();
		  si != handConstraint.end();
		  si++)
	{	
		EffectorJointConstraint* cons = dynamic_cast<EffectorJointConstraint*>(si->second);//rotConstraint[i];
		SrVec targetPos = motionParameter->getMotionFrameJoint(outMotionFrame,cons->efffectorName.c_str())->gmat().get_translation();
		for (int k=0;k<3;k++)
			cons->targetJoint->pos()->value(k,targetPos[k]);
		//cons->efffectorName		
		cons->targetJoint->update_gmat();
	}

	updateChannelBuffer(frame,outMotionFrame);
	//mcuCBHandle::singleton().mark("main",0,"B");
	return true;
}



void MeCtExampleBodyReach::updateState(double time)
{	
	bool reachIsFinish = interactiveReach ? reachCompleteTime >= reachCompleteDuration : finishReaching;
	if (curReachState == REACH_START)
	{
		// after touch the object, stay there for a pre-defined duration
		// the hand can still move around during this period
		if (!attachedPawn && reachTargetPawn && curGrabState == PICK_UP_OBJECT)
		{
			std::string eventType = "reach";		
			MotionEvent motionEvent;
			motionEvent.setType(eventType);		
			std::string cmd;
			std::string targetName = reachTargetPawn->name;
			cmd = "bml char " + characterName + " <sbm:grab sbm:handle=\"" + characterName + "_gc\" sbm:wrist=\"r_wrist\" sbm:grab-state=\"reach\" target=\"" + targetName  + "\"/>";
			cout << "reach target cmd = " << cmd << endl;
			motionEvent.setParameters(cmd);
			EventManager* manager = EventManager::getEventManager();		
			manager->handleEvent(&motionEvent,time);	

			// object attachedment
			attachedPawn = reachTargetPawn;
			SrMat pawnWorld = attachedPawn->get_world_offset_joint()->gmat();
			SrMat effectorWorld = motionParameter->getMotionFrameJoint(ikMotionFrame,reachEndEffector->name().get_string())->gmat();
			pawnAttachMat = pawnWorld*effectorWorld.inverse();
			reachTargetPawn = NULL;
			reachTargetPos = ikTarget;//effectorWorld.get_translation();	
		}	
		else if (attachedPawn && curGrabState == PUT_DOWN_OBJECT)
		{
			std::string eventType = "reach";		
			MotionEvent motionEvent;
			motionEvent.setType(eventType);		
			std::string cmd;
			cmd = "bml char " + characterName + " <sbm:grab sbm:handle=\"" + characterName + "_gc\" sbm:grab-state=\"finish\"/>";
			cout << "reach finish cmd = " << cmd << endl;
			//printf("reach finish cmd = %s\n",cmd);
			motionEvent.setParameters(cmd);
			EventManager* manager = EventManager::getEventManager();		
			manager->handleEvent(&motionEvent,time);

			attachedPawn = NULL;
			pawnAttachMat = SrMat();
		}

		curReachState = REACH_COMPLETE;
		interpStartFrame = interpMotionFrame;
		reachCompleteTime = 0.0;
		//reachTargetPos = curEffectorPos;
		//useTargetJoint = false;
		finishReaching = false;
	}
	else if (curReachState == REACH_COMPLETE && reachIsFinish)//reachCompleteTime >= reachCompleteDuration)
	{		
		curReachState = REACH_RETURN;	
		curPercentTime = 0.0;			
		finishReaching = false;

		if (curGrabState == PUT_DOWN_OBJECT)
		{
			std::string eventType = "reach";		
			MotionEvent motionEvent;
			motionEvent.setType(eventType);		
			std::string cmd;
			cmd = "bml char " + characterName + " <sbm:grab sbm:handle=\"" + characterName + "_gc\" sbm:grab-state=\"return\"/>";
			cout << "reach finish cmd = " << cmd << endl;
			//printf("reach finish cmd = %s\n",cmd);
			motionEvent.setParameters(cmd);
			EventManager* manager = EventManager::getEventManager();		
			manager->handleEvent(&motionEvent,time);

			attachedPawn = NULL;
			pawnAttachMat = SrMat();
		}
	}
	else if (curReachState == REACH_RETURN)
	{
		// stay idle in the current place
		//reachTime = 0.0;
		curPercentTime = 0.0;			
		curReachState = REACH_IDLE;
	}
	else if (curReachState == REACH_IDLE)
	{
		// reset reachTime to zero
		reachTime = 0.0;
	}
}



SrVec MeCtExampleBodyReach::getCurrentHandPos( BodyMotionFrame& motionFrame )
{
	SrVec handPos;
	dVector curReachPara;
	motionParameter->getPoseParameter(motionFrame,curReachPara);
	for (int i=0;i<3;i++)
		handPos[i] = (float)curReachPara[i];
	return handPos;
}

void MeCtExampleBodyReach::setEndEffectorRoot( const char* rootName )
{
	EffectorConstantConstraint* cons = dynamic_cast<EffectorConstantConstraint*>(reachPosConstraint[reachEndEffector->name().get_string()]);
	if (skeletonRef->search_joint(rootName) != NULL)
		cons->rootName = rootName;
}


SkJoint* MeCtExampleBodyReach::findRootJoint( SkSkeleton* sk )
{
	SkJoint* rootJoint = sk->root()->child(0); // skip world offset
	bool bStop = false;
	while (!bStop)
	{
		SkJoint* child = rootJoint->child(0);
		SkJointPos* skRootPos = rootJoint->pos();
		SkJointPos* skPos = child->pos();
		bool rootFrozen = (skRootPos->frozen(0) && skRootPos->frozen(1) && skRootPos->frozen(2));
		bool childFrozen = (skPos->frozen(0) && skPos->frozen(1) && skPos->frozen(2));
		if (childFrozen && !rootFrozen)
		{
			bStop = true;
		}
		else
		{
			rootJoint = child;
		}
	}
	return rootJoint;
}

void MeCtExampleBodyReach::init()
{
	assert(skeletonRef);	
	// root is "world_offset", so we use root->child to get the base joint.
	SkJoint* rootJoint = findRootJoint(skeletonCopy);//findRootJoint(skeletonRef);//skeletonRef->root()->child(0);//skeletonCopy->root()->child(0);//skeletonRef->root()->child(0);	
	ikScenario.buildIKTreeFromJointRoot(rootJoint);
	ikCCDScenario.buildIKTreeFromJointRoot(rootJoint);
	

	EffectorConstantConstraint* cons = new EffectorConstantConstraint();
	cons->efffectorName = reachEndEffector->name().get_string();
	cons->rootName = "r_sternoclavicular";//"r_shoulder";//rootJoint->name().get_string();		
	reachPosConstraint[cons->efffectorName] = cons;

	// if there is a child
	
	if (reachEndEffector->child(0))
	{
		EffectorConstantConstraint* rotCons = new EffectorConstantConstraint();				
		rotCons->efffectorName = reachEndEffector->name().get_string();//->child(0)->name().get_string();
		rotCons->rootName = "r_sternoclavicular";//"r_shoulder";//rootJoint->name().get_string();		
		reachRotConstraint[cons->efffectorName] = rotCons;
	}	
	// setup foot constraint
#if USE_FOOT_IK
	for (int i=0;i<2;i++)
	{
		EffectorConstantConstraint* lFoot = new EffectorConstantConstraint();
		lFoot->efffectorName = lFootName[i];
		lFoot->rootName = "";
		leftFootConstraint[lFoot->efffectorName] = lFoot;

		EffectorConstantConstraint* rFoot = new EffectorConstantConstraint();
		rFoot->efffectorName = rFootName[i];
		rFoot->rootName = "";
		rightFootConstraint[rFoot->efffectorName] = rFoot;
	}	
#endif	
	
// 	reachPosConstraint[lFoot->efffectorName] = lFoot;
// 	reachPosConstraint[rFoot->efffectorName] = rFoot;

	ikScenario.ikPosEffectors = &reachPosConstraint;
	ikScenario.ikRotEffectors = &reachRotConstraint;
	
	const IKTreeNodeList& nodeList = ikScenario.ikTreeNodes;	
	idleMotionFrame.jointQuat.resize(nodeList.size());
	inputMotionFrame.jointQuat.resize(nodeList.size());

	for (int i=0;i<3;i++)
		_channels.add(rootJoint->name().get_string(), (SkChannel::Type)(SkChannel::XPos+i));
	
	affectedJoints.clear();	
	for (unsigned int i=0;i<nodeList.size();i++)
	{
		MeCtIKTreeNode* node = nodeList[i];
		SkJoint* joint = skeletonCopy->linear_search_joint(node->nodeName.c_str());
		SkJointQuat* skQuat = joint->quat();		
		affectedJoints.push_back(joint);	
		_channels.add(joint->name().get_string(), SkChannel::Quat);		
	}		

	SkJoint* copyEffector = skeletonCopy->linear_search_joint(reachEndEffector->name().get_string());
	motionParameter = new ReachMotionParameter(skeletonCopy,affectedJoints,copyEffector);
	motionExamples.initMotionExampleSet(motionParameter);	


	// initialize all parameters according to scale	
	ikReachRegion = characterHeight*0.02f;	
	reachVelocity = characterHeight*0.3f;
	ikDamp        = ikReachRegion*ikReachRegion*14.0;//characterHeight*0.1f;

	// initialize position planning
#if AVOID_OBSTACLE
	posPlanner = new SkPosPlanner();
	planStep = reachVelocity*0.1f;
	planNumTries = 1;
	planError = ikReachRegion;
	obstacleScale = characterHeight/30.f*1.2;
#endif

#if AVOID_OBSTACLE_BLEND
	blendPlanner = new SkBlendPlanner();
	planStep = reachVelocity*0.3f;
	planNumTries = 1;
	planError = ikReachRegion;
#endif

	MeController::init();	
}

void MeCtExampleBodyReach::updateMotionExamples( const MotionDataSet& inMotionSet )
{	
	if (inMotionSet.size() == 0)
		return;

	// set world offset to zero
	
	
	const char* rootName = ikScenario.ikTreeRoot->joint->parent()->name().get_string();
	SkJoint* root = skeletonRef->search_joint(rootName);
	if (root)
	{
		root->quat()->value(SrQuat());
	}
	skeletonCopy->root()->quat()->value(SrQuat());
	for (int i=0;i<3;i++)
	{
		skeletonCopy->root()->pos()->value(i,0.f);
		root->pos()->value(i,0.f);
	}


	BOOST_FOREACH(SkMotion* motion, inMotionSet)
	{
		if (motionData.find(motion) != motionData.end())
			continue; // we do not process example motions that are already used for this controller instance
		if (!refMotion)
			refMotion = motion;
		
		motionData.insert(motion);
		MotionExample* ex = new MotionExample();
		ex->motion = motion;
		ex->timeWarp = new SimpleTimeWarp(refMotion->duration(),motion->duration());
		ex->motionParameterFunc = motionParameter;
		ex->getMotionParameter(ex->parameter);		
		// set initial index & weight for the motion example
		// by default, the index should be this motion & weight should be 1
		InterpWeight w;
		w.first = motionExamples.getExamples().size();
		w.second = 1.f;
		ex->weight.push_back(w);		

		// add the example parameter for visualization purpose
		SrVec reachPos;
		for (int i=0;i<3;i++)
			reachPos[i] = (float)ex->parameter[i];
		examplePts.push_back(reachPos);		
		motionExamples.addMotionExample(ex);
	}	

	if (dataInterpolator)
		delete dataInterpolator;

	dataInterpolator = createInterpolator();
	dataInterpolator->init(&motionExamples);
	dataInterpolator->buildInterpolator();	

	
	//testDataInterpolator = new BarycentricInterpolator();
	//testDataInterpolator->init(&motionExamples);
	//testDataInterpolator->buildInterpolator();

	// test barycentric interpolator
	//dataInterpolator = testDataInterpolator;
	//simplexList = testDataInterpolator->simplexList;	
	
	for (unsigned int i=0;i<resampleData->size();i++)
	{
		InterpolationExample* ex = (*resampleData)[i];
		SrVec reachPos;
		for (int k=0;k<3;k++)
			reachPos[k] = (float)ex->parameter[k];
		resamplePts.push_back(reachPos);
		paraBound.extend(reachPos);
	}

	if (interpMotion)
		delete interpMotion;
	interpMotion = createInterpMotion();
	blendPlannerMotion = createInterpMotion();

	// initialize the interpolation weights
	dVector para; para.resize(3);
	for (int i=0;i<3;i++)
		para[i] = curEffectorPos[i];
	if (interpMotion && dataInterpolator)
		dataInterpolator->predictInterpWeights(para,interpMotion->weight);

	// update example set
#if AVOID_OBSTACLE
	sr_out << "para bound = " << paraBound << srnl;
	posPlanner->init(paraBound);
	cfgStart = posPlanner->cman()->alloc();
	cfgEnd   = posPlanner->cman()->alloc();
	cfgPath  = posPlanner->cman()->alloc();
#endif

#if AVOID_OBSTACLE_BLEND
	printf("init blend planner\n");
	blendPlanner->init(motionParameter,blendPlannerMotion,refMotion->duration());
	cfgBlendStart = blendPlanner->cman()->alloc();
	cfgBlendEnd   = blendPlanner->cman()->alloc();
	cfgBlendPath  = blendPlanner->cman()->alloc();

	// hard coded joint collider
	const std::string elbowName = "r_elbow";
	SkJoint* elbow = skeletonCopy->search_joint(elbowName.c_str());
	SbmColCapsule* cap = new SbmColCapsule(SrVec(0,0,0),elbow->child(0)->offset()+elbow->child(0)->child(0)->offset(),0.02f);
	jointColliderMap[elbowName] = cap;
	CollisionJoint colJoint;
	colJoint.joint = elbow;
	colJoint.colGeo = cap;
	blendPlanner->cman()->colJoints.push_back(colJoint);

	const std::string fingerName = "r_middle1";
	SkJoint* middle = skeletonCopy->search_joint(fingerName.c_str());
	SbmColSphere* sph = new SbmColSphere(0.06f);
	jointColliderMap[fingerName] = sph;
	CollisionJoint colJointFinger;
	colJointFinger.joint = middle;
	colJointFinger.colGeo = sph;
	blendPlanner->cman()->colJoints.push_back(colJointFinger);
#endif

// 	if (curReachState != REACH_IDLE)
// 		curReachState = REACH_IDLE;
	curReachState = REACH_RETURN;
	finishReaching = false;
	useInterpolation = true;
}

void MeCtExampleBodyReach::addObstacle(const char* name,  SbmColObject* objCol )
{	
#if AVOID_OBSTACLE
	std::string obsName = name;		
	if (obstacleMap.find(obsName) == obstacleMap.end())
	{		
		obstacleMap[obsName] = objCol;

		VecOfSbmColObj& colList = posPlanner->cman()->ColObstacles();
		colList.push_back(objCol);
	}	
#endif

#if AVOID_OBSTACLE_BLEND
	std::string obsName = name;		
	if (obstacleMap.find(obsName) == obstacleMap.end())
	{		
		obstacleMap[obsName] = objCol;

		VecOfSbmColObj& colList = blendPlanner->cman()->ColObstacles();
		colList.push_back(objCol);
	}
	//printf("size of planner obstacle list = %d\n",blendPlanner->cman()->ColObstacles().size());
#endif
	//printf("size of obstacle map = %d\n",obstacleMap.size());
}


bool MeCtExampleBodyReach::updatePlannerBlendPath( VecOfInterpWeight& curWeight, float curRefTime, VecOfInterpWeight& targetWeight, float targetRefTime )
{
	static VecOfInterpWeight prevTarget;
	static float prevTargetTime;

	// update bounding box for sampling
	std::map<std::string, SbmColObject*>::iterator mi;
	bool obstacleChange = false;
	for ( mi  = obstacleMap.begin();
		mi != obstacleMap.end();
		mi++)
	{
		SbmColObject* colObj = mi->second;
		if (colObj->isUpdate)
		{	
			colObj->isUpdate = false;
			obstacleChange = true;
		}				
	}

	//printf("updatePlannerPath::planner obstalce size = %d\n",blendPlanner->cman()->colObstacles.size());
// 	printf("curW = ");
// 	for (int i=0;i<curWeight.size();i++)
// 		printf("%f ",curWeight[i].second);
// 	printf("\n");
// 
// 	printf("targetW = ");
// 	for (int i=0;i<targetWeight.size();i++)
// 		printf("%f ",targetWeight[i].second);
// 	printf("\n");

	static float realTime = 0.03f;
	bool pathUpdate = false;
	bool replan = (prevTarget != targetWeight || prevTargetTime != targetRefTime || obstacleChange );
	if (replan || !blendPlanner->solved())
	{
		if (replan)
		{
			cfgBlendStart->weight = curWeight;
			cfgBlendEnd->weight = targetWeight;

			cfgBlendStart->refTime = curRefTime;
			cfgBlendEnd->refTime = targetRefTime;

			cfgBlendStart->updateReachPt();
			cfgBlendEnd->updateReachPt();

			prevTarget = targetWeight;
			prevTargetTime = targetRefTime;
			blendPlanner->start(cfgBlendStart,cfgBlendEnd);
		}
		
		SrTimer timer;
		timer.start();		
		int nSteps = 0;
		while ( !blendPlanner->solved() )
		{
			blendPlanner->update ( planStep, planNumTries, planError );
			nSteps++;
			if ( timer.t() > realTime*0.8 )
				break; 
		}
		//printf("nSteps = %d\n",nSteps);	
		if (blendPlanner->solved())
		{
			//printf("Path solved\n");
			//printf("Path size = %d\n",blendPlanner->path()->size());
			blendPlanner->path()->smooth_ends ( planError );
			blendPlanner->path()->smooth_init ( planError );
			SrTimer timer2;
			timer2.start();
			while ( timer2.t() < realTime*0.3 )
				//for (int i=0;i<50;i++)
				blendPlanner->path()->smooth_step();		
		}

		if (blendPlanner->solved())
			pathUpdate = true;
	}	

// 	if (blendPlanner->solved())
// 	{
// 		//printf("Path solved\n");
// 		//printf("Path size = %d\n",blendPlanner->path()->size());		
// 		SrTimer timer2;
// 		timer2.start();
// 		while ( timer2.t() < realTime )
// 			//for (int i=0;i<50;i++)
// 			blendPlanner->path()->smooth_step();		
// 	}

	

	return pathUpdate;	
}

bool MeCtExampleBodyReach::updatePlannerPath( SrVec& curPos, SrVec& targetPos )
{
	static SrVec prevTarget;
	skeletonRef->invalidate_global_matrices();
	skeletonRef->update_global_matrices();
	const char* rootName = ikScenario.ikTreeRoot->joint->parent()->name().get_string();
	SrMat gMat = skeletonRef->root()->gmat();//search_joint(rootName)->gmat();
	SrMat gMatInv = gMat.inverse();

	// update bounding box for sampling
	std::map<std::string, SbmColObject*>::iterator mi;
	SrBox& posBox = posPlanner->cman()->SkPosBound();//paraBound.a
	posBox.a = paraBound.a*gMat;
	posBox.b = paraBound.b*gMat;

	bool obstacleChange = false;
	for ( mi  = obstacleMap.begin();
		  mi != obstacleMap.end();
		  mi++)
	{
		SbmColObject* colObj = mi->second;
		if (colObj->isUpdate)
		{	
			colObj->isUpdate = false;
			obstacleChange = true;
		}				
	}

	bool replan = (prevTarget != targetPos || obstacleChange );
	if (replan || !posPlanner->solved())
	{
		(SrVec&)*cfgStart = curPos;
		(SrVec&)*cfgEnd   = targetPos;

		prevTarget = targetPos;

		posPlanner->start(cfgStart,cfgEnd);
		SrTimer timer;
		timer.start();
		static float realTime = 0.03f;
		int nSteps = 0;
		while ( !posPlanner->solved() )
		{
			posPlanner->update ( planStep, planNumTries, planError );
			nSteps++;
			if ( timer.t() > realTime )
				break; 
		}
		//printf("nSteps = %d\n",nSteps);

		if (posPlanner->solved())
		{
			posPlanner->path()->smooth_ends ( planError );
			posPlanner->path()->smooth_init ( planError );
			SrTimer timer2;
			timer2.start();
			while ( timer2.t() < realTime*0.1f )
			//for (int i=0;i<50;i++)
				posPlanner->path()->smooth_step();
		}
		return true;
	}	
	return false;
}

DataInterpolator* MeCtExampleBodyReach::createInterpolator()
{	
	KNNInterpolator* interpolator = new KNNInterpolator(500,ikReachRegion*3.f);
	resampleData = &interpolator->resampleData;
	interpExampleData = interpolator->getInterpExamples();
	return interpolator;
}

ResampleMotion* MeCtExampleBodyReach::createInterpMotion()
{
	ResampleMotion* ex = new ResampleMotion(motionExamples.getMotionData());
	ex->motionParameterFunc = motionParameter;
	return ex;
}

void MeCtExampleBodyReach::updateSkeletonCopy()
{
	// copy world offset to the copy of skeleton
	skeletonCopy->root()->quat()->value(skeletonRef->root()->quat()->value());
	for (int i=0;i<3;i++)
		skeletonCopy->root()->pos()->value(i,skeletonRef->root()->pos()->value(i));
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
			//frame.channelUpdated( count );
		}
	}

	if (motionFrame.jointQuat.size() != affectedJoints.size())
		motionFrame.jointQuat.resize(affectedJoints.size());
	//BOOST_FOREACH(SrQuat& quat, motionFrame.jointQuat)
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

}

void MeCtExampleBodyReach::controller_map_updated()
{

}

SrVec MeCtExampleBodyReach::getIKTarget()
{
	return ikTarget+ikOffset;
}