#ifndef SBANIMATIONTRANSITIONRULE_H
#define SBANIMATIONTRANSITIONRULE_H

namespace SmartBody {

class SBCharacter;
class SBAnimationBlend;

class SBAnimationTransitionRule
{
	public:
		SBAnimationTransitionRule();
		~SBAnimationTransitionRule();

		virtual bool check(SBCharacter* character, SBAnimationBlend* blend);

};


}

#endif