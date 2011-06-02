#include <assert.h>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <boost/foreach.hpp>
#include <SR/sr_timer.h>
#include "MeCtReachEngine.h"
#include "mcontrol_util.h"
#include "me_ct_barycentric_interpolation.h"
#include "sbm/Event.h"
#include "MeCtBodyReachState.h"


EffectorConstantConstraint& EffectorConstantConstraint::operator=( const EffectorConstantConstraint& rhs )
{
	efffectorName = rhs.efffectorName;
	rootName    = rhs.rootName;	
	targetPos = rhs.targetPos;//SrQuat(SrVec(0,1,0),M_PI);
	targetRot = rhs.targetRot;
	return *this;
}

/************************************************************************/
/* Reach Engine                                                         */
/************************************************************************/

const std::string lFootName[] = {"l_forefoot", "l_ankle" };
const std::string rFootName[] = {"r_forefoot", "r_ankle" };

MeCtReachEngine::MeCtReachEngine( SbmCharacter* sbmChar, SkSkeleton* sk, SkJoint* effector )
{
	character = sbmChar;
	skeletonCopy = new SkSkeleton(sk); 
	skeletonRef  = sk;
	dataInterpolator = NULL;
	refMotion = NULL;

	reachCompleteDuration = -1.0;
	
	footIKFix = false;
	initStart = true;
	reachEndEffector = effector;
	curHandActionState  = TOUCH_OBJECT;		
	interpMotion = NULL;
	motionParameter = NULL;
	curReachState = NULL;	
}

MeCtReachEngine::~MeCtReachEngine( void )
{
#define FREE_DATA(data) if (data) delete data; data=NULL;
	FREE_DATA(dataInterpolator);
	FREE_DATA(interpMotion);
	FREE_DATA(motionParameter);
	FREE_DATA(skeletonCopy);	
}

void MeCtReachEngine::init()
{
	assert(skeletonRef);	
	assert(character);
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

// 	for (int i=0;i<3;i++)
// 		_channels.add(rootJoint->name().get_string(), (SkChannel::Type)(SkChannel::XPos+i));

	affectedJoints.clear();	
	for (unsigned int i=0;i<nodeList.size();i++)
	{
		MeCtIKTreeNode* node = nodeList[i];
		SkJoint* joint = skeletonCopy->linear_search_joint(node->nodeName.c_str());
		SkJointQuat* skQuat = joint->quat();		
		affectedJoints.push_back(joint);	
		//_channels.add(joint->name().get_string(), SkChannel::Quat);		
	}		

	SkJoint* copyEffector = skeletonCopy->linear_search_joint(reachEndEffector->name().get_string());
	motionParameter = new ReachMotionParameter(skeletonCopy,affectedJoints,copyEffector);
	motionExamples.initMotionExampleSet(motionParameter);	

	// initialize all parameters according to scale	
	float characterHeight = character->getHeight();
	ikReachRegion = characterHeight*0.02f;	
	ikDefaultVelocity = characterHeight*0.3f;
	ikDamp        = ikReachRegion*ikReachRegion*14.0;//characterHeight*0.1f;

	SbmCharacter* curCharacter = character;

	reachData = new ReachStateData();
	reachData->characterHeight = characterHeight;
	//reachData->reachControl = this;
	reachData->autoReturnTime = reachCompleteDuration;
	reachData->charName = character->name;
	reachData->reachRegion = ikReachRegion;
	reachData->angularVel = 10.0f;
	reachData->linearVel = ikDefaultVelocity*0.5f;
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
}

void MeCtReachEngine::updateMotionExamples( const MotionDataSet& inMotionSet )
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
	//dVector para; para.resize(3);
	//for (int i=0;i<3;i++)
	//	para[i] = curEffectorPos[i];
	//if (interpMotion && dataInterpolator)
	//	dataInterpolator->predictInterpWeights(para,interpMotion->weight);

	reachData->interpMotion = interpMotion;
	reachData->dataInterpolator = dataInterpolator;
	reachData->useExample = true;
}



