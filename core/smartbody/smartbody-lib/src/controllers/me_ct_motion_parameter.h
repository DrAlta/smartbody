#pragma once
#include "controllers/me_ct_data_driven_reach.hpp"
#include "controllers/me_ct_motion_example.hpp"

class MotionParameter
{
public:
	SkSkeleton* skeletonRef;
	std::vector<SkJoint*> affectedJoints;
public:
	MotionParameter(SkSkeleton* skel, std::vector<SkJoint*>& joints);
	virtual ~MotionParameter(void);

	virtual void getPoseParameter(const BodyMotionFrame& frame, dVector& outPara) = 0;
	virtual void getMotionFrameParameter(BodyMotionInterface* motion, float refTime, dVector& outPara) = 0;
	virtual void getMotionParameter(BodyMotionInterface* motion, dVector& outPara) = 0;
	virtual SkJoint* getMotionFrameJoint(const BodyMotionFrame& frame, const char* jointName);
};


class ReachMotionParameter : public MotionParameter
{
protected:
	SkJoint* reachJoint;
	SkJoint* rootJoint;
public:
	ReachMotionParameter(SkSkeleton* skel, std::vector<SkJoint*>& joints, SkJoint* rjoint, SkJoint* rootJoint);
	virtual ~ReachMotionParameter();

	virtual void getPoseParameter(const BodyMotionFrame& frame, dVector& outPara);
	virtual void getMotionParameter(BodyMotionInterface* motion, dVector& outPara);
	virtual void getMotionFrameParameter(BodyMotionInterface* motion, float refTime, dVector& outPara);	
};

class LocomotionParameter : public MotionParameter
{
protected:
	std::string baseJointName;	
public:
	LocomotionParameter(SkSkeleton* skel, std::vector<SkJoint*>& joints, const std::string& baseName);
	~LocomotionParameter();

	virtual void getPoseParameter(const BodyMotionFrame& frame, dVector& outPara);
	virtual void getMotionFrameParameter(BodyMotionInterface* motion, float refTime, dVector& outPara);
	virtual void getMotionParameter(BodyMotionInterface* motion, dVector& outPara);	

protected:
	float getMotionSpeed( BodyMotionInterface* motion, const std::string& jointName);
	float getMotionAngularSpeed( BodyMotionInterface* motion, const std::string& jointName);
	float getMotionSpeedAxis( BodyMotionInterface* motion, const std::string& axis, const std::string& jointName);

};

class JumpParameter : public MotionParameter
{
protected:
	std::string baseJointName;	
public:
	JumpParameter(SkSkeleton* skel, std::vector<SkJoint*>& joints, const std::string& baseName);
	~JumpParameter();

	virtual void getPoseParameter(const BodyMotionFrame& frame, dVector& outPara);
	virtual void getMotionFrameParameter(BodyMotionInterface* motion, float refTime, dVector& outPara);
	virtual void getMotionParameter(BodyMotionInterface* motion, dVector& outPara);	

protected:
	float getMotionHeight( BodyMotionInterface* motion, const std::string& jointName);
	float getMotionDirection( BodyMotionInterface* motion, const std::string& jointName);
	float getMotionXZDistance( BodyMotionInterface* motion, const std::string& jointName);

};

