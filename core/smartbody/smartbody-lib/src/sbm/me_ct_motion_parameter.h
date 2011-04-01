#pragma once
#include "me_ct_data_driven_reach.hpp"
#include "me_ct_motion_example.hpp"

class MotionParameter
{
public:
	SkSkeleton* skeletonRef;
	std::vector<SkJoint*> affectedJoints;
public:
	MotionParameter(SkSkeleton* skel, std::vector<SkJoint*>& joints);
	~MotionParameter(void);

	virtual void getPoseParameter(const BodyMotionFrame& frame, dVector& outPara) = 0;
	virtual void getMotionFrameParameter(BodyMotionInterface* motion, float refTime, dVector& outPara) = 0;
	virtual void getMotionParameter(BodyMotionInterface* motion, dVector& outPara) = 0;
	virtual SkJoint* getMotionFrameJoint(const BodyMotionFrame& frame, const char* jointName);
};


class ReachMotionParameter : public MotionParameter
{
protected:
	SkJoint* reachJoint;
public:
	ReachMotionParameter(SkSkeleton* skel, std::vector<SkJoint*>& joints, SkJoint* rjoint);
	~ReachMotionParameter();

	virtual void getPoseParameter(const BodyMotionFrame& frame, dVector& outPara);
	virtual void getMotionParameter(BodyMotionInterface* motion, dVector& outPara);
	virtual void getMotionFrameParameter(BodyMotionInterface* motion, float refTime, dVector& outPara);	
};
