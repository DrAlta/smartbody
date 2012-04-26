#include "SBAnimationTransition.h"
#include <sbm/SBAnimationState.h>

namespace SmartBody {

SBAnimationTransition::SBAnimationTransition() : PATransition()
{
}

SBAnimationTransition::SBAnimationTransition(std::string name) : PATransition()
{
}

SBAnimationTransition::~SBAnimationTransition()
{
}

double SBAnimationTransition::getEaseInStart()
{
	return easeInStart;
}

double SBAnimationTransition::getEaseInEnd()
{
	return easeInEnd;
}


void SBAnimationTransition::set(SBAnimationState* source, SBAnimationState* dest)
{
	fromState = source;
	toState = dest;
}

void SBAnimationTransition::setEaseInInterval(std::string destMotion, float start, float end)
{
	toMotionName = destMotion;
	easeInStart = start;
	easeInEnd = end;
}

int SBAnimationTransition::getNumEaseOutIntervals()
{
	return easeOutStart.size();
}

std::vector<double> SBAnimationTransition::getEaseOutInterval(int num)
{
	std::vector<double> interval;
	if (num <0 || num >= (int) easeOutStart.size())
		return interval;

	interval.push_back((float) easeOutStart[num]);
	interval.push_back((float) easeOutEnd[num]);

	return interval;
}

void SBAnimationTransition::addEaseOutInterval(std::string sourceMotion, float start, float end)
{
	fromMotionName = sourceMotion;
	easeOutStart.push_back(start);
	easeOutEnd.push_back(end);
}

void SBAnimationTransition::removeEaseOutInterval(int num)
{
}

void SBAnimationTransition::setSourceState(SBAnimationState* source)
{
	fromState = source;
}

void SBAnimationTransition::setDestinationState(SBAnimationState* dest)
{
	toState = dest;
}

SBAnimationState* SBAnimationTransition::getSourceState()
{
	SBAnimationState* from = dynamic_cast<SBAnimationState*>(fromState);
	return from;
}

SBAnimationState* SBAnimationTransition::getDestinationState()
{
	SBAnimationState* to = dynamic_cast<SBAnimationState*>(toState);
	return to;
}

const std::string& SBAnimationTransition::getSourceMotionName()
{
	return fromMotionName;
}

const std::string& SBAnimationTransition::getDestinationMotionName()
{
	return toMotionName;
}

}