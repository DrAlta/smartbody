#include "SBAnimationTransition.h"
#include <sb/SBAnimationState.h>

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


void SBAnimationTransition::set(SBAnimationBlend* source, SBAnimationBlend* dest)
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

void SBAnimationTransition::setSourceBlend(SBAnimationBlend* source)
{
	fromState = source;
}

void SBAnimationTransition::setDestinationBlend(SBAnimationBlend* dest)
{
	toState = dest;
}

SBAnimationBlend* SBAnimationTransition::getSourceBlend()
{
	SBAnimationBlend* from = dynamic_cast<SBAnimationBlend*>(fromState);
	return from;
}

SBAnimationBlend* SBAnimationTransition::getDestinationBlend()
{
	SBAnimationBlend* to = dynamic_cast<SBAnimationBlend*>(toState);
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