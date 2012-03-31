#ifndef _SBSTATEMANAGER_H
#define _SBSTATEMANAGER_H

#include <string>
#include <vector>

namespace SmartBody {

class SBAnimationState;
class SBAnimationState0D;
class SBAnimationState1D;
class SBAnimationState2D;
class SBAnimationState3D;

class SBAnimationTransition;

class SBAnimationStateManager
{
	public:
		SBAnimationStateManager();
		~SBAnimationStateManager();

		SBAnimationState0D* createState0D(const std::string& name);
		SBAnimationState1D* createState1D(const std::string& name);
		SBAnimationState2D* createState2D(const std::string& name);
		SBAnimationState3D* createState3D(const std::string& name);
		SBAnimationTransition* createTransition(const std::string& source, const std::string& dest);

		SBAnimationState* getState(const std::string&name);
		int getNumStates();
		std::vector<std::string> getStateNames();

		SBAnimationTransition* getTransition(const std::string& source, const std::string& dest);
		int getNumTransitions();
		std::vector<std::string> getTransitionNames();

};
}
#endif