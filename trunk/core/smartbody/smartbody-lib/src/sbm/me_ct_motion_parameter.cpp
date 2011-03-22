#include "me_ct_motion_parameter.h"

MotionParameter::MotionParameter(SkSkeleton* skel, std::vector<SkJoint*>& joints)
{
	skeletonRef = skel;
	affectedJoints = joints;
}

MotionParameter::~MotionParameter(void)
{

}

ReachMotionParameter::ReachMotionParameter( SkSkeleton* skel, std::vector<SkJoint*>& joints, SkJoint* rjoint ) : MotionParameter(skel,joints)
{
	reachJoint = rjoint;	
}

ReachMotionParameter::~ReachMotionParameter()
{

}

void ReachMotionParameter::getPoseParameter( const BodyMotionFrame& frame, VecOfDouble& outPara )
{
	// set root 
	for (int i=0;i<3;i++)
		skeletonRef->root()->child(0)->pos()->value(i,frame.rootPos[i]);

	for (unsigned int i=0;i<affectedJoints.size();i++)
	{
		SkJoint* joint = affectedJoints[i];
		joint->quat()->value(frame.jointQuat[i]);
	}		
	skeletonRef->invalidate_global_matrices();
	skeletonRef->update_global_matrices();
	SrVec endPos = reachJoint->gmat().get_translation();
	outPara.resize(3);
	for (int i=0;i<3;i++)
		outPara[i] = endPos[i];
}

void ReachMotionParameter::getMotionParameter(BodyMotionInterface* motion, VecOfDouble& outPara )
{
	double timeRef = motion->motionDuration(BodyMotionInterface::DURATION_REF)*0.999;
	getMotionFrameParameter(motion,(float)timeRef,outPara);	
}

void ReachMotionParameter::getMotionFrameParameter( BodyMotionInterface* motion, float refTime, VecOfDouble& outPara )
{
	BodyMotionFrame tempFrame;
	motion->getMotionFrame((float)refTime,skeletonRef,affectedJoints,tempFrame);
	getPoseParameter(tempFrame,outPara);	
}