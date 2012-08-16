#include "SBPython.h"
#include "SBPythonClass.h"
#include "controllers/me_ct_scheduler2.h"
#include "SBFaceDefinition.h"
#include "sbm/nvbg.h"
#include "SBBehavior.h"
#include <sb/SBMotion.h>
#include <sb/SBParseNode.h>
#include <sbm/mcontrol_util.h>
#include <sb/SBScene.h>
#include <sb/SBScript.h>
#include <sb/SBService.h>
#include <sb/SBServiceManager.h>
#include <sb/SBSimulationManager.h>
#include <sb/SBBmlProcessor.h>
#include <sb/SBAnimationState.h>
#include <sb/SBMotionBlendBase.h>
#include <sb/SBAnimationTransition.h>
#include <sb/SBAnimationStateManager.h>
#include <sb/SBSteerManager.h>
#include <sb/SBPhysicsManager.h>
#include <sb/SBReach.h>
#include <sb/SBReachManager.h>
#include <sb/SBGestureMap.h>
#include <sb/SBGestureMapManager.h>
#include <sb/SBJointMap.h>
#include <sb/SBJointMapManager.h>
#include <sb/SBParser.h>
#include <sb/SBBoneBusManager.h>
#include <sb/SBCollisionManager.h>
#include <sb/SBSteerAgent.h>
#include <sb/SBPhoneme.h>
#include <sb/SBPhonemeManager.h>
#include <sb/SBBehaviorSet.h>
#include <sb/SBBehaviorSetManager.h>
#include <sr/sr_box.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include "SBPythonInternal.h"


#ifdef USE_PYTHON
#include <boost/python/suite/indexing/vector_indexing_suite.hpp> 
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/args.hpp>
#endif

typedef std::map<std::string,SrQuat> QuatMap;
typedef std::map<std::string,SrVec> VecMap;
typedef std::map<std::string, std::string> StringMap;


#ifdef USE_PYTHON



