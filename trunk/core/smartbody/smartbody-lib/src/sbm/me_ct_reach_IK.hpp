#pragma once
#include "me_ct_ik.hpp"

class MeCtReachIK :
	public MeCtIK
{
public:
	MeCtReachIK(void);
	~MeCtReachIK(void);
public:
	virtual void update(MeCtIKScenario* scenario);
	virtual void calc_target(SrVec& orientation, SrVec& offset);
};
