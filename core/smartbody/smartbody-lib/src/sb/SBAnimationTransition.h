#ifndef _SBTRANSITION_H_
#define _SBTRANSITION_H_

#include <vector>
#include <controllers/me_ct_param_animation_data.h>

namespace SmartBody {

class SBAnimationBlend;
class SBAnimationTransitionRule;

class SBAnimationTransition : public PATransition
{
	public:
		SBAnimationTransition();
		SBAnimationTransition(std::string name);
		~SBAnimationTransition();

		void setSourceBlend(SBAnimationBlend* source);
		void setDestinationBlend(SBAnimationBlend* dest);
		SBAnimationBlend* getSourceBlend();
		SBAnimationBlend* getDestinationBlend();

		void set(SBAnimationBlend* source, SBAnimationBlend* dest);
		double getEaseInStart();
		double getEaseInEnd();
		void setEaseInInterval(std::string destMotion, float start, float end);
		void addEaseOutInterval(std::string sourceMotion, float start, float end);
		const std::string& getSourceMotionName();
		const std::string& getDestinationMotionName();
		void removeEaseOutInterval(int num);
		int getNumEaseOutIntervals();
		std::vector<double> getEaseOutInterval(int num);
		SBAnimationTransitionRule* getTransitionRule();
		void setTransitionRule(SBAnimationTransitionRule* rule);
		
		std::string saveToString();

	protected:
		SBAnimationTransitionRule* _rule;

};

}
#endif