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

const char* endEffectorName = "r_wrist"; // a hard coded hack

MeCtExampleBodyReach::MeCtExampleBodyReach( SkSkeleton* sk )
{
	skeletonRef = sk;
	prev_time = -1.0;
	dataInterpolator = NULL;
	refMotion = NULL;
	reachTime = 0.0;	
	useIKConstraint = true;
	useInterpolation = true;
	reachEndEffector = skeletonRef->search_joint(endEffectorName);	

	curReachState = REACH_START;
	

	reachTargetJoint = NULL;
	interpMotion = NULL;
	motionParameter = NULL;

	reachVelocity = 50.f;
	reachCompleteDuration = 2.0f;
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
}

// compute the blending weights based on reach position
void MeCtExampleBodyReach::setReachTarget( SrVec& reachPos )
{
	vector<double> para; para.resize(3);
	for (int i=0;i<3;i++)
		para[i] = reachPos[i];

	SrVec currentTarget = reachEndEffector->gmat().get_translation();
	if (dataInterpolator && interpMotion && useInterpolation)
	{			
		VecOfDouble acutualReachTarget;
		if (curReachState == REACH_START || curReachState == REACH_COMPLETE)
		{
			dataInterpolator->predictInterpWeights(para,interpMotion->weight);	 	
			interpMotion->getExampleParameter(acutualReachTarget);	
		}
		else if (curReachState == REACH_RETURN || curReachState == REACH_IDLE)
		// this is a hack to get the initial hand position
		{			
			motionParameter->getMotionFrameParameter(interpMotion,0.f,acutualReachTarget);
		}

		for (int i=0;i<3;i++)
			currentTarget[i] = (float)acutualReachTarget[i];
	}	
	// By default, the reach error is the offset we need to move the end effector to the target
	// If we have an interpolated motion, then the error is simply the difference to the desired target.
	reachError = reachPos - currentTarget;	
}

