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
#define USE_PICK_UP 1
#define AVOID_OBSTACLE 0
#define AVOID_OBSTACLE_BLEND 0

const std::string lFootName[] = {"l_forefoot", "l_ankle" };
const std::string rFootName[] = {"r_forefoot", "r_ankle" };

MeCtExampleBodyReach::MeCtExampleBodyReach(std::string charName, SkSkeleton* sk, SkJoint* effectorJoint ) : FadingControl()
{	
	// here we create a copy of skeleton as an intermediate structure.
	// this will make it much easier to grab a key-frame from a SkMotion.
	// we use the "copy" of original skeleton to avoid corrupting the channel data by these internal operations.
	characterName = charName;
	skeletonCopy = new SkSkeleton(sk); 
	skeletonRef  = sk;

	dataInterpolator = NULL;
	
	refMotion = NULL;
	_duration = -1.0f;
	
	
	interactiveReach = false;
	footIKFix = false;
	reachEndEffector = effectorJoint;
		
	curHandActionState  = TOUCH_OBJECT;		
	interpMotion = NULL;
	motionParameter = NULL;
	curReachState = NULL;

	defaultVelocity = 50.f;
	reachCompleteDuration = 0.5f;		
}

ReachStateInterface* MeCtExampleBodyReach::getState( std::string stateName )
{
	return stateTable[stateName];
}

void MeCtExampleBodyReach::setHandActionState( HandActionState newState )
{
	curHandActionState = newState;	
}

MeCtExampleBodyReach::~MeCtExampleBodyReach( void )
{
	#define FREE_DATA(data) if (data) delete data; data=NULL;
	FREE_DATA(dataInterpolator);
	FREE_DATA(interpMotion);
	FREE_DATA(motionParameter);
	FREE_DATA(skeletonCopy);	
}

void MeCtExampleBodyReach::setFinishReaching( bool isFinish )
{
	reachData->endReach = isFinish;
}

void MeCtExampleBodyReach::setFootIK( bool useIK )
{
	footIKFix = useIK;
}

void MeCtExampleBodyReach::setLinearVelocity( float vel )
{
	reachData->linearVel = vel;
}

