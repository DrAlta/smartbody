#include <assert.h>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <sr/sr_timer.h>
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

std::string MeCtReachEngine::ReachTypeTag[REACH_TYPE_SIZE] = { "Right", "Left" };

MeCtReachEngine::MeCtReachEngine( SbmCharacter* sbmChar, SkSkeleton* sk)
{
	character = sbmChar;
	skeletonCopy = new SkSkeleton(sk); 
	skeletonCopy->ref();
	skeletonRef  = sk;
	dataInterpolator = NULL;
	refMotion = NULL;

	valid = false;
	ikInit = false;

	reachCompleteDuration = -1.0;
	fadingWeight = 0.f;	
	footIKFix = false;
	initStart = true;
	reachEndEffector = NULL;
	curHandActionState  = TOUCH_OBJECT;		
	interpMotion = NULL;
	motionParameter = NULL;
	curReachState = NULL;	
	reachData = NULL;
}

MeCtReachEngine::~MeCtReachEngine( void )
{
	for (std::map<std::string,EffectorConstraint*>::iterator iter = rightFootConstraint.begin();
		iter != rightFootConstraint.end();
		iter++)
	{
		delete (*iter).second;
	}	
	for (std::map<std::string,EffectorConstraint*>::iterator iter = leftFootConstraint.begin();
		iter != leftFootConstraint.end();
		iter++)
	{
		delete (*iter).second;
	}
	for (std::map<std::string,ReachStateInterface*>::iterator iter = stateTable.begin();
		 iter != stateTable.end();
		 iter++)
	{
		delete (*iter).second;
	}
	if (reachData)
	{
		delete reachData;
	}
	skeletonCopy->unref();
//	if (dataInterpolator)
//		delete dataInterpolator;
//	if (interpMotion)
//		delete interpMotion;
//	FREE_DATA(motionParameter);
}

