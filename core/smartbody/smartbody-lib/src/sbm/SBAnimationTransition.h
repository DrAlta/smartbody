#ifndef _SBTRANSITION_H_
#define _SBTRANSITION_H_

#include <vector>
#include <sbm/me_ct_param_animation_data.h>

namespace SmartBody {

class SBAnimationState;

class SBAnimationTransition : public PATransition
{
	public:
		SBAnimationTransition();
		SBAnimationTransition(std::string name);
		~SBAnimationTransition();

		void set(SBAnimationState* source, SBAnimationState* dest);
		void addCorrespondencePoint(std::string sourceMotion, std::string destMotion, float sourceFromTime, float sourceToTime, float destFromTime, float destToTime);
		int getNumCorrespondencePoints();
		std::vector<float> getCorrespondencePoint(int num);

		SBAnimationState* getFromState();
		SBAnimationState* getToState();

	protected:

};

}
#endif