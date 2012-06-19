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

class SBAnimationBlend;
class SBAnimationBlend0D;
class SBAnimationBlend1D;
class SBAnimationBlend2D;
class SBAnimationBlend3D;

class SBAnimationTransition;

typedef boost::property<boost::vertex_name_t,std::string> StateVertexProperty;
typedef boost::labeled_graph<boost::adjacency_list<boost::listS,boost::listS, boost::directedS, StateVertexProperty>,std::string> BoostGraph;


class SBAnimationBlendManager
{
	protected:
		BoostGraph stateGraph;		
	public:
		SBAnimationBlendManager();
		~SBAnimationBlendManager();

		SBAnimationBlend0D* createBlend0D(const std::string& name);
		SBAnimationBlend1D* createBlend1D(const std::string& name);
		SBAnimationBlend2D* createBlend2D(const std::string& name);
		SBAnimationBlend3D* createBlend3D(const std::string& name);
		SBAnimationTransition* createTransition(const std::string& source, const std::string& dest);

		SBAnimationBlend* getBlend(const std::string&name);
		int getNumBlends();
		std::vector<std::string> getBlendNames();

		SBAnimationTransition* getTransition(const std::string& source, const std::string& dest);
		SBAnimationTransition* getTransitionByIndex(int id);
		int getNumTransitions();
		std::vector<std::string> getTransitionNames();

		std::string getCurrentBlend(const std::string& characterName);
		SrVec getCurrentBlendParameters(const std::string& characterName);
		std::vector<std::string> getAutoBlendTransitions(const std::string& characterName, const std::string& targetBlend);
		bool isBlendScheduled(const std::string& characterName, const std::string& blendName);
protected:
		bool addBlendToGraph(const std::string& name);
		bool addTransitionEdgeToGraph(const std::string& source, const std::string& dest);
};
}
#endif