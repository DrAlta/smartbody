#ifndef _SBSTATEMANAGER_H
#define _SBSTATEMANAGER_H

#include <sbm/SBState.h>
#include <sbm/SBTransition.h>

class SBStateManager
{
	public:
		SBStateManager();
		~SBStateManager();

		SBState0D* createState0D(std::string name);
		SBState1D* createState1D(std::string name);
		SBState2D* createState2D(std::string name);
		SBState3D* createState3D(std::string name);
		SBTransition* createTransition(std::string source, std::string dest);

		SBState* getState(std::string name);
		int getNumStates();
		std::vector<std::string> getStateNames();

		SBTransition* getTransition(std::string source, std::string dest);
		int getNumTransitions();
		std::vector<std::string> getTransitionNames();

};

#endif