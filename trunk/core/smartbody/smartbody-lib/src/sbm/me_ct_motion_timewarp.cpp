#include <assert.h>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <boost/foreach.hpp>
#include "me_ct_motion_timewarp.hpp"

using namespace boost;


SimpleTimeWarp::SimpleTimeWarp( double refLen, double targetLen )
{
	refDuration = refLen;
	targetDuration = targetLen;
	slope = targetDuration/refDuration;
	invSlope = refDuration/targetDuration;
}

double SimpleTimeWarp::timeWarp( double u )
{
	//if (u > refDuration)
	//	u = refDuration - GWIZ::epsilon10();

	float mu = floatMod(u,refDuration);
	return mu*slope;
}

double SimpleTimeWarp::invTimeWarp( double t )
{
	float mt = floatMod(t,targetDuration);
	return mt*invSlope;
}

double SimpleTimeWarp::floatMod( double a, double b )
{
	int result = static_cast<int>( a / b );
	return a - static_cast<double>( result ) * b;
}