bool MeCtExampleBodyReach::controller_evaluate( double t, MeFrameData& frame )
{
	float dt = 0.001f;
	double du = 0.0;
	if (prev_time == -1.0) // first start
	{
		dt = 0.001f;		
		// for first frame, update from frame buffer to joint quat in the limb
		// any future IK solving will simply use the joint quat from the previous frame.
		updateChannelBuffer(frame,currentMotionFrame,true);
		curEffectorPos = getCurrentHandPos(currentMotionFrame);		
		curReachState = REACH_RETURN;
	}
	else
	{		
		dt = ((float)(t-prev_time));
	}
	prev_time = (float)t;		

	updateChannelBuffer(frame,currentMotionFrame,true);


	// To-Do (Wei-Wen) : these run-time steps seem more convoluted than it should be. 
	// For the things we want to achieve ( moving the hand to the target, stay there for some time, then return to the original location )
	// there seems to be better and more compact ways to handle them. For now things work fine, but later I should re-organize these parts
	// for better code maintainence in the future.
	if (reachTargetJoint)
	{
		SrVec newReachTarget = reachTargetJoint->gmat().get_translation();

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
			ikTarget = reachTarget;		
		}
		else if (curReachState == REACH_RETURN)
		{
			if (interpMotion && useInterpolation)
			{
				VecOfDouble initPos;
				motionParameter->getMotionFrameParameter(interpMotion,0.f,initPos);
				for (int i=0;i<3;i++)
					returnTarget[i] = (float)initPos[i];
			}
			else
			{
				returnTarget = getCurrentHandPos(currentMotionFrame);
			}
			ikTarget = returnTarget;
		}

		setReachTarget(ikTarget);
	}	
	
	bool interpHasReach = false;	
	if (interpMotion && useInterpolation)
	{		
		interpMotion->getMotionFrame(reachTime,skeletonRef,affectedJoints,currentMotionFrame);
		curPercentTime = interpMotion->motionPercent(reachTime);		
		{
			double deltaPercent = fabs(curPercentTime-prevPercentTime);
			double timeRemain = 1.0;
			if (curReachState == REACH_START) 
				timeRemain = 1.0 - prevPercentTime;
			else if (curReachState == REACH_RETURN)
				timeRemain = prevPercentTime;

			if (timeRemain <= 0.05 || curReachState == REACH_COMPLETE || curReachState == REACH_IDLE)
				interpHasReach = true;

			double ratio = 0.0;
			if (timeRemain > 0.0)
			    ratio = 1.0/timeRemain;

			SrVec interpPos = getCurrentHandPos(currentMotionFrame);
			
			if ( curReachState == REACH_COMPLETE )
			// use normal IK to compute the hand movement after touching the object
			{				
				SrVec offset = (ikTarget - curEffectorPos);
				if (offset.norm() > reachVelocity*dt)
				{
					offset.normalize();
					offset = offset*reachVelocity*dt;
				}
				//offset.normalize();					
				curReachIKTrajectory = curEffectorPos + offset;// + offset;//curReachOffset;					
				curReachIKOffset += (reachError - curReachIKOffset);
			}
			else
			{
				curReachIKOffset += (reachError - curReachIKOffset)*(float)ratio*(float)deltaPercent;				
				{
					curEffectorPos = 	interpPos;
					curReachIKTrajectory = curEffectorPos + curReachIKOffset;
				}				
			}			
		}		
		du = interpMotion->getRefDeltaTime(reachTime,dt);
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
			curReachIKTrajectory = curEffectorPos + reachDir;
		}			
	}

	static bool bInit = false;
	// update interpolated motion using IK constraint
	if (useIKConstraint)
	{
		if (!bInit)
		{			
			ikScenario.setTreeNodeQuat(currentMotionFrame.jointQuat,QUAT_INIT);
			bInit = true;
		}		
		EffectorConstantConstraint* cons = dynamic_cast<EffectorConstantConstraint*>(reachPosConstraint[reachEndEffector->name().get_string()]);
		cons->targetPos = curReachIKTrajectory;
		if (useInterpolation) // use more damping if we are interpolating from the motion
		{
			ik.dampJ = 150.0;
			ik.refDampRatio = 0.1;	
			cons->rootName = "";
		}
		else
		{
			ik.dampJ = 150.0;
			ik.refDampRatio = 0.1;
			cons->rootName = "";
		}

		skeletonRef->invalidate_global_matrices();
		skeletonRef->update_global_matrices();
		ikScenario.ikGlobalMat = ikScenario.ikTreeRoot->joint->parent()->gmat();		
		ikScenario.setTreeNodeQuat(currentMotionFrame.jointQuat,QUAT_REF);
		ik.setDt(dt);
		ik.update(&ikScenario);		
		ikScenario.copyTreeNodeQuat(QUAT_CUR,QUAT_INIT);
		ikScenario.getTreeNodeQuat(currentMotionFrame.jointQuat,QUAT_CUR);	
		
	}	

	curEffectorPos = getCurrentHandPos(currentMotionFrame);

	bool ikHasReach = useInterpolation ? false : (curEffectorPos - ikTarget).norm() < 2.0;
	if ( ikHasReach || interpHasReach)
	{
		if (curReachState == REACH_START)
		{
			// after touch the object, stay there for a pre-defined duration
			// the hand can still move around during this period
			curReachState = REACH_COMPLETE;
			reachCompleteTime = 0.0;
		}
		else if (curReachState == REACH_COMPLETE && reachCompleteTime >= reachCompleteDuration)
		{			
			curReachState = REACH_RETURN;			
		}
		else if (curReachState == REACH_RETURN)
		{
			// stay idle in the current place
			curReachState = REACH_IDLE;
		}
	}

	prevPercentTime = curPercentTime;	
	reachCompleteTime += dt;

	if (curReachState == REACH_RETURN || curReachState == REACH_START)
		reachTime += (float)du; // add the reference delta time
	updateChannelBuffer(frame,currentMotionFrame);
	
	return true;
}



SrVec MeCtExampleBodyReach::getCurrentHandPos( BodyMotionFrame& motionFrame )
{
	SrVec handPos;
	VecOfDouble curReachPara;
	motionParameter->getPoseParameter(currentMotionFrame,curReachPara);
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
	cons->efffectorName = endEffectorName;
	cons->rootName = "";//"r_shoulder";//rootJoint->name().get_string();	
	reachPosConstraint[cons->efffectorName] = cons;

	ikScenario.ikPosEffectors = &reachPosConstraint;
	ikScenario.ikRotEffectors = &reachRotConstraint;
	
	const IKTreeNodeList& nodeList = ikScenario.ikTreeNodes;
	currentMotionFrame.jointQuat.resize(nodeList.size());

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
	KNNInterpolator* interpolator = new KNNInterpolator(500,8.f);
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
		}
	}

	BOOST_FOREACH(SrQuat& quat, motionFrame.jointQuat)
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

void MeCtExampleBodyReach::print_state( int tabs )
{

}

void MeCtExampleBodyReach::controller_start()
{

}

void MeCtExampleBodyReach::controller_map_updated()
{

}