bool MeCtReachEngine::hasEffectorRotConstraint( ReachStateData* rd )
{	
	if (rd->curHandAction == handActionTable[PICK_UP_OBJECT] || rd->curHandAction == handActionTable[PUT_DOWN_OBJECT])
		return true;
	else if (rd->curHandAction == handActionTable[TOUCH_OBJECT])
		return false;
	return false;
}

void MeCtReachEngine::solveIK( ReachStateData* rd, BodyMotionFrame& outFrame )
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
		cons->constraintWeight = 0.f;//1.f - rd->blendWeight;

		//if (rd->curHandAction == handActionTable[PICK_UP_OBJECT] || rd->curHandAction == handActionTable[PUT_DOWN_OBJECT])
		if (hasEffectorRotConstraint(rd))
			ikScenario.ikRotEffectors = &reachRotConstraint;
		//else if (rd->curHandAction == handActionTable[TOUCH_OBJECT])
		else
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
			lfoot->targetPos = motionParameter->getMotionFrameJoint(inputMotionFrame,lFootName[i].c_str())->gmat().get_translation();
			EffectorConstantConstraint* rfoot = dynamic_cast<EffectorConstantConstraint*>(rightFootConstraint[rFootName[i]]);
			rfoot->targetPos = motionParameter->getMotionFrameJoint(inputMotionFrame,rFootName[i].c_str())->gmat().get_translation();	
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

void MeCtReachEngine::updateSkeletonCopy()
{
	skeletonCopy->root()->quat()->value(skeletonRef->root()->quat()->value());
	for (int i=0;i<3;i++)
		skeletonCopy->root()->pos()->value(i,skeletonRef->root()->pos()->value(i));
}

ReachStateInterface* MeCtReachEngine::getState( std::string stateName )
{
	return stateTable[stateName];
}

SkJoint* MeCtReachEngine::findRootJoint( SkSkeleton* sk )
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

DataInterpolator* MeCtReachEngine::createInterpolator()
{
	KNNInterpolator* interpolator = new KNNInterpolator(3000,ikReachRegion*1.f);
	resampleData = &interpolator->resampleData;
	interpExampleData = interpolator->getInterpExamples();
	return interpolator;
}

ResampleMotion* MeCtReachEngine::createInterpMotion()
{
	ResampleMotion* ex = new ResampleMotion(motionExamples.getMotionData());
	ex->motionParameterFunc = motionParameter;
	return ex;
}

void MeCtReachEngine::updateReach(float t, float dt, BodyMotionFrame& inputFrame)
{
	float du = 0.0;
	if (initStart) // first start
	{		
		idleMotionFrame = inputFrame;
		reachData->idleRefFrame = reachData->currentRefFrame = reachData->targetRefFrame = idleMotionFrame;
		curReachState = getState("Idle");
		ikMotionFrame = idleMotionFrame;		
		initStart = false;
	}		
	inputMotionFrame = inputFrame;
	reachData->idleRefFrame = inputFrame;

	SbmCharacter* curCharacter = character;
	skeletonRef->update_global_matrices();	
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
		//printf("cur State = %s\n",nextState->curStateName().c_str());
		reachData->stateTime = 0.f;
		curReachState = nextState;
	}

	ikMaxOffset = ikDefaultVelocity*3.f*dt;
	solveIK(reachData,ikMotionFrame);	
}

bool MeCtReachEngine::addHandConstraint( SkJoint* targetJoint, const char* effectorName )
{
	MeCtIKTreeNode* node = ikScenario.findIKTreeNode(effectorName);
	if (!node || !targetJoint)
		return false;

	std::string str = effectorName;		
	ConstraintMap::iterator ci = handConstraint.find(str);
	if (ci != handConstraint.end())
	{		
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