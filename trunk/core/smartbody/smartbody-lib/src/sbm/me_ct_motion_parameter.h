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

	virtual void getPoseParameter(const BodyMotionFrame& frame, VecOfDouble& outPara) = 0;
	virtual void getMotionFrameParameter(BodyMotionInterface* motion, float refTime, VecOfDouble& outPara) = 0;
	virtual void getMotionParameter(BodyMotionInterface* motion, VecOfDouble& outPara) = 0;
};


class ReachMotionParameter : public MotionParameter
{
protected:
	SkJoint* reachJoint;
public:
	ReachMotionParameter(SkSkeleton* skel, std::vector<SkJoint*>& joints, SkJoint* rjoint);
	~ReachMotionParameter();

	virtual void getPoseParameter(const BodyMotionFrame& frame, VecOfDouble& outPara);
	virtual void getMotionParameter(BodyMotionInterface* motion, VecOfDouble& outPara);
	virtual void getMotionFrameParameter(BodyMotionInterface* motion, float refTime, VecOfDouble& outPara);
};
