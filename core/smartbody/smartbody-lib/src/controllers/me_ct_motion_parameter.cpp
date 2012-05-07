#include "controllers/me_ct_motion_parameter.h"

MotionParameter::MotionParameter(SkSkeleton* skel, std::vector<SkJoint*>& joints)
{
	skeletonRef = skel;
	affectedJoints = joints;
}

MotionParameter::~MotionParameter(void)
{

}

SkJoint* MotionParameter::getMotionFrameJoint( const BodyMotionFrame& frame, const char* jointName )
{
	SkJoint* outJoint = NULL;	
	for (int i=0;i<3;i++)
	{		
		//skeletonRef->->pos()->value(i,frame.rootPos[i]);
		affectedJoints[0]->pos()->value(i,frame.rootPos[i]);
	}			
	for (unsigned int i=0;i<affectedJoints.size();i++)
	{
		SkJoint* joint = affectedJoints[i];				
		joint->quat()->value(frame.jointQuat[i]);
		joint->update_lmat();
		if (strcmp(joint->name().c_str(),jointName) == 0)
			outJoint = joint;
	}			
	skeletonRef->invalidate_global_matrices();
	skeletonRef->update_global_matrices();
	return outJoint;
}

ReachMotionParameter::ReachMotionParameter( SkSkeleton* skel, std::vector<SkJoint*>& joints, SkJoint* rjoint, SkJoint* root ) : MotionParameter(skel,joints)
{
	reachJoint = rjoint;	
	rootJoint = root;
}

ReachMotionParameter::~ReachMotionParameter()
{

}

void ReachMotionParameter::getPoseParameter( const BodyMotionFrame& frame, dVector& outPara )
{
	// set root 		
	SkJoint* rJoint = getMotionFrameJoint(frame,reachJoint->name().c_str());	
	rJoint->update_lmat();
	rJoint->update_gmat_up();
	skeletonRef->update_global_matrices();
	//printf("reach joint name = %s\n",reachJoint->name().get_string());
	SrMat ginv = skeletonRef->search_joint(rootJoint->name().c_str())->gmat().inverse();
	SrVec endPos = rJoint->gmat().get_translation()*ginv;
	outPara.resize(3);
	for (int i=0;i<3;i++)
		outPara[i] = endPos[i];	
}

void ReachMotionParameter::getMotionParameter(BodyMotionInterface* motion, dVector& outPara )
{
	double timeRef = motion->strokeEmphasisTime();//motion->motionDuration(BodyMotionInterface::DURATION_REF)*0.999;
	//printf("timeRef = %f\n",timeRef);
	getMotionFrameParameter(motion,(float)timeRef,outPara);	
}

void ReachMotionParameter::getMotionFrameParameter( BodyMotionInterface* motion, float refTime, dVector& outPara )
{
	BodyMotionFrame tempFrame;
	motion->getMotionFrame((float)refTime,skeletonRef,affectedJoints,tempFrame);
// 	int noffset = 30;
// 	for (int i=noffset;i<5+noffset;i++)
// 		sr_out << "motion frame quat " << i+noffset << " : " << tempFrame.jointQuat[i+noffset] << srnl;
	getPoseParameter(tempFrame,outPara);	
}