#include "SBTransition.h"
namespace SmartBody {

SBTransition::SBTransition() : PATransitionData()
{
}

SBTransition::SBTransition(std::string name) : PATransitionData()
{
}

SBTransition::~SBTransition()
{
}

void SBTransition::set(SBState* source, SBState* dest)
{
	fromState = source;
	toState = dest;
}

void SBTransition::addCorrespondancePoint(std::string  sourceMotion, std::string destMotion, float sourceFromTime, float sourceToTime, float destFromTime, float destToTime)
{
	fromMotionName = sourceMotion;
	toMotionName = destMotion;

	easeOutStart.push_back(sourceFromTime);
	easeOutEnd.push_back(sourceToTime);
	easeInStart = destFromTime;
	easeInEnd = destToTime;
}

int SBTransition::getNumCorrespondancePoints()
{
	return 0;
}

std::vector<float> SBTransition::getCorrespondancePoint(int num)
{
	return std::vector<float>();
}

SBState* SBTransition::getFromState()
{
	SBState* from = dynamic_cast<SBState*>(fromState);
	return from;
}

SBState* SBTransition::getToState()
{
	SBState* to = dynamic_cast<SBState*>(toState);
	return to;
}

}