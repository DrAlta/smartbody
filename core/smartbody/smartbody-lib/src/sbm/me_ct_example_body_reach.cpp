#include <assert.h>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <boost/foreach.hpp>
#include "me_ct_example_body_reach.hpp"

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

//const char* endEffectorName = "r_wrist"; // a hard coded hack

MeCtExampleBodyReach::MeCtExampleBodyReach( SkSkeleton* sk, SkJoint* effectorJoint )
{
	skeletonRef = sk;
	prev_time = -1.0;
	dataInterpolator = NULL;
	refMotion = NULL;
	reachTime = 0.0;	
	useIKConstraint = true;
	useInterpolation = true;
	useTargetJoint   = true;
	reachEndEffector = effectorJoint;
	//skeletonRef->search_joint(endEffectorName);	

	curReachState = REACH_START;
	reachTargetJoint = NULL;
	interpMotion = NULL;
	motionParameter = NULL;

	reachVelocity = 50.f;
	reachCompleteDuration = 20000.0f;
	curPercentTime = 0.0;
	prevPercentTime = 0.0;
}

MeCtExampleBodyReach::~MeCtExampleBodyReach( void )
{
	#define FREE_DATA(data) if (data) delete data; data=NULL;
	FREE_DATA(dataInterpolator);
	FREE_DATA(interpMotion);
	FREE_DATA(motionParameter);
}

void MeCtExampleBodyReach::setReachTargetJoint( SkJoint* val )
{
	reachTargetJoint = val;	
	useTargetJoint = true;
}


void MeCtExampleBodyReach::setReachTargetPos( SrVec& targetPos )
{
	reachTargetPos = targetPos;
	useTargetJoint = false;
}


