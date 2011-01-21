#pragma once
#include "me_ct_ik.hpp"
#include "gwiz_math.h"

class MeCtReachIK :
	public MeCtIK
{
public:
	MeCtReachIK(void);
	~MeCtReachIK(void);
public:	
	virtual void update(MeCtIKScenario* scenario);

public:
	static vector_t quat2SwingTwist(quat_t& quat);
	static quat_t   swingTwist2Quat(vector_t& quat);
	float getDt() const { return dt; }
	void setDt(float val) { dt = val; }

protected:
	virtual void adjust();		
	virtual void calc_target(SrVec& orientation, SrVec& offset);	
	int check_joint_limit(SrQuat* quat, int index); // new routine for most detailed joint angle limit constraints
	void ccdRotate(SrVec& src, int start_index); // use cyclic coordinate descend to find local minimum.	
protected: // a hack to get rotation axis re-align	
	float dt; // for constraining rotation speed
};
