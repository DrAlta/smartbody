#include "SBStateManager.h"
#include <sbm/mcontrol_util.h>

SBStateManager::SBStateManager()
{
}

SBStateManager::~SBStateManager()
{
}

SBState0D* SBStateManager::createState0D(std::string name)
{
	SBState0D* state = new SBState0D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.addPAState(state);
	return state;
}

SBState1D* SBStateManager::createState1D(std::string name)
{
	SBState1D* state = new SBState1D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.addPAState(state);
	return state;
}

SBState2D* SBStateManager::createState2D(std::string name)
{
	SBState2D* state = new SBState2D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.addPAState(state);
	return state;
}

SBState3D* SBStateManager::createState3D(std::string name)
{
	SBState3D* state = new SBState3D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.addPAState(state);
	return state;
}

SBTransition* SBStateManager::createTransition(std::string source, std::string dest)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SBState* sourceState = getState(source);
	if (!sourceState)
	{
		LOG("Source state %s does not exist. No transition created.", source.c_str());
		return NULL;
	}
	SBState* destState = getState(dest);
	if (!destState)
	{
		LOG("Destination state %s does not exist. No transition created.", dest.c_str());
		return NULL;
	}
	SBTransition* transition = new SBTransition(source + "/" + dest);
	transition->set(sourceState, destState);
	mcu.addPATransition(transition);
	return transition;
}

SBState* SBStateManager::getState(std::string name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	PAStateData* stateData = mcu.lookUpPAState(name);
	SBState* state = dynamic_cast<SBState*>(stateData);
	return state;
}

int SBStateManager::getNumStates()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	return mcu.param_anim_states.size();
}

std::vector<std::string> SBStateManager::getStateNames()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	std::vector<std::string> states;
	for (size_t i = 0; i < mcu.param_anim_states.size(); i++)
	{
		states.push_back(mcu.param_anim_states[i]->stateName);
	}
	return states;
}

SBTransition* SBStateManager::getTransition(std::string source, std::string dest)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	PATransitionData* transitionData = mcu.lookUpPATransition(source, dest);
	SBTransition* transition = dynamic_cast<SBTransition*>(transitionData);

	return transition;
}

int SBStateManager::getNumTransitions()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	return mcu.param_anim_transitions.size();
}

std::vector<std::string> SBStateManager::getTransitionNames()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	std::vector<string> transitionNames;
	for (size_t i = 0; i < mcu.param_anim_transitions.size(); i++)
	{
		transitionNames.push_back(mcu.param_anim_transitions[i]->fromState->stateName +
								  "/" + mcu.param_anim_transitions[i]->toState->stateName );
	}
	return transitionNames;
}