void MeCtExampleBodyReach::findReachTarget( SrVec& rTarget, SrVec& rError )
{
	//if (reachTargetJoint)
	{
		SrVec newReachTarget = (reachTargetJoint && useTargetJoint) ? reachTargetJoint->gmat().get_translation() : reachTargetPos;
		if ( (newReachTarget - reachTarget).norm() > 0.01 ) // interrupt and reset reach state if the reach target is moved
		{				
			if (curReachState == REACH_IDLE)
			{
				curReachState = REACH_START;
				curReachIKOffset = SrVec();
				reachTime = 0.f;
			}				
			reachTarget = newReachTarget;				
		}

		if (curReachState == REACH_START || curReachState == REACH_COMPLETE)
		{
			rTarget = reachTarget;		
		}
		else if (curReachState == REACH_RETURN || curReachState == REACH_IDLE)
		{
			if (interpMotion && useInterpolation)
			{
				VecOfDouble initPos;
				double refDuration = interpMotion->motionDuration(BodyMotionInterface::DURATION_REF);
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
		vector<double> para; para.resize(3);
		for (int i=0;i<3;i++)
			para[i] = rTarget[i];		
		currentInterpTarget = reachEndEffector->gmat().get_translation();
		if (dataInterpolator && interpMotion && useInterpolation)
		{			
			VecOfDouble acutualReachTarget;
			if (curReachState == REACH_START || curReachState == REACH_COMPLETE)
			{
				// compute the new interpolation weight based on current target
				dataInterpolator->predictInterpWeights(para,interpMotion->weight);	
				double refTime = interpMotion->strokeEmphasisTime();							
				motionParameter->getMotionFrameParameter(interpMotion,(float)refTime,acutualReachTarget);	
			}
			else if (curReachState == REACH_RETURN || curReachState == REACH_IDLE)				
			{	
				// when generating the return motion, use the same weight for reaching.
				double refDuration = interpMotion->motionDuration(BodyMotionInterface::DURATION_REF);			
				motionParameter->getMotionFrameParameter(interpMotion,(float)refDuration*0.999f,acutualReachTarget);			
			}
			for (int i=0;i<3;i++)
				currentInterpTarget[i] = (float)acutualReachTarget[i];		
		}			
		rError = rTarget - currentInterpTarget;	
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
	ikScenario.ikGlobalMat = ikScenario.ikTreeRoot->joint->parent()->gmat();	
	ikScenario.setTreeNodeQuat(refFrame.jointQuat,QUAT_REF);							
	//ik.setDt(dt);
	ik.refDampRatio = 0.1;
	ik.update(&ikScenario);		
	ikScenario.copyTreeNodeQuat(QUAT_CUR,QUAT_INIT);		
	ikScenario.getTreeNodeQuat(outFrame.jointQuat,QUAT_CUR);	
}

bool MeCtExampleBodyReach::updateInterpolation(float dt, BodyMotionFrame& outFrame, float& du)
{
	bool interpHasReach = false;	
	interpMotion->getMotionFrame(reachTime,skeletonRef,affectedJoints,interpMotionFrame);			
	{			
		double strokTime = interpMotion->strokeEmphasisTime();
		double refDuration = interpMotion->motionDuration(BodyMotionInterface::DURATION_REF);

		if (curReachState == REACH_START)
		{
			curPercentTime = reachTime/strokTime;				
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
		if ( curReachState == REACH_COMPLETE )
			// use normal IK to compute the hand movement after touching the object
		{				
			SrVec offset = (ikTarget - curEffectorPos);
			float offsetLength = offset.norm();				
			if (offset.norm() > reachVelocity*dt)
			{
				offset.normalize();
				offset = offset*reachVelocity*dt;
			}
			float morphWeight = offsetLength > 0.f ? (offset.norm()+reachVelocity*dt)/(offsetLength+reachVelocity*dt) : 1.f;
			BodyMotionFrame morphFrame;				
			MotionExampleSet::blendMotionFrame(interpStartFrame,interpMotionFrame,morphWeight,morphFrame);				
			curReachIKOffset = (ikTarget - interpPos);
			curReachIKTrajectory = curEffectorPos + offset;
			interpStartFrame = morphFrame;
			interpMotionFrame = morphFrame;				
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
	float dt = 0.001f;
	float du = 0.0;
	if (prev_time == -1.0) // first start
	{
		dt = 0.001f;		
		// for first frame, update from frame buffer to joint quat in the limb
		// any future IK solving will simply use the joint quat from the previous frame.
		updateChannelBuffer(frame,idleMotionFrame,true);
		curEffectorPos = getCurrentHandPos(idleMotionFrame);	
		VecOfDouble para; 
		for (int i=0;i<3;i++)
			para.push_back(curEffectorPos[i]);
		if (interpMotion && dataInterpolator)
			dataInterpolator->predictInterpWeights(para,interpMotion->weight);
		curReachState = REACH_IDLE;

		reachTarget = (reachTargetJoint && useTargetJoint) ? reachTargetJoint->gmat().get_translation() : reachTargetPos;;
	}
	else
	{		
		dt = ((float)(t-prev_time));
	}
	prev_time = (float)t;		

	// To-Do (Wei-Wen) : these run-time steps seem more convoluted than it should be. 
	// For the things we want to achieve ( moving the hand to the target, stay there for some time, then return to the original location )
	// there seems to be better and more compact ways to handle them. For now I just separated the steps into some ugly functions, but later I should 
	// re-organize these parts ( maybe as a state machine ? ) for better code maintanence in the future.

	findReachTarget(ikTarget,reachError);

	bool interpHasReach = false;
	if (interpMotion && useInterpolation)
	{		
		interpHasReach = updateInterpolation(dt,interpMotionFrame,du);
		ikMotionFrame = interpMotionFrame;
	}	
	else // we don't have any data, so just infer the hand trajectory directly
	{
		 // assuming some constant hand moving speed
		SrVec reachDir = ikTarget - curEffectorPos;	
		{
			if (reachDir.norm() > reachVelocity*dt)
			{
				reachDir.normalize();
				reachDir = reachDir*reachVelocity*dt;
			}		
// 			sr_out << "reachDir = " << reachDir << srnl;
// 			sr_out << "currentPos = " << curEffectorPos << srnl;
			curReachIKTrajectory = curEffectorPos + reachDir;
		}			
		interpMotionFrame = idleMotionFrame;
		SrVec interpPos = getCurrentHandPos(interpMotionFrame);
	}

	//sr_out << "ik trajectory = " << curReachIKTrajectory << srnl;
	// for some reason, if I don't call this function at this time, some data is not updated or corrupted. There must be a bug here, but 
	// decided to put it on-hold and will fix it later.
	interpPos = getCurrentHandPos(interpMotionFrame);
	
	// update interpolated motion using IK constraint
	if (useIKConstraint)
	{
		updateIK(curReachIKTrajectory,interpMotionFrame,ikMotionFrame);					
	}	

	ikRoot = ikScenario.ikTreeRoot->gmat.get_translation();

	curEffectorPos = getCurrentHandPos(ikMotionFrame);

	bool ikHasReach = (useInterpolation && dataInterpolator) ? false : (curEffectorPos - ikTarget).norm() < 2.0;
	if ( ikHasReach || interpHasReach)
	{
		updateState();		
	}	

	prevPercentTime = curPercentTime;	
	reachCompleteTime += dt;
	if (curReachState == REACH_RETURN || curReachState == REACH_START)
		reachTime += du; // add the reference delta time

	updateChannelBuffer(frame,ikMotionFrame);
	return true;
}



void MeCtExampleBodyReach::updateState()
{
	if (curReachState == REACH_START)
	{
		// after touch the object, stay there for a pre-defined duration
		// the hand can still move around during this period
		curReachState = REACH_COMPLETE;
		interpStartFrame = interpMotionFrame;
		reachCompleteTime = 0.0;
		finishReaching = false;
	}
	else if (curReachState == REACH_COMPLETE && finishReaching)//&& reachCompleteTime >= reachCompleteDuration)
	{			
		curReachState = REACH_RETURN;	
		curPercentTime = 0.0;			
		finishReaching = false;
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
	VecOfDouble curReachPara;
	motionParameter->getPoseParameter(motionFrame,curReachPara);
	for (int i=0;i<3;i++)
		handPos[i] = (float)curReachPara[i];
	return handPos;
}

void MeCtExampleBodyReach::init()
{
	assert(skeletonRef);	
	// root is "world_offset", so we use root->child to get the base joint.
	SkJoint* rootJoint = skeletonRef->root()->child(0);
	ikScenario.buildIKTreeFromJointRoot(rootJoint);
	MeCtIKTreeNode* endNode = ikScenario.findIKTreeNode(reachEndEffector->name().get_string());
	//ikScenario.ikEndEffectors.push_back(endNode);

	EffectorConstantConstraint* cons = new EffectorConstantConstraint();
	cons->efffectorName = reachEndEffector->name().get_string();
	cons->rootName = "r_shoulder";//rootJoint->name().get_string();	
	reachPosConstraint[cons->efffectorName] = cons;

	ikScenario.ikPosEffectors = &reachPosConstraint;
	ikScenario.ikRotEffectors = &reachRotConstraint;
	
	const IKTreeNodeList& nodeList = ikScenario.ikTreeNodes;	
	idleMotionFrame.jointQuat.resize(nodeList.size());

	for (int i=0;i<3;i++)
		_channels.add(rootJoint->name().get_string(), (SkChannel::Type)(SkChannel::XPos+i));
	
	affectedJoints.clear();	
	for (unsigned int i=0;i<nodeList.size();i++)
	{
		SkJoint* joint = nodeList[i]->joint;
		affectedJoints.push_back(joint);	
		_channels.add(joint->name().get_string(), SkChannel::Quat);		
	}		

	motionParameter = new ReachMotionParameter(skeletonRef,affectedJoints,reachEndEffector);
	motionExamples.initMotionExampleSet(motionParameter);	
	MeController::init();	
}

void MeCtExampleBodyReach::updateMotionExamples( const MotionDataSet& inMotionSet )
{	
	if (inMotionSet.size() == 0)
		return;

	BOOST_FOREACH(SkMotion* motion, inMotionSet)
	{
		if (motionData.find(motion) != motionData.end())
			continue; // we do not process example motions that are already used for this controller instance
		if (!refMotion)
			refMotion = motion;

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
}

DataInterpolator* MeCtExampleBodyReach::createInterpolator()
{	
	KNNInterpolator* interpolator = new KNNInterpolator(1500,8.f);
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

	//BOOST_FOREACH(SrQuat& quat, motionFrame.jointQuat)
	for (unsigned int i=0;i<motionFrame.jointQuat.size();i++)
	{
		SrQuat& quat = motionFrame.jointQuat[i];		
		int index = frame.toBufferIndex(_toContextCh[count++]);	
		//printf("buffer index = %d\n",index);
		if (bRead)
		{
			quat.w = buffer[index] ;
			quat.x = buffer[index + 1] ;
			quat.y = buffer[index + 2] ;
			quat.z = buffer[index + 3] ;			
		}
		else
		{
// 			if (i >= motionFrame.jointQuat.size() - 33)
// 				continue;
			if (i==32)
				continue;

			buffer[index] = quat.w;
			buffer[index + 1] = quat.x;
			buffer[index + 2] = quat.y;
			buffer[index + 3] = quat.z;
			//frame.channelUpdated( count );
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