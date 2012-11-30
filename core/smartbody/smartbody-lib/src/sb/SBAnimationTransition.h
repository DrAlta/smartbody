#ifndef _SBTRANSITION_H_
#define _SBTRANSITION_H_

#include <sb/SBTypes.h>
#include <vector>
#include <controllers/me_ct_param_animation_data.h>

namespace SmartBody {

class SBAnimationBlend;
class SBAnimationTransitionRule;

class SBAnimationTransition : public PATransition
{
	public:
		SBAPI SBAnimationTransition();
		SBAPI SBAnimationTransition(std::string name);
		SBAPI ~SBAnimationTransition();

		SBAPI void setSourceBlend(SBAnimationBlend* source);
		SBAPI void setDestinationBlend(SBAnimationBlend* dest);
		SBAPI SBAnimationBlend* getSourceBlend();
		SBAPI SBAnimationBlend* getDestinationBlend();

		SBAPI void set(SBAnimationBlend* source, SBAnimationBlend* dest);
		SBAPI double getEaseInStart();
		SBAPI double getEaseInEnd();
		SBAPI void setEaseInInterval(std::string destMotion, float start, float end);
		SBAPI void addEaseOutInterval(std::string sourceMotion, float start, float end);
		SBAPI const std::string& getSourceMotionName();
		SBAPI const std::string& getDestinationMotionName();
		SBAPI void removeEaseOutInterval(int num);
		SBAPI int getNumEaseOutIntervals();
		SBAPI std::vector<double> getEaseOutInterval(int num);
		SBAPI SBAnimationTransitionRule* getTransitionRule();
		SBAPI void setTransitionRule(SBAnimationTransitionRule* rule);
		
		SBAPI std::string saveToString();

	protected:
		SBAnimationTransitionRule* _rule;

};

}
#endif