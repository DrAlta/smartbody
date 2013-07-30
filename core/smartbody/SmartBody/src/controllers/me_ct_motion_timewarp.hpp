#pragma once
#include "me_ct_data_driven_reach.hpp"
#include "sbm/sr_linear_curve.h"

/************************************************************************/
/* Time warp function maps a reference time to the actual time in a     */
/* motion. For warping a single motion, this reference time could be    */
/* just the real global time in the motion controller. For multiple     */
/* motion, a reference time system should be maintained to get the      */
/* consistent frames from all motions.                                  */
/************************************************************************/ 

class MotionTimeWarpFunc // motion time warp function interface, each motion should be associated with a time warping function
{
public:
	MotionTimeWarpFunc() {}
	virtual ~MotionTimeWarpFunc() {}

public:
	// t : actual time
	// u : reference time parameter for the time warp curve
	virtual double timeWarp(double u) = 0; // return t = S(u)
	virtual double invTimeWarp(double t) = 0; // return u = S_inv(t)
	virtual double timeSlope(double u) = 0; // return dS/du at ref time u			
	virtual double refTimeLength() = 0; // motion length in the reference time system
	virtual double actualTimeLength() = 0; // actual motion length
	
};

// simplest test time warp,
// just linearly warp the reference time duration to motion time duration
class SimpleTimeWarp : public MotionTimeWarpFunc 
{
protected:
	double refDuration, targetDuration;
	double slope, invSlope;
public:
	SimpleTimeWarp(double refLen, double targetLen);	
	virtual ~SimpleTimeWarp() {} ;
	virtual double timeWarp(double u) ;
	virtual double invTimeWarp(double t) ;
	virtual double timeSlope(double u) { return slope; }
	virtual double refTimeLength() { return refDuration; }
	virtual double actualTimeLength() { return targetDuration; }
	static double floatMod(double a, double b);	
};



