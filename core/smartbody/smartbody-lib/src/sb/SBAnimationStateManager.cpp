#include "SBAnimationStateManager.h"
#include <sbm/mcontrol_util.h>
#include <sb/SBAnimationState.h>
#include <sb/SBAnimationTransition.h>
#include <sb/SBCharacter.h>

namespace SmartBody {

SBAnimationStateManager::SBAnimationStateManager()
{
}

SBAnimationStateManager::~SBAnimationStateManager()
{
}

std::vector<std::string> SBAnimationStateManager::getAutoStateTransitions( const std::string& characterName, const std::string& targetState )
{
	std::vector<std::string> pathVec;
	

	return pathVec;		
}

bool SBAnimationStateManager::addStateToGraph( const std::string& name )
{
	BoostGraph::vertex_descriptor v = stateGraph.vertex(name);
	
	if (v == BoostGraph::null_vertex()) // the state does not exist in the graph
	{		
		stateGraph.add_vertex(name);	
		//boost::put(boost::get(boost::vertex_name_t,stateGraph),stateGraph,name,name);
		//boost::put(boost::vertex_name_t,)
		
	}
	return true;
}

bool SBAnimationStateManager::addTransitionEdgeToGraph( const std::string& source, const std::string& dest )
{
	BoostGraph::vertex_descriptor vs = stateGraph.vertex(source), vd = stateGraph.vertex(dest);
	
	if (vs == BoostGraph::null_vertex() || vd == BoostGraph::null_vertex()) return false;
	std::pair<BoostGraph::edge_descriptor,bool> transitionEdge = boost::edge_by_label(source,dest,stateGraph);
	// edge already exist
	if (transitionEdge.second) return false;

	// otherwise add transition edge
	boost::add_edge_by_label(source,dest,stateGraph); return true;
}

SBAnimationState0D* SBAnimationStateManager::createState0D(const std::string& name)
{
	SBAnimationState0D* state = new SBAnimationState0D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	addStateToGraph(name);
	mcu.addPAState(state);
	return state;
}

SBAnimationState1D* SBAnimationStateManager::createState1D(const std::string& name)
{
	SBAnimationState1D* state = new SBAnimationState1D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	addStateToGraph(name);
	mcu.addPAState(state);
	return state;
}

SBAnimationState2D* SBAnimationStateManager::createState2D(const std::string& name)
{
	SBAnimationState2D* state = new SBAnimationState2D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	addStateToGraph(name);
	mcu.addPAState(state);
	return state;
}

SBAnimationState3D* SBAnimationStateManager::createState3D(const std::string& name)
{
	SBAnimationState3D* state = new SBAnimationState3D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	addStateToGraph(name);
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

	PAState* stateData = mcu.lookUpPAState(name);
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

	PATransition* transition = mcu.lookUpPATransition(source, dest);
	SBAnimationTransition* animTransition = dynamic_cast<SBAnimationTransition*>(transition);

	return animTransition;
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

std::string SBAnimationStateManager::getCurrentState(const std::string& characterName)
{
	SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(characterName);
	if (!character)
		return "";

	if (!character->param_animation_ct)
		return "";

	PAStateData* stateData = character->param_animation_ct->getCurrentPAStateData();
	if (!stateData)
		return "";
	return stateData->state->stateName;
}

SrVec SBAnimationStateManager::getCurrentStateParameters(const std::string& characterName)
{
	SrVec params;
	SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(characterName);
	if (!character)
		return params;

	if (!character->param_animation_ct)
		return params;

	
	PAStateData* stateData = character->param_animation_ct->getCurrentPAStateData();
	if (!stateData)
		return params;

	SmartBody::SBAnimationState0D* state0D = dynamic_cast<SmartBody::SBAnimationState0D*>(stateData->state);
	if (state0D)
		return params;

	SmartBody::SBAnimationState1D* state1D = dynamic_cast<SmartBody::SBAnimationState1D*>(stateData->state);
	SmartBody::SBAnimationState2D* state2D = dynamic_cast<SmartBody::SBAnimationState2D*>(stateData->state);
	SmartBody::SBAnimationState3D* state3D = dynamic_cast<SmartBody::SBAnimationState3D*>(stateData->state);
	if (state1D)
	{
		stateData->state->getParametersFromWeights(params[0], stateData->weights);
		return params;
	}
	if (state2D)
	{
		stateData->state->getParametersFromWeights(params[0], params[1], stateData->weights);
		return params;
	}
	if (state3D)
	{
		stateData->state->getParametersFromWeights(params[0], params[1], params[2], stateData->weights);
		return params;
	}
	return params;
}

bool SBAnimationStateManager::isStateScheduled(const std::string& characterName, const std::string& stateName)
{
	SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(characterName);
	if (!character)
		return false;

	if (!character->param_animation_ct)
		return false;

	return character->param_animation_ct->hasPAState(stateName);
}

}