void MeCtExampleBodyReach::setReachTargetPawn( SbmPawn* targetPawn )
{
	//reachTargetPawn = targetPawn;	
	ReachTarget& t = reachData->reachTarget;
	t.setTargetPawn(targetPawn);	
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

void MeCtExampleBodyReach::solveIK( ReachStateData* rd, BodyMotionFrame& outFrame )
{
	static bool bInit = false;
	BodyMotionFrame& refFrame = rd->currentRefFrame;
	if (!bInit)
	{			
		ikScenario.setTreeNodeQuat(refFrame.jointQuat,QUAT_INIT);
		ikScenario.setTreeNodeQuat(refFrame.jointQuat,QUAT_CUR);	
		bInit = true;
	}		
	EffectorState& estate = rd->effectorState;

	EffectorConstantConstraint* cons = dynamic_cast<EffectorConstantConstraint*>(reachPosConstraint[reachEndEffector->name().get_string()]);
	cons->targetPos = estate.curTargetState.tran;	
	
	ikScenario.ikGlobalMat = rd->gmat;//skeletonRef->search_joint(rootName)->gmat();//ikScenario.ikTreeRoot->joint->parent()->gmat();	
	ikScenario.ikTreeRootPos = refFrame.rootPos;
	ikScenario.setTreeNodeQuat(refFrame.jointQuat,QUAT_REF);		
	ikScenario.ikPosEffectors = &reachPosConstraint;

	{
		EffectorConstantConstraint* cons = dynamic_cast<EffectorConstantConstraint*>(reachRotConstraint[reachEndEffector->name().get_string()]);		
		cons->targetRot = estate.curTargetState.rot;//ikRotTrajectory;//ikRotTarget;//motionParameter->getMotionFrameJoint(interpMotionFrame,reachEndEffector->name().get_string())->gmat();//ikRotTarget;	
		
		if (rd->curHandAction == handActionTable[PICK_UP_OBJECT] || rd->curHandAction == handActionTable[PUT_DOWN_OBJECT])
			ikScenario.ikRotEffectors = &reachRotConstraint;
		else if (rd->curHandAction == handActionTable[TOUCH_OBJECT])
			ikScenario.ikRotEffectors = &reachNoRotConstraint;
	}

	//sr_out << "target pos = " << estate.curTargetState.tran << " , target rot = " << estate.curTargetState.rot << srnl;
	ik.maxOffset = ikMaxOffset;
	ik.dampJ = ikDamp;
	ik.refDampRatio = 0.05;
	for (int i=0;i<2;i++)
	{
		ik.update(&ikScenario);		
		ikScenario.copyTreeNodeQuat(QUAT_CUR,QUAT_INIT);		
	}

    if (footIKFix)
	{
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
	}

	outFrame = refFrame;
	ikScenario.getTreeNodeQuat(outFrame.jointQuat,QUAT_CUR); 	
}

bool MeCtExampleBodyReach::controller_evaluate( double t, MeFrameData& frame )
{
	//float dt = 0.001f;
	float du = 0.0;
	if (prev_time == -1.0) // first start
	{
		//dt = 0.001f;		
		// for first frame, update from frame buffer to joint quat in the limb
		// any future IK solving will simply use the joint quat from the previous frame.
		updateChannelBuffer(frame,idleMotionFrame,true);
		reachData->idleRefFrame = reachData->currentRefFrame = reachData->targetRefFrame = idleMotionFrame;
		curReachState = getState("Idle");
		ikMotionFrame = idleMotionFrame;		
	}	

	updateDt((float)t);
	
// 	if (restart)
// 	{
// 		dt = 0.001f;
// 		restart = false;
// 	}
// 	else
// 	{		
// 		dt = ((float)(t-prev_time));
// 	}
// 	prev_time = (float)t;	

	SbmCharacter* curCharacter = mcuCBHandle::singleton().character_map.lookup(characterName.c_str());
	skeletonRef->update_global_matrices();
	updateChannelBuffer(frame,inputMotionFrame,true);
	updateSkeletonCopy();	

	// update reach data
	const char* rootName = ikScenario.ikTreeRoot->joint->parent()->name().get_string();
	reachData->curTime = (float)t;
	reachData->dt = dt;	
	reachData->stateTime += dt;
	reachData->curHandAction = handActionTable[curHandActionState];	
	reachData->updateReachState(skeletonRef->search_joint(rootName)->gmat(),ikMotionFrame);
	if (curCharacter)
	{
		reachData->locomotionComplete = curCharacter->_reachTarget;
	}

	curReachState->updateEffectorTargetState(reachData);		
	curReachState->update(reachData);	

	ReachStateInterface* nextState = getState(curReachState->nextState(reachData));

	if (nextState != curReachState)
	{
		printf("cur State = %s\n",nextState->curStateName().c_str());
		reachData->stateTime = 0.f;
		curReachState = nextState;
	}
	
	ikMaxOffset = defaultVelocity*3.f*dt;
	solveIK(reachData,ikMotionFrame);
	// blending the input frame with ikFrame based on current fading
	bool finishFadeOut = updateFading(dt);
	BodyMotionFrame outMotionFrame;
	MotionExampleSet::blendMotionFrame(inputMotionFrame,ikMotionFrame,blendWeight,outMotionFrame);	
	updateChannelBuffer(frame,outMotionFrame);

	return true;
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

	ikScenario.ikPosEffectors = &reachPosConstraint;
	ikScenario.ikRotEffectors = &reachRotConstraint;
	
	const IKTreeNodeList& nodeList = ikScenario.ikTreeNodes;	
	idleMotionFrame.jointQuat.resize(nodeList.size());
	inputMotionFrame.jointQuat.resize(nodeList.size());
	ikMotionFrame.jointQuat.reserve(nodeList.size());

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
	defaultVelocity = characterHeight*0.3f;
	ikDamp        = ikReachRegion*ikReachRegion*14.0;//characterHeight*0.1f;

	SbmCharacter* curCharacter = mcuCBHandle::singleton().character_map.lookup(characterName.c_str());
	
	reachData = new ReachStateData();
	reachData->characterHeight = characterHeight;
	reachData->reachControl = this;
	reachData->autoReturnTime = -1.f;//reachCompleteDuration;
	reachData->charName = characterName;
	reachData->reachRegion = ikReachRegion;
	reachData->angularVel = 10.0f;
	reachData->linearVel = defaultVelocity*0.5f;
	reachData->curRefTime = 0.f;
	reachData->motionParameter = motionParameter;
	reachData->idleRefFrame = reachData->currentRefFrame = reachData->targetRefFrame = idleMotionFrame;	

	EffectorState& estate = reachData->effectorState;
	estate.effectorName = reachEndEffector->name().get_string();
	estate.curTargetState = reachData->getPoseState(idleMotionFrame);

	
	stateTable["Idle"] = new ReachStateIdle();
	stateTable["Start"] = new ReachStateStart();
	stateTable["Move"] = new ReachStateMove();
	stateTable["Complete"] = new ReachStateComplete();
	stateTable["NewTarget"] = new ReachStateNewTarget();
	stateTable["Return"] = new ReachStateReturn();

	handActionTable[PICK_UP_OBJECT] = new ReachHandPickUpAction();
	handActionTable[TOUCH_OBJECT] = new ReachHandAction(); // default hand action
	handActionTable[PUT_DOWN_OBJECT] = new ReachHandPutDownAction();

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
	
	for (unsigned int i=0;i<resampleData->size();i++)
	{
		InterpolationExample* ex = (*resampleData)[i];
		SrVec reachPos;
		for (int k=0;k<3;k++)
			reachPos[k] = (float)ex->parameter[k];
		resamplePts.push_back(reachPos);		
	}

	if (interpMotion)
		delete interpMotion;
	interpMotion = createInterpMotion();

	// initialize the interpolation weights
	dVector para; para.resize(3);
	for (int i=0;i<3;i++)
		para[i] = curEffectorPos[i];
	if (interpMotion && dataInterpolator)
		dataInterpolator->predictInterpWeights(para,interpMotion->weight);

	reachData->interpMotion = interpMotion;
	reachData->dataInterpolator = dataInterpolator;
	reachData->useExample = true;		
}

DataInterpolator* MeCtExampleBodyReach::createInterpolator()
{	
	KNNInterpolator* interpolator = new KNNInterpolator(3000,ikReachRegion*1.f);
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
	//restart = true;
	controlRestart();
}

void MeCtExampleBodyReach::controller_map_updated()
{

}

