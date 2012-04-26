#ifndef _SBSTATEMANAGER_H
#define _SBSTATEMANAGER_H

#include <string>
#include <vector>
#include <sr/sr_vec.h>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/labeled_graph.hpp>
#undef INTMAX_C
#undef UINTMAX_C


namespace SmartBody {

class SBAnimationState;
class SBAnimationState0D;
class SBAnimationState1D;
class SBAnimationState2D;
class SBAnimationState3D;

class SBAnimationTransition;

typedef boost::property<boost::vertex_name_t,std::string> StateVertexProperty;
typedef boost::labeled_graph<boost::adjacency_list<boost::listS,boost::listS, boost::directedS, StateVertexProperty>,std::string> BoostGraph;


class SBAnimationStateManager
{
	protected:
		BoostGraph stateGraph;		
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

		std::string getCurrentState(const std::string& characterName);
		SrVec getCurrentStateParameters(const std::string& characterName);
		std::vector<std::string> getAutoStateTransitions(const std::string& characterName, const std::string& targetState);
		bool isStateScheduled(const std::string& characterName, const std::string& stateName);
protected:
		bool addStateToGraph(const std::string& name);
		bool addTransitionEdgeToGraph(const std::string& source, const std::string& dest);
};
}
#endif