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
	{
		skeletonRef->root()->child(0)->pos()->value(i,frame.rootPos[i]);
	}

	for (unsigned int i=0;i<affectedJoints.size();i++)
	{
		SkJoint* joint = affectedJoints[i];
		joint->quat()->value(frame.jointQuat[i]);
		joint->update_lmat();
	}			
	skeletonRef->invalidate_global_matrices();
	//skeletonRef->update_global_matrices();
	reachJoint->update_lmat();
	reachJoint->update_gmat_up();
	//printf("reach joint name = %s\n",reachJoint->name().get_string());
	SrVec endPos = reachJoint->gmat().get_translation();
	outPara.resize(3);
	for (int i=0;i<3;i++)
		outPara[i] = endPos[i];
}

void ReachMotionParameter::getMotionParameter(BodyMotionInterface* motion, VecOfDouble& outPara )
{
	double timeRef = motion->strokeEmphasisTime();//motion->motionDuration(BodyMotionInterface::DURATION_REF)*0.999;
	//printf("timeRef = %f\n",timeRef);
	getMotionFrameParameter(motion,(float)timeRef,outPara);	
}

void ReachMotionParameter::getMotionFrameParameter( BodyMotionInterface* motion, float refTime, VecOfDouble& outPara )
{
	BodyMotionFrame tempFrame;
	motion->getMotionFrame((float)refTime,skeletonRef,affectedJoints,tempFrame);
// 	int noffset = 30;
// 	for (int i=noffset;i<5+noffset;i++)
// 		sr_out << "motion frame quat " << i+noffset << " : " << tempFrame.jointQuat[i+noffset] << srnl;
	getPoseParameter(tempFrame,outPara);	
}