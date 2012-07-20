#include "SBAnimationStateManager.h"
#include <sbm/mcontrol_util.h>
#include <sb/SBAnimationState.h>
#include <sb/SBMotionBlendBase.h>
#include <sb/SBAnimationTransition.h>
#include <sb/SBCharacter.h>


namespace SmartBody {

SBAnimationBlendManager::SBAnimationBlendManager()
{
}

SBAnimationBlendManager::~SBAnimationBlendManager()
{
}

std::vector<std::string> SBAnimationBlendManager::getAutoBlendTransitions( const std::string& characterName, const std::string& targetBlend )
{
	std::vector<std::string> pathVec;
	

	return pathVec;		
}

bool SBAnimationBlendManager::addBlendToGraph( const std::string& name )
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

bool SBAnimationBlendManager::addTransitionEdgeToGraph( const std::string& source, const std::string& dest )
{
	BoostGraph::vertex_descriptor vs = stateGraph.vertex(source), vd = stateGraph.vertex(dest);
	
	if (vs == BoostGraph::null_vertex() || vd == BoostGraph::null_vertex()) return false;
	std::pair<BoostGraph::edge_descriptor,bool> transitionEdge = boost::edge_by_label(source,dest,stateGraph);
	// edge already exist
	if (transitionEdge.second) return false;

	// otherwise add transition edge
	boost::add_edge_by_label(source,dest,stateGraph); return true;
}

SBAnimationBlend0D* SBAnimationBlendManager::createBlend0D(const std::string& name)
{
	SBAnimationBlend0D* blend = new SBAnimationBlend0D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	addBlendToGraph(name);
	mcu.addPABlend(blend);
	return blend;
}

SBAnimationBlend1D* SBAnimationBlendManager::createBlend1D(const std::string& name)
{
	SBAnimationBlend1D* blend = new SBAnimationBlend1D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	addBlendToGraph(name);
	mcu.addPABlend(blend);
	return blend;
}

SBAnimationBlend2D* SBAnimationBlendManager::createBlend2D(const std::string& name)
{
	SBAnimationBlend2D* blend = new SBAnimationBlend2D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	addBlendToGraph(name);
	mcu.addPABlend(blend);
	return blend;
}

SBAnimationBlend3D* SBAnimationBlendManager::createBlend3D(const std::string& name)
{
	SBAnimationBlend3D* blend = new SBAnimationBlend3D(name);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	addBlendToGraph(name);
	mcu.addPABlend(blend);
	return blend;
}

SBMotionBlendBase* SBAnimationBlendManager::createMotionBlendBase( const std::string& name, const std::string& skelName, int dimension )
{
	SBMotionBlendBase* blend = new SBMotionBlendBase(name,skelName, dimension);
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	//	addBlendToGraph(name);
	mcu.addPABlend(blend);
	return blend;
}

SBAnimationTransition* SBAnimationBlendManager::createTransition(const std::string& source, const std::string& dest)
{	
	
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	SBAnimationBlend* sourceBlend = getBlend(source);
	if (!sourceBlend)
	{
		LOG("Source state %s does not exist. No transition created.", source.c_str());
		return NULL;
	}
	SBAnimationBlend* destBlend = getBlend(dest);
	if (!destBlend)
	{
		LOG("Destination state %s does not exist. No transition created.", dest.c_str());
		return NULL;
	}
	SBAnimationTransition* transition = new SBAnimationTransition(source + "/" + dest);
	transition->set(sourceBlend, destBlend);

	mcu.addPATransition(transition);
	return transition;
}

SBAnimationBlend* SBAnimationBlendManager::getBlend(const std::string& name)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	PABlend* blendData = mcu.lookUpPABlend(name);
	SBAnimationBlend* blend = dynamic_cast<SBAnimationBlend*>(blendData);
	return blend;
}

int SBAnimationBlendManager::getNumBlends()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	return mcu.param_anim_blends.size();
}

