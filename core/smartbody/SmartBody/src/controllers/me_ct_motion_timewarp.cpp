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

	if (u > refDuration)
		u = refDuration - gwiz::epsilon10();
// 	u = floatMod(u,refDuration*2.0);
// 	if (u > refDuration)
// 		u = refDuration*2.0 - u;

	if (u <= 0.0)
		u = 0.0;

	double mu = u;//floatMod(u,refDuration);
	return mu*slope;
}

double SimpleTimeWarp::invTimeWarp( double t )
{
	if (t > targetDuration)
		t = targetDuration - gwiz::epsilon10();
	if (t < 0.0)
		t = 0.0;
// 	t = floatMod(t,targetDuration*2.0);
// 	if (t > targetDuration)
// 		t = targetDuration*2.0 - t;

	double mt = t;//floatMod(t,targetDuration);
	return mt*invSlope;
}

double SimpleTimeWarp::floatMod( double a, double b )
{
	int result = static_cast<int>( a / b );
	return a - static_cast<double>( result ) * b;
}
