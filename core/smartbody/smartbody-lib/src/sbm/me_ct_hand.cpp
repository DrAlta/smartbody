#include <assert.h>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <boost/foreach.hpp>
#include "mcontrol_util.h"
#include "me_ct_hand.hpp"

using namespace boost;

void GrabSphere::setSphere( const SrVec& pt, const float r )
{
	center = pt;
	radius = r;
}

SrVec GrabSphere::getCentroid()
{
	return center;
}

bool GrabSphere::isCollided( const SrVec& inPos )
{
	return (inPos-center).norm() < radius;
}

bool GrabSphere::isCollided( const SrVec& p1, const SrVec& p2 )
{
	// at least one point inside the sphere
	if ( (center-p1).norm() < radius || (p2-center).norm() < radius)
		return true;

	SrVec p2p1 = p2-p1;
	SrVec p3p1 = center-p1;
	float u = dot(p2p1,p3p1)/dot(p2p1,p2p1);
	if (u > 0.f && u < 1.f && (p1+p2p1*u-center).norm() < radius)
		return true;

	return false;
}

bool GrabSphere::isCollided( std::vector<SrVec>& lineSeg )
{
	for (unsigned int i=0;i<lineSeg.size()-1;i++)
	{
		if (isCollided(lineSeg[i],lineSeg[i+1]))
			return true;
	}
	return false;
}

/************************************************************************/
/* FingerChain                                                          */
/************************************************************************/

void FingerChain::init( MeCtIKTreeNode* figTip )
{
	fingerTip = figTip;
	MeCtIKTreeNode* node = fingerTip;
	fingerNodes.clear();
	while (node->parent != NULL)
	{
		fingerNodes.push_back(node);
		node = node->parent;
	}
	//fingerQuats.resize(fingerNodes.size());
}

void FingerChain::getLineSeg( std::vector<SrVec>& lineSeg )
{
	lineSeg.clear();
	for (unsigned int i=0;i<fingerNodes.size();i++)
	{
		lineSeg.push_back(fingerNodes[i]->gmat.get_translation());		
	}
}

/************************************************************************/
/* Hand Controller                                                      */
/************************************************************************/

const char* MeCtHand::CONTROLLER_TYPE = "Hand";

MeCtHand::MeCtHand( SkSkeleton* sk, SkJoint* wrist)
{		
	skeletonRef  = sk;	
	// work on the copy of skeleton to avoid problems
	skeletonCopy = new SkSkeleton(sk); 	
	if (wrist)
	{
		wristJoint = wrist;//skeletonCopy->search_joint(wrist->name().get_string());
	}	
	grabTarget = NULL;
	currentGrabState = GRAB_RETURN;
	grabVelocity = 6.f;
	_duration = -1.f;
}

MeCtHand::~MeCtHand( void )
{
	
}

void MeCtHand::setGrabState( GrabState state )
{
	if (currentGrabState != state)
	{
		for (int i=0;i<MeCtHand::F_NUM_FINGERS;i++)
		{
			FingerChain& fig = fingerChains[i];
			fig.isLock = false;
		}
	}
	currentGrabState = state;
}

void MeCtHand::setGrabTargetPos( SrVec& targetPos )
{
	static float radius = 7.f;
	GrabSphere* sphTarget = dynamic_cast<GrabSphere*>(grabTarget);
	if (sphTarget)
	{
		sphTarget->setSphere(targetPos,radius);
		setGrabState(GRAB_START);
	}
}

void MeCtHand::init(const MotionDataSet& reachPose, const MotionDataSet& grabPose)
{
	ikScenario.buildIKTreeFromJointRoot(wristJoint);
	fingerChains.resize(MeCtHand::F_NUM_FINGERS);
	for (unsigned int i=0;i<ikScenario.ikTreeNodes.size();i++)
	{
		MeCtIKTreeNode* node = ikScenario.ikTreeNodes[i];
		if (!node->child)
		{
			FingerID fID = findFingerID(node->nodeName.c_str());
			FingerChain& fchain = fingerChains[fID];
			fchain.init(node);			
			// add constraint
			EffectorConstantConstraint* cons = new EffectorConstantConstraint();
			cons->efffectorName = node->nodeName;
			cons->rootName = wristJoint->name().get_string();//"r_shoulder";//rootJoint->name().get_string();		
			handPosConstraint[cons->efffectorName] = cons;
		}
	}

	const IKTreeNodeList& nodeList = ikScenario.ikTreeNodes;		
	restFrame.jointQuat.resize(nodeList.size());
	currentFrame = restFrame;
	targetFrame  = restFrame;
	affectedJoints.clear();	
	for (unsigned int i=0;i<nodeList.size();i++)
	{
		MeCtIKTreeNode* node = nodeList[i];
		SkJoint* joint = skeletonCopy->linear_search_joint(node->nodeName.c_str());
		SkJointQuat* skQuat = joint->quat();		
		affectedJoints.push_back(joint);	
		_channels.add(joint->name().get_string(), SkChannel::Quat);		
	}	
	if (grabPose.size() > 0 && reachPose.size() > 0)
	{
		restFrame.setMotionPose(0.f,skeletonCopy,affectedJoints,*reachPose.begin());
		targetFrame.setMotionPose(0.f,skeletonCopy,affectedJoints,*grabPose.begin());
	}
	else
	{
		getPinchFrame(targetFrame,SrVec(-8,-6,0));
	}
	grabTarget = new GrabSphere(); // hard coded to sphere for now		
}