std::vector<std::string> SBAnimationBlendManager::getBlendNames()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	std::vector<std::string> states;
	for (size_t i = 0; i < mcu.param_anim_blends.size(); i++)
	{
		states.push_back(mcu.param_anim_blends[i]->stateName);
	}
	return states;
}

SBAnimationTransition* SBAnimationBlendManager::getTransition(const std::string& source, const std::string& dest)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	PATransition* transition = mcu.lookUpPATransition(source, dest);
	SBAnimationTransition* animTransition = dynamic_cast<SBAnimationTransition*>(transition);

	return animTransition;
}

SBAnimationTransition* SBAnimationBlendManager::getTransitionByIndex(int id)
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();
	if (id >= 0 && id < (int)mcu.param_anim_transitions.size())
	{
		PATransition* transition = mcu.param_anim_transitions[id];
		SBAnimationTransition* animTransition = dynamic_cast<SBAnimationTransition*>(transition);
		return animTransition;
	}
	
	return NULL;
}


int SBAnimationBlendManager::getNumTransitions()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	return mcu.param_anim_transitions.size();
}

std::vector<std::string> SBAnimationBlendManager::getTransitionNames()
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

std::string SBAnimationBlendManager::getCurrentBlend(const std::string& characterName)
{
	SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(characterName);
	if (!character)
		return "";

	if (!character->param_animation_ct)
		return "";

	PABlendData* blendData = character->param_animation_ct->getCurrentPABlendData();
	if (!blendData)
		return "";
	return blendData->state->stateName;
}

SrVec SBAnimationBlendManager::getCurrentBlendParameters(const std::string& characterName)
{
	SrVec params;
	SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(characterName);
	if (!character)
		return params;

	if (!character->param_animation_ct)
		return params;

	
	PABlendData* blendData = character->param_animation_ct->getCurrentPABlendData();
	if (!blendData)
		return params;

	SmartBody::SBAnimationBlend0D* state0D = dynamic_cast<SmartBody::SBAnimationBlend0D*>(blendData->state);
	if (state0D)
		return params;

	SmartBody::SBAnimationBlend1D* state1D = dynamic_cast<SmartBody::SBAnimationBlend1D*>(blendData->state);
	SmartBody::SBAnimationBlend2D* state2D = dynamic_cast<SmartBody::SBAnimationBlend2D*>(blendData->state);
	SmartBody::SBAnimationBlend3D* state3D = dynamic_cast<SmartBody::SBAnimationBlend3D*>(blendData->state);
	if (state1D)
	{
		blendData->state->getParametersFromWeights(params[0], blendData->weights);
		return params;
	}
	if (state2D)
	{
		blendData->state->getParametersFromWeights(params[0], params[1], blendData->weights);
		return params;
	}
	if (state3D)
	{
		blendData->state->getParametersFromWeights(params[0], params[1], params[2], blendData->weights);
		return params;
	}
	return params;
}

bool SBAnimationBlendManager::isBlendScheduled(const std::string& characterName, const std::string& stateName)
{
	SBCharacter* character = SmartBody::SBScene::getScene()->getCharacter(characterName);
	if (!character)
		return false;

	if (!character->param_animation_ct)
		return false;

	return character->param_animation_ct->hasPABlend(stateName);
}

void SBAnimationBlendManager::removeAllBlends()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	// remove the transitions, too
	removeAllTransitions();

	for (size_t i = 0; i < mcu.param_anim_blends.size(); i++)
	{
		delete mcu.param_anim_blends[i];
	}

	stateGraph = BoostGraph();

}

void SBAnimationBlendManager::removeAllTransitions()
{
	mcuCBHandle& mcu = mcuCBHandle::singleton();

	for (size_t i = 0; i < mcu.param_anim_transitions.size(); i++)
	{
		delete mcu.param_anim_transitions[i];
	}
	mcu.param_anim_transitions.clear();
}

}