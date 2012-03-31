#include "SBAnimationTransition.h"
#include <sbm/SBAnimationState.h>

namespace SmartBody {

SBAnimationTransition::SBAnimationTransition() : PATransitionData()
{
}

SBAnimationTransition::SBAnimationTransition(std::string name) : PATransitionData()
{
}

SBAnimationTransition::~SBAnimationTransition()
{
}

void SBAnimationTransition::set(SBAnimationState* source, SBAnimationState* dest)
{
	fromState = source;
	toState = dest;
}

void SBAnimationTransition::addCorrespondancePoint(std::string  sourceMotion, std::string destMotion, float sourceFromTime, float sourceToTime, float destFromTime, float destToTime)
{
	fromMotionName = sourceMotion;
	toMotionName = destMotion;

	easeOutStart.push_back(sourceFromTime);
	easeOutEnd.push_back(sourceToTime);
	easeInStart = destFromTime;
	easeInEnd = destToTime;
}

int SBAnimationTransition::getNumCorrespondancePoints()
{
	return 0;
}

std::vector<float> SBAnimationTransition::getCorrespondancePoint(int num)
{
	return std::vector<float>();
}

SBAnimationState* SBAnimationTransition::getFromState()
{
	SBAnimationState* from = dynamic_cast<SBAnimationState*>(fromState);
	return from;
}

SBAnimationState* SBAnimationTransition::getToState()
{
	SBAnimationState* to = dynamic_cast<SBAnimationState*>(toState);
	return to;
}

}