namespace SmartBody 
{
void pythonFuncs3()
{
	boost::python::class_<SBBmlProcessor>("BmlProcessor")
		.def("execBML", &SBBmlProcessor::execBML, boost::python::return_value_policy<boost::python::return_by_value>(), "Execute a generic BML instruction to a given character. Adds the <?xml..> and <act><bml>...</bml></act> elements.")
		.def("execBMLFile", &SBBmlProcessor::execBMLFile, boost::python::return_value_policy<boost::python::return_by_value>(), "Execute the BML instructions contained in a file for a given character.")
		.def("execXML", &SBBmlProcessor::execXML, boost::python::return_value_policy<boost::python::return_by_value>(), "Execute a generic XML instruction to a given character. Adds the <?xml..> header.")
		;

	boost::python::class_<SBAnimationBlend>("SBAnimationBlend")
		.def("addCorrespondencePoints", &SBAnimationBlend::addCorrespondencePoints, "Correspondence points for motions inside the blend.")
		.def("addCorrespondancePoints", &SBAnimationBlend::addCorrespondencePoints, "Correspondence points for motions inside the blend.")
		.def("setIncrementWorldOffsetY", &SBAnimationBlend::setIncrementWorldOffsetY, "Boolean flag that increment world offset y-axis value according to the base joint value.")
		.def("getNumMotions", &SBAnimationBlend::getNumMotions, "Number of motions inside the blend.")
		.def("getMotion", &SBAnimationBlend::getMotion, boost::python::return_value_policy<boost::python::return_by_value>(), "Return the motion name given index. \n Input: index of motion \n Output: motion name")
		.def("getNumCorrespondancePoints", &SBAnimationBlend::getNumCorrespondencePoints, "Number of correspondence points for the motions in the blend")
		.def("getNumCorrespondencePoints", &SBAnimationBlend::getNumCorrespondencePoints, "Number of correspondence points for the motions in the blend")
		.def("getCorrespondancePoints", &SBAnimationBlend::getCorrespondencePoints, boost::python::return_value_policy<boost::python::return_by_value>(), "Return the correspondence points in one motion given the index. \n Input: index of motion \n Output: correspondence points vector of this motion")
		.def("getCorrespondencePoints", &SBAnimationBlend::getCorrespondencePoints, boost::python::return_value_policy<boost::python::return_by_value>(), "Return the correspondence points in one motion given the index. \n Input: index of motion \n Output: correspondence points vector of this motion")
		.def("setCorrespondencePoints", &SBAnimationBlend::setCorrespondencePoints, "Sets the correspondence points given a motion index, a parameter index and a value.")
		.def("getDimension", &SBAnimationBlend::getDimension, boost::python::return_value_policy<boost::python::return_by_value>(), "Return the dimension of the state. Dimension represents the number of parameter for each motion. 0D means no parameter, 1D means one parameter for each motion etc.")
		.def("addEvent", &SBAnimationBlend::addEvent, "Adds an event to the blend at a specific local time for the given motion.")
		.def("removeAllEvents", &SBAnimationBlend::removeAllEvents, "Removes all events from the blend at a specific local time for the given motion.")
		.def("getNumEvents", &SBAnimationBlend::getNumEvents, "Returns the number of events associated with this blend.")
		.def("getEvent", &SBAnimationBlend::getEvent, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns the event of a given index.")
		.def("removeEvent", &SBAnimationBlend::removeEvent, "Removes the event of a given index.") 
		.def("buildVisSurfaces", &SBAnimationBlend::buildVisSurfaces, "Build a visualization surface. \n Input : Error Type, Surface Type, Num of Segements, Grid Resolutions \n Output: NULL")
		.def("createMotionVectorFlow", &SBAnimationBlend::createMotionVectorFlow, "create Vector Flow visualization. \n Input: motion name. \n Output: NULL")
		.def("clearMotionVectorFlow", &SBAnimationBlend::clearMotionVectorFlow, "clear Vector Flow visualization. \n Input: NULL. \n Output: NULL")
		.def("plotMotion", &SBAnimationBlend::plotMotion, "Plot motion frames with stick skeleton. \n Input: motion name, intervals, ifClearAll \n Output: NULL")
		.def("plotMotionFrameTime", &SBAnimationBlend::plotMotionFrameTime, "Plot one single motion frame (at given time) with stick skeleton. \n Input: motion name, time, ifClearAll \n Output: NULL")
		.def("plotMotionJointTrajectory", &SBAnimationBlend::plotMotionJointTrajectory, "Plot joint trajectory over entire motion (at given time). \n Input: motion name, jointName, ifClearAll \n Output: NULL")
		.def("clearPlotMotion", &SBAnimationBlend::clearPlotMotion, "clear Plotted motions. \n Input: NULL. \n Output: NULL")
		;

	boost::python::class_<SBAnimationBlend0D, boost::python::bases<SBAnimationBlend> >("SBAnimationBlend0D")
		.def("addMotion", &SBAnimationBlend0D::addMotion, "Add motion to 0D state. \n Input: motion name. \n Output: NULL")
	;

	boost::python::class_<SBAnimationBlend1D, boost::python::bases<SBAnimationBlend> >("SBAnimationBlend1D")
		.def("addMotion", &SBAnimationBlend1D::addMotion, "Add motion and one parameter to 1D state. \n Input: motion name, parameter. \n Output: NULL")
		.def("setParameter", &SBAnimationBlend1D::setParameter, "Set/Change the parameter for given motion. \n Input: motion name, parameter. \n Output: NULL")
	;

	boost::python::class_<SBAnimationBlend2D, boost::python::bases<SBAnimationBlend> >("SBAnimationBlend2D")
		.def("addMotion", &SBAnimationBlend2D::addMotion, "Add motion and two parameters to 2D state. \n Input: motion name, parameter1, parameter2. \n Output: NULL")
		.def("setParameter", &SBAnimationBlend2D::setParameter, "Set/Change the parameter for given motion. \n Input: motion name, parameter1, parameter2. \n Output: NULL")
		.def("addTriangle", &SBAnimationBlend2D::addTriangle, "Add triangles to the state. By changing the point inside triangle, you can get different blending weights and different results")
	;

	boost::python::class_<SBAnimationBlend3D, boost::python::bases<SBAnimationBlend> >("SBAnimationBlend3D")
		.def("addMotion", &SBAnimationBlend3D::addMotion, "Add motion and three parameters to 3D state. \n Input: motion name, parameter1, parameter2, parameter3. \n Output: NULL")
		.def("setParameter", &SBAnimationBlend3D::setParameter, "Set/Change the parameter for given motion. \n Input: motion name, parameter1, parameter2, parameter3. \n Output: NULL")
		.def("addTetrahedron", &SBAnimationBlend3D::addTetrahedron, "Add tetrahedrons to the state. By changing the point inside tetrahedron, you can get different blending weights and different results")
	;

	boost::python::class_<SBMotionBlendBase, boost::python::bases<SBAnimationBlend> >("SBMotionBlendBase")
		.def("addMotion", &SBMotionBlendBase::addMotion, "Add motion and its parameters to animation state. \n Input: motion name, vector of parameters. \n Output: NULL")
		.def("setParameter", &SBMotionBlendBase::setMotionParameter, "Set/Change the parameter for given motion. \n Input: motion name, vector of parameters. \n Output: NULL")
		.def("getParameter", &SBMotionBlendBase::getMotionParameter, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the parameter of a given motion")
		.def("buildBlendBase", &SBMotionBlendBase::buildBlendBase, "Initialize BlendBase. \n Input : Motion Parameter Name, Interpolator Type \n Output: NULL")
		.def("addTetrahedron", &SBMotionBlendBase::addTetrahedron, "Add tetrahedrons to the state. By changing the point inside tetrahedron, you can get different blending weights and different results")
		;


	boost::python::class_<SBAnimationTransition>("SBAnimationTransition")
		.def("set", &SBAnimationTransition::set, "")
		.def("setSourceState", &SBAnimationTransition::setSourceBlend, "")
		.def("getSourceState", &SBAnimationTransition::getSourceBlend, boost::python::return_value_policy<boost::python::reference_existing_object>(), "")
		.def("setSourceBlend", &SBAnimationTransition::setSourceBlend, "")
		.def("getSourceBlend", &SBAnimationTransition::getSourceBlend, boost::python::return_value_policy<boost::python::reference_existing_object>(), "")
		.def("setDestinationState", &SBAnimationTransition::setDestinationBlend, "")
		.def("getDestinationState", &SBAnimationTransition::getDestinationBlend, boost::python::return_value_policy<boost::python::reference_existing_object>(), "")
		.def("setDestinationBlend", &SBAnimationTransition::setDestinationBlend, "")
		.def("getDestinationBlend", &SBAnimationTransition::getDestinationBlend, boost::python::return_value_policy<boost::python::reference_existing_object>(), "")
		.def("getSourceMotionName", &SBAnimationTransition::getSourceMotionName, boost::python::return_value_policy<boost::python::return_by_value>(), "")
		.def("setEaseInInterval", &SBAnimationTransition::setEaseInInterval, "")
		.def("addEaseOutInterval", &SBAnimationTransition::addEaseOutInterval, "")
		.def("removeEaseOutInterval", &SBAnimationTransition::removeEaseOutInterval, "")
		.def("getNumEaseOutIntervals", &SBAnimationTransition::getNumEaseOutIntervals, "")
		.def("getEaseOutInterval", &SBAnimationTransition::getEaseOutInterval, boost::python::return_value_policy<boost::python::return_by_value>(), "")
		.def("getDestinationMotionName", &SBAnimationTransition::getDestinationMotionName, boost::python::return_value_policy<boost::python::return_by_value>(), "")
		.def("getEaseInStart", &SBAnimationTransition::getEaseInStart, "")
		.def("getEaseInEnd", &SBAnimationTransition::getEaseInEnd, "")
		;

	boost::python::class_<SBAnimationBlendManager>("SBAnimationBlendManager")
		.def("createState0D", &SBAnimationBlendManager::createBlend0D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 1D blend.")
		.def("createState1D", &SBAnimationBlendManager::createBlend1D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 1D blend.")
		.def("createState2D", &SBAnimationBlendManager::createBlend2D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 2D blend.")
		.def("createState3D", &SBAnimationBlendManager::createBlend3D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 3D blend.")
		.def("createBlend0D", &SBAnimationBlendManager::createBlend0D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 1D blend.")
		.def("createBlend1D", &SBAnimationBlendManager::createBlend1D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 1D blend.")
		.def("createBlend2D", &SBAnimationBlendManager::createBlend2D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 2D blend.")
		.def("createBlend3D", &SBAnimationBlendManager::createBlend3D, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a 3D blend.")
		.def("createMotionBlendBase", &SBAnimationBlendManager::createMotionBlendBase, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a motion blend base.")
		.def("createTransition", &SBAnimationBlendManager::createTransition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Creates a transition.")
		.def("getState", &SBAnimationBlendManager::getBlend, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a blend of a given name.")
		.def("getBlend", &SBAnimationBlendManager::getBlend, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a blend of a given name.")
		.def("getNumStates", &SBAnimationBlendManager::getNumBlends, "Returns the number of states.")
		.def("getNumBlends", &SBAnimationBlendManager::getNumBlends, "Returns the number of states.")
		.def("getStateNames", &SBAnimationBlendManager::getBlendNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the blend names.")
		.def("getBlendNames", &SBAnimationBlendManager::getBlendNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the blend names.")
		.def("getTransition", &SBAnimationBlendManager::getTransition, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a transition with a given name.")
		.def("getTransitionByIndex", &SBAnimationBlendManager::getTransitionByIndex, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Returns a transition with a given name.")
		.def("getNumTransitions", &SBAnimationBlendManager::getNumTransitions, "Returns the state names.")
		.def("getTransitionNames", &SBAnimationBlendManager::getTransitionNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the blend names.")
		.def("getCurrentState", &SBAnimationBlendManager::getCurrentBlend, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the character's current blend name.")
		.def("getCurrentStateParameters", &SBAnimationBlendManager::getCurrentBlendParameters, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the character's current blend name.")
		.def("getCurrentBlend", &SBAnimationBlendManager::getCurrentBlend, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the character's current blend name.")
		.def("getCurrentBlendParameters", &SBAnimationBlendManager::getCurrentBlendParameters, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns the character's current blend name.")
		.def("isStateScheduled", &SBAnimationBlendManager::isBlendScheduled, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns whether the character has the blend scheduled. Used to avoid scheduling the same blend.")
		.def("isBlendScheduled", &SBAnimationBlendManager::isBlendScheduled, boost::python::return_value_policy<boost::python::return_by_value>(), "Returns whether the character has the blend scheduled. Used to avoid scheduling the same blend.")
		;

	boost::python::class_<SBSteerManager, boost::python::bases<SBService> >("SBSteerManager")
		.def("createSteerAgent", &SBSteerManager::createSteerAgent, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Create a steer agent.")
		.def("removeSteerAgent", &SBSteerManager::removeSteerAgent, "Remove a steer agent.")
		.def("getNumSteerAgents", &SBSteerManager::getNumSteerAgents, "Return number of steer agents.")
		.def("getSteerAgent", &SBSteerManager::getSteerAgent, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return steer agent given its name.")
		.def("getSteerAgentNames", &SBSteerManager::getSteerAgentNames, boost::python::return_value_policy<boost::python::return_by_value>(), "Return steer agent names.")
		.def("start", &SBSteerManager::start, "Start the steer simulation.")
		.def("stop", &SBSteerManager::stop, "Stop the steer simulation.")
		;

	boost::python::class_<SBCollisionManager, boost::python::bases<SBService> >("SBCollisionManager")
		.def("start", &SBCollisionManager::start, "Starts the collision manager.")
		.def("stop", &SBCollisionManager::stop, "Stops the collision manager.")
		;


	boost::python::class_<SBPhysicsManager, boost::python::bases<SBService> >("SBPhysicsManager")
		.def("createPhysicsCharacter", &SBPhysicsManager::createPhysicsCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Create a physics character.")
		.def("createPhysicsPawn", &SBPhysicsManager::createPhysicsPawn, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Create a physics rigid body and attach it to the pawn.")
		.def("getPhysicsEngine", &SBPhysicsManager::getPhysicsSimulationEngine, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return the current physics engine")  
		.def("getPhysicsCharacter", &SBPhysicsManager::getPhysicsCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return a physics-based character")  
		.def("getPhysicsJoint", &SBPhysicsManager::getPhysicsJoint, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return a physics-based joint")  
		.def("getJointObj", &SBPhysicsManager::getJointObj, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return a physics-based body link") 
		.def("getPhysicsPawn", &SBPhysicsManager::getPhysicsPawn, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return a rigid body pawn")
		.def("start", &SBPhysicsManager::start, "Start the physics simulation.(not implemented yet)") 
		.def("stop", &SBPhysicsManager::stop, "Stop the physics simulation.(not implemented yet)")		
		;

	boost::python::class_<SBBoneBusManager, boost::python::bases<SBService> >("SBBoneBusManager")
		;

	boost::python::class_<SBSteerAgent>("SBSteerAgent")
		.def("setSteerStateNamePrefix", &SBSteerAgent::setSteerStateNamePrefix, "Set the animation state name prefix used for steering, only applies to steering type locomotion.")
		.def("getSteerStateNamePrefix", &SBSteerAgent::getSteerStateNamePrefix, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the animation state name prefix used for steering, only applies to steering type locomotion.")
		.def("setSteerType", &SBSteerAgent::setSteerType, "Sets the type of steering locomotion, can be one of the following: basic, example, procedural")
		.def("getSteerType", &SBSteerAgent::getSteerType, boost::python::return_value_policy<boost::python::return_by_value>(), "Gets the type of steering locomotion, is one of the following: basic, example, procedural")
		.def("getCurrentSBCharacter", &SBSteerAgent::getCurrentSBCharacter, boost::python::return_value_policy<boost::python::reference_existing_object>(), "Return SBCharacter that SBSteerAgent is attached to.")		
		;
}
}


#endif