void MeCtHand::getPinchFrame( BodyMotionFrame& pinchFrame, SrVec& wristOffset )
{
	ikScenario.updateNodeGlobalMat(ikScenario.ikTreeRoot,QUAT_INIT);	
	SrVec wristPos = ikScenario.ikTreeRoot->gmat.get_translation();
	SrVec ikTarget = wristPos + wristOffset*ikScenario.ikTreeRoot->gmat.get_rotation();
	for (int i=0;i<MeCtHand::F_NUM_FINGERS;i++)
	{
		FingerChain& fig = fingerChains[i];
		MeCtIKTreeNode* node = fig.fingerTip;
		EffectorConstantConstraint* cons = dynamic_cast<EffectorConstantConstraint*>(handPosConstraint[node->nodeName]);
		cons->targetPos = ikTarget;		
	}
	for (int i=0;i<100;i++)
		solveIK(1.f);
	ikScenario.getTreeNodeQuat(pinchFrame.jointQuat,QUAT_CUR);
}

void MeCtHand::solveIK(float dt)
{
	ikScenario.setTreeNodeQuat(restFrame.jointQuat,QUAT_REF);								
	ikScenario.ikPosEffectors = &handPosConstraint;
	ikScenario.ikRotEffectors = &handRotConstraint;
	ik.maxOffset = grabVelocity*dt;
	ik.dampJ = 0.5f;
	ik.refDampRatio = 0.1;
	for (int i=0;i<1;i++)
	{
		ik.update(&ikScenario);		
		ikScenario.copyTreeNodeQuat(QUAT_CUR,QUAT_INIT);		
	}
}

bool MeCtHand::controller_evaluate( double t, MeFrameData& frame )
{	
	float dt = 0.001f;
	float du = 0.0;
	if (prevTime == -1.0) // first start
	{
		dt = 0.001f;
		updateChannelBuffer(frame,currentFrame,true);
	}
	else
	{		
		dt = ((float)(t-prevTime));
	}
	prevTime = (float)t;

	static bool bInit = false;
	if (!bInit)
	{			
		ikScenario.setTreeNodeQuat(restFrame.jointQuat,QUAT_INIT);
		ikScenario.setTreeNodeQuat(restFrame.jointQuat,QUAT_CUR);		
		bInit = true;
	}

	BodyMotionFrame& curTargetFrame = currentGrabState == GRAB_START ? targetFrame : restFrame;

	updateFingerChains(curTargetFrame,0.01f);
	skeletonRef->invalidate_global_matrices();
	skeletonRef->update_global_matrices();
	const char* rootName = ikScenario.ikTreeRoot->joint->parent()->name().get_string();
	ikScenario.ikGlobalMat = skeletonRef->search_joint(rootName)->gmat();
	ikScenario.updateNodeGlobalMat(ikScenario.ikTreeRoot,QUAT_CUR);

	for (int i=0;i<MeCtHand::F_NUM_FINGERS;i++)
	{
		FingerChain& fig = fingerChains[i];
		MeCtIKTreeNode* node = fig.fingerTip;				
		//SrVec curPos = node->gmat.get_translation();		
		std::vector<SrVec> chainSeg;
		fig.getLineSeg(chainSeg);
		if (!fig.isLock && currentGrabState == GRAB_START && grabTarget->isCollided(chainSeg))		
		{			
			fig.isLock = true;				
		}		
	}
	
	BodyMotionFrame outMotionFrame = restFrame;
	ikScenario.getTreeNodeQuat(outMotionFrame.jointQuat,QUAT_CUR);
	updateChannelBuffer(frame,outMotionFrame);
	return true;
}

void MeCtHand::updateFingerChains( BodyMotionFrame& targetMotionFrame, float maxAngDelta )
{	
	for (unsigned int i=0;i<fingerChains.size();i++)
	{
		FingerChain& fig = fingerChains[i];
		if (fig.isLock)
			continue;
		for (unsigned int k=0;k<fig.fingerNodes.size();k++)
		{
			MeCtIKTreeNode* node = fig.fingerNodes[k];
			SrQuat q = node->getQuat();
			SrQuat qT = targetMotionFrame.jointQuat[node->nodeIdx];
			SrQuat diff = qT*q.inverse();
			diff.normalize();
			float angle = diff.angle();
			if (angle > maxAngDelta)
				angle = maxAngDelta;
			SrQuat newQ;
			if (fabs(angle) < 0.001)
				newQ = qT;
			else
				newQ = SrQuat(diff.axis(),angle)*q;
			//newQ.normalize();
			node->setQuat(newQ);
		}
	}	
}
void MeCtHand::updateChannelBuffer( MeFrameData& frame, BodyMotionFrame& handMotionFrame, bool bRead /*= false*/ )
{
	SrBuffer<float>& buffer = frame.buffer();
	int count = 0;
	// update root translation
	if (handMotionFrame.jointQuat.size() != affectedJoints.size())
		handMotionFrame.jointQuat.resize(affectedJoints.size());
	//BOOST_FOREACH(SrQuat& quat, motionFrame.jointQuat)
	for (unsigned int i=0;i<handMotionFrame.jointQuat.size();i++)
	{
		SrQuat& quat = handMotionFrame.jointQuat[i];		
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


void MeCtHand::print_state( int tabs )
{

}

void MeCtHand::controller_start()
{

}

void MeCtHand::controller_map_updated()
{

}

MeCtHand::FingerID MeCtHand::findFingerID( const char* jointName )
{
	const char fingerName[][20] = {"thumb", "index", "middle", "ring", "pinky"};
	std::string jointStr = jointName;
	for (int i=0;i<5;i++)
	{
		size_t found = jointStr.find(fingerName[i]);
		if (found != std::string::npos)
			return (MeCtHand::FingerID)i;
	}
	return MeCtHand::F_THUMB;
}