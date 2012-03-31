#include "SBAnimationStateManager.h"
#include <sbm/mcontrol_util.h>
#include <sbm/SBAnimationState.h>
#include <sbm/SBAnimationTransition.h>

namespace SmartBody {

SBAnimationStateManager::SBAnimationStateManager()
{
}

SBAnimationStateManager::~SBAnimationStateManager()
{
}

SBAnimationState0D* SBAnimationStateManager::createState0D(const std::string& name)
{
	SBAnimationState0D* state = new SBAnimationState0D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.addPAState(state);
	return state;
}

SBAnimationState1D* SBAnimationStateManager::createState1D(const std::string& name)
{
	SBAnimationState1D* state = new SBAnimationState1D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.addPAState(state);
	return state;
}

SBAnimationState2D* SBAnimationStateManager::createState2D(const std::string& name)
{
	SBAnimationState2D* state = new SBAnimationState2D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.addPAState(state);
	return state;
}

SBAnimationState3D* SBAnimationStateManager::createState3D(const std::string& name)
{
	SBAnimationState3D* state = new SBAnimationState3D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	mcu.addPAState(state);
	return state;
}

SBAnimationTransition* SBAnimationStateManager::createTransition(const std::string& source, const std::string& dest)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SBAnimationState* sourceState = getState(source);
	if (!sourceState)
	{
		LOG("Source state %s does not exist. No transition created.", source.c_str());
		return NULL;
	}
	SBAnimationState* destState = getState(dest);
	if (!destState)
	{
		LOG("Destination state %s does not exist. No transition created.", dest.c_str());
		return NULL;
	}
	SBAnimationTransition* transition = new SBAnimationTransition(source + "/" + dest);
	transition->set(sourceState, destState);
	mcu.addPATransition(transition);
	return transition;
}

SBAnimationState* SBAnimationStateManager::getState(const std::string& name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	PAStateData* stateData = mcu.lookUpPAState(name);
	SBAnimationState* state = dynamic_cast<SBAnimationState*>(stateData);
	return state;
}

int SBAnimationStateManager::getNumStates()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	return mcu.param_anim_states.size();
}

std::vector<std::string> SBAnimationStateManager::getStateNames()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	std::vector<std::string> states;
	for (size_t i = 0; i < mcu.param_anim_states.size(); i++)
	{
		states.push_back(mcu.param_anim_states[i]->stateName);
	}
	return states;
}

SBAnimationTransition* SBAnimationStateManager::getTransition(const std::string& source, const std::string& dest)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	PATransitionData* transitionData = mcu.lookUpPATransition(source, dest);
	SBAnimationTransition* transition = dynamic_cast<SBAnimationTransition*>(transitionData);

	return transition;
}

int SBAnimationStateManager::getNumTransitions()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	return mcu.param_anim_transitions.size();
}

std::vector<std::string> SBAnimationStateManager::getTransitionNames()
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

}