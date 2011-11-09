#ifndef _SBSTATEMANAGER_H
#define _SBSTATEMANAGER_H

#include <sbm/SBAnimationState.h>
#include <sbm/SBAnimationTransition.h>

namespace SmartBody {

class SBAnimationStateManager
{
	public:
		SBAnimationStateManager();
		~SBAnimationStateManager();

		SBAnimationState0D* createState0D(std::string name);
		SBAnimationState1D* createState1D(std::string name);
		SBAnimationState2D* createState2D(std::string name);
		SBAnimationState3D* createState3D(std::string name);
		SBAnimationTransition* createTransition(std::string source, std::string dest);

		SBAnimationState* getState(std::string name);
		int getNumStates();
		std::vector<std::string> getStateNames();

		SBAnimationTransition* getTransition(std::string source, std::string dest);
		int getNumTransitions();
		std::vector<std::string> getTransitionNames();

};
}
#endif