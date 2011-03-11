#include <assert.h>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <boost/foreach.hpp>
#include "me_ct_example_body_reach.hpp"

using namespace boost;

/************************************************************************/
/* Exampled-Based Reach Controller                                      */
/************************************************************************/

const char* MeCtExampleBodyReach::CONTROLLER_TYPE = "BodyReach";

const char* endEffectorName = "r_wrist_tip"; // a hard coded hack

MeCtExampleBodyReach::MeCtExampleBodyReach( SkSkeleton* sk )
{
	skeletonRef = sk;
	prev_time = -1.0;
	dataInterpolator = NULL;
	refMotion = NULL;
	reachTime = 0.0;
	reachRefTime = 0.0;

	useIKConstraint = false;

	reachEndEffector = skeletonRef->search_joint(endEffectorName);	
	reachTargetJoint = NULL;
	interpMotion = NULL;
	motionParameter = NULL;

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

// compute the blending weights based on reach position
void MeCtExampleBodyReach::setReachTarget( SrVec& reachPos )
{
	vector<double> para; para.resize(3);
	for (int i=0;i<3;i++)
		para[i] = reachPos[i];
	if (dataInterpolator && interpMotion)
	{
		dataInterpolator->predictInterpWeights(para,interpMotion->weight);	
 		//for (int i=0;i<interpMotion->weight.size();i++)
 		//	printf("w %d = %f, ",interpMotion->weight[i].first,interpMotion->weight[i].second);
 		//printf("\n");
		//if (interpMotion->weight[0].second == 0.f)
		//	printf("w0 = %f, w1 = %f\n",interpMotion->weight[0].second,interpMotion->weight[1].second);

		VecOfDouble acutualReachTarget;
		interpMotion->getExampleParameter(acutualReachTarget);
		for (int i=0;i<3;i++)
			reachError[i] = reachPos[i] - acutualReachTarget[i];
		
	}	
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
	}
	else
	{		
		dt = ((float)(t-prev_time));
	}
	prev_time = (float)t;		
	BodyMotionFrame outMotionFrame;	

	if (reachTargetJoint)
	{
		SrVec reachPos = reachTargetJoint->gmat().get_translation();
		setReachTarget(reachPos);
	}	
	
	if (interpMotion)
	{
		interpMotion->getMotionFrame(reachTime,skeletonRef,affectedJoints,outMotionFrame);
		curPercentTime = interpMotion->motionPercent(reachTime);
		if (curPercentTime < prevPercentTime) // loop back to beginning
		{
			curReachOffset = reachError*curPercentTime;
			ikScenario.ikInitQuatList = outMotionFrame.jointQuat;
		}
		else
		{
			double deltaPercent = (curPercentTime-prevPercentTime);
			double ratio = 1.0/(1.0-prevPercentTime);
			curReachOffset += (reachError - curReachOffset)*ratio*deltaPercent;
		}

		VecOfDouble curReachPos;
		motionParameter->getPoseParameter(outMotionFrame,curReachPos);
		for (int i=0;i<3;i++)
			curReachTrajectory[i] = curReachPos[i] + curReachOffset[i];
		du = interpMotion->getRefDeltaTime(reachTime,dt);
	}

	static bool bInit = false;
	// update interpolated motion using IK constraint
	if (useIKConstraint)
	{
		if (!bInit)
		{
			ikScenario.ikInitQuatList = outMotionFrame.jointQuat;
			bInit = true;
		}

		ikScenario.ikEndEffectors[0]->targetPos = curReachTrajectory;
		ikScenario.ikRefQuatList = outMotionFrame.jointQuat;
		ik.setDt(dt);
		ik.update(&ikScenario);
		ikScenario.ikInitQuatList = ikScenario.ikQuatList;
		outMotionFrame.jointQuat = ikScenario.ikQuatList;
		//sr_out << "joint quat = " << ikScenario.ikQuatList[2] << srnl;
	}	
	
	prevPercentTime = curPercentTime;	
	reachTime += du; // add the reference delta time
	updateChannelBuffer(frame,outMotionFrame);
	return true;
}

void MeCtExampleBodyReach::init()
{
	assert(skeletonRef);	
	// root is "world_offset", so we use root->child to get the base joint.
	SkJoint* rootJoint = skeletonRef->root()->child(0);
	ikScenario.buildIKTreeFromJointRoot(rootJoint);
	MeCtIKTreeNode* endNode = ikScenario.findIKTreeNode(reachEndEffector->name().get_string());
	ikScenario.ikEndEffectors.push_back(endNode);

	affectedJoints.clear();	
	const IKTreeNodeList& nodeList = ikScenario.ikTreeNodes;
	
	for (int i=0;i<3;i++)
		_channels.add(rootJoint->name().get_string(), (SkChannel::Type)(SkChannel::XPos+i));
	
	for (int i=0;i<nodeList.size();i++)
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
			reachPos[i] = ex->parameter[i];
		examplePts.push_back(reachPos);		
		motionExamples.addMotionExample(ex);
	}	

	if (dataInterpolator)
		delete dataInterpolator;

	dataInterpolator = createInterpolator();
	dataInterpolator->init(&motionExamples);
	dataInterpolator->buildInterpolator();	
	
	for (int i=0;i<resampleData->size();i++)
	{
		InterpolationExample* ex = (*resampleData)[i];
		SrVec reachPos;
		for (int k=0;k<3;k++)
			reachPos[k] = ex->parameter[k];
		resamplePts.push_back(reachPos);
	}

	if (interpMotion)
		delete interpMotion;
	interpMotion = createInterpMotion();	
}

DataInterpolator* MeCtExampleBodyReach::createInterpolator()
{	
	KNNInterpolator* interpolator = new KNNInterpolator(3000,8.f);
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

void MeCtExampleBodyReach::getParameter(VecOfSrQuat& quatList, float time, VecOfDouble& outPara )
{	
	for (int i=0;i<affectedJoints.size();i++)
	{
		SkJoint* joint = affectedJoints[i];
		joint->quat()->value(quatList[i]);
	}		
	skeletonRef->invalidate_global_matrices();
	skeletonRef->update_global_matrices();

	SrVec endPos = reachEndEffector->gmat().get_translation();
	examplePts.push_back(endPos);
	outPara.resize(3);
	for (int i=0;i<3;i++)
		outPara[i] = endPos[i];
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