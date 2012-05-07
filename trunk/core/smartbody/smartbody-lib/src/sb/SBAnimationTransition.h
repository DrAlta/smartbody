#ifndef _SBTRANSITION_H_
#define _SBTRANSITION_H_

#include <vector>
#include <controllers/me_ct_param_animation_data.h>

namespace SmartBody {

class SBAnimationState;

class SBAnimationTransition : public PATransition
{
	public:
		SBAnimationTransition();
		SBAnimationTransition(std::string name);
		~SBAnimationTransition();

		void setSourceState(SBAnimationState* source);
		void setDestinationState(SBAnimationState* dest);
		SBAnimationState* getSourceState();
		SBAnimationState* getDestinationState();

		void set(SBAnimationState* source, SBAnimationState* dest);
		double getEaseInStart();
		double getEaseInEnd();
		void setEaseInInterval(std::string destMotion, float start, float end);
		void addEaseOutInterval(std::string sourceMotion, float start, float end);
		const std::string& getSourceMotionName();
		const std::string& getDestinationMotionName();
		void removeEaseOutInterval(int num);
		int getNumEaseOutIntervals();
		std::vector<double> getEaseOutInterval(int num);
		
		

	protected:

};

}
#endif