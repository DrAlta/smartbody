#ifndef _SBTRANSITION_H_
#define _SBTRANSITION_H_

#include <sbm/me_ct_param_animation_data.h>
#include <sbm/SBState.h>

class SBTransition : public PATransitionData
{
	public:
		SBTransition();
		SBTransition(std::string name);
		~SBTransition();

		void set(SBState* source, SBState* dest);
		void addCorrespondancePoint(std::string sourceMotion, std::string destMotion, float sourceFromTime, float sourceToTime, float destFromTime, float destToTime);
		int getNumCorrespondancePoints();
		std::vector<float> getCorrespondancePoint(int num);

		SBState* getFromState();
		SBState* getToState();

	protected:


};
#endif