void MeCtReachEngine::init(int rtype, SkJoint* effectorJoint)
{
	assert(skeletonRef);	
	assert(character);
	// root is "world_offset", so we use root->child to get the base joint.
	reachType = rtype;
	reachEndEffector = effectorJoint;
	SkJoint* rootJoint = findRootJoint(skeletonCopy);//findRootJoint(skeletonRef);//skeletonRef->root()->child(0);//skeletonCopy->root()->child(0);//skeletonRef->root()->child(0);	
	ikScenario.buildIKTreeFromJointRoot(rootJoint);
	ikCCDScenario.buildIKTreeFromJointRoot(rootJoint);	

	EffectorConstantConstraint* cons = new EffectorConstantConstraint();
	cons->efffectorName = reachEndEffector->name().c_str();
	std::string consRootName = "r_sternoclavicular";
	if (reachType == LEFT_ARM)
		consRootName = "l_sternoclavicular";
	cons->rootName = consRootName;
	reachPosConstraint[cons->efffectorName] = cons;
	// if there is a child	
	if (reachEndEffector->child(0))
	{
		EffectorConstantConstraint* rotCons = new EffectorConstantConstraint();				
		rotCons->efffectorName = reachEndEffector->name().c_str();
		rotCons->rootName = consRootName;
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
	ikMotionFrame.jointQuat.resize(nodeList.size());

	affectedJoints.clear();	
	for (unsigned int i=0;i<nodeList.size();i++)
	{
		MeCtIKTreeNode* node = nodeList[i];
		SkJoint* joint = skeletonCopy->linear_search_joint(node->nodeName.c_str());		
		SkJointQuat* skQuat = joint->quat();		
		affectedJoints.push_back(joint);			
	}		

	SkJoint* copyEffector = skeletonCopy->linear_search_joint(reachEndEffector->name().c_str());
	SkJoint* copyRoot = skeletonCopy->linear_search_joint(rootJoint->parent()->name().c_str());
	motionParameter = new ReachMotionParameter(skeletonCopy,affectedJoints,copyEffector,copyRoot);
	motionExamples.initMotionExampleSet(motionParameter);	

	// initialize all parameters according to scale	
	float characterHeight = character->getHeight();
	ikReachRegion = characterHeight*0.02f;	
	ikDefaultVelocity = characterHeight*0.3f;
	ikDamp        = ikReachRegion*ikReachRegion*20.0;//characterHeight*0.1f;

	SbmCharacter* curCharacter = character;

	reachData = new ReachStateData();
	reachData->characterHeight = characterHeight;		
	reachData->charName = character->getName();
	reachData->reachRegion = ikReachRegion;	
	reachData->linearVel = ikDefaultVelocity;
	reachData->curRefTime = 0.f;
	reachData->motionParameter = motionParameter;
	reachData->idleRefFrame = reachData->currentRefFrame = reachData->targetRefFrame = idleMotionFrame;	
	reachData->reachType = reachType;

	EffectorState& estate = reachData->effectorState;
	estate.effectorName = reachEndEffector->name().c_str();
	estate.curIKTargetState = reachData->getPoseState(idleMotionFrame);


	stateTable["Idle"] = new ReachStateIdle();
	stateTable["Start"] = new ReachStateStart();
	//stateTable["Move"] = new ReachStateMove();
	stateTable["Complete"] = new ReachStateComplete();
	stateTable["NewTarget"] = new ReachStateNewTarget();
	stateTable["PreReturn"] = new ReachStatePreReturn();
	stateTable["Return"] = new ReachStateReturn();

	handActionTable[PICK_UP_OBJECT] = new ReachHandPickUpAction();
	handActionTable[TOUCH_OBJECT] = new ReachHandAction(); // default hand action
	handActionTable[PUT_DOWN_OBJECT] = new ReachHandPutDownAction();

	curReachState = getState("Idle");
}

void MeCtReachEngine::updateMotionExamples( const MotionDataSet& inMotionSet )
{
	if (inMotionSet.size() == 0)
		return;

	// set world offset to zero	
	std::string rootName = ikScenario.ikTreeRoot->joint->parent()->name();
	SkJoint* root = skeletonRef->search_joint(rootName.c_str());

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

	SkJoint* rootJoint = affectedJoints[0];
	BOOST_FOREACH(TagMotion tagMotion, inMotionSet)
	{
		if (tagMotion.first != reachType) // only process motion with correct tag 
			continue;

		SkMotion* motion = tagMotion.second;
		if (motionData.find(tagMotion) != motionData.end())
			continue; // we do not process example motions that are already used for this controller instance
		if (!refMotion)
			refMotion = motion;

		motionData.insert(tagMotion);
		MotionExample* ex = new MotionExample();
		ex->motion = motion;		
		ex->timeWarp = new SimpleTimeWarp(refMotion->duration(),motion->duration());
		ex->motionParameterFunc = motionParameter;
		ex->motionProfile = new MotionProfile(motion);
		ex->updateRootOffset(skeletonCopy,rootJoint);
#if 0
		ex->motionProfile->buildVelocityProfile(0.f,motion->duration()*0.999f,0.005f);
		ex->motionProfile->buildInterpolationProfile(0.f,(float)motion->time_stroke_emphasis(),0.005f);
		ex->motionProfile->buildEulerCurveProfile(0.f,(float)motion->time_stroke_emphasis(),0.005f);
#endif
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

	if (motionExamples.getExamples().empty()) // no examples
	{
		//valid = false;
		return;
	}

	if (dataInterpolator)
		delete dataInterpolator;

	dataInterpolator = createInterpolator();
	dataInterpolator->init(&motionExamples);
	dataInterpolator->buildInterpolator();		

	if (resampleData)
	{
		for (unsigned int i=0;i<resampleData->size();i++)
		{
			InterpolationExample* ex = (*resampleData)[i];
			SrVec reachPos;
			for (int k=0;k<3;k++)
				reachPos[k] = (float)ex->parameter[k];
			resamplePts.push_back(reachPos);		
		}
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
	// 	if (rd->curHandAction == handActionTable[PICK_UP_OBJECT] || rd->curHandAction == handActionTable[PUT_DOWN_OBJECT])
	// 		return true;
	// 	else if (rd->curHandAction == handActionTable[TOUCH_OBJECT])
	// 		return false;
	// 	return false;
	return true;
}

void MeCtReachEngine::solveIK( ReachStateData* rd, BodyMotionFrame& outFrame )
{	
	BodyMotionFrame& refFrame = rd->currentRefFrame;
	if (!ikInit)
	{			
		ikScenario.setTreeNodeQuat(refFrame.jointQuat,QUAT_INIT);
		ikScenario.setTreeNodeQuat(refFrame.jointQuat,QUAT_CUR);	
		ikInit = true;
	}		
	EffectorState& estate = rd->effectorState;

	EffectorConstantConstraint* cons = dynamic_cast<EffectorConstantConstraint*>(reachPosConstraint[reachEndEffector->name().c_str()]);
	cons->targetPos = estate.curIKTargetState.tran;	

	ikScenario.ikGlobalMat = rd->gmat;//skeletonRef->search_joint(rootName)->gmat();//ikScenario.ikTreeRoot->joint->parent()->gmat();	
	ikScenario.ikTreeRootPos = refFrame.rootPos;
	ikScenario.setTreeNodeQuat(refFrame.jointQuat,QUAT_REF);		
	ikScenario.ikPosEffectors = &reachPosConstraint;

	{
		EffectorConstantConstraint* cons = dynamic_cast<EffectorConstantConstraint*>(reachRotConstraint[reachEndEffector->name().c_str()]);		
		cons->targetRot = estate.curIKTargetState.rot;//ikRotTrajectory;//ikRotTarget;//motionParameter->getMotionFrameJoint(interpMotionFrame,reachEndEffector->name().get_string())->gmat();//ikRotTarget;	
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

ReachStateInterface* MeCtReachEngine::getState( const std::string& stateName )
{
	return stateTable[stateName];
}

SkJoint* MeCtReachEngine::findRootJoint( SkSkeleton* sk )
{
	SkJoint* rootJoint = sk->root()->child(0); // skip world offset
	//LOG("ReachEngine Root Name = %s\n",rootJoint->name().get_string());
	bool bStop = false;
	while (!bStop)
	{
		if (rootJoint->num_children() == 0)
			return rootJoint;
		SkJoint* child = rootJoint->child(0);
		SkJointPos* skRootPos = rootJoint->pos();		
		SkJointPos* skPos = child->pos();
		bool rootFrozen = (skRootPos->frozen(0) && skRootPos->frozen(1) && skRootPos->frozen(2));
		bool childFrozen = (skPos->frozen(0) && skPos->frozen(1) && skPos->frozen(2));
		if (!rootFrozen) // find the top joint (excluding world offset) that has both translation + rotation
		{
			bStop = true;
		}
		else if (child)
		{
			rootJoint = child;
		}
		else
		{
			// error ? 
			rootJoint = sk->root()->child(0);
			bStop = true;
		}
	}
	return rootJoint;
}

DataInterpolator* MeCtReachEngine::createInterpolator()
{
	KNNInterpolator* interpolator = new KNNInterpolator(1000,ikReachRegion*1.f);
	//RBFInterpolator* interpolator = new RBFInterpolator();
	resampleData = &interpolator->resampleData;		
	// 	InverseInterpolation* interpolator = new InverseInterpolation();
	// 	resampleData = NULL;
	interpExampleData = interpolator->getInterpExamples();
	return interpolator;
}

ResampleMotion* MeCtReachEngine::createInterpMotion()
{
	ResampleMotion* ex = new ResampleMotion(motionExamples.getMotionData());
	ex->motionParameterFunc = motionParameter;
	ex->motionProfile = NULL;
	return ex;
}

void MeCtReachEngine::updateReach(float t, float dt, BodyMotionFrame& inputFrame, float blendWeight)
{
	float du = 0.0;
	//LOG("reach type = %s, dt = %f",getReachTypeTag().c_str(),dt);
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
	std::string rootName = ikScenario.ikTreeRoot->joint->parent()->name();
	reachData->curTime = (float)t;
	reachData->dt = dt;	
	reachData->stateTime += dt;
	reachData->curHandAction = handActionTable[curHandActionState];	
	reachData->updateReachState(skeletonRef->search_joint(rootName.c_str())->gmat(),ikMotionFrame);
	if (curCharacter)
	{		
		reachData->locomotionComplete = (curCharacter->_reachTarget && !curCharacter->_lastReachStatus);		
	}

	mcuCBHandle& mcu = mcuCBHandle::singleton();
	//reachData->hasSteering = (mcu._scene->getSteerManager()->isInitialized());

	curReachState->updateEffectorTargetState(reachData);		
	curReachState->update(reachData);	
	ReachStateInterface* nextState = getState(curReachState->nextState(reachData));
	if (nextState != curReachState)
	{
		//printf("engine type = %s,  cur State = %s\n",this->getReachTypeTag().c_str(), nextState->curStateName().c_str());
		reachData->stateTime = 0.f;
		reachData->effectorState.startTargetState = reachData->effectorState.curIKTargetState;
		reachData->effectorState.curBlendState = reachData->effectorState.startTargetState;
		reachData->startRefFrame = reachData->currentRefFrame;
		curReachState = nextState;
	}

	ikMaxOffset = ikDefaultVelocity*3.f*dt;	
	mcu.mark("Reach",0,"reachIK");
	if (footIKFix)
	{
		solveIK(reachData,ikMotionFrame);	
	}
	else
	{
		ikMotionFrame = reachData->currentRefFrame;
	}
	
	//ikMotionFrame = reachData->currentRefFrame;
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

std::string MeCtReachEngine::getReachTypeTag()
{
	return ReachTypeTag[reachType];
}

int MeCtReachEngine::getReachTypeID()
{
	return reachType;
}

int MeCtReachEngine::getReachType( const std::string& tag )
{
	for (int i=0;i<REACH_TYPE_SIZE;i++)
	{
		if (boost::iequals(tag,ReachTypeTag[i]))
			return i;
	}
	return